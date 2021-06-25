/*
	Copyright (C) 2021 Mark Tyler

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

#include "lanter.h"



namespace {

static int print_version ()
{
	printf ( BIN_NAME " - Part of %s\n\n", VERSION );

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

}		// namespace {



Core::Core ()
{
}

Core::~Core ()
{
}

int Core::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	mtKit::Arg args;

	int stop = 0;

	args.add ( "-help",	stop, 1, print_help );
	args.add ( "-version",	stop, 1, print_version );
	args.add ( "dir",	m_anim_dir );
	args.add ( "fps",	m_anim_fps );
	args.add ( "oh",	m_output_height );
	args.add ( "ow",	m_output_width );
	args.add ( "prefs",	m_prefs_filename );
	args.add ( "savemap",	m_save_map );
	args.add ( "seed",	m_map_seed );
	args.add ( "size",	m_map_size );
	args.add ( "tsv",	m_anim_tsv );
	args.add ( "v",		m_verbose, 1 );

	if ( args.parse ( argc, argv ) || stop )
	{
		return 1;		// Quit program
	}

	prefs_init ();

	return 0;			// Continue program
}

void Core::prefs_init ()
{
	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );
	uprefs.add_int ( PREFS_WINDOW_W, mprefs.window_w, 500 );
	uprefs.add_int ( PREFS_WINDOW_H, mprefs.window_h, 500 );
	uprefs.add_int ( PREFS_WINDOW_MAXIMIZED, mprefs.window_maximized, 0 );

	uprefs.set_invisible ( PREFS_WINDOW_X );
	uprefs.set_invisible ( PREFS_WINDOW_Y );
	uprefs.set_invisible ( PREFS_WINDOW_W );
	uprefs.set_invisible ( PREFS_WINDOW_H );
	uprefs.set_invisible ( PREFS_WINDOW_MAXIMIZED );

	uprefs.load ( m_prefs_filename, BIN_NAME );
}

