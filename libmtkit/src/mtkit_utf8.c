/*
	Copyright (C) 2007-2018 Mark Tyler

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program in the file COPYING.
*/

#include "private.h"



/*
Portions of this UTF-8 conversion code comes from ConvertUTF.c with
adjustments by Mark Tyler in March 2008 to make it work inside the mtKit
context.
*/

/*
 * Copyright 2001-2004 Unicode, Inc.
 *
 * Disclaimer
 *
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 *
 * Limitations on Rights to Redistribute This Code
 *
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8. Source code file.
    Author: Mark E. Davis, 1994.
    Rev History: Rick McGowan, fixes & updates May 2001.
    Sept 2001: fixed const & error conditions per
	mods suggested by S. Parent & A. Lillich.
    June 2002: Tim Dodd added detection and handling of incomplete
	source sequences, enhanced error detection, added casts
	to eliminate compiler warnings.
    July 2003: slight mods to back out aggressive FFFE detection.
    Jan 2004: updated switches in from-UTF8 conversions.
    Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.

    See the header file "ConvertUTF.h" for complete documentation.

------------------------------------------------------------------------ */

#define UNI_MAX_LEGAL_UTF32	0x0010FFFF
#define UNI_SUR_HIGH_START	0xD800
#define UNI_SUR_HIGH_END	0xDBFF
#define UNI_SUR_LOW_START	0xDC00
#define UNI_SUR_LOW_END		0xDFFF

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
static char	const	trailingBytesForUTF8[256] =
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static uint32_t const	offsetsFromUTF8[5] =
{
0, 0x00000000, 0x00003080, 0x000E2080, 0x03C82080
};

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
//static unsigned char const firstByteMark[7] =
//	{ 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };



static int mtkit_utf8_is_legal (
	unsigned char	const * const	src,
	int			const	bytes	// Length of source buffer 1-4
	)
	// 1 = legal
{
	if ( ! src )
	{
		return 0;
	}

	unsigned char		a;
	unsigned char	const * srcptr = src + bytes;

	switch ( bytes )
	{
		default: return 0;
		/* Everything else falls through when "true"... */

		case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return 0;
			// fallthrough

		case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return 0;
			// fallthrough

		case 2: if ((a = (*--srcptr)) > 0xBF) return 0;

		switch ( src[0] )
		{
		/* no fall-through in this inner switch */
			case 0xE0: if (a < 0xA0) return 0; break;
			case 0xED: if (a > 0x9F) return 0; break;
			case 0xF0: if (a < 0x90) return 0; break;
			case 0xF4: if (a > 0x8F) return 0; break;
			default:   if (a < 0x80) return 0; break;
		}
			// fallthrough

		case 1:
			if ( src[0] >= 0x80 && src[0] < 0xC2 )
			{
				return 0;
			}
			// fallthrough
	}

	if ( src[0] > 0xF4 )
	{
		return 0;
	}

	return 1;
}

int mtkit_utf8_string_legal (
	unsigned char	const *		src,
	size_t			const	bytes
	)
{
	int		i;


	if ( bytes < 1 )
	{
		while ( src[0] )
		{
			if ( src[0] < 128 )
			{
				src ++;
			}
			else
			{
				i = mtkit_utf8_to_utf32 ( src, NULL );

				if ( i < 1 )
				{
					return 0;
				}

				src += i;
			}
		}
	}
	else
	{
		unsigned char	const * end;


		end = src + bytes;

		while ( src < end )
		{
			if ( src[0] < 128 )
			{
				src ++;

				continue;
			}

			i = mtkit_utf8_to_utf32 ( src, NULL );

			if ( i < 1 )
			{
				return 0;
			}

			src += i;
		}
	}

	return 1;			// Legal
}

