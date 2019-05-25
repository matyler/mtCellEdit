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



int mtkit_string_encoding_conversion (
	char	const * const	text_in,
	char	const * const	text_in_encoding,
	char	**	const	text_out,
	char	const * const	text_out_encoding
	)
{
	char		* bufin, * bufree;
	size_t		size_in, size_out, s;
	iconv_t		cd;


	if (	! text_in		||
		! text_in_encoding	||
		! text_out		||
		! text_out_encoding
		)
	{
		return -666;
	}

	size_in = strlen ( text_in );
	if ( size_in > (SIZE_MAX / 4 - 5) )
	{
		return -666;
	}

	size_out = (size_in + 1) * 4 + 5;

#if DEBUG
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-qual"
#endif

	// We sadly have to lose the const due to iconv
	bufin = (char *)text_in;

#if DEBUG
#	pragma GCC diagnostic pop
#endif

	bufree = (char *)calloc ( size_out, 1 );

	if ( ! bufree )
	{
		return -1;		// Not enough memory
	}

	cd = iconv_open ( text_out_encoding, text_in_encoding );
	if ( cd == (iconv_t)(-1) )
	{
		free ( bufree );
		return -2;		// iconv_open failure
	}
	else
	{
		char		* bufout = bufree;


		s = iconv ( cd, &bufin, &size_in, &bufout, &size_out );
		iconv_close ( cd );
		if ( s )
		{
			free ( bufree );
			return -3;		// iconv failure
		}
	}

	s = strlen ( bufree ) + 1;	// Get real size of output buffer
	char * nb = (char *)realloc ( bufree, s ); // Remove any slack memory

	if ( ! nb )
	{
		free ( bufree );
		return -4;		// realloc failure
	}

	text_out[0] = nb;

	return 0;			// Success
}

int mtkit_string_argv_free (
	char		**	const	args
	)
{
	int		i;


	if ( ! args )
	{
		return 0;
	}

	for ( i = 0; args[i]; i++ )
	{
		free ( args[i] );
	}

	free ( args );

	return 0;
}

char ** mtkit_string_argv (
	char	const *	input
	)
{
	char	const	* src;
	char	const	* input_end;
	char		** args,
			* nm;
	int		arg,
			arg_tot,
			q,
			cpos;


	if ( ! input )
	{
		return NULL;
	}

	// Ignore leading whitespaces
	while ( isspace ( input[0] ) )
	{
		input ++;
	}

	// Count the number of arguments in the input string
	arg = 0;
	q = 0;

	for ( src = input; src[0]; src ++ )
	{
		if ( src[0] == '"' )
		{
			q = ! q;

			continue;
		}
		else if ( src[0] == '\\' )
		{
			src ++;

			if ( src[0] == 0 )
			{
				break;
			}
		}
		else if ( isspace ( src[0] ) )
		{
			if ( ! q )
			{
				while ( isspace ( src[0] ) )
				{
					src ++;
				}

				if ( src[0] == 0 )
				{
					break;
				}

				src --;
				arg ++;

				continue;
			}
		}
	}

	input_end = src;
	arg_tot = arg + 1;

	args = (char **)calloc ( (size_t)(arg_tot + 1), sizeof ( char * ) );

	if ( ! args )
	{
		return NULL;
	}

	// Create each of the args
	arg = 0;
	q = 0;
	cpos = 0;
	args[0] = (char *)malloc ( (size_t)(input_end - input + 1) );

	if ( ! args[0] )
	{
		goto fail;
	}

	for ( src = input; src[0]; src ++ )
	{
		if ( src[0] == '"' )
		{
			q = ! q;

			continue;
		}
		else if ( src[0] == '\\' )
		{
			src ++;
			if ( src[0] == 0 )
			{
				break;
			}
		}
		else if ( isspace ( src[0] ) )
		{
			if ( ! q )
			{
				while ( isspace ( src[0] ) )
				{
					src ++;
				}

				if ( src[0] == 0 )
				{
					break;
				}

				// Finish off this arg string
				args[arg][cpos] = 0;

				nm = (char *)realloc ( args[arg],
					(size_t)(cpos + 1) );

				if ( ! nm )
				{
					goto fail;
				}

				args[arg] = nm;

				src --;
				arg ++;

				// Prepare the next arg string
				if ( src > input_end )
				{
					goto fail;	// Shouldn't happen
				}

				args[arg] = (char *)malloc( (size_t)(input_end -
					src + 1) );
				cpos = 0;

				if ( ! args[arg] )
				{
					goto fail;
				}

				continue;
			}
		}

		args[arg][cpos] = src[0];
		cpos ++;
	}

	// Finish off the last arg string
	args[arg][cpos] = 0;
	args[arg] = (char *)realloc ( args[arg], (size_t)(cpos + 1) );

	return args;

fail:
	mtkit_string_argv_free ( args );

	return NULL;
}

