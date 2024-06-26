/*
	Copyright (C) 2016-2024 Mark Tyler

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
class MemPrefs;



#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"
#define PREFS_WINDOW_STATE		"main.window.state"
#define PREFS_WINDOW_FILE_LIST_SPLIT	"main.window.file_list.split"

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
#define PREFS_FILE_NEW_PALETTE_FILE	"file.new.palette.file"
#define PREFS_FILE_NEW_PALETTE_NUM	"file.new.palette.num"
#define PREFS_FILE_NEW_PALETTE_TYPE	"file.new.palette.type"

#define PREFS_HELP_FILE			"help.file"
#define PREFS_HELP_BROWSER		"help.browser"

#define PREFS_PALETTE_NUMBER_OPACITY	"palette.number.opacity"

#define PREFS_TEXT_FONT_NAME		"text.font.name"
#define PREFS_TEXT_FONT_STYLE		"text.font.style"
#define PREFS_TEXT_FONT_SIZE		"text.font.size"
#define PREFS_TEXT_FONT_BOLD		"text.font.bold"
#define PREFS_TEXT_FONT_ITALIC		"text.font.italic"
#define PREFS_TEXT_FONT_UNDERLINE	"text.font.underline"
#define PREFS_TEXT_FONT_STRIKETHROUGH	"text.font.strikethrough"
#define PREFS_TEXT_ENTRY		"text.entry"

#define PREFS_UNDO_MB_MAX		"undo.mb.max"
#define PREFS_UNDO_STEPS_MAX		"undo.steps.max"


enum
{
	RECENT_MENU_TOTAL		= 20,
	PREFS_RECENT_MAXLEN_DEFAULT	= 80,
	PREFS_RECENT_MAXLEN_MIN		= 20,
	PREFS_RECENT_MAXLEN_MAX		= 500
};


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



class MemPrefs
{
public:
	std::string get_font_name_full () const;

// -----------------------------------------------------------------------------

	int			window_x		= 0;
	int			window_y		= 0;
	int			window_w		= 0;
	int			window_h		= 0;
	int			window_maximized	= 0;
	double			window_file_list_split	= 0.5;
	std::string		window_state;

	int			cursor_nudge_pixels	= 0;
	double			cursor_brush_opacity	= 0;

	int			canvas_easel_rgb	= 0;
	int			canvas_zoom_grid_grey	= 0;
	int			canvas_zoom_grid_show	= 0;

	int			file_recent_maxlen	= 0;
	int			file_new_width		= 0;
	int			file_new_height		= 0;
	int			file_new_type		= 0;
	int			file_compression_jpeg	= 0;
	int			file_compression_png	= 0;
	std::string		file_new_palette_file;
	int			file_new_palette_num	= 0;
	int			file_new_palette_type	= 0;

	std::string		help_file;
	std::string		help_browser;

	double			palette_number_opacity	= 0;

	int			ui_scale		= 0;
	int			ui_scale_palette	= 0;

	std::string		text_font_name;
	std::string		text_font_style;
	int			text_font_size		= 0;
	int			text_font_bold		= 0;
	int			text_font_italic	= 0;
	int			text_font_underline	= 0;
	int			text_font_strikethrough	= 0;

	std::string		text_entry;

	int			undo_mb_max		= 0;
	int			undo_steps_max		= 0;

	mtKit::UPrefUIEdit	ui_editor;
	mtKit::RecentFile	recent_image;
};



class Backend
{
public:
	Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program with 0

	char const * get_cline_filename () const;
	std::vector< char const * > const & get_cline_files () const
	{ return m_cline_files; }

	int get_cline_screenshot () const;
	int get_ui_scale () const;
	int get_ui_scale_palette () const;
	std::string get_titlebar_text () const;

	void detect_ui_scale ( int menu_height );
	void calc_ui_scale ();

	std::string get_last_directory () const;

/// ----------------------------------------------------------------------------

	mtPixyUI::Clipboard	clipboard;
	mtPixyUI::File		file;

	mtPalette		palette;	// For Palette->Store/Restore

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();
	int get_ui_scale_generic ( int num ) const;

/// ----------------------------------------------------------------------------

	int		m_ui_scale		= 0;
	int		m_ui_scale_detected	= 1;
	int		m_screenshot		= 0;

	char	const *	m_prefs_filename	= nullptr;

	std::vector< char const * > m_cline_files;
};

