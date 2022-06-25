/*
	Copyright (C) 2008-2022 Mark Tyler

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
	uprefs.add_directory ( PREFS_LAST_DIR, mprefs.last_dir, "" );
	uprefs.set_invisible ( PREFS_LAST_DIR );

	uprefs.add_int ( PREFS_WINDOW_X, mprefs.window_x, 50 );
	uprefs.add_int ( PREFS_WINDOW_Y, mprefs.window_y, 50 );
	uprefs.add_int ( PREFS_WINDOW_W, mprefs.window_w, 400 );
	uprefs.add_int ( PREFS_WINDOW_H, mprefs.window_h, 400 );
	uprefs.add_int ( PREFS_WINDOW_STATE, mprefs.window_state, 0 );
	uprefs.add_option ( PREFS_WINDOW_RENDERER, mprefs.window_renderer, 0,
		{ "Cairo", "Pango" } );
		// Order must match CuiRender::RENDERER_*

	uprefs.set_invisible ( PREFS_WINDOW_X );
	uprefs.set_invisible ( PREFS_WINDOW_Y );
	uprefs.set_invisible ( PREFS_WINDOW_W );
	uprefs.set_invisible ( PREFS_WINDOW_H );
	uprefs.set_invisible ( PREFS_WINDOW_STATE );

	uprefs.add_int ( PREFS_SHARED_WINDOW_X, mprefs.shared_window_x, 50 );
	uprefs.add_int ( PREFS_SHARED_WINDOW_Y, mprefs.shared_window_y, 50 );
	uprefs.add_int ( PREFS_SHARED_WINDOW_W, mprefs.shared_window_w, 400 );
	uprefs.add_int ( PREFS_SHARED_WINDOW_H, mprefs.shared_window_h, 400 );

	uprefs.set_invisible ( PREFS_SHARED_WINDOW_X );
	uprefs.set_invisible ( PREFS_SHARED_WINDOW_Y );
	uprefs.set_invisible ( PREFS_SHARED_WINDOW_W );
	uprefs.set_invisible ( PREFS_SHARED_WINDOW_H );

	uprefs.add_string ( PREFS_HELP_FILE, mprefs.help_file,
		DATA_INSTALL "/doc/" BIN_NAME "/en_GB/chap_00.html" );
	uprefs.add_string ( PREFS_HELP_BROWSER, mprefs.help_browser, "" );

	uprefs.add_option ( PREFS_FILE_LOCK_LOAD, mprefs.file_lock_load, 0,
		{ "RW", "RWL", "RO" } );
	uprefs.add_option ( PREFS_FILE_LOCK_SAVE, mprefs.file_lock_save, 0,
		{ "RW", "RWL", "RO" } );

	uprefs.add_string ( PREFS_FONT_PANGO_NAME, mprefs.font_pango_name,
		"Sans" );
	uprefs.add_int ( PREFS_FONT_SIZE, mprefs.font_size, 16, 1, 128 );

	uprefs.add_int ( PREFS_ROW_PAD,	mprefs.row_pad, 2, 0, 4 );
	uprefs.add_int ( PREFS_RECENT_FILENAME_LEN, mprefs.recent_filename_len,
		80, 50, 500 );

	uprefs.add_int ( PREFS_YEAR_START_2D, mprefs.year_start_2d, -1, -1,
		5879510 );
	uprefs.set_description ( PREFS_YEAR_START_2D,  "Resets two digit years "
		"to between this and 99 years later.  -1 = 50 years ago to 49 "
		"years time." );

	uprefs.add_bool ( PREFS_FIND_WILDCARDS,	mprefs.find_wildcards, 0 );
	uprefs.add_bool ( PREFS_FIND_CASE_SENSITIVE, mprefs.find_case_sensitive,
		0 );
	uprefs.add_bool ( PREFS_FIND_VALUE, mprefs.find_value, 0 );
	uprefs.add_bool ( PREFS_FIND_ALL_SHEETS, mprefs.find_all_sheets, 0 );
	uprefs.add_string ( PREFS_FIND_TEXT, mprefs.find_text, "" );

	uprefs.set_invisible ( PREFS_FIND_WILDCARDS );
	uprefs.set_invisible ( PREFS_FIND_CASE_SENSITIVE );
	uprefs.set_invisible ( PREFS_FIND_VALUE );
	uprefs.set_invisible ( PREFS_FIND_ALL_SHEETS );
	uprefs.set_invisible ( PREFS_FIND_TEXT );

	uprefs.add_bool ( PREFS_CLIPBOARD_USE_SYSTEM,
		mprefs.clipboard_use_system, 1 );
	uprefs.set_invisible ( PREFS_CLIPBOARD_USE_SYSTEM );
	uprefs.set_description ( PREFS_CLIPBOARD_USE_SYSTEM,
		"Use system clipboard when copying and pasting" );

	uprefs.add_int ( CUI_INIFILE_PAGE_WIDTH, mprefs.page_width, 297 );
	uprefs.add_int ( CUI_INIFILE_PAGE_HEIGHT, mprefs.page_height, 210 );
	uprefs.add_int ( CUI_INIFILE_PAGE_MARGIN_X, mprefs.page_margin_x, 10 );
	uprefs.add_int ( CUI_INIFILE_PAGE_MARGIN_Y, mprefs.page_margin_y, 10 );

	// NOTE: these options come from CUI_HEADER_OPTIONS

#define PAGE_OPTION_LIST { "None", "Filename (long)", \
	"Filename (short)", "Sheet Name", "Page #", "Date", "Date and Time" }

	uprefs.add_option ( CUI_INIFILE_PAGE_HEADER_LEFT,
		mprefs.page_header_left, 2, PAGE_OPTION_LIST );

	uprefs.add_option ( CUI_INIFILE_PAGE_HEADER_CENTRE,
		mprefs.page_header_centre, 0, PAGE_OPTION_LIST );

	uprefs.add_option ( CUI_INIFILE_PAGE_HEADER_RIGHT,
		mprefs.page_header_right, 3, PAGE_OPTION_LIST );

	uprefs.add_option ( CUI_INIFILE_PAGE_FOOTER_LEFT,
		mprefs.page_footer_left, 6, PAGE_OPTION_LIST );

	uprefs.add_option ( CUI_INIFILE_PAGE_FOOTER_CENTRE,
		mprefs.page_footer_centre, 0, PAGE_OPTION_LIST );

	uprefs.add_option ( CUI_INIFILE_PAGE_FOOTER_RIGHT,
		mprefs.page_footer_right, 4, PAGE_OPTION_LIST );

	uprefs.add_double ( PREFS_GRAPH_SCALE, mprefs.graph_scale,1, 0.1, 1000);

	uprefs.add_bool ( PREFS_SHEET_PREFS_PERSIST, mprefs.sheet_prefs_persist,
		0 );
	uprefs.set_description ( PREFS_SHEET_PREFS_PERSIST,
		"Keep sheet preferences (cursor area, frozen panes) when "
		"loading a new text file after a previous text file" );

	uprefs.add_ui_defaults ( mprefs.ui_editor );
	mprefs.recent_file.init ( uprefs, PREFS_RECENT_FILE, 20 );

	uprefs.load ( prefs_filename, BIN_NAME );
}

void Backend::ui_shared_prefs_init ( mtKit::UserPrefs & prefs )
{
	prefs.set ( MTKIT_PREFS_WINDOW_X, mprefs.shared_window_x );
	prefs.set ( MTKIT_PREFS_WINDOW_Y, mprefs.shared_window_y );
	prefs.set ( MTKIT_PREFS_WINDOW_W, mprefs.shared_window_w );
	prefs.set ( MTKIT_PREFS_WINDOW_H, mprefs.shared_window_h );
}

void Backend::ui_shared_prefs_finish ( mtKit::UserPrefs & prefs )
{
	mprefs.shared_window_x = prefs.get_int ( MTKIT_PREFS_WINDOW_X );
	mprefs.shared_window_y = prefs.get_int ( MTKIT_PREFS_WINDOW_Y );
	mprefs.shared_window_w = prefs.get_int ( MTKIT_PREFS_WINDOW_W );
	mprefs.shared_window_h = prefs.get_int ( MTKIT_PREFS_WINDOW_H );
}

int be_color_swatch_get ( int const i )
{
	static int const swatch_color_table [ COLOR_SWATCH_TOTAL + 1 ] = {
	pixy_rgb_2_int (   0,   0,   0 ),
	pixy_rgb_2_int (  90,   0,   0 ),
	pixy_rgb_2_int ( 100,  72,   0 ),
	pixy_rgb_2_int ( 100, 100,   0 ),
	pixy_rgb_2_int (   0,  90,   0 ),
	pixy_rgb_2_int (   0,  90,  90 ),
	pixy_rgb_2_int (   0,   0, 100 ),
	pixy_rgb_2_int (  90,   0,  90 ),

	pixy_rgb_2_int ( 100, 100, 100 ),
	pixy_rgb_2_int ( 170,   0,   0 ),
	pixy_rgb_2_int ( 180, 106,   0 ),
	pixy_rgb_2_int ( 170, 170,   0 ),
	pixy_rgb_2_int (   0, 160,   0 ),
	pixy_rgb_2_int (   0, 160, 160 ),
	pixy_rgb_2_int (   0, 100, 180 ),
	pixy_rgb_2_int ( 150,   0, 150 ),

	pixy_rgb_2_int ( 180, 180, 180 ),
	pixy_rgb_2_int ( 230, 140, 140 ),
	pixy_rgb_2_int ( 230, 150,   0 ),
	pixy_rgb_2_int ( 220, 220, 100 ),
	pixy_rgb_2_int ( 120, 220, 120 ),
	pixy_rgb_2_int ( 100, 230, 230 ),
	pixy_rgb_2_int ( 100, 200, 255 ),
	pixy_rgb_2_int ( 230, 140, 230 ),

	pixy_rgb_2_int ( 220, 220, 220 ),
	pixy_rgb_2_int ( 255, 200, 200 ),
	pixy_rgb_2_int ( 255, 216, 160 ),
	pixy_rgb_2_int ( 255, 255, 160 ),
	pixy_rgb_2_int ( 180, 255, 180 ),
	pixy_rgb_2_int ( 180, 255, 255 ),
	pixy_rgb_2_int ( 180, 230, 255 ),
	pixy_rgb_2_int ( 255, 200, 255 ),

	pixy_rgb_2_int ( 255, 255, 255 ),
	pixy_rgb_2_int ( 255,   0,   0 ),
	pixy_rgb_2_int ( 255, 200,   0 ),
	pixy_rgb_2_int ( 255, 255,   0 ),
	pixy_rgb_2_int (   0, 255,   0 ),
	pixy_rgb_2_int (   0, 255, 255 ),
	pixy_rgb_2_int (   0,   0, 255 ),
	pixy_rgb_2_int ( 255,   0, 255 )
	};


	if ( i >= COLOR_SWATCH_TOTAL || i < 0 )
	{
		return 0;
	}

	return swatch_color_table [ i ];
}