int mtkit_utf8_to_utf32 (
	unsigned char	const	*	src,
	uint32_t		* const	unicode
	)
{
	int		i,
			length;


	if ( ! src )
	{
		return -1;
	}

	length = 1 + trailingBytesForUTF8[ src[0] ];

	if ( length > 4 || length < 1 )
	{
		// Only 1-4 bytes per valid UTF-8 char

		return -1;
	}

	for ( i = 1; i < length; i++ )
	{
		if ( src[i] == 0 )
		{
			// Detect premature end to UTF-8 char

			return -1;
		}
	}

	if ( ! mtkit_utf8_is_legal ( src, length ) )
	{
		return -1;		// Illegal UTF-8 character
	}

	if ( unicode )
	{
		uint32_t	ch = 0;


		switch ( length )
		{
			case 4: ch += *src++; ch <<= 6;
			// fallthrough
			case 3: ch += *src++; ch <<= 6;
			// fallthrough
			case 2: ch += *src++; ch <<= 6;
			// fallthrough
			case 1: ch += *src++;
		}

		ch -= offsetsFromUTF8[ length ];

/*
 * UTF-16 surrogate values are illegal in UTF-32, and anything
 * over Plane 17 (> 0x10FFFF) is illegal.
 */
		if ( ch > UNI_MAX_LEGAL_UTF32 )
		{
			return -1;
		}

		if (	ch >= UNI_SUR_HIGH_START &&
			ch <= UNI_SUR_LOW_END
			)
		{
			return -1;
		}

		unicode[0] = ch;
	}

	return length;
}

char * mtkit_utf8_from_cstring (
	char		const * const	cstring
	)
{
	if ( ! cstring )
	{
		return NULL;
	}

	if ( ! mtkit_utf8_string_legal ( (unsigned char const *)cstring, 0 ) )
	{
		return mtkit_iso8859_to_utf8 ( cstring, 0, NULL );
	}

	return strdup ( cstring );	// Already UTF8 so just duplicate
}

int mtkit_utf8_len (
	unsigned char	const *		src,
	size_t			const	bytes
	)
{
	unsigned char	const * end;
	int			tot = 0,
				i;


	if ( ! src )
	{
		return 0;
	}

	end = src + bytes;
	if ( bytes < 1 )
	{
		while ( src[0] != 0 )
		{
			if ( src[0] < 128 )
			{
				tot ++;
				src ++;

				continue;
			}

			i = mtkit_utf8_to_utf32 ( src, NULL );

			if ( i == -1 )
			{
				return 0;
			}

			src += i;
			tot ++;
		}
	}
	else
	{
		while ( src < end )
		{
			if ( src[0] < 128 )
			{
				tot ++;
				src ++;

				continue;
			}

			i = mtkit_utf8_to_utf32 ( src, NULL );

			if ( i == -1 )
			{
				return 0;
			}

			src += i;
			tot ++;
		}
	}

	return tot;
}

int mtkit_utf8_offset (
	unsigned char	const *	const	src,
	int			const	num
	)
{
	int		offset = 0,
			i;


	for ( i = 0; i < num; i++ )
	{
		if ( 0 == src[ offset ] )
		{
			return -1;	// UTF-8 string too short
		}

		offset	+= 1 + trailingBytesForUTF8[ src[ offset ] ];
	}

	return offset;
}



typedef struct
{
	unsigned char	len;
	char		c1,
			c2;
} iso2utf;



