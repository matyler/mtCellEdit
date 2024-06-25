/*
	Copyright (C) 2024 Mark Tyler

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

#ifndef BE_H_
#define BE_H_



#include "mtwasp.h"



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API



#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"

#define PREFS_FILE_RECENT_MAXLEN	"file.recent.maxlen"
#define PREFS_FILE_RECENT_FILENAME	"file.recent.filename"

#define PREFS_AUDIO_OUTPUT_DEVICE	"audio.output.device"



enum
{
	RECENT_MENU_TOTAL		= 20,
	PREFS_RECENT_MAXLEN_DEFAULT	= 80,
	PREFS_RECENT_MAXLEN_MIN		= 20,
	PREFS_RECENT_MAXLEN_MAX		= 500
};



#ifdef __cplusplus
}

// C++ API



class CommandLine;
class MemPrefs;



class MemPrefs
{
public:
//	MemPrefs () {}

	int			window_x		= 0;
	int			window_y		= 0;
	int			window_w		= 0;
	int			window_h		= 0;
	int			window_maximized	= 0;

	int			file_recent_maxlen	= 0;

	int			audio_output_device	= -1;

	mtKit::UPrefUIEdit	ui_editor;

//private:
//	MTKIT_RULE_OF_FIVE( MemPrefs )
};



class CommandLine
{
public:
	CommandLine();
	~CommandLine();

	int parse ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	std::vector< char const * > const & get_cline_files () const
	{
		return m_cline_files;
	}

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::RecentFile	recent_files;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();

/// ----------------------------------------------------------------------------

	char	const	* m_prefs_filename	= nullptr;

	std::vector< char const * > m_cline_files;

	MTKIT_RULE_OF_FIVE( CommandLine )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// BE_H_

