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

extern "C" {

	#include <stdlib.h>
	#include <time.h>
}

#include <mtkit.h>



#define ST_MAX 150



static void test_mtstring ( void )
{
	char		buf [ ST_MAX ];
	mtString	* st;
	int		i, l;


	st = mtkit_string_new ( NULL );

	while ( 1 )
	{
		l = rand () % ST_MAX;

		for ( i = 0; i < l; i++ )
		{
			buf [ i ] = (char)('a' + rand () % 26);
		}

		buf[i] = 0;

		if ( mtkit_string_append ( st, buf ) )
		{
			break;
		}

		printf ( "%s", buf );
	}

	printf ( "\n%s\n", mtkit_string_get_buf ( st ) );

	mtkit_string_destroy ( st );
}

int main (
	int			const	ARG_UNUSED ( argc ),
	char	const * const * const	ARG_UNUSED ( argv )
	)
{
	srand ( (unsigned int)time ( NULL ) );

	test_mtstring ();

	return 0;
}

