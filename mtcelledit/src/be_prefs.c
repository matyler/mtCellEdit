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

#include "be.h"



static mtPrefs		* prefs_mem;
static char		prefs_filename [ 2048 ];



void prefs_init (
	char	const * const	filename
	)
{
	mtPrefTable	const prefs_table[] = {

{ GUI_INIFILE_LAST_DIR,	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },

{ GUI_INIFILE_MAIN_WINDOW"_state", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_WINDOW"_x", MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_WINDOW"_y", MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_WINDOW"_w", MTKIT_PREF_TYPE_INT, "400", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_WINDOW"_h", MTKIT_PREF_TYPE_INT, "400", NULL, NULL, 0, NULL, NULL },

{ GUI_INIFILE_SHARED_WINDOW"_x", MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_SHARED_WINDOW"_y", MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_SHARED_WINDOW"_w", MTKIT_PREF_TYPE_INT, "400", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_SHARED_WINDOW"_h", MTKIT_PREF_TYPE_INT, "400", NULL, NULL, 0, NULL, NULL },

{ GUI_INIFILE_HELP_FILE, MTKIT_PREF_TYPE_FILE, "/usr/share/doc/" BIN_NAME "/en_GB/chap_00.html", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_HELP_BROWSER, MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },

{ GUI_INIFILE_MAIN_FILE"1", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"2", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"3", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"4", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"5", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"6", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"7", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"8", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"9", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"10",MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"11", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"12", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"13", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"14", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"15", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"16", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"17", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"18", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"19", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_MAIN_FILE"20", MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },

{ GUI_INIFILE_FILE_LOCK_LOAD, MTKIT_PREF_TYPE_OPTION, "0", NULL, NULL, 0, "RW	RWL	RO", NULL },
{ GUI_INIFILE_FILE_LOCK_SAVE, MTKIT_PREF_TYPE_OPTION, "0", NULL, NULL, 0, "RW	RWL	RO", NULL },

{ GUI_INIFILE_FONT_PANGO_NAME, MTKIT_PREF_TYPE_STR, "Sans", NULL, pref_change_font, 0, NULL, NULL },
{ GUI_INIFILE_FONT_SIZE,	MTKIT_PREF_TYPE_INT, "16", NULL, pref_change_font, 0, "1	128", NULL },

{ GUI_INIFILE_ROW_PAD,	MTKIT_PREF_TYPE_INT, "1", NULL, pref_change_row_pad, 0, "0	4", NULL },

{ GUI_INIFILE_RECENT_FILENAME_LEN,	MTKIT_PREF_TYPE_INT, "80", NULL, pref_change_recent_filename_len, 0, "50	500", NULL },

{ GUI_INIFILE_2DYEAR_START,	MTKIT_PREF_TYPE_INT, "-1", "Resets two digit years to between this and 99 years later.  -1 = 50 years ago to 49 years time.", NULL, 0, "-1	5879510", NULL },

{ GUI_INIFILE_GRAPH_PANE_POS,	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_FIND_PANE_POS,	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_FIND_WILDCARDS,	MTKIT_PREF_TYPE_BOOL, "0", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_FIND_CASE_SENSITIVE, MTKIT_PREF_TYPE_BOOL, "0", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_FIND_VALUE,	MTKIT_PREF_TYPE_BOOL, "0", NULL, NULL, 0, NULL, NULL },
{ GUI_INIFILE_FIND_ALL_SHEETS,	MTKIT_PREF_TYPE_BOOL, "0", NULL, NULL, 0, NULL, NULL },

{ GUI_INIFILE_FIND_TEXT,	MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },

{ GUI_INIFILE_CLIPBOARD_USE_SYSTEM, MTKIT_PREF_TYPE_BOOL, "1", "Use system clipboard when copying and pasting", NULL, 0, NULL, NULL },

{ CUI_INIFILE_PAGE_WIDTH,	MTKIT_PREF_TYPE_INT, "297", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_HEIGHT,	MTKIT_PREF_TYPE_INT, "210", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_MARGIN_X,	MTKIT_PREF_TYPE_INT, "10", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_MARGIN_Y,	MTKIT_PREF_TYPE_INT, "10", NULL, NULL, 0, NULL, NULL },

{ CUI_INIFILE_PAGE_HEADER_LEFT, MTKIT_PREF_TYPE_OPTION, "2", NULL, NULL, 0, CUI_HEADER_OPTIONS, NULL },
{ CUI_INIFILE_PAGE_HEADER_CENTRE, MTKIT_PREF_TYPE_OPTION, "0", NULL, NULL, 0, CUI_HEADER_OPTIONS, NULL },
{ CUI_INIFILE_PAGE_HEADER_RIGHT, MTKIT_PREF_TYPE_OPTION, "3", NULL, NULL, 0, CUI_HEADER_OPTIONS, NULL },
{ CUI_INIFILE_PAGE_FOOTER_LEFT, MTKIT_PREF_TYPE_OPTION, "6", NULL, NULL, 0, CUI_HEADER_OPTIONS, NULL },
{ CUI_INIFILE_PAGE_FOOTER_CENTRE, MTKIT_PREF_TYPE_OPTION, "0", NULL, NULL, 0, CUI_HEADER_OPTIONS, NULL },
{ CUI_INIFILE_PAGE_FOOTER_RIGHT, MTKIT_PREF_TYPE_OPTION, "4", NULL, NULL, 0, CUI_HEADER_OPTIONS, NULL },

{ GUI_INIFILE_GRAPH_SCALE,	MTKIT_PREF_TYPE_DOUBLE, "1", NULL, pref_change_graph_scale, 0, "0.1	1000", NULL },

{ GUI_INIFILE_SHEET_PREFS_PERSIST, MTKIT_PREF_TYPE_BOOL, "0", "Keep sheet preferences (cursor area, frozen panes) when loading a new text file after a previous text file", NULL, 0, NULL, NULL },

	{NULL}
	};


	prefs_mem = mtkit_prefs_new ( prefs_table );

	if ( filename )
	{
		mtkit_strnncpy ( prefs_filename, filename,
			sizeof ( prefs_filename ) );
	}
	else
	{
		snprintf ( prefs_filename, sizeof ( prefs_filename ),
			"%s/.config", mtkit_file_home () );

		mkdir ( prefs_filename, S_IRWXU | S_IRWXG | S_IRWXO );

		mtkit_strnncat ( prefs_filename, "/" BIN_NAME,
			sizeof ( prefs_filename ) );

		mkdir ( prefs_filename, S_IRWXU | S_IRWXG | S_IRWXO );

		mtkit_strnncat ( prefs_filename, "/prefs.txt",
			sizeof ( prefs_filename ) );
	}
}

