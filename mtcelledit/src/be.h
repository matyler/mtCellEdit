/*
	Copyright (C) 2008-2016 Mark Tyler

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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include <mtkit.h>
#include <mtcelledit.h>
#include <mtcedui.h>



#ifdef U_NLS
	#include <libintl.h>
	#define _(text) gettext(text)
	#define __(text) gettext(text)
#else
	#define _(text) text
	#define __(text) text
#endif



#define GUI_INIFILE_FONT_PANGO_NAME	"main_font_pango_name"
#define GUI_INIFILE_FONT_SIZE		"main_font_size"
#define GUI_INIFILE_ROW_PAD		"main_row_pad"
#define GUI_INIFILE_LAST_DIR		"main_last_dir"
#define GUI_INIFILE_MAIN_WINDOW		"main_window"
#define GUI_INIFILE_PREFS_WINDOW	"prefs.window"
#define GUI_INIFILE_SHARED_WINDOW	"~/shared/window"

#define GUI_INIFILE_HELP_FILE		"help_file"
#define GUI_INIFILE_HELP_BROWSER	"help_browser"

#define GUI_INIFILE_RECENT_FILENAME_LEN	"main_recent_filename_len"
#define GUI_INIFILE_MAIN_FILE		"main_file"
#define GUI_INIFILE_MAIN_FPICK		"main_fpick"
#define GUI_INIFILE_MAIN_FPICK_PRE	"main_"

#define GUI_INIFILE_FILE_LOCK_LOAD	"file_lock_load"
#define GUI_INIFILE_FILE_LOCK_SAVE	"file_lock_save"

#define GUI_INIFILE_GRAPH_PANE_POS	"graph_pane_pos"
#define GUI_INIFILE_FIND_PANE_POS	"find_pane_pos"
#define GUI_INIFILE_FIND_ALL_SHEETS	"find_all_sheets"
#define GUI_INIFILE_FIND_WILDCARDS	"find_wildcards"
#define GUI_INIFILE_FIND_CASE_SENSITIVE "find_case_sensitive"
#define GUI_INIFILE_FIND_VALUE		"find_value"
#define GUI_INIFILE_FIND_TEXT		"find_text"

#define GUI_INIFILE_SHEET_PREFS_PERSIST	"sheet_prefs_persist"
#define GUI_INIFILE_GRAPH_SCALE		"graph_scale"
#define GUI_INIFILE_2DYEAR_START	"date.2digit_year_start"
#define GUI_INIFILE_CLIPBOARD_USE_SYSTEM "clipboard.use_system"


#define PREFS_WINDOW_X		GUI_INIFILE_MAIN_WINDOW"_x"
#define PREFS_WINDOW_Y		GUI_INIFILE_MAIN_WINDOW"_y"
#define PREFS_WINDOW_W		GUI_INIFILE_MAIN_WINDOW"_w"
#define PREFS_WINDOW_H		GUI_INIFILE_MAIN_WINDOW"_h"
#define RECENT_MENU_TOTAL	20
#define	CEDVIEW_FRZ_PANE_TOT	7
#define MTCELLEDIT_CLIP_CODE	123456
#define MTCELLEDIT_CLIP_NAME	"application/x-mtcelledit-clipboard"
#define MAX_SORT		10
#define COLOR_SWATCH_SIZE	24
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

enum
{
	CEDVIEW_AREA_TL,
	CEDVIEW_AREA_TR,		// Main drawing areas
	CEDVIEW_AREA_BL,
	CEDVIEW_AREA_BR,		// Scrollable area

	CEDVIEW_AREA_CORNER,		// Top left corner area - visual bell

	CEDVIEW_TITLE_C1,
	CEDVIEW_TITLE_C2,		// Row/column title header areas
	CEDVIEW_TITLE_R1,
	CEDVIEW_TITLE_R2,

	CEDVIEW_FRZ_COL,		// Frozen pane areas
	CEDVIEW_FRZ_ROW,
	CEDVIEW_FRZ_V_TOP,
	CEDVIEW_FRZ_V_BOTTOM,
	CEDVIEW_FRZ_H_LEFT,
	CEDVIEW_FRZ_H_RIGHT,
	CEDVIEW_FRZ_CORNER,

	CEDVIEW_AREA_TOTAL
};



void prefs_init (		// Setup memory, filename, etc.
	char	const *	filename
	);

void prefs_load ( void );	// Load prefs file from disk into memory

void prefs_save ( void );	// Save current prefs from memory to disk

void prefs_close ( void );	// Free all memory created in prefs_init ()

mtPrefs * prefs_file ( void );

void prefs_set_int (
	char	const	* key,
	int		value
	);

void prefs_set_string (
	char	const	* key,
	char	const	* value
	);

int prefs_get_int (
	char	const	* key
	);

char const * prefs_get_string (
	char	const	* key
	);

double prefs_get_double (
	char	const	* key
	);

void be_cline (
	int			argc,
	char	const * const *	argv,
	char	const **	prefs_file,	// From command line
	char	const **	input_file	// From command line
	);

int be_get_force_tsvcsv ( void );	// CED_FILE_FORCE_*

int be_snip_filename (			// Snip input string to fit recently
					// used list.  Length set by
					// CED_INIFILE_RECENT_FILENAME_LEN.
	int		num,		// 1..20
	char		* buf,		// Output buffer
	size_t		buflen		// Bytes available in buffer
	);
	// 0 = Buffer filled
	// 1 = Buffer not filled

int be_titlebar_text (
	CuiFile		* file,
	char		* buf,		// Put title bar text here
	size_t		buflen,
	int		changed
	);
	// 0 = file isn't a book
	// 1 = file is a book



// NOTE: The functions below are implemented by the toolkit code

void pref_change_font (
	mtPrefValue	* piv,
	int		callback_data,
	void		* callback_ptr
	);

void pref_change_row_pad (
	mtPrefValue	* piv,
	int		callback_data,
	void		* callback_ptr
	);

void pref_change_recent_filename_len (
	mtPrefValue	* piv,
	int		callback_data,
	void		* callback_ptr
	);

void pref_change_graph_scale (
	mtPrefValue	* piv,
	int		callback_data,
	void		* callback_ptr
	);

// NOTE: end



void be_remember_last_dir (
	char	const	* filename
	);

int be_register_project (
	CuiFile		* file
	);

char const * be_get_recent_filename (
	int		i		// 1..20
	);

int be_cedrender_set_font_width (
	CuiRender	* render
	);
	// 0 = font_width set

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
	int		error,
	char		* buf,
	size_t		buflen
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
	CuiFile		* file
	);

int be_export_sheet (
	CedSheet	* src,
	char	const	* filename,
	int		filetype
	);



// NOTE: The functions below are implemented by the toolkit code

void fe_commit_prefs_set (
	int		pref_id,
	int		pref_num,
	char	const	* pref_charp,
	int		change_cursor,
	void		* callback_ptr
	);

void fe_save_pref_window_prefs (
	mtPrefs		* prefs
	);

mtPrefs * fe_load_pref_window_prefs (
	mtPrefTable	const * table
	);

void fe_book_prefs_changed (
	mtPrefValue	* piv,
	int		callback_data,
	void		* callback_ptr
	);

// NOTE: end

int be_commit_prefs_set (
	CedSheet	* sheet,
	CuiBook		* cubook,
	int		pref_id,
	int		pref_num,
	char	const	* pref_charp
	);

int be_commit_prefs_set_check (
	int		res,
	CedSheet	* sheet,
	int		change_cursor,
	int		pref_id
	);

void be_cellpref_changed (
	mtPrefValue	* piv,
	int		callback_data,
	void		* callback_ptr
	);

mtPrefs * be_cellpref_init (
	CedSheet	* sheet,
	mtPrefCB	callback,
	void		* ptr
	);

int be_prepare_prefs_set (
	CedSheet	* sheet
	);

void be_cellpref_cleanup (
	CedSheet	* sheet
	);

void be_save_pref_window_prefs (
	mtPrefs		* prefs
	);

void be_load_pref_window_prefs (
	mtPrefs		* prefs
	);

mtPrefs * be_book_prefs_init (
	CedBook		* book
	);

void be_book_prefs_finish (
	mtPrefs		* prefs,
	CedBook		* book
	);

int be_color_swatch_get (
	int		i
	);

char const * be_graph_new (
	CedBook		* book
	);

char * be_graph_duplicate (
	CuiBook		* cubook
	);
	// Caller must free result after use

int be_graph_selection_clip (
	CedSheet	* sheet,
	char		* buf,
	size_t		buflen
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

