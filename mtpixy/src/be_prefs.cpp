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

#include "be.h"



void Backend::prefs_init ()
{
	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );
	uprefs.add_int ( PREFS_WINDOW_W, mprefs.window_w, 800 );
	uprefs.add_int ( PREFS_WINDOW_H, mprefs.window_h, 600 );
	uprefs.add_int ( PREFS_WINDOW_MAXIMIZED, mprefs.window_maximized, 1 );
	uprefs.add_double ( PREFS_WINDOW_FILE_LIST_SPLIT,
		mprefs.window_file_list_split, 0.5, 0.0, 1.0 );
	uprefs.add_string ( PREFS_WINDOW_STATE, mprefs.window_state, "" );

	uprefs.set_invisible ( PREFS_WINDOW_X );
	uprefs.set_invisible ( PREFS_WINDOW_Y );
	uprefs.set_invisible ( PREFS_WINDOW_W );
	uprefs.set_invisible ( PREFS_WINDOW_H );
	uprefs.set_invisible ( PREFS_WINDOW_MAXIMIZED );
	uprefs.set_invisible ( PREFS_WINDOW_FILE_LIST_SPLIT );
	uprefs.set_invisible ( PREFS_WINDOW_STATE );

	uprefs.add_int ( PREFS_CURSOR_NUDGE_PIXELS, mprefs.cursor_nudge_pixels,
		16, 1, 4096 );
	uprefs.add_double ( PREFS_CURSOR_BRUSH_OPACITY,
		mprefs.cursor_brush_opacity, 0.4, 0.0, 1.0 );

	uprefs.add_rgb ( PREFS_CANVAS_EASEL_RGB, mprefs.canvas_easel_rgb,
		11842740 );
	uprefs.add_int ( PREFS_CANVAS_ZOOM_GRID_GREY,
		mprefs.canvas_zoom_grid_grey, 50, 0, 255 );
	uprefs.add_bool ( PREFS_CANVAS_ZOOM_GRID_SHOW,
		mprefs.canvas_zoom_grid_show, 1 );
	uprefs.set_invisible ( PREFS_CANVAS_ZOOM_GRID_SHOW );

	uprefs.add_int ( PREFS_FILE_RECENT_MAXLEN, mprefs.file_recent_maxlen,
		PREFS_RECENT_MAXLEN_DEFAULT, PREFS_RECENT_MAXLEN_MIN,
		PREFS_RECENT_MAXLEN_MAX );
	uprefs.add_int ( PREFS_FILE_NEW_WIDTH, mprefs.file_new_width, 256 );
	uprefs.add_int ( PREFS_FILE_NEW_HEIGHT, mprefs.file_new_height, 192 );
	uprefs.add_int ( PREFS_FILE_NEW_TYPE, mprefs.file_new_type, 2 );
	uprefs.add_int ( PREFS_FILE_COMPRESSION_JPEG,
		mprefs.file_compression_jpeg, 85, 1, 100 );
	uprefs.add_int ( PREFS_FILE_COMPRESSION_PNG,
		mprefs.file_compression_png, 6, 0, 9 );
	uprefs.add_filename ( PREFS_FILE_NEW_PALETTE_FILE,
		mprefs.file_new_palette_file, "" );
	uprefs.add_int ( PREFS_FILE_NEW_PALETTE_NUM,
		mprefs.file_new_palette_num, 3, 2, 6 );
	uprefs.add_option ( PREFS_FILE_NEW_PALETTE_TYPE,
		mprefs.file_new_palette_type, 1, { "Uniform", "Balanced",
		"File" } );

	uprefs.set_invisible ( PREFS_FILE_NEW_WIDTH );
	uprefs.set_invisible ( PREFS_FILE_NEW_HEIGHT );
	uprefs.set_invisible ( PREFS_FILE_NEW_TYPE );
	uprefs.set_invisible ( PREFS_FILE_NEW_PALETTE_FILE );
	uprefs.set_invisible ( PREFS_FILE_NEW_PALETTE_NUM );
	uprefs.set_invisible ( PREFS_FILE_NEW_PALETTE_TYPE );

	uprefs.add_string ( PREFS_HELP_FILE, mprefs.help_file,
		DATA_INSTALL "/doc/" BIN_NAME "/en_GB/chap_00.html" );
	uprefs.add_string ( PREFS_HELP_BROWSER, mprefs.help_browser, "" );

	uprefs.add_double ( PREFS_PALETTE_NUMBER_OPACITY,
		mprefs.palette_number_opacity, 0.4, 0.0, 1.0 );

	uprefs.add_int ( PREFS_UI_SCALE, mprefs.ui_scale, 0, 0, 10 );
	uprefs.add_int ( PREFS_UI_SCALE_PALETTE, mprefs.ui_scale_palette, 0, 0,
		10 );

	uprefs.add_string ( PREFS_TEXT_FONT_NAME, mprefs.text_font_name,"Sans");
	uprefs.add_string ( PREFS_TEXT_FONT_STYLE, mprefs.text_font_style, "");
	uprefs.add_int ( PREFS_TEXT_FONT_SIZE, mprefs.text_font_size, 12 );
	uprefs.add_int ( PREFS_TEXT_FONT_BOLD, mprefs.text_font_bold, 0 );
	uprefs.add_int ( PREFS_TEXT_FONT_ITALIC, mprefs.text_font_italic, 0 );
	uprefs.add_int ( PREFS_TEXT_FONT_UNDERLINE, mprefs.text_font_underline,
		0 );
	uprefs.add_int ( PREFS_TEXT_FONT_STRIKETHROUGH,
		mprefs.text_font_strikethrough, 0 );
	uprefs.add_string ( PREFS_TEXT_ENTRY, mprefs.text_entry,
		"Enter Text Here" );

	uprefs.set_invisible ( PREFS_TEXT_FONT_NAME );
	uprefs.set_invisible ( PREFS_TEXT_FONT_STYLE );
	uprefs.set_invisible ( PREFS_TEXT_FONT_SIZE );
	uprefs.set_invisible ( PREFS_TEXT_FONT_BOLD );
	uprefs.set_invisible ( PREFS_TEXT_FONT_ITALIC );
	uprefs.set_invisible ( PREFS_TEXT_FONT_UNDERLINE );
	uprefs.set_invisible ( PREFS_TEXT_FONT_STRIKETHROUGH );
	uprefs.set_invisible ( PREFS_TEXT_ENTRY );

	uprefs.add_int ( PREFS_UNDO_MB_MAX, mprefs.undo_mb_max, 1024, 1, 10000);
	uprefs.add_int ( PREFS_UNDO_STEPS_MAX, mprefs.undo_steps_max, 100, 1,
		1000 );

	uprefs.add_ui_defaults ( mprefs.ui_editor );

	mprefs.recent_image.init ( uprefs, PREFS_FILE_RECENT_IMAGE,
		RECENT_MENU_TOTAL );

	uprefs.load ( m_prefs_filename, BIN_NAME );

	file.set_undo_mb_max ( mprefs.undo_mb_max );
	file.set_undo_steps_max ( mprefs.undo_steps_max );
}

std::string MemPrefs::get_font_name_full () const
{
	std::string font_name ( this->text_font_name );

	if ( this->text_font_style.size() > 0 )
	{
		font_name += " ";
		font_name += this->text_font_style;
	}

	return font_name;
}

