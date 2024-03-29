/*
	Copyright (C) 2012-2022 Mark Tyler

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
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include <mtkit.h>
#include <mtcelledit.h>
#include <mtcedui.h>

#include "cli_static.h"



#undef BIN_NAME
#define BIN_NAME "mtcedcli"

#define	MAIN_ROW_PAD		"main_row_pad"
#define	MAIN_FONT_NAME		"main_font_name"
#define	MAIN_FONT_SIZE		"main_font_size"



class Backend;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class Backend
{
public:
	enum
	{
		FILE_TOTAL = 5
	};

	Backend ();
	~Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program with 0

	void main_loop ();

/// ----------------------------------------------------------------------------

	mtKit::Exit	exit;

private:
	int get_help ( char const * const * argv ) const;

	void recalc_sheet_core ();
	void recalc_book_core ();
	static int undo_report_updates ( int error );
	void update_changes_chores ();

	inline CuiFile * file ()	{ return m_file_p; };
	inline CuiClip * clipboard ()	{ return m_clipboard; };
	inline mtKit::UserPrefs & prefs () { return m_uprefs; };

	CedSheet * sheet ( bool print_error = true );
	CedBookFile * graph ( bool print_error = true );

	int set_file ( int n );
	int get_file_number () const;

	void prefs_init ();

	int transform_clipboard ( int item );
	int paste_clipboard_at_cursor ( int mode );
	int clear_selection ( int mode );
	int copy_selection_to_clip ();

	int cb_scan ( CedFuncScanArea const callback );

	CedSheet * prep_insert (
		char const * const * args, int &r1, int &c1, int &r2, int &c2 );

	int jtf_about			( char const * const * );
	int jtf_clear			( char const * const * );
	int jtf_clear_content		( char const * const * );
	int jtf_clear_prefs		( char const * const * );
	int jtf_clip_flip_h		( char const * const * );
	int jtf_clip_flip_v		( char const * const * );
	int jtf_clip_load		( char const * const * );
	int jtf_clip_rotate_a		( char const * const * );
	int jtf_clip_rotate_c		( char const * const * );
	int jtf_clip_save		( char const * const * );
	int jtf_clip_transpose		( char const * const * );
	int jtf_copy			( char const * const * );
	int jtf_copy_output		( char const * const * );
	int jtf_copy_values		( char const * const * );
	int jtf_cut			( char const * const * );
	int jtf_delete_column		( char const * const * );
	int jtf_delete_graph		( char const * const * );
	int jtf_delete_row		( char const * const * );
	int jtf_delete_sheet		( char const * const * );
	int jtf_duplicate_sheet		( char const * const * );
	int jtf_export_graph		( char const * const * );
	int jtf_export_output_graph	( char const * const * );
	int jtf_export_output_sheet	( char const * const * );
	int jtf_export_sheet		( char const * const * );
	int jtf_find			( char const * const * );
	int jtf_help			( char const * const * );
	int jtf_import_book		( char const * const * );
	int jtf_import_graph		( char const * const * );
	int jtf_info			( char const * const * );
	int jtf_insert_column		( char const * const * );
	int jtf_insert_row		( char const * const * );
	int jtf_list_files		( char const * const * );
	int jtf_list_graphs		( char const * const * );
	int jtf_list_sheets		( char const * const * );
	int jtf_load			( char const * const * );
	int jtf_new_book		( char const * const * );
	int jtf_new_sheet		( char const * const * );
	int jtf_paste			( char const * const * );
	int jtf_paste_content		( char const * const * );
	int jtf_paste_prefs		( char const * const * );
	int jtf_print			( char const * const * );
	int jtf_print_cell_num		( char const * const * );
	int jtf_print_cell_text		( char const * const * );
	int jtf_print_cell_type		( char const * const * );
	int jtf_print_prefs_book	( char const * const * );
	int jtf_print_prefs_cell	( char const * const * );
	int jtf_print_prefs_sheet	( char const * const * );
	int jtf_print_prefs_state	( char const * const * );
	int jtf_quit			( char const * const * );
	int jtf_recalc_book		( char const * const * );
	int jtf_recalc_sheet		( char const * const * );
	int jtf_redo			( char const * const * );
	int jtf_rename_graph		( char const * const * );
	int jtf_rename_sheet		( char const * const * );
	int jtf_save			( char const * const * );
	int jtf_save_as			( char const * const * );
	int jtf_select			( char const * const * );
	int jtf_set_2dyear		( char const * const * );
	int jtf_set_book		( char const * const * );
	int jtf_set_cell		( char const * const * );
	int jtf_set_graph		( char const * const * );
	int jtf_set_prefs_book		( char const * const * );
	int jtf_set_prefs_cell		( char const * const * );
	int jtf_set_prefs_cellborder	( char const * const * );
	int jtf_set_prefs_sheet		( char const * const * );
	int jtf_set_prefs_state		( char const * const * );
	int jtf_set_sheet		( char const * const * );
	int jtf_set_width		( char const * const * );
	int jtf_sort_column		( char const * const * );
	int jtf_sort_row		( char const * const * );
	int jtf_undo			( char const * const * );

/// ----------------------------------------------------------------------------

	CuiFile		* m_file_p			= nullptr;
	CuiFile		* m_file [ FILE_TOTAL ]		= { nullptr };
	CuiClip		* m_clipboard			= nullptr;

	mtKit::CliShell	m_clishell;
	mtKit::CliTab	m_clitab;

///	PREFS

	int		m_row_pad		= 0;
	std::string	m_font_name;
	int		m_font_size		= 0;
	int		m_page_width		= 0;
	int		m_page_height		= 0;
	int		m_page_margin_x		= 0;
	int		m_page_margin_y		= 0;
	int		m_page_header_left	= 0;
	int		m_page_header_centre	= 0;
	int		m_page_header_right	= 0;
	int		m_page_footer_left	= 0;
	int		m_page_footer_centre	= 0;
	int		m_page_footer_right	= 0;

	mtKit::UserPrefs m_uprefs;
};

