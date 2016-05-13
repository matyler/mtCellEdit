/*
	Copyright (C) 2009-2014 Mark Tyler

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

#include "be.h"



void be_cline (
	int			const	argc,
	char	const * const * const	argv
	)
{
	if ( argc > 1 )
	{
		if ( strcmp ( argv[1], "--version" ) == 0 )
		{
			printf ("%s\n\n", VERSION);

			exit ( 0 );
		}
		else if ( strcmp ( argv[1], "--help" ) == 0 )
		{
			printf ( "%s\n\n"
				"For further information consult the man page "
				"%s(1) or the mtCellEdit Handbook.\n"
				"\n", VERSION, BIN_NAME );

			exit ( 0 );
		}
		else
		{
			fprintf ( stderr, "Unknown argument: '%s'\n\n",
				argv [ 1 ] );

			exit ( 0 );
		}
	}
}

