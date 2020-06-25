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

#include "be.h"



Backend::Backend ()
	:
	recent_db		( PREFS_RECENT_DB, PREFS_RECENT_DB_TOTAL ),
	recent_dir		( PREFS_RECENT_DIR, PREFS_RECENT_DIR_TOTAL ),
	m_db_path		( NULL ),
	m_prefs_filename	( NULL )
{
}

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

	recent_db.set_filename ( db.get_path ().c_str () );

	return 0;
}

static int file_func (
	char	const	* const	filename,
	void		* const	user_data
	)
{
	Backend * const backend = static_cast<Backend *>(user_data);

	backend->cline_add_filename ( filename );

	return 0;			// Continue parsing
}

static int error_func (
	int		const	error,
	int		const	arg,
	int		const	argc,
	char	const	* const	argv[],
	void		* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "error_func: Argument ERROR! - num=%i arg=%i/%i",
		error, arg, argc );

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n" );

	return 0;			// Keep parsing
}

int Backend::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	int		show_version = 0;
	mtArg	const	arg_list[] = {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ "db",		MTKIT_ARG_STRING, &m_db_path, 0, NULL },
		{ "prefs",	MTKIT_ARG_STRING, &m_prefs_filename, 0, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};


	mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func, this );

	switch ( show_version )
	{
	case 1:
		printf ( "%s\n\n", VERSION );

		return 1;		// Quit program

	case 2:
		printf (
		"%s\n\n"
		"For further information consult the man page "
		"%s(1) or the mtDataWell Handbook.\n"
		"\n", VERSION, BIN_NAME );

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
	mtPrefTable const prefs_table[] = {
	{ PREFS_WINDOW_X, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_Y, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_W, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_H, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_MAXIMIZED, MTKIT_PREF_TYPE_INT, "1", NULL, NULL, 0, NULL, NULL },

	{ PREFS_LAST_DIRECTORY, MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },

	{ PREFS_HELP_FILE, MTKIT_PREF_TYPE_FILE, DATA_INSTALL "/doc/" BIN_NAME "/en_GB/chap_00.html", NULL, NULL, 0, NULL, NULL },
	{ PREFS_HELP_BROWSER, MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },

	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	prefs.addTable ( prefs_table );
	prefs.initWindowPrefs ();

	recent_dir.init_prefs ( &prefs );
	recent_db.init_prefs ( &prefs );

	prefs.load ( m_prefs_filename, BIN_NAME );
}

