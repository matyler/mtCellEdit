/*
	Copyright (C) 2008-2021 Mark Tyler

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
#include <mtcelledit.h>
#include <mtcedui.h>

#include "static.h"



class Backend;
class CuiCellPrefChange;



#define PREFS_LAST_DIR			"main_last_dir"

#define PREFS_WINDOW_X			"main_window_x"
#define PREFS_WINDOW_Y			"main_window_y"
#define PREFS_WINDOW_W			"main_window_w"
#define PREFS_WINDOW_H			"main_window_h"
#define PREFS_WINDOW_STATE		"main_window_state"
#define PREFS_WINDOW_RENDERER		"main_window_renderer"

#define PREFS_SHARED_WINDOW_X		"~/shared/window_x"
#define PREFS_SHARED_WINDOW_Y		"~/shared/window_y"
#define PREFS_SHARED_WINDOW_W		"~/shared/window_w"
#define PREFS_SHARED_WINDOW_H		"~/shared/window_h"

#define PREFS_HELP_FILE			"help_file"
#define PREFS_HELP_BROWSER		"help_browser"

#define PREFS_FILE_LOCK_LOAD		"file_lock_load"
#define PREFS_FILE_LOCK_SAVE		"file_lock_save"

#define PREFS_FONT_PANGO_NAME		"main_font_pango_name"
#define PREFS_FONT_SIZE			"main_font_size"
#define PREFS_ROW_PAD			"main_row_pad"

#define PREFS_RECENT_FILENAME_LEN	"main_recent_filename_len"
#define PREFS_YEAR_START_2D		"date.2digit_year_start"

#define PREFS_FIND_ALL_SHEETS		"find_all_sheets"
#define PREFS_FIND_WILDCARDS		"find_wildcards"
#define PREFS_FIND_CASE_SENSITIVE	"find_case_sensitive"
#define PREFS_FIND_VALUE		"find_value"
#define PREFS_FIND_TEXT			"find_text"

#define PREFS_CLIPBOARD_USE_SYSTEM	"clipboard.use_system"
#define PREFS_GRAPH_SCALE		"graph_scale"
#define PREFS_SHEET_PREFS_PERSIST	"sheet_prefs_persist"

#define PREFS_RECENT_FILE		"recent.file"



#define RECENT_MENU_TOTAL	20
#define MAX_SORT		10
#define COLOR_SWATCH_ROWS	5
#define COLOR_SWATCH_COLS	8
#define COLOR_SWATCH_TOTAL	40
#define FIND_MAX_MATCHES	10000



enum
{
	SORT_TABLE_ROWCOL,
	SORT_TABLE_DIRECTION,
	SORT_TABLE_CASE_SENSITIVE,

	SORT_TABLE_TOTAL
};

enum
{
	SORT_ROWS	= 0,
	SORT_COLUMNS	= 1
};



int be_titlebar_text (
	CuiFile		* file,
	char		* buf,		// Put title bar text here
	size_t		buflen,
	int		changed
	);
	// 0 = file isn't a book
	// 1 = file is a book



void be_cedrender_set_header_width (
	CuiRender	* render,
	int		max
	);

void be_sheet_ref (
	CedSheet	* sheet,
	char		* buf,		// Fill buffer with sheet cell ref.
	size_t		buflen
	);

void be_quicksum_label (
	CedSheet	* sheet,
	char		* buf,
	size_t		buflen,
	int		operation
	);

char const * be_get_error_update_text (
	int		error
	);

void be_update_file_to_book (
	CuiFile		* file
	);

int be_clip_clear_selection (
	CuiFile		* file,
	CedSheet	* sheet,
	int		mode
	);

CedSheet * be_clip_transform_start (
	CuiClip		* clip,
	int		mode
	);

int be_clip_transform_finish (
	CuiClip		* clip,
	CedSheet	* sheet,
	int		mode
	);

int be_clip_copy_values (
	CedSheet	* sheet
	);

int be_clip_copy_output (
	CedSheet	* sheet
	);

int be_selection_row_extent (
	CedSheet	* sheet,
	int		* row,
	int		* rowtot
	);

int be_selection_col_extent (
	CedSheet	* sheet,
	int		* col,
	int		* coltot
	);

int be_fix_years (
	CuiFile		* file,
	int		yr
	);

int be_export_sheet (
	CedSheet	* src,
	char	const	* filename,
	int		filetype
	);

int be_color_swatch_get (
	int i
	);

char const * be_graph_new (
	CedBook * book
	);

char * be_graph_duplicate (
	CuiBook * cubook
	);
	// Caller must free result after use

char * be_graph_selection_clip (
	CedSheet * sheet
	);

void be_find (
	CuiFile		* file,
	CedSheet	* sheet,
	char	const	* txt,
	int		wildc,
	int		casen,
	int		mval,
	int		allsheets,
	CedFuncScanArea	callback,
	void		* user_data
	);


class MemPrefs
{
public:
	std::string	last_dir;

	int		window_x		= 0;
	int		window_y		= 0;
	int		window_w		= 0;
	int		window_h		= 0;
	int		window_state		= 0;
	int		window_renderer		= 0;

	int		shared_window_x		= 0;
	int		shared_window_y		= 0;
	int		shared_window_w		= 0;
	int		shared_window_h		= 0;

	std::string	help_file;
	std::string	help_browser;

	int		file_lock_load		= 0;
	int		file_lock_save		= 0;

	std::string	font_pango_name;
	int		font_size		= 0;

	int		row_pad			= 0;
	int		recent_filename_len	= 0;
	int		year_start_2d		= 0;

	int		find_wildcards		= 0;
	int		find_case_sensitive	= 0;
	int		find_value		= 0;
	int		find_all_sheets		= 0;

	std::string	find_text;

	int		clipboard_use_system	= 0;

	int		page_width		= 0;
	int		page_height		= 0;
	int		page_margin_x		= 0;
	int		page_margin_y		= 0;

	int		page_header_left	= 0;
	int		page_header_centre	= 0;
	int		page_header_right	= 0;
	int		page_footer_left	= 0;
	int		page_footer_centre	= 0;
	int		page_footer_right	= 0;

	double		graph_scale		= 0.0;

	int		sheet_prefs_persist	= 0;

	mtKit::UPrefUIEdit	ui_editor;

	mtKit::RecentFile	recent_file;
};



class Backend
{
public:
	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program with 0

	char const * get_cline_filename () const;
	int get_force_tsvcsv () const;

	void remember_last_dir ( char const * filename );
	int register_project ( CuiFile const * file );

	void ui_shared_prefs_init ( mtKit::UserPrefs & prefs );
	void ui_shared_prefs_finish ( mtKit::UserPrefs & prefs );

/// ----------------------------------------------------------------------------

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();

/// ----------------------------------------------------------------------------

	int		force_tsvcsv		= 0;
	char	const *	cline_filename		= nullptr;
	char	const *	prefs_filename		= nullptr;
};

