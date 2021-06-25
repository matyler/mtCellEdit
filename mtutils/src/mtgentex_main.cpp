/*
	Copyright (C) 2015-2020 Mark Tyler

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

#include "mtgentex.h"



static int	exit_val	= 0;



static int file_func ( char const * const filename )
{
	char		const * errtxt	= NULL;
	mtUtreeNode	* rootnode	= NULL;
	int		buflen		= 0;


	char * buf = mtkit_file_load( filename, &buflen, MTKIT_FILE_ZERO, NULL);

	if ( ! buf )
	{
		fprintf ( stderr, "Unable to load file '%s'\n", filename );

		exit_val = 1;
		goto finish;
	}

	rootnode = mtkit_utree_load_mem ( NULL, buf, (size_t)buflen, &errtxt );

	if ( ! rootnode || errtxt )
	{
		fprintf ( stderr, "Error: %s = %i bytes.  parsed %i bytes\n",
			filename, buflen, (int)(errtxt - buf) );
		fprintf ( stderr, "%s\n", errtxt );

		exit_val = 1;
		goto finish;
	}

	parse_node ( rootnode );

finish:

	mtkit_utree_destroy_node ( rootnode );
	free ( buf );

	return 1;		// Stop parsing command line
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	srand ( (unsigned int)time ( NULL ) );

	int	show_version	= 0;

	mtKit::Arg args ( file_func );

	args.add ( "-help",	show_version, 2 );
	args.add ( "-version",	show_version, 1 );

	args.parse ( argc, argv );

	if ( show_version )
	{
		printf ( "%s (part of %s)\n\n", argv[0], VERSION );

		if ( 2 == show_version )
		{
			printf (
				"For further information consult the man page "
				"%s(1) or the mtCellEdit Handbook.\n"
				"\n", argv[0] );
		}

		return 0;		// Quit program
	}

	return exit_val;
}

