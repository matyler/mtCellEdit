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



static CedSheet		* pref_paste_sheet;
static int		pref_r1,
			pref_c1,
			pref_r2,
			pref_c2,
			old_cursor_r1,
			old_cursor_c1,
			old_cursor_r2,
			old_cursor_c2;



static void be_cellpref_cleanup_sheet ( void )
{
	ced_sheet_destroy ( pref_paste_sheet );
	pref_paste_sheet = NULL;
}

void be_cellpref_cleanup ( CedSheet * sheet )
{
	if ( sheet )
	{
		// Restore the cursor to its original position

		sheet->prefs.cursor_r1 = old_cursor_r1;
		sheet->prefs.cursor_r2 = old_cursor_r2;
		sheet->prefs.cursor_c1 = old_cursor_c1;
		sheet->prefs.cursor_c2 = old_cursor_c2;
	}

	be_cellpref_cleanup_sheet ();
}

int be_prepare_prefs_set (
	CedSheet	* const	sheet
	)
{
	if ( ! sheet )
	{
		return 1;
	}

	if ( pref_paste_sheet )
	{
		be_cellpref_cleanup_sheet ();
	}

	cui_cellprefs_init ( sheet, &pref_r1, &pref_c1, &pref_r2, &pref_c2,
		&pref_paste_sheet );
	if ( ! pref_paste_sheet )
	{
		return 1;
	}

	// NOTE: we have to do this as well as pref_*
	old_cursor_r1 = sheet->prefs.cursor_r1;
	old_cursor_r2 = sheet->prefs.cursor_r2;
	old_cursor_c1 = sheet->prefs.cursor_c1;
	old_cursor_c2 = sheet->prefs.cursor_c2;

	return 0;			// Success
}

static int be_cellpref_bulk_import (
	mtPrefs			* const	prefs,
	CedCellPrefs	const	* const	cell_prefs
	)
{
	if ( ! prefs || ! cell_prefs )
	{
		return 1;
	}

	mtkit_prefs_block_callback ( prefs );

	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_ALIGN_HORIZONTAL,
		cell_prefs->align_horizontal );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_COLOR_BACKGROUND,
		cell_prefs->color_background );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_COLOR_FOREGROUND,
		cell_prefs->color_foreground );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_FORMAT,
		cell_prefs->format );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_NUM_DECIMAL_PLACES,
		cell_prefs->num_decimal_places );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_NUM_ZEROS,
		cell_prefs->num_zeros );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_TEXT_STYLE,
		cell_prefs->text_style );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_LOCKED,
		cell_prefs->locked );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_BORDER_TYPE,
		cell_prefs->border_type );
	mtkit_prefs_set_int ( prefs, CED_FILE_PREFS_CELL_BORDER_COLOR,
		cell_prefs->border_color );

	mtkit_prefs_set_str ( prefs, CED_FILE_PREFS_CELL_FORMAT_DATETIME,
		cell_prefs->format_datetime );
	mtkit_prefs_set_str ( prefs, CED_FILE_PREFS_CELL_NUM_THOUSANDS,
		cell_prefs->num_thousands );
	mtkit_prefs_set_str ( prefs, CED_FILE_PREFS_CELL_TEXT_PREFIX,
		cell_prefs->text_prefix );
	mtkit_prefs_set_str ( prefs, CED_FILE_PREFS_CELL_TEXT_SUFFIX,
		cell_prefs->text_suffix );

	mtkit_prefs_unblock_callback ( prefs );

	return 0;
}

void be_cellpref_changed (
	mtPrefValue	* const	piv,
	int		const	callback_data,
	void		* const	callback_ptr
	)
{
	int		num = 0;
	char	const	* charp = NULL;


	switch ( callback_data )
	{
	case CUI_CELLPREFS_align_horizontal:
	case CUI_CELLPREFS_color_background:
	case CUI_CELLPREFS_color_foreground:
	case CUI_CELLPREFS_format:
	case CUI_CELLPREFS_num_decimal_places:
	case CUI_CELLPREFS_num_zeros:
	case CUI_CELLPREFS_text_style:
	case CUI_CELLPREFS_locked:
	case CUI_CELLPREFS_border_type:
	case CUI_CELLPREFS_border_color:
		mtkit_strtoi ( piv->value, &num, NULL, 0 );
		break;

	case CUI_CELLPREFS_format_datetime:
	case CUI_CELLPREFS_num_thousands:
	case CUI_CELLPREFS_text_prefix:
	case CUI_CELLPREFS_text_suffix:
		charp = piv->value;
		break;

	default:
		return;
	}

	fe_commit_prefs_set ( callback_data, num, charp, 1, callback_ptr );
}