int mtkit_strnncpy (
	char		* const	dest,
	char	const	* const	src,
	size_t		const	destSize
	)
{
	if ( ! dest || ! src || destSize < 1 )
	{
		return 1;
	}


	size_t		len = strlen ( src );


	if ( len > (destSize - 1) )
	{
		// src string is too long for buffer so truncate

		len = (destSize - 1);
	}

	memcpy ( dest, src, len );

	dest [ len ] = 0;

	return 0;
}

int mtkit_strnncat (
	char		*	dest,
	char	const	* const	src,
	size_t			destSize
	)
{
	if ( ! dest || ! src || destSize < 1 )
	{
		return 1;
	}


	size_t		len = strlen ( dest );


	dest += len;
	destSize -= len;

	// NOTE: destSize is always at least 1 here (i.e. the NUL terminator)

	len = strlen ( src );

	if ( len > (destSize - 1) )
	{
		// src string is too long for buffer so truncate

		len = (destSize - 1);
	}

	memcpy ( dest, src, len );

	dest [ len ] = 0;

	return 0;
}

int mtkit_strtod (
	char	const	* const	input,
	double		* const	result,
	char	*	* const	next,
	int		const	strict
	)
{
	double		d;
	char		* s;


	errno = 0;
	d = strtod ( input, &s );

	if ( errno || ! s || s == input )
	{
		return 1;	/* Error or nothing parsed */
	}

	if ( strict && mtkit_strnonspaces ( s ) )
	{
		return 1;	/* String not a pure number */
	}

	if ( next )
	{
		next[0] = s;
	}

	if ( result )
	{
		result[0] = d;
	}

	return 0;
}

int mtkit_strtoi (
	char	const	* const	input,
	int		* const	result,
	char	*	* const	next,
	int		const	strict
	)
{
	int		i,
			base = 10;
	char		* s;


	if ( input[0] == '0' && input[1] == 'x' )
	{
		base = 16;
	}

	errno = 0;
	i = (int)strtol ( input, &s, base );

	if ( errno || ! s || s == input )
	{
		return 1;	/* Error or nothing parsed */
	}

	if ( strict && mtkit_strnonspaces ( s ) )
	{
		return 1;	/* String not a pure number */
	}

	if ( next )
	{
		next[0] = s;
	}

	if ( result )
	{
		result[0] = i;
	}

	return 0;
}

char * mtkit_strtok (
	char	const *	const	input,
	char	const *	const	delim,
	int		const	ntok
	)
{
	int		del_found,
			n		= ntok;
	char	const	* sp;
	char	const	* sp2;
	char	const	* sd;


	if ( ! input || ! delim || n < 0 )
	{
		return NULL;
	}

	// Find the n'th delimeter
	for ( sp = input; n > 0; sp ++ )
	{
		if ( sp[0] == 0 )
		{
			return NULL;	// End of string reached
		}

		for ( sd = delim; sd[0] != 0; sd ++ )
		{
			if ( sp[0] == sd[0] )	// Delimeter found
			{
				n --;

				break;
			}
		}
	}

	// Get length of this field
	del_found = 0;
	for ( sp2 = sp; sp2[0] != 0 && ! del_found; sp2 ++ )
	{
		for ( sd = delim; sd[0] != 0; sd ++ )
		{
			if ( sp2[0] == sd[0] )
			{
				del_found = 1;
				sp2 --;

				break;
			}
		}
	}


	size_t		newlen = (size_t)(sp2 - sp);
	char		* ns = (char *)calloc ( newlen + 1, 1 );


	if ( ! ns )
	{
		return NULL;
	}

	if ( newlen > 0 )
	{
		memcpy ( ns, sp, newlen );
	}

	return ns;
}