void prefs_load ( void )
{
	mtkit_prefs_load ( prefs_mem, prefs_filename );
}

void prefs_save ( void )
{
	mtkit_prefs_save ( prefs_mem, prefs_filename );
}

void prefs_close ( void )
{
	mtkit_prefs_destroy ( prefs_mem );
	prefs_mem = NULL;
}

mtPrefs * prefs_file ( void )
{
	return prefs_mem;
}

void prefs_set_int (
	char	const	* const	key,
	int		const	value
	)
{
	mtkit_prefs_set_int ( prefs_mem, key, value );
}

void prefs_set_string (
	char	const	* const	key,
	char	const	* const	value
	)
{
	mtkit_prefs_set_str ( prefs_mem, key, value );
}

int prefs_get_int (
	char	const	* const	key
	)
{
	int		res = 0;


	mtkit_prefs_get_int ( prefs_mem, key, &res );

	return res;
}

char const * prefs_get_string (
	char	const	* const	key
	)
{
	static char	const *	nothing = "";
	char		const *	res;


	mtkit_prefs_get_str ( prefs_mem, key, &res );

	if ( ! res )
	{
		return nothing;
	}

	return res;
}

double prefs_get_double (
	char	const	* const	key
	)
{
	double		res = 0;


	mtkit_prefs_get_double ( prefs_mem, key, &res );

	return res;
}

static void be_book_prefs_transfer (
	mtPrefs		* const	prefs,
	CedBook		* const	book,
	int		const	write_mode
	)
{
	mtBulkInt	const	table_i[] = {
			{ CED_FILE_PREFS_BOOK_DISABLE_LOCKS, &book->prefs.disable_locks },
			{ CED_FILE_PREFS_BOOK_AUTO_RECALC, &book->prefs.auto_recalc },
			{ NULL }
			};
	mtBulkStr	const	table_s[] = {
			{ CED_FILE_PREFS_BOOK_AUTHOR, &book->prefs.author },
			{ CED_FILE_PREFS_BOOK_COMMENT, &book->prefs.comment },
			{ NULL }
			};


	if ( write_mode )
	{
		// Write to book from prefs
		mtkit_prefs_bulk_get ( prefs, table_i, NULL, table_s );
	}
	else
	{
		// Read from book to prefs
		mtkit_prefs_bulk_set ( prefs, table_i, NULL, table_s );
	}
}

