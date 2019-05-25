/*
	Copyright (C) 2016-2018 Mark Tyler

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
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include <mtkit.h>
#include <mtpixy.h>

#include "static.h"



class Backend;



#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"
#define PREFS_WINDOW_STATE		"main.window.state"

#define PREFS_CANVAS_EASEL_RGB		"canvas.easel.rgb"
#define PREFS_CANVAS_ZOOM_GRID_GREY	"canvas.zoom.grid.grey"
#define PREFS_CANVAS_ZOOM_GRID_SHOW	"canvas.zoom.grid.show"

#define PREFS_CURSOR_NUDGE_PIXELS	"cursor.nudge.pixels"
#define PREFS_CURSOR_BRUSH_OPACITY	"cursor.brush.opacity"

#define PREFS_FILE_COMPRESSION_JPEG	"file.compression.jpeg"
#define PREFS_FILE_COMPRESSION_PNG	"file.compression.png"
#define PREFS_FILE_RECENT_MAXLEN	"file.recent.maxlen"
#define PREFS_FILE_RECENT_IMAGE		"file.recent.image"
#define PREFS_FILE_NEW_WIDTH		"file.new.width"
#define PREFS_FILE_NEW_HEIGHT		"file.new.height"
#define PREFS_FILE_NEW_TYPE		"file.new.type"
#define PREFS_FILE_NEW_PALETTE_NUM	"file.new.palette.num"
#define PREFS_FILE_NEW_PALETTE_TYPE	"file.new.palette.type"

#define PREFS_HELP_FILE			"help.file"
#define PREFS_HELP_BROWSER		"help.browser"

#define PREFS_PALETTE_NUMBER_OPACITY	"palette.number.opacity"

#define PREFS_TEXT_FONT_NAME		"text.font.name"
#define PREFS_TEXT_FONT_SIZE		"text.font.size"
#define PREFS_TEXT_FONT_BOLD		"text.font.bold"
#define PREFS_TEXT_FONT_ITALIC		"text.font.italic"
#define PREFS_TEXT_FONT_UNDERLINE	"text.font.underline"
#define PREFS_TEXT_FONT_STRIKETHROUGH	"text.font.strikethrough"
#define PREFS_TEXT_ENTRY		"text.entry"

#define PREFS_UNDO_MB_MAX		"undo.mb.max"
#define PREFS_UNDO_STEPS_MAX		"undo.steps.max"



/*	------------------
	Scalable UI Design
	------------------

Several UI elements have a scale factor to enlarge them on larger monitors or
higher resolutions.  By default the program detects the height of the menu bar
and uses a scale factor 1-10 based on this value.

This default can be overridden by the user in the preferences as below, either
as a scale factor or in the case of the icons by using a different icon set.

Text size is inherited from the users desktop environment and cannot be changed
from within this program.

*/

#define PREFS_UI_SCALE			"ui.scale"
#define PREFS_UI_SCALE_PALETTE		"ui.scale.palette"



class Backend
{
public:
	Backend ();
	~Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program with 0

	char const * get_cline_filename () const;
	int get_cline_screenshot () const;
	int get_ui_scale () const;
	int get_ui_scale_palette ();
	void get_titlebar_text ( char * buf, size_t buflen );

	void detect_ui_scale ( int menu_height );
	void calc_ui_scale ();

	char * get_last_directory ();	// Caller must free the result after use

/// ----------------------------------------------------------------------------

	mtPixyUI::Clipboard	clipboard;
	mtPixyUI::File		file;

	mtPixy::Palette		palette;	// For Palette->Store/Restore

	mtKit::Prefs		prefs;
	mtKit::RecentFile	recent_image;

private:
	void prefs_init ();
	int get_ui_scale_generic ( int num ) const;

/// ----------------------------------------------------------------------------

	int		m_ui_scale;
	int		m_ui_scale_detected;
	int		m_screenshot;

	char	const *	m_cline_filename;
	char	const *	m_prefs_filename;
};

