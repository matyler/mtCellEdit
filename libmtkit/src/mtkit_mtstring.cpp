/*
	Copyright (C) 2015-2016 Mark Tyler

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



#define MTKIT_STRING_BUFLEN_MAX		1048576
#define MTKIT_STRING_BUFLEN_CHUNK	64



struct mtString
{
	char		* buf;
	size_t		buflen;		// Total size of buffer
	size_t		len;		// Length of string
};



mtString * mtkit_string_new (
	char	const * const	cs
	)
{
	mtString	* ns;


	ns = (mtString *)calloc ( 1, sizeof ( *ns ) );
	if ( ! ns )
	{
		return NULL;
	}

	ns->buf = (char *)calloc ( 1, MTKIT_STRING_BUFLEN_CHUNK );
	ns->buflen = MTKIT_STRING_BUFLEN_CHUNK;
	ns->len = 0;

	if (	! ns->buf ||
		( cs && mtkit_string_append ( ns, cs ) )
		)
	{
		mtkit_string_destroy ( ns );

		return NULL;
	}

	return ns;
}

int mtkit_string_destroy (
	mtString	* const	str
	)
{
	if ( str )
	{
		free ( str->buf );
		free ( str );
	}

	return 0;
}

char * mtkit_string_destroy_get_buf (
	mtString	* str
	)
{
	if ( str )
	{
		char	* tmp = (char *)realloc ( str->buf, str->len + 1 );


		if ( ! tmp )
		{
			// Failed to shrink so return current allocation
			tmp = str->buf;
		}

		free ( str );

		return tmp;
	}

	return NULL;
}

size_t mtkit_string_get_len (
	mtString	* str
	)
{
	if ( str )
	{
		return str->len;
	}

	return 0;
}

char const * mtkit_string_get_buf (
	mtString	* str
	)
{
	if ( str )
	{
		return str->buf;
	}

	return NULL;
}

int mtkit_string_append (
	mtString	* const	str,
	char	const * const	cs
	)
{
	if ( ! str || ! cs )
	{
		// Nothing to do
		return 0;
	}

	size_t cslen = strlen ( cs );
	if ( cslen > MTKIT_STRING_BUFLEN_MAX )
	{
		// Argument string too long
		return 1;
	}
	else if ( cslen < 1 )
	{
		// Nothing to append
		return 0;
	}

	size_t catlen = cslen + str->len;
	if ( catlen > MTKIT_STRING_BUFLEN_MAX )
	{
		// New string too long
		return 1;
	}

	if ( catlen >= str->buflen )
	{
		char		* tb;
		size_t	const	tblen = catlen + MTKIT_STRING_BUFLEN_CHUNK;


		// Current buffer isn't large enough so enlarge it

		tb = (char *)realloc ( str->buf, tblen );
		if ( ! tb )
		{
			return 1;
		}

		str->buf = tb;
		str->buflen = tblen;
	}

	memcpy ( str->buf + str->len, cs, cslen );
	str->buf [ catlen ] = 0;
	str->len = catlen;

	return 0;
}

static char * mtkit_string_join_list (
	char	const * const *	const	list
	)
{
	int		i;
	mtString	* st;


	if (	! list ||
		! ( st = mtkit_string_new ( NULL ) )
		)
	{
		return NULL;
	}

	for ( i = 0; list[i]; i++ )
	{
		if ( mtkit_string_append ( st, list[i] ) )
		{
			mtkit_string_destroy ( st );

			return NULL;
		}
	}

	return mtkit_string_destroy_get_buf ( st );
}

char * mtkit_string_join (
	char	const * const	sta,
	char	const * const	stb,
	char	const * const	stc,
	char	const * const	std
	)
{
	char	const * const	list[5] = { sta, stb, stc, std, NULL };


	return mtkit_string_join_list ( list );
}

