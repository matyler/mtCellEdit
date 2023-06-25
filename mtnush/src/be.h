/*
	Copyright (C) 2022-2023 Mark Tyler

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
#include <mtdatawell_math.h>



#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"

#define PREFS_CALC_FLOAT_BITS		"calc.float.bits"
#define PREFS_CALC_SNIP_SIZE		"calc.snip_size"



class Cline;
class MemPrefs;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class MemPrefs
{
public:
	int			window_x		= 0;
	int			window_y		= 0;
	int			window_w		= 0;
	int			window_h		= 0;
	int			window_maximized	= 0;

	int			calc_float_bits		= 10;
	int			calc_snip_size		= 1000;

	mtKit::UPrefUIEdit	ui_prefs;
};



class Cline
{
public:
	int parse_command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void		prefs_init ();

/// ----------------------------------------------------------------------------

	char		const *	m_prefs_filename	= nullptr;
};


