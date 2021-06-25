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
	uprefs.add_string ( PREFS_CRUL_SPLIT_MAIN, mprefs.crul_split_main, "" );
	uprefs.set_invisible ( PREFS_CRUL_SPLIT_MAIN );

	uprefs.add_double ( PREFS_GL_POINT_RANGE, mprefs.gl_point_range, 1000,
		10, 1000000 );
	uprefs.add_double ( PREFS_GL_POINT_SIZE, mprefs.gl_point_size, 1, 1,20);
	uprefs.add_double ( PREFS_GL_LINE_BUTT_SIZE, mprefs.gl_line_butt_size,
		0, -10, 10 );
	uprefs.add_double ( PREFS_GL_LINE_THICKNESS, mprefs.gl_line_thickness,
		1, 1, 20 );

	uprefs.set_invisible ( PREFS_GL_POINT_SIZE );
	uprefs.set_invisible ( PREFS_GL_LINE_BUTT_SIZE );
	uprefs.set_invisible ( PREFS_GL_LINE_THICKNESS );

	uprefs.add_bool ( PREFS_GL_LIGHT_CAMERA, mprefs.gl_light_camera, 0 );
	uprefs.add_double ( PREFS_GL_LIGHT_X, mprefs.gl_light_x, -1000 );
	uprefs.add_double ( PREFS_GL_LIGHT_Y, mprefs.gl_light_y, -800 );
	uprefs.add_double ( PREFS_GL_LIGHT_Z, mprefs.gl_light_z, 500 );

	uprefs.add_int ( PREFS_CLOUD_RATE_LOW, mprefs.cloud_rate_low, 0, -1024,
		1022 );
	uprefs.add_int ( PREFS_CLOUD_RATE_MEDIUM, mprefs.cloud_rate_medium, -2,
		-1023, 1023 );

	uprefs.add_int ( PREFS_VIEW_NUDGE_SIZE, mprefs.view_nudge_size, 0, -10,
		10 );
	uprefs.set_invisible ( PREFS_VIEW_NUDGE_SIZE );

	uprefs.add_bool ( PREFS_VIEW_SPLIT_ON, mprefs.view_split_on, 1 );
	uprefs.add_string ( PREFS_VIEW_SPLIT_POS, mprefs.view_split_pos, "" );
	uprefs.add_bool ( PREFS_VIEW_SPLIT_VERT, mprefs.view_split_vert, 1 );

	uprefs.set_invisible ( PREFS_VIEW_SPLIT_ON );
	uprefs.set_invisible ( PREFS_VIEW_SPLIT_POS );
	uprefs.set_invisible ( PREFS_VIEW_SPLIT_VERT );

	uprefs.add_double ( PREFS_VIEW_A PREFS_CAM_X, mprefs.view_a_cam_x, 0 );
	uprefs.add_double ( PREFS_VIEW_A PREFS_CAM_Y, mprefs.view_a_cam_y, 0 );
	uprefs.add_double ( PREFS_VIEW_A PREFS_CAM_Z, mprefs.view_a_cam_z, 0 );
	uprefs.add_double ( PREFS_VIEW_A PREFS_CAM_XROT, mprefs.view_a_cam_xrot,
		275 );
	uprefs.add_double ( PREFS_VIEW_A PREFS_CAM_YROT, mprefs.view_a_cam_yrot,
		0 );
	uprefs.add_double ( PREFS_VIEW_A PREFS_CAM_ZROT, mprefs.view_a_cam_zrot,
		80 );

	uprefs.set_invisible ( PREFS_VIEW_A PREFS_CAM_X );
	uprefs.set_invisible ( PREFS_VIEW_A PREFS_CAM_Y );
	uprefs.set_invisible ( PREFS_VIEW_A PREFS_CAM_Z );
	uprefs.set_invisible ( PREFS_VIEW_A PREFS_CAM_XROT );
	uprefs.set_invisible ( PREFS_VIEW_A PREFS_CAM_YROT );
	uprefs.set_invisible ( PREFS_VIEW_A PREFS_CAM_ZROT );

	uprefs.add_double ( PREFS_VIEW_B PREFS_CAM_X, mprefs.view_b_cam_x, 0 );
	uprefs.add_double ( PREFS_VIEW_B PREFS_CAM_Y, mprefs.view_b_cam_y, 0 );
	uprefs.add_double ( PREFS_VIEW_B PREFS_CAM_Z, mprefs.view_b_cam_z, 300);
	uprefs.add_double ( PREFS_VIEW_B PREFS_CAM_XROT, mprefs.view_b_cam_xrot,
		180 );
	uprefs.add_double ( PREFS_VIEW_B PREFS_CAM_YROT, mprefs.view_b_cam_yrot,
		0 );
	uprefs.add_double ( PREFS_VIEW_B PREFS_CAM_ZROT, mprefs.view_b_cam_zrot,
		0 );

	uprefs.set_invisible ( PREFS_VIEW_B PREFS_CAM_X );
	uprefs.set_invisible ( PREFS_VIEW_B PREFS_CAM_Y );
	uprefs.set_invisible ( PREFS_VIEW_B PREFS_CAM_Z );
	uprefs.set_invisible ( PREFS_VIEW_B PREFS_CAM_XROT );
	uprefs.set_invisible ( PREFS_VIEW_B PREFS_CAM_YROT );
	uprefs.set_invisible ( PREFS_VIEW_B PREFS_CAM_ZROT );

	uprefs.add_bool ( PREFS_VIEW_SHOW_ANTIALIASING,
		mprefs.view_show_antialiasing, 1 );
	uprefs.add_bool ( PREFS_VIEW_SHOW_CLOUD, mprefs.view_show_cloud, 1 );
	uprefs.add_bool ( PREFS_VIEW_SHOW_CROSSHAIR, mprefs.view_show_crosshair,
		1 );
	uprefs.add_bool ( PREFS_VIEW_SHOW_MODEL, mprefs.view_show_model, 1 );
	uprefs.add_bool ( PREFS_VIEW_SHOW_RULER_PLANE,
		mprefs.view_show_ruler_plane, 1 );
	uprefs.add_bool ( PREFS_VIEW_SHOW_RULERS, mprefs.view_show_rulers, 1 );
	uprefs.add_bool ( PREFS_VIEW_SHOW_STATUSBAR, mprefs.view_show_statusbar,
		1 );

	uprefs.set_invisible ( PREFS_VIEW_SHOW_ANTIALIASING );
	uprefs.set_invisible ( PREFS_VIEW_SHOW_CLOUD );
	uprefs.set_invisible ( PREFS_VIEW_SHOW_CROSSHAIR );
	uprefs.set_invisible ( PREFS_VIEW_SHOW_MODEL );
	uprefs.set_invisible ( PREFS_VIEW_SHOW_RULER_PLANE );
	uprefs.set_invisible ( PREFS_VIEW_SHOW_RULERS );
	uprefs.set_invisible ( PREFS_VIEW_SHOW_STATUSBAR );

	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );
	uprefs.add_int ( PREFS_WINDOW_W, mprefs.window_w, 500 );
	uprefs.add_int ( PREFS_WINDOW_H, mprefs.window_h, 500 );
	uprefs.add_int ( PREFS_WINDOW_MAXIMIZED, mprefs.window_maximized, 1 );

	uprefs.set_invisible ( PREFS_WINDOW_X );
	uprefs.set_invisible ( PREFS_WINDOW_Y );
	uprefs.set_invisible ( PREFS_WINDOW_W );
	uprefs.set_invisible ( PREFS_WINDOW_H );
	uprefs.set_invisible ( PREFS_WINDOW_MAXIMIZED );

	uprefs.add_ui_defaults ( mprefs.ui_editor );

	mprefs.recent_crul_db.init ( uprefs, PREFS_CRUL_RECENT_DB,
		PREFS_CRUL_RECENT_DB_TOTAL );

	uprefs.load ( m_prefs_filename, BIN_NAME );
}