mtPrefs * be_book_prefs_init (
	CedBook		* const	book
	)
{
	mtPrefs		* prefs;
	mtPrefTable	const item_table[] = {
	{ CED_FILE_PREFS_BOOK_AUTHOR, MTKIT_PREF_TYPE_STR_MULTI, "", _("Book Author"), fe_book_prefs_changed, 0, NULL, NULL },
	{ CED_FILE_PREFS_BOOK_COMMENT, MTKIT_PREF_TYPE_STR_MULTI, "", _("Book Comment"), fe_book_prefs_changed, 0, NULL, NULL },
	{ CED_FILE_PREFS_BOOK_DISABLE_LOCKS, MTKIT_PREF_TYPE_BOOL, "0", _("Disable all cell locks"), fe_book_prefs_changed, 0, NULL, NULL },
	{ CED_FILE_PREFS_BOOK_AUTO_RECALC, MTKIT_PREF_TYPE_OPTION, "1", _("Automatically recalculate the sheet or book after changes"), fe_book_prefs_changed, 0, _("None	Sheet	Book"), NULL },
	{ NULL }
	};


	// Load window prefs
	prefs = fe_load_pref_window_prefs ( item_table );

	mtkit_prefs_block_callback ( prefs );
	be_book_prefs_transfer ( prefs, book, 0 );
	mtkit_prefs_unblock_callback ( prefs );

	return prefs;
}

void be_book_prefs_finish (
	mtPrefs		* const	prefs,
	CedBook		* const	book
	)
{
	be_book_prefs_transfer ( prefs, book, 1 );
}

int be_color_swatch_get (
	int		const	i
	)
{
	static int const swatch_color_table [ COLOR_SWATCH_TOTAL + 1 ] = {
	MTKIT_RGB_2_INT (   0,   0,   0 ),
	MTKIT_RGB_2_INT (  90,   0,   0 ),
	MTKIT_RGB_2_INT ( 100,  72,   0 ),
	MTKIT_RGB_2_INT ( 100, 100,   0 ),
	MTKIT_RGB_2_INT (   0,  90,   0 ),
	MTKIT_RGB_2_INT (   0,  90,  90 ),
	MTKIT_RGB_2_INT (   0,   0, 100 ),
	MTKIT_RGB_2_INT (  90,   0,  90 ),

	MTKIT_RGB_2_INT ( 100, 100, 100 ),
	MTKIT_RGB_2_INT ( 170,   0,   0 ),
	MTKIT_RGB_2_INT ( 180, 106,   0 ),
	MTKIT_RGB_2_INT ( 170, 170,   0 ),
	MTKIT_RGB_2_INT (   0, 160,   0 ),
	MTKIT_RGB_2_INT (   0, 160, 160 ),
	MTKIT_RGB_2_INT (   0, 100, 180 ),
	MTKIT_RGB_2_INT ( 150,   0, 150 ),

	MTKIT_RGB_2_INT ( 180, 180, 180 ),
	MTKIT_RGB_2_INT ( 230, 140, 140 ),
	MTKIT_RGB_2_INT ( 230, 150,   0 ),
	MTKIT_RGB_2_INT ( 220, 220, 100 ),
	MTKIT_RGB_2_INT ( 120, 220, 120 ),
	MTKIT_RGB_2_INT ( 100, 230, 230 ),
	MTKIT_RGB_2_INT ( 100, 200, 255 ),
	MTKIT_RGB_2_INT ( 230, 140, 230 ),

	MTKIT_RGB_2_INT ( 220, 220, 220 ),
	MTKIT_RGB_2_INT ( 255, 200, 200 ),
	MTKIT_RGB_2_INT ( 255, 216, 160 ),
	MTKIT_RGB_2_INT ( 255, 255, 160 ),
	MTKIT_RGB_2_INT ( 180, 255, 180 ),
	MTKIT_RGB_2_INT ( 180, 255, 255 ),
	MTKIT_RGB_2_INT ( 180, 230, 255 ),
	MTKIT_RGB_2_INT ( 255, 200, 255 ),

	MTKIT_RGB_2_INT ( 255, 255, 255 ),
	MTKIT_RGB_2_INT ( 255,   0,   0 ),
	MTKIT_RGB_2_INT ( 255, 200,   0 ),
	MTKIT_RGB_2_INT ( 255, 255,   0 ),
	MTKIT_RGB_2_INT (   0, 255,   0 ),
	MTKIT_RGB_2_INT (   0, 255, 255 ),
	MTKIT_RGB_2_INT (   0,   0, 255 ),
	MTKIT_RGB_2_INT ( 255,   0, 255 )
	};


	if ( i >= COLOR_SWATCH_TOTAL || i < 0 )
	{
		return 0;
	}

	return swatch_color_table [ i ];
}

