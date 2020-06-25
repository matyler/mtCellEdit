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

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <mtkit.h>
#include <mtdatawell.h>

#include "static.h"



#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"

#define PREFS_LAST_DIRECTORY		"main.last_directory"

#define PREFS_RECENT_DB			"main.recent.db"
#define PREFS_RECENT_DB_TOTAL		20

#define PREFS_RECENT_DIR		"main.recent.dir"
#define PREFS_RECENT_DIR_TOTAL		20

#define PREFS_HELP_FILE			"help.file"
#define PREFS_HELP_BROWSER		"help.browser"



class Backend;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class Backend
{
public:
	Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	void cline_add_filename ( char const * filename );

	int init_well ();	// Called once by GUI main window startup

	inline std::vector<char const *> const & get_cline_files () const
		{ return m_cline_files; }

	int open_database ( char const * path );
		// On success, path is added to recent_db

/// ----------------------------------------------------------------------------

	mtDW::Database		db;
	mtDW::Homoglyph		hg_index;
	mtDW::Utf8Font		font_index;
	mtKit::Exit		exit;
	mtKit::Prefs		prefs;
	mtKit::RecentFile	recent_db;
	mtKit::RecentFile	recent_dir;

private:
	void		prefs_init ();

/// ----------------------------------------------------------------------------

	char		const *	m_db_path;
	char		const *	m_prefs_filename;

	std::vector<char const *> m_cline_files;
};

