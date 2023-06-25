/*
	Copyright (C) 2022-2023 Mark Tyler

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

int Cline::parse_command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtKit::Arg args ( []( char const * const ARG_UNUSED( filename ) )
		{
			// Quietly ignore command line data
			return 0; 	// Continue parsing
		} );

	int stop = 0;

	args.add ( "-help",	stop, 1, print_help );
	args.add ( "-version",	stop, 1, print_version );
	args.add ( "prefs",	m_prefs_filename );

	args.parse ( argc, argv );

	if ( stop )
	{
		return 1;		// Quit program
	}

	prefs_init ();

	return 0;			// Success
}

void Cline::prefs_init ()
{
	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );
	uprefs.add_int ( PREFS_WINDOW_W, mprefs.window_w, 500 );
	uprefs.add_int ( PREFS_WINDOW_H, mprefs.window_h, 500 );
	uprefs.add_int ( PREFS_WINDOW_MAXIMIZED, mprefs.window_maximized, 1 );

	uprefs.add_int ( PREFS_CALC_FLOAT_BITS, mprefs.calc_float_bits, 10,
		10, 32 );
	uprefs.add_int ( PREFS_CALC_SNIP_SIZE, mprefs.calc_snip_size, 1000,
		mtDW::Number::STRING_SNIP_MIN,
		mtDW::Number::STRING_SNIP_MAX );

	uprefs.set_invisible ( PREFS_WINDOW_X );
	uprefs.set_invisible ( PREFS_WINDOW_Y );
	uprefs.set_invisible ( PREFS_WINDOW_W );
	uprefs.set_invisible ( PREFS_WINDOW_H );
	uprefs.set_invisible ( PREFS_WINDOW_MAXIMIZED );

	uprefs.add_ui_defaults ( mprefs.ui_prefs );

	uprefs.load ( m_prefs_filename, BIN_NAME );
}