int mtkit_strtok_num (
	char	const	* const	input,
	char	const	* const	delim,
	int		const	n,
	double		* const	result
	)
{
	int		res;
	char		* tok;


	tok = mtkit_strtok ( input, delim, n );
	if ( ! tok )
	{
		return 1;
	}

	res = mtkit_strtod ( tok, result, NULL, 0 );
	free ( tok );

	return res;
}

char * mtkit_strcasestr (
	char	const * const	haystack,
	char	const * const	needle
	)
{
	int		i;
	char	const * hp;


	if ( ! haystack || ! needle )
	{
		return NULL;
	}

	if ( needle[0] == 0 )
	{
#if DEBUG
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-qual"
#endif
		return (char *)haystack;
#if DEBUG
#	pragma GCC diagnostic pop
#endif
	}

	for ( hp = haystack; hp[0] != 0; hp ++ )
	{
		for ( i = 0; needle[i] != 0; i++ )
		{
			if ( tolower ( hp[i] ) != tolower ( needle[i] ) )
			{
				break;
			}
		}

		if ( needle[i] == 0 )
		{
			// Whole of needle exists here in the haystack

#if DEBUG
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-qual"
#endif
			return (char *)hp;
#if DEBUG
#	pragma GCC diagnostic pop
#endif
		}
	}

	return NULL;			// needle not found
}

int mtkit_strfreedup (
	char	*	* const	spp,
	char	const	* const	str
	)
{
	char		* ns = NULL;


	if ( ! spp )
	{
		return 1;
	}

	if ( str )
	{
		ns = strdup ( str );

		if ( ! ns )
		{
			return 1;
		}
	}

	free ( spp[0] );
	spp[0] = ns;

	return 0;
}



enum
{
	HTML_AMP_LEN	= 5,		// &amp;
	HTML_QUOT_LEN	= 6,		// &quot;
	HTML_LT_LEN	= 4,		// &lt;
	HTML_GT_LEN	= 4		// &gt;
};



char * mtkit_strtohtml (
	char	const * const	input
	)
{
	size_t		len;
	char	const	* src;
	char		* ns = NULL, * dest;


	if ( ! input )
	{
		return NULL;
	}

	// Calculate the length of the new string
	for ( len = 0, src = input; src[0] != 0; src ++ )
	{
		switch ( src[0] )
		{
		case '&':	len += HTML_AMP_LEN;	break;
		case '"':	len += HTML_QUOT_LEN;	break;
		case '<':	len += HTML_LT_LEN;	break;
		case '>':	len += HTML_GT_LEN;	break;
		default:	len ++;
		}
	}

	ns = (char *)calloc ( len + 1, 1 );
	if ( ! ns )
	{
		return NULL;
	}

	// Populate the new string
	for ( dest = ns, src = input; src[0] != 0; src ++ )
	{
		if ( src[0] < 32 && src[0] >= 0 )
		{
			*dest++ = 32;

			continue;
		}

		switch ( src[0] )
		{
		case '&':
			memcpy ( dest, "&amp;", HTML_AMP_LEN );
			dest += HTML_AMP_LEN;
			break;

		case '"':
			memcpy ( dest, "&quot;", HTML_QUOT_LEN );
			dest += HTML_QUOT_LEN;
			break;

		case '<':
			memcpy ( dest, "&lt;", HTML_LT_LEN );
			dest += HTML_LT_LEN;
			break;

		case '>':
			memcpy ( dest, "&gt;", HTML_GT_LEN );
			dest += HTML_GT_LEN;
			break;

		default:
			*dest++ = src[0];
		}
	}

	// calloc has terminated the string for us

	return ns;
}



