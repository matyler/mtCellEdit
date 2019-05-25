/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include "tap.h"



static int decode_rgb (
	mtKit::ByteFileWrite	&file,
	uint8_t	const *	const	src_mem,
	uint64_t	const	src_len
	)
{
	uint8_t		buf[8192];
	uint64_t const	buflen = (uint64_t)sizeof(buf);
	uint8_t	const	* src = src_mem;

	for ( uint64_t j = 0; j < src_len; )
	{
		uint64_t const remaining = src_len - j;
		uint64_t const tot = MIN ( buflen, remaining / 8 );

		if ( tot < 1 )
		{
			break;
		}

		for ( uint64_t i = 0; i < tot; i++ )
		{
			buf[i] = (uint8_t)(
				((src[0] & 1) << 0) |
				((src[1] & 1) << 1) |
				((src[2] & 1) << 2) |
				((src[3] & 1) << 3) |
				((src[4] & 1) << 4) |
				((src[5] & 1) << 5) |
				((src[6] & 1) << 6) |
				((src[7] & 1) << 7) );

			src += 8;
		}

		if ( file.write ( buf, tot ) )
		{
			return 1;
		}

		j += tot * 8;
	}

	return 0;
}

int mtDW::Tap::Op::decode_image (
	mtPixy::Image	* const	image,
	char	const * const	output,
	int			& type
	)
{
	uint64_t const pixels = (uint64_t)(image->get_width () *
					image->get_height ());

	try
	{
		mtKit::ByteFileWrite file;

		if ( file.open ( output ) )
		{
			return report_error ( ERROR_IMAGE_OPEN_OUTPUT );
		}

		if ( decode_rgb ( file, image->get_canvas (), pixels * 3 ) )
		{
			remove ( output );

			return report_error ( ERROR_IMAGE_WRITE );
		}

		SodaFile soda;
		if ( 0 == soda.open ( output ) )
		{
			type = TapFile::TYPE_RGB_1;
			return 0;
		}

		// Not a Soda file so remove temp file
		remove ( output );

		type = TapFile::TYPE_RGB;
		return 0;
	}
	catch ( ... )
	{
		remove ( output );
	}

	return report_error ( ERROR_IMAGE_DECODE_EXCEPTION );
}

