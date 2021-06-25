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

#ifndef MTREC_H_
#define MTREC_H_

#undef BIN_NAME
#define BIN_NAME "mtsorec"



#include <mtgin_sdl.h>



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API

class Core;
class MemPrefs;



class MemPrefs
{
public:
	MemPrefs () {}

	int	window_x		= 0;
	int	window_y		= 0;
	int	window_w		= 0;
	int	window_h		= 0;
	int	window_maximized	= 0;

private:
	MTKIT_RULE_OF_FIVE( MemPrefs )
};



class Core
{
public:
	Core();
	~Core();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	inline char const * get_output_filename () const
	{ return m_output_filename; }

	inline int get_gui () const
	{ return m_gui; }

	inline int get_verbose () const
	{ return m_verbose; }

	inline int get_dev_record () const
	{ return m_dev_record; }

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

	// Create SDL app last, and destroy it first
	mtGin::AudioFileWrite	audio_file;
	mtGin::AudioRecord	audio_record;
	mtGin::App		gin;

private:
	void prefs_init ();
	int audio_init ();

/// ----------------------------------------------------------------------------

	int			m_dev_record		= -1;
	int			m_gui			= 0;
	char		const *	m_output_filename	= nullptr;
	char		const *	m_prefs_filename	= nullptr;
	int			m_verbose		= 0;

	MTKIT_RULE_OF_FIVE( Core )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTREC_H_

