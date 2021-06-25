/*
	Copyright (C) 2008-2020 Mark Tyler

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



char const * Backend::get_cline_filename () const
{
	return cline_filename;
}

int Backend::get_force_tsvcsv () const
{
	return force_tsvcsv;
}

static int print_version ()
{
	printf ( "%s\n\n", VERSION );

	return 1;		// Stop parsing
}

static int print_help ()
{
	print_version ();

	printf ("For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n"
		, BIN_NAME );

	return 1;		// Stop parsing
}

int Backend::command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtKit::Arg args ( [this]( char const * const filename )
		{
			this->cline_filename = filename;
			return 0; 	// Continue parsing
		} );

	int stop = 0;

	args.add ( "-help",	stop, 1, print_help );
	args.add ( "-version",	stop, 1, print_version );
	args.add ( "csv",	force_tsvcsv, CED_FILE_FORCE_CSV );
	args.add ( "prefs",	prefs_filename );
	args.add ( "tsv",	force_tsvcsv, CED_FILE_FORCE_TSV );

	args.parse ( argc, argv );

	if ( stop )
	{
		return 1;		// Quit program
	}

	prefs_init ();

	return 0;
}