static iso2utf const	iso2utf_table[256] = {
{ 1, 0, 0 }, { 1, 1, 0 }, { 1, 2, 0 }, { 1, 3, 0 },
{ 1, 4, 0 }, { 1, 5, 0 }, { 1, 6, 0 }, { 1, 7, 0 },
{ 1, 8, 0 }, { 1, 9, 0 }, { 1, 10, 0 }, { 1, 11, 0 },
{ 1, 12, 0 }, { 1, 13, 0 }, { 1, 14, 0 }, { 1, 15, 0 },
{ 1, 16, 0 }, { 1, 17, 0 }, { 1, 18, 0 }, { 1, 19, 0 },
{ 1, 20, 0 }, { 1, 21, 0 }, { 1, 22, 0 }, { 1, 23, 0 },
{ 1, 24, 0 }, { 1, 25, 0 }, { 1, 26, 0 }, { 1, 27, 0 },
{ 1, 28, 0 }, { 1, 29, 0 }, { 1, 30, 0 }, { 1, 31, 0 },
{ 1, 32, 0 }, { 1, 33, 0 }, { 1, 34, 0 }, { 1, 35, 0 },
{ 1, 36, 0 }, { 1, 37, 0 }, { 1, 38, 0 }, { 1, 39, 0 },
{ 1, 40, 0 }, { 1, 41, 0 }, { 1, 42, 0 }, { 1, 43, 0 },
{ 1, 44, 0 }, { 1, 45, 0 }, { 1, 46, 0 }, { 1, 47, 0 },
{ 1, 48, 0 }, { 1, 49, 0 }, { 1, 50, 0 }, { 1, 51, 0 },
{ 1, 52, 0 }, { 1, 53, 0 }, { 1, 54, 0 }, { 1, 55, 0 },
{ 1, 56, 0 }, { 1, 57, 0 }, { 1, 58, 0 }, { 1, 59, 0 },
{ 1, 60, 0 }, { 1, 61, 0 }, { 1, 62, 0 }, { 1, 63, 0 },
{ 1, 64, 0 }, { 1, 65, 0 }, { 1, 66, 0 }, { 1, 67, 0 },
{ 1, 68, 0 }, { 1, 69, 0 }, { 1, 70, 0 }, { 1, 71, 0 },
{ 1, 72, 0 }, { 1, 73, 0 }, { 1, 74, 0 }, { 1, 75, 0 },
{ 1, 76, 0 }, { 1, 77, 0 }, { 1, 78, 0 }, { 1, 79, 0 },
{ 1, 80, 0 }, { 1, 81, 0 }, { 1, 82, 0 }, { 1, 83, 0 },
{ 1, 84, 0 }, { 1, 85, 0 }, { 1, 86, 0 }, { 1, 87, 0 },
{ 1, 88, 0 }, { 1, 89, 0 }, { 1, 90, 0 }, { 1, 91, 0 },
{ 1, 92, 0 }, { 1, 93, 0 }, { 1, 94, 0 }, { 1, 95, 0 },
{ 1, 96, 0 }, { 1, 97, 0 }, { 1, 98, 0 }, { 1, 99, 0 },
{ 1, 100, 0 }, { 1, 101, 0 }, { 1, 102, 0 }, { 1, 103, 0 },
{ 1, 104, 0 }, { 1, 105, 0 }, { 1, 106, 0 }, { 1, 107, 0 },
{ 1, 108, 0 }, { 1, 109, 0 }, { 1, 110, 0 }, { 1, 111, 0 },
{ 1, 112, 0 }, { 1, 113, 0 }, { 1, 114, 0 }, { 1, 115, 0 },
{ 1, 116, 0 }, { 1, 117, 0 }, { 1, 118, 0 }, { 1, 119, 0 },
{ 1, 120, 0 }, { 1, 121, 0 }, { 1, 122, 0 }, { 1, 123, 0 },
{ 1, 124, 0 }, { 1, 125, 0 }, { 1, 126, 0 }, { 1, 127, 0 },
{ 2, -62, -128 }, { 2, -62, -127 }, { 2, -62, -126 }, { 2, -62, -125 },
{ 2, -62, -124 }, { 2, -62, -123 }, { 2, -62, -122 }, { 2, -62, -121 },
{ 2, -62, -120 }, { 2, -62, -119 }, { 2, -62, -118 }, { 2, -62, -117 },
{ 2, -62, -116 }, { 2, -62, -115 }, { 2, -62, -114 }, { 2, -62, -113 },
{ 2, -62, -112 }, { 2, -62, -111 }, { 2, -62, -110 }, { 2, -62, -109 },
{ 2, -62, -108 }, { 2, -62, -107 }, { 2, -62, -106 }, { 2, -62, -105 },
{ 2, -62, -104 }, { 2, -62, -103 }, { 2, -62, -102 }, { 2, -62, -101 },
{ 2, -62, -100 }, { 2, -62, -99 }, { 2, -62, -98 }, { 2, -62, -97 },
{ 2, -62, -96 }, { 2, -62, -95 }, { 2, -62, -94 }, { 2, -62, -93 },
{ 2, -62, -92 }, { 2, -62, -91 }, { 2, -62, -90 }, { 2, -62, -89 },
{ 2, -62, -88 }, { 2, -62, -87 }, { 2, -62, -86 }, { 2, -62, -85 },
{ 2, -62, -84 }, { 2, -62, -83 }, { 2, -62, -82 }, { 2, -62, -81 },
{ 2, -62, -80 }, { 2, -62, -79 }, { 2, -62, -78 }, { 2, -62, -77 },
{ 2, -62, -76 }, { 2, -62, -75 }, { 2, -62, -74 }, { 2, -62, -73 },
{ 2, -62, -72 }, { 2, -62, -71 }, { 2, -62, -70 }, { 2, -62, -69 },
{ 2, -62, -68 }, { 2, -62, -67 }, { 2, -62, -66 }, { 2, -62, -65 },
{ 2, -61, -128 }, { 2, -61, -127 }, { 2, -61, -126 }, { 2, -61, -125 },
{ 2, -61, -124 }, { 2, -61, -123 }, { 2, -61, -122 }, { 2, -61, -121 },
{ 2, -61, -120 }, { 2, -61, -119 }, { 2, -61, -118 }, { 2, -61, -117 },
{ 2, -61, -116 }, { 2, -61, -115 }, { 2, -61, -114 }, { 2, -61, -113 },
{ 2, -61, -112 }, { 2, -61, -111 }, { 2, -61, -110 }, { 2, -61, -109 },
{ 2, -61, -108 }, { 2, -61, -107 }, { 2, -61, -106 }, { 2, -61, -105 },
{ 2, -61, -104 }, { 2, -61, -103 }, { 2, -61, -102 }, { 2, -61, -101 },
{ 2, -61, -100 }, { 2, -61, -99 }, { 2, -61, -98 }, { 2, -61, -97 },
{ 2, -61, -96 }, { 2, -61, -95 }, { 2, -61, -94 }, { 2, -61, -93 },
{ 2, -61, -92 }, { 2, -61, -91 }, { 2, -61, -90 }, { 2, -61, -89 },
{ 2, -61, -88 }, { 2, -61, -87 }, { 2, -61, -86 }, { 2, -61, -85 },
{ 2, -61, -84 }, { 2, -61, -83 }, { 2, -61, -82 }, { 2, -61, -81 },
{ 2, -61, -80 }, { 2, -61, -79 }, { 2, -61, -78 }, { 2, -61, -77 },
{ 2, -61, -76 }, { 2, -61, -75 }, { 2, -61, -74 }, { 2, -61, -73 },
{ 2, -61, -72 }, { 2, -61, -71 }, { 2, -61, -70 }, { 2, -61, -69 },
{ 2, -61, -68 }, { 2, -61, -67 }, { 2, -61, -66 }, { 2, -61, -65 }
};