mtPrefs * Backend::cellpref_init (
	CedSheet	* const	sheet,
	mtPrefCB	const	callback,
	void		* const	ptr
	)
{
	mtPrefs			* mtpr;
	CedCell			* cell = NULL;
	CedCellPrefs	const	* cell_prefs;
	mtPrefTable item_table[] = {
	{ CED_FILE_PREFS_CELL_ALIGN_HORIZONTAL, MTKIT_PREF_TYPE_OPTION, "0", "Horizontal text alignment", callback, CUI_CELLPREFS_align_horizontal, "None	Left	Centre	Right", ptr },
	{ CED_FILE_PREFS_CELL_COLOR_BACKGROUND, MTKIT_PREF_TYPE_RGB, "16777215", "Cell background colour", callback, CUI_CELLPREFS_color_background, NULL, ptr },
	{ CED_FILE_PREFS_CELL_COLOR_FOREGROUND, MTKIT_PREF_TYPE_RGB, "0", "Text foreground colour", callback, CUI_CELLPREFS_color_foreground, NULL, ptr },
	{ CED_FILE_PREFS_CELL_FORMAT, MTKIT_PREF_TYPE_OPTION, "0", "Cell format", callback, CUI_CELLPREFS_format, "General	Text	Fixed Decimal	Hexadecimal	Binary	Scientific	Percentage	Datetime", ptr },
	{ CED_FILE_PREFS_CELL_FORMAT_DATETIME, MTKIT_PREF_TYPE_STR, "", "Datetime format e.g. d/m/y HH:MM:SS", callback, CUI_CELLPREFS_format_datetime, "32", ptr },
	{ CED_FILE_PREFS_CELL_NUM_DECIMAL_PLACES, MTKIT_PREF_TYPE_INT, "0", "Number of decimal places for fixed type", callback, CUI_CELLPREFS_num_decimal_places, "0	99", ptr },
	{ CED_FILE_PREFS_CELL_NUM_THOUSANDS, MTKIT_PREF_TYPE_STR, "", "Thousands separator character", callback, CUI_CELLPREFS_num_thousands, "1", ptr },
	{ CED_FILE_PREFS_CELL_NUM_ZEROS, MTKIT_PREF_TYPE_INT, "0", "Leading zeros", callback, CUI_CELLPREFS_num_zeros, "0	32", ptr },
	{ CED_FILE_PREFS_CELL_TEXT_STYLE, MTKIT_PREF_TYPE_INT, "0", "Text style", callback, CUI_CELLPREFS_text_style, NULL, ptr },
	{ CED_FILE_PREFS_CELL_LOCKED, MTKIT_PREF_TYPE_BOOL, "0", "Cell locked - read only", callback, CUI_CELLPREFS_locked, NULL, ptr },
	{ CED_FILE_PREFS_CELL_BORDER_TYPE, MTKIT_PREF_TYPE_INT, "0", "Border type", callback, CUI_CELLPREFS_border_type, "0	1073741823", ptr },
	{ CED_FILE_PREFS_CELL_BORDER_COLOR, MTKIT_PREF_TYPE_RGB, "0", "Border colour", callback, CUI_CELLPREFS_border_color, NULL, ptr },
	{ CED_FILE_PREFS_CELL_TEXT_PREFIX, MTKIT_PREF_TYPE_STR, "", "Cell text prefix", callback, CUI_CELLPREFS_text_prefix, NULL, ptr },
	{ CED_FILE_PREFS_CELL_TEXT_SUFFIX, MTKIT_PREF_TYPE_STR, "", "Cell text suffix", callback, CUI_CELLPREFS_text_suffix, NULL, ptr },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};


	if ( be_prepare_prefs_set ( sheet ) )
	{
		return NULL;
	}

	cell = ced_sheet_get_cell ( sheet, old_cursor_r1, old_cursor_c1 );

	if ( cell && cell->prefs )
	{
		cell_prefs = cell->prefs;
	}
	else
	{
		cell_prefs = ced_cell_prefs_default ();
	}

	// Load window prefs from disk, cell prefs from sheet/default
	mtpr = load_pref_window_prefs ( item_table );
	if ( mtpr && cell_prefs )
	{
		be_cellpref_bulk_import ( mtpr, cell_prefs );
	}

	return mtpr;
}