/*
Wildcard expansion for * and ? characters.  A classic problem, and here is how
I solved it from scratch!  Built in support for case (in)sensitivity and a few
of my favourite \ substitutions.
M.Tyler 23-8-2009
*/

// The character after a \ needs to be substituted in some cases
static char char_subst (
	char	const	ch
	)
{
	switch ( ch )
	{
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	}

	return ch;
}

// Create new dynamic string with substitutions for \, i.e. "\*" -> "*",
// "\?" -> "?" etc.
// Terminators are 0 * ? characters

static char * str_form (
	char	const * const	st,
	char	const ** const	pnext
	)
{
	char		* nst, * np;
	char	const	* cp;
	int		tot;


	for ( cp = st, tot = 0; ; cp ++, tot ++ )
	{
		if (	cp[0] == 0	||
			cp[0] == '*'	||
			cp[0] == '?'
			)
		{
			break;
		}

		if ( cp[0] == '\\' )
		{
			cp ++;

			if ( cp[0] == 0 )
			{
				break;
			}
		}
	}

	pnext[0] = cp;

	if ( tot < 1 )
	{
		// Shouldn't happen if str_form called properly

		return NULL;
	}

	nst = (char *)malloc ( (size_t)(1 + tot) );

	if ( ! nst )
	{
		return NULL;
	}

	nst[tot] = 0;

	for ( cp = st, np = nst; ; cp ++, np ++ )
	{
		char ch = cp[0];

		if (	ch == 0		||
			ch == '*'	||
			ch == '?'
			)
		{
			break;
		}

		if ( ch == '\\' )
		{
			cp ++;

			if ( cp[0] == 0 )
			{
				break;
			}

			ch = char_subst ( cp[0] );
		}

		np[0] = ch;
	}

	return nst;
}

int mtkit_strmatch (
	char	const * const	string,
	char	const * const	pattern,
	int		const	mode
	)
{
	int		match = -1,
			sp = 0,
			pp = 0,
			res;
	size_t		substr_len;
	char		* substr;
	char	const	* st;
	char	const	* pnext;


	if ( ! string || ! pattern )
	{
		return -2;
	}

	while ( 1 )
	{
		int s = string[ sp ++ ];
		int p = pattern[ pp ++ ];

		if ( ! ( mode & 1 ) )
		{
			s = tolower ( s );
			p = tolower ( p );
		}

		switch ( p )
		{
		case '?':
			if ( s == 0 )
			{
				// No more characters, but pattern needs one

				return -1;
			}
			break;

		case '*':
			// Once in here, we return and never break
			while ( 1 )
			{
				p = pattern[ pp ++ ];

				switch ( p )
				{
				case 0:
					// Pattern ending in * or ? always
					// matches here

					if ( match < 0 )
					{
						match = 0;
					}

					return match;

				case '*':
					// Repeat *'s can be ignored here
					// because:
					// "*" = "**" = "****", "*?*" = "*?",
					// "*??*" = "*??" = "*?*?*"

					break;

				case '?':
					if ( s == 0 )
					{
						// No more characters, but
						// pattern needs one

						return -1;
					}

					s = string[ sp ++ ];

					// No need to bother with case check
					// for ?

					break;

				default:
					if ( s == 0 )
					{
						// No more characters, but
						// pattern needs one

						return -1;
					}

					substr = str_form ( pattern + pp - 1,
						&pnext );

					if ( ! substr )
					{
						return -3;
					}

					substr_len = strlen ( substr );
					st = string + sp - 2;

					do
					{
						if ( mode & 1 )
						{
							st = strstr ( st + 1,
								substr );
						}
						else
						{
							st = mtkit_strcasestr (
								st + 1,
								substr );
						}

						if ( ! st )
						{
							// No match
							free ( substr );

							return -1;
						}

						res = mtkit_strmatch ( st +
							substr_len, pnext,
							mode );

						if ( res < -1 )
						{
							// Unexpected error
							free ( substr );

							return res;
						}
					}
					while ( res < 0 );

					free ( substr );

					if ( match < 0 )
					{
						match = (int)(st - string);
					}

					// Recursion also matches so this was
					// first matching char

					return match;
				}
			}
			break;

		default:
			if ( match < 0 )
			{
				// Set match point if not already done so
				match = sp - 1;
			}

			if ( s == 0 && p == 0 )
			{
				// End of both so they must match

				return match;
			}

			if ( p == '\\' )
			{
				p = char_subst ( pattern[ pp ++ ] );

				if ( s == 0 && p == 0 )
				{
					// End of both so they must match
					// "abcdef" == "abcdef\"

					return match;
				}
			}

			if ( s == 0 || p == 0 )
			{
				// "abc" != "abcdef" or "abcdef" != "abc\"
				// "abcdef" != "abc"

				return -1;
			}

			if ( s != p )
			{
				// Chars don't match so pattern doesn't

				return -1;
			}

			break;		// These chars match so move on in loop
		}
	}

	return -1;			// No match
}

