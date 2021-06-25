/*
	Copyright (C) 2018-2020 Mark Tyler

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



int Backend::init_well ()
{
	return open_database ( m_db_path );
}

int Backend::open_database ( char const * const path )
{
	if ( db.open ( path ) )
	{
		return 1;
	}

	mprefs.recent_db.set ( db.get_path() );

	return 0;
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
		"%s(1) or the mtDataWell Handbook.\n"
		"\n"
		, BIN_NAME );

	return 1;		// Stop parsing
}

int Backend::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	mtKit::Arg args ( [this]( char const * const filename )
		{
			cline_add_filename ( filename );
			return 0; 	// Continue parsing
		} );

	int stop = 0;

	args.add ( "-help",	stop, 1, print_help );
	args.add ( "-version",	stop, 1, print_version );
	args.add ( "db",	m_db_path );
	args.add ( "prefs",	m_prefs_filename );

	args.parse ( argc, argv );

	if ( stop )
	{
		return 1;		// Quit program
	}

	prefs_init ();

	return 0;			// Continue program
}

void Backend::cline_add_filename (
	char	const * const	filename
	)
{
	m_cline_files.push_back ( filename );
}

void Backend::prefs_init ()
{
	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );
	uprefs.add_int ( PREFS_WINDOW_W, mprefs.window_w, 500 );
	uprefs.add_int ( PREFS_WINDOW_H, mprefs.window_h, 500 );
	uprefs.add_int ( PREFS_WINDOW_MAXIMIZED, mprefs.window_maximized, 1 );

	uprefs.add_string ( PREFS_LAST_DIRECTORY, mprefs.last_directory, "" );

	uprefs.add_string ( PREFS_HELP_FILE, mprefs.help_file,
		DATA_INSTALL "/doc/" BIN_NAME "/en_GB/chap_00.html" );
	uprefs.add_string ( PREFS_HELP_BROWSER, mprefs.help_browser, "" );

	uprefs.add_ui_defaults ( mprefs.ui_prefs );

	mprefs.recent_db.init ( uprefs, PREFS_RECENT_DB, PREFS_RECENT_DB_TOTAL);
	mprefs.recent_dir.init (uprefs,PREFS_RECENT_DIR,PREFS_RECENT_DIR_TOTAL);

	uprefs.load ( m_prefs_filename, BIN_NAME );
}