void Backend::save_pref_window_prefs (
	mtPrefs		* const	mtpr
	)
{
	mtPrefTrans	const	winprefs_main_main[] = {
		{ GUI_INIFILE_PREFS_WINDOW"_x", GUI_INIFILE_PREFS_WINDOW"_x" },
		{ GUI_INIFILE_PREFS_WINDOW"_y", GUI_INIFILE_PREFS_WINDOW"_y" },
		{ GUI_INIFILE_PREFS_WINDOW"_w", GUI_INIFILE_PREFS_WINDOW"_w" },
		{ GUI_INIFILE_PREFS_WINDOW"_h", GUI_INIFILE_PREFS_WINDOW"_h" },
		{ NULL, NULL }
		},
		winprefs_shared_main[] = {
		{ GUI_INIFILE_SHARED_WINDOW"_x", GUI_INIFILE_PREFS_WINDOW"_x" },
		{ GUI_INIFILE_SHARED_WINDOW"_y", GUI_INIFILE_PREFS_WINDOW"_y" },
		{ GUI_INIFILE_SHARED_WINDOW"_w", GUI_INIFILE_PREFS_WINDOW"_w" },
		{ GUI_INIFILE_SHARED_WINDOW"_h", GUI_INIFILE_PREFS_WINDOW"_h" },
		{ NULL, NULL }
		};


	mtkit_prefs_value_copy ( preferences.getPrefsMem (), mtpr,
		winprefs_shared_main );

	mtkit_prefs_value_copy ( mtpr, preferences.getPrefsMem (),
		winprefs_main_main );
}

void Backend::load_pref_window_prefs (
	mtPrefs		* const	mtpr
	)
{
	mtPrefTrans	const	winprefs_main_shared[] = {
		{ GUI_INIFILE_PREFS_WINDOW"_x", GUI_INIFILE_SHARED_WINDOW"_x" },
		{ GUI_INIFILE_PREFS_WINDOW"_y", GUI_INIFILE_SHARED_WINDOW"_y" },
		{ GUI_INIFILE_PREFS_WINDOW"_w", GUI_INIFILE_SHARED_WINDOW"_w" },
		{ GUI_INIFILE_PREFS_WINDOW"_h", GUI_INIFILE_SHARED_WINDOW"_h" },
		{ NULL, NULL }
		};


	mtkit_prefs_value_copy ( mtpr, preferences.getPrefsMem (),
		winprefs_main_shared );
}

int be_commit_prefs_set (
	CedSheet	* const	sheet,
	CuiBook		* const	cubook,
	int		const	pref_id,
	int		const	pref_num,
	char	const	* const	pref_charp
	)
{
	return cui_cellprefs_change ( cubook, sheet,
		pref_r1, pref_c1, pref_r2, pref_c2,
		pref_paste_sheet, pref_id, pref_charp, pref_num );
}

int be_commit_prefs_set_check (
	int		const	res,
	CedSheet	* const	sheet,
	int		const	change_cursor,
	int		const	pref_id
	)
{
	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	if (	change_cursor &&
		(	pref_id == CUI_CELLPREFS_color_background ||
			pref_id == CUI_CELLPREFS_color_foreground
			)
		)
	{

		/*
		Set the cursor to invisible, i.e. 0,0 -> 0,0 so that the user
		can see background colour updates.
		*/

		sheet->prefs.cursor_r1 = 0;
		sheet->prefs.cursor_r2 = 0;
		sheet->prefs.cursor_c1 = 0;
		sheet->prefs.cursor_c2 = 0;
	}

	return 0;
}

