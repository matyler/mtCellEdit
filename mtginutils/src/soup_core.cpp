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

#include "soup.h"



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
	:
	gin	( SDL_INIT_AUDIO )
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
	mtKit::Arg args ( [this]( char const * const filename )
	{
		m_input_filename = filename;
		return 0; 	// Continue parsing
	} );

	int stop = 0;

	args.add ( "-help",		stop, 1, print_help );
	args.add ( "-version",		stop, 1, print_version );
	args.add ( "dev",		m_device );
	args.add ( "gui",		m_gui, 1 );
	args.add ( "ldev",		[]()
	{
		int const count = SDL_GetNumAudioDevices ( 0 );

		for ( int i = 0; i < count; ++i )
		{
			printf ("Device %i: %s\n", i,
				SDL_GetAudioDeviceName ( i, 0 ) );
		}

		if ( count < 1 )
		{
			puts ( "No output devices found." );
		}

		return 1;		// Stop parsing and close program
	} );
	args.add ( "prefs",		m_prefs_filename );
	args.add ( "v",			m_verbose, 1 );

	if ( args.parse ( argc, argv ) || stop )
	{
		return 1;		// Quit program
	}

	if ( ! m_input_filename )
	{
		std::cerr << "Error - No input filename given!\n";
		return 1;		// Stop program
	}

	if ( m_verbose )
	{
		puts ( m_input_filename );
	}

	prefs_init ();

	if ( audio_init () )
	{
		return 1;
	}

	return 0;			// Continue program
}

void Core::prefs_init ()
{
	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );
	uprefs.add_int ( PREFS_WINDOW_W, mprefs.window_w, 320 );
	uprefs.add_int ( PREFS_WINDOW_H, mprefs.window_h, 200 );
	uprefs.add_int ( PREFS_WINDOW_MAXIMIZED, mprefs.window_maximized, 0 );

	uprefs.set_invisible ( PREFS_WINDOW_X );
	uprefs.set_invisible ( PREFS_WINDOW_Y );
	uprefs.set_invisible ( PREFS_WINDOW_W );
	uprefs.set_invisible ( PREFS_WINDOW_H );
	uprefs.set_invisible ( PREFS_WINDOW_MAXIMIZED );

	uprefs.load ( m_prefs_filename, BIN_NAME );
}

int Core::audio_init ()
{
	SDL_AudioSpec spec;

	SDL_zero ( spec );
	spec.freq = 48000;
	spec.format = AUDIO_S16SYS;
	spec.channels = 2;
	spec.samples = 4096;
	spec.callback = nullptr;

	if (	audio_play.open_device ( get_device (), spec )
		|| audio_file.open ( m_input_filename, audio_play.get_spec () )
		)
	{
		return 1;
	}

	audio_play.set_file ( &audio_file );

	return 0;
}