int mtkit_strnonspaces (
	char	const *	input
	)
{
	while ( input[0] )
	{
		if ( ! isspace ( input[0] ) )
		{
			return 1;
		}

		input ++;
	}

	return 0;
}

int mtkit_strtothou (
	char		* const	dest,
	char	const	* const	src,
	int		const	dest_size,
	char		const	separator,
	char		const	minus,
	char		const	dpoint,
	int		const	sep_num,
	int		const	right_justify
	)
{
	int		i, j, k, start, end, oldlen, newlen;
	char	const	* ch;


	if (	! dest		||
		! src		||
		sep_num < 1	||
		dest_size < 1
		)
	{
		return -1;
	}

	size_t const len = strlen ( src );
	if ( len > INT_MAX / 4 )
	{
		return -1;
	}

	oldlen = (int)len;

	// First character to be subjected to separating
	start = 0;

	// Last character to be subjected to separating
	end = oldlen - 1;

	if ( ( ch = strrchr ( src, dpoint ) ) )
	{
		// Decimal point detected, adjust end accordingly
		end = (int)(ch - src - 1);
	}

	if ( ( ch = strchr ( src, minus ) ) )
	{
		// Minus sign detected, adjust start accordingly
		start = (int)(ch - src + 1);
	}

	if ( ( end + 1 ) < start )
	{
		// Decimal point is before minus sign so bail out

		return -1;
	}

	newlen = oldlen + ( end - start ) / sep_num;
	if ( newlen + 1 > dest_size )
	{
		// Output buffer is not large enough to hold the result, so
		// bail out

		return -1;
	}

	i = oldlen - 1;			// Input buffer pointer
	k = dest_size - 1;		// Output buffer pointer
	dest[k--] = 0;			// Output string terminator

	while ( i > end )
	{
		dest[ k-- ] = src[ i-- ];

		// Copy characters from the end to the '.'
	}

	for ( j = 0; i >= start; j++ )
	{
		if ( j && ( (j % sep_num) == 0 ) )
		{
			dest[ k-- ] = separator;	// Add separator
		}

		dest[k--] = src[i--];			// Copy char
	}

	while ( i >= 0 )
	{
		// Copy characters from the '-' to the beginning
		dest[ k-- ] = src[ i-- ];
	}

	if ( right_justify )
	{
		// Right align so pad beginning of output with spaces
		while ( k >= 0 )
		{
			dest[ k-- ] = ' ';
		}
	}
	else
	{
		// Left align so shift string flush to beginning
		k++;
		j = 0;

		do
		{
			dest[ j++ ] = dest[ k++ ];
		}
		while ( dest[ j - 1 ] != 0 );
	}

	return 0;
}

