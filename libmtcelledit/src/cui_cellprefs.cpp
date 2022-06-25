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

#include "cui.h"



CuiCellPrefChange::CuiCellPrefChange ()
	:
	cell_vals ()
{
}

CuiCellPrefChange::~CuiCellPrefChange ()
{
	cleanup ();
}

int CuiCellPrefChange::init ( CedSheet * const sheet )
{
	if ( ! sheet )
	{
		return 1;
	}

	if ( pref_paste_sheet )
	{
		cleanup_sheet ();
	}

	cui_cellprefs_init ( sheet, &pref_r1, &pref_c1, &pref_r2, &pref_c2,
		&pref_paste_sheet );
	if ( ! pref_paste_sheet )
	{
		return 1;
	}

	src = sheet;

	// NOTE: we have to do this as well as pref_*
	old_cursor_r1 = sheet->prefs.cursor_r1;
	old_cursor_r2 = sheet->prefs.cursor_r2;
	old_cursor_c1 = sheet->prefs.cursor_c1;
	old_cursor_c2 = sheet->prefs.cursor_c2;

	CedCell const * const cell = ced_sheet_get_cell ( sheet, old_cursor_r1,
		old_cursor_c1 );

	if ( cell && cell->prefs )
	{
		cell_prefs = cell->prefs;
	}
	else
	{
		cell_prefs = ced_cell_prefs_default ();
	}

	return 0;			// Success
}

void CuiCellPrefChange::cleanup ()
{
	if ( src )
	{
		// Restore the cursor to its original position

		src->prefs.cursor_r1 = old_cursor_r1;
		src->prefs.cursor_r2 = old_cursor_r2;
		src->prefs.cursor_c1 = old_cursor_c1;
		src->prefs.cursor_c2 = old_cursor_c2;

		src = nullptr;
	}

	cleanup_sheet ();
}

void CuiCellPrefChange::cleanup_sheet ()
{
	ced_sheet_destroy ( pref_paste_sheet );
	pref_paste_sheet = NULL;
}


int CuiCellPrefChange::commit_prefs_set (
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

int CuiCellPrefChange::commit_prefs_set_check (
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

		// Set the cursor to invisible, i.e. 0,0 -> 0,0 so that the user
		// can see background colour updates.

		sheet->prefs.cursor_r1 = 0;
		sheet->prefs.cursor_r2 = 0;
		sheet->prefs.cursor_c1 = 0;
		sheet->prefs.cursor_c2 = 0;
	}

	return 0;
}

void CuiCellPrefChange::cellpref_init ( mtKit::UserPrefs & prefs )
{
	prefs.add_option ( CED_FILE_PREFS_CELL_ALIGN_HORIZONTAL, cell_vals.
		align_horizontal, 0, { "None", "Left", "Centre", "Right" } );

	prefs.add_rgb ( CED_FILE_PREFS_CELL_COLOR_BACKGROUND,
		cell_vals.color_background, 16777215 );

	prefs.add_rgb ( CED_FILE_PREFS_CELL_COLOR_FOREGROUND,
		cell_vals.color_foreground, 0 );

	prefs.add_option ( CED_FILE_PREFS_CELL_FORMAT, cell_vals.format, 0,
		{ "General", "Text", "Fixed Decimal", "Hexadecimal", "Binary",
		"Scientific", "Percentage", "Datetime" } );

	prefs.add_string ( CED_FILE_PREFS_CELL_FORMAT_DATETIME,
		cell_format_datetime, "", 32 );

	prefs.add_int ( CED_FILE_PREFS_CELL_NUM_DECIMAL_PLACES,
		cell_vals.num_decimal_places, 0, 0, 99 );

	prefs.add_string ( CED_FILE_PREFS_CELL_NUM_THOUSANDS,
		cell_num_thousands, "", 1 );

	prefs.add_int ( CED_FILE_PREFS_CELL_NUM_ZEROS, cell_vals.num_zeros, 0,
		0, 32 );

	prefs.add_int ( CED_FILE_PREFS_CELL_TEXT_STYLE, cell_vals.text_style,0);
	prefs.add_bool ( CED_FILE_PREFS_CELL_LOCKED, cell_vals.locked, 0 );

	prefs.add_int ( CED_FILE_PREFS_CELL_BORDER_TYPE, cell_vals.border_type,
		0, 0, 1073741823 );

	prefs.add_rgb ( CED_FILE_PREFS_CELL_BORDER_COLOR,
		cell_vals.border_color, 0 );

	prefs.add_string ( CED_FILE_PREFS_CELL_TEXT_PREFIX, cell_text_prefix,
		"" );

	prefs.add_string ( CED_FILE_PREFS_CELL_TEXT_SUFFIX, cell_text_suffix,
		"" );


///	Populate the current prefs values from the initial cell state ----------


	cell_vals = *cell_prefs;

	auto move_string = [this]( std::string & str, char const * const chp )
		{
			if ( chp )	str = chp;
			else		str = "";
		};

	move_string ( cell_format_datetime, cell_vals.format_datetime );
	move_string ( cell_num_thousands, cell_vals.num_thousands );
	move_string ( cell_text_prefix, cell_vals.text_prefix );
	move_string ( cell_text_suffix, cell_vals.text_suffix );


///	Descriptions -----------------------------------------------------------


	prefs.set_description ( CED_FILE_PREFS_CELL_ALIGN_HORIZONTAL,
		"Horizontal text alignment" );

	prefs.set_description ( CED_FILE_PREFS_CELL_COLOR_BACKGROUND,
		"Cell background colour" );

	prefs.set_description ( CED_FILE_PREFS_CELL_COLOR_FOREGROUND,
		"Text foreground colour" );

	prefs.set_description ( CED_FILE_PREFS_CELL_FORMAT, "Cell format" );

	prefs.set_description ( CED_FILE_PREFS_CELL_FORMAT_DATETIME,
		"Datetime format e.g. d/m/y HH:MM:SS" );

	prefs.set_description ( CED_FILE_PREFS_CELL_NUM_DECIMAL_PLACES,
		"Number of decimal places for fixed type" );

	prefs.set_description ( CED_FILE_PREFS_CELL_NUM_THOUSANDS,
		"Thousands separator character" );

	prefs.set_description ( CED_FILE_PREFS_CELL_NUM_ZEROS, "Leading zeros");
	prefs.set_description ( CED_FILE_PREFS_CELL_TEXT_STYLE, "Text style" );

	prefs.set_description ( CED_FILE_PREFS_CELL_LOCKED,
		"Cell locked - read only" );

	prefs.set_description ( CED_FILE_PREFS_CELL_BORDER_TYPE, "Border type");
	prefs.set_description ( CED_FILE_PREFS_CELL_BORDER_COLOR,
		"Border colour" );

	prefs.set_description ( CED_FILE_PREFS_CELL_TEXT_PREFIX,
		"Cell text prefix" );

	prefs.set_description ( CED_FILE_PREFS_CELL_TEXT_SUFFIX,
		"Cell text suffix" );
}

