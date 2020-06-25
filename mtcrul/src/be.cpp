/*
	Copyright (C) 2020 Mark Tyler

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
	recent_crul_db	( PREFS_CRUL_RECENT_DB, PREFS_CRUL_RECENT_DB_TOTAL ),
	m_db_path	( NULL ),
	m_prefs_filename( NULL )
{
}

Backend::~Backend ()
{
}

void Backend::get_data_dir ( std::string & path ) const
{
	if ( m_db_path )
	{
		path = m_db_path;
	}
	else
	{
		mtKit::get_data_dir ( path, DATA_INSTALL );
	}

	path += MTKIT_DIR_SEP;
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
		"%s(1) or the mtCellEdit Handbook.\n"
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
	{ PREFS_CRUL_SPLIT_MAIN, MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },

	{ PREFS_GL_POINT_RANGE, MTKIT_PREF_TYPE_DOUBLE, "1000", NULL, NULL, 0, "10	1000000", NULL },
	{ PREFS_GL_POINT_SIZE, MTKIT_PREF_TYPE_DOUBLE, "1", NULL, NULL, 0, "1	20", NULL },
	{ PREFS_GL_LINE_BUTT_SIZE, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, "-10	10", NULL },
	{ PREFS_GL_LINE_THICKNESS, MTKIT_PREF_TYPE_DOUBLE, "1", NULL, NULL, 0, "1	20", NULL },
	{ PREFS_GL_LIGHT_CAMERA, MTKIT_PREF_TYPE_BOOL, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_GL_LIGHT_X, MTKIT_PREF_TYPE_DOUBLE, "-1000", NULL, NULL, 0, NULL, NULL },
	{ PREFS_GL_LIGHT_Y, MTKIT_PREF_TYPE_DOUBLE, "-800", NULL, NULL, 0, NULL, NULL },
	{ PREFS_GL_LIGHT_Z, MTKIT_PREF_TYPE_DOUBLE, "500", NULL, NULL, 0, NULL, NULL },

	{ PREFS_CLOUD_RATE_LOW, MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, "-1024	1022", NULL },
	{ PREFS_CLOUD_RATE_MEDIUM, MTKIT_PREF_TYPE_INT, "-2", NULL, NULL, 0, "-1023	1023", NULL },

	{ PREFS_VIEW_NUDGE_SIZE, MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, "-10	10", NULL },
	{ PREFS_VIEW_SPLIT_ON, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SPLIT_POS, MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SPLIT_VERT, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },

	{ PREFS_VIEW_A PREFS_CAM_X, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_A PREFS_CAM_Y, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_A PREFS_CAM_Z, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_A PREFS_CAM_XROT, MTKIT_PREF_TYPE_DOUBLE, "275", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_A PREFS_CAM_YROT, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_A PREFS_CAM_ZROT, MTKIT_PREF_TYPE_DOUBLE, "80", NULL, NULL, 0, NULL, NULL },

	{ PREFS_VIEW_B PREFS_CAM_X, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_B PREFS_CAM_Y, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_B PREFS_CAM_Z, MTKIT_PREF_TYPE_DOUBLE, "300", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_B PREFS_CAM_XROT, MTKIT_PREF_TYPE_DOUBLE, "180", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_B PREFS_CAM_YROT, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_B PREFS_CAM_ZROT, MTKIT_PREF_TYPE_DOUBLE, "0", NULL, NULL, 0, NULL, NULL },

	{ PREFS_VIEW_SHOW_ANTIALIASING, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SHOW_CLOUD, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SHOW_CROSSHAIR, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SHOW_MODEL, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SHOW_RULER_PLANE, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SHOW_RULERS, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_VIEW_SHOW_STATUSBAR, MTKIT_PREF_TYPE_BOOL, "1", NULL, NULL, 0, NULL, NULL },

	{ PREFS_WINDOW_X, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_Y, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_W, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_H, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_MAXIMIZED, MTKIT_PREF_TYPE_INT, "1", NULL, NULL, 0, NULL, NULL },

	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	prefs.addTable ( prefs_table );
	prefs.initWindowPrefs ();

	recent_crul_db.init_prefs ( &prefs );

	prefs.load ( m_prefs_filename, BIN_NAME );
}