char * mtkit_iso8859_to_utf8 (
	char		const	* const	input,
	size_t				bytes,
	size_t			* const	new_size
	)
{
	if ( ! input )
	{
		return NULL;
	}

	if ( bytes == 0 )
	{
		bytes = strlen ( input );
	}

	// Pass 1 - Calculate length of new string in bytes

	size_t			totlen = 1;		// NUL terminator
	unsigned char const	* src = (unsigned char const *) input;
	unsigned char const	* end = src + bytes;
	unsigned char		uc;

	while ( src < end )
	{
		uc = *src++;
		size_t const clen = iso2utf_table[uc].len;

		if ( totlen > (SIZE_MAX - clen) )
		{
			return NULL;
		}

		totlen += clen;
	}

	char * const newstr = (char *)calloc ( totlen, 1 );

	if ( ! newstr )
	{
		return NULL;
	}

	if ( new_size )
	{
		new_size[0] = totlen;
	}

	// Pass 2 - Do conversion

	src = (unsigned char const *) input;
	char * dest = newstr;
	end = src + bytes;

	while ( src < end )
	{
		uc = *src++;
		if ( iso2utf_table[uc].len == 1 )
		{
			*dest++ = iso2utf_table[uc].c1;
		}
		else
		{
			*dest++ = iso2utf_table[uc].c1;
			*dest++ = iso2utf_table[uc].c2;
		}
	}

	return newstr;
}

