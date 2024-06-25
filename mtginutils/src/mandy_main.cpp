/*
	Copyright (C) 2021-2024 Mark Tyler

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

#include "mandy_main.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	CommandLine	cline;

	if ( cline.command_line ( argc, argv ) )
	{
		return cline.exit.value();
	}

	// Needed before creating default constants
	mpfr_set_default_prec ( MandelbrotFloat::NUMBER_PRECISION_BITS );

	Mainwindow	window ( cline );

	return cline.exit.value();
}



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



CommandLine::CommandLine ()
{
}

CommandLine::~CommandLine ()
{
}

int CommandLine::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	mtKit::Arg args;

	int stop = 0;

	args.add ( "-help",	stop, 1, print_help );
	args.add ( "-version",	stop, 1, print_version );
	args.add ( "cx",	m_axis_cx_st );
	args.add ( "cy",	m_axis_cy_st );
	args.add ( "depth",	m_depth_max );
	args.add ( "dz_double", m_deep_zoom_type, DEEP_ZOOM_DOUBLE );
	args.add ( "dz_float",	m_deep_zoom_type, DEEP_ZOOM_FLOAT );
	args.add ( "dz_float_fast", m_deep_zoom_type, DEEP_ZOOM_FLOAT_FAST );
	args.add ( "dz_none", 	m_deep_zoom_type, DEEP_ZOOM_NONE );
	args.add ( "dz_always", m_deep_zoom_on, 0 );
	args.add ( "dz_on", 	m_deep_zoom_on, 1 );
	args.add ( "dz_rational", m_deep_zoom_type, DEEP_ZOOM_RATIONAL );
	args.add ( "oh",	m_output_height );
	args.add ( "ow",	m_output_width );
	args.add ( "prefs",	m_prefs_filename );
	args.add ( "q",		m_verbose, 0 );
	args.add ( "range",	m_axis_range_st );
	args.add ( "threads",	m_thread_total );

	if ( args.parse ( argc, argv ) || stop )
	{
		return 1;		// Quit program
	}

	if ( m_thread_total < 1 )
	{
		m_thread_total = SDL_GetCPUCount ();
	}

	m_thread_total = mtkit_int_bound ( m_thread_total,
		THREAD_TOTAL_MIN, THREAD_TOTAL_MAX );

	m_output_width = mtkit_int_bound ( m_output_width,
		OUTPUT_WH_MIN, OUTPUT_WH_MAX );
	m_output_height = mtkit_int_bound ( m_output_height,
		OUTPUT_WH_MIN, OUTPUT_WH_MAX );

	prefs_init ();

	return 0;			// Continue program
}

void CommandLine::prefs_init ()
{
	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );

	uprefs.set_invisible ( PREFS_WINDOW_X );
	uprefs.set_invisible ( PREFS_WINDOW_Y );

	uprefs.load ( m_prefs_filename, BIN_NAME );
}

