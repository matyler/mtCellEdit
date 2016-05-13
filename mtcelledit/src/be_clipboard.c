/*
	Copyright (C) 2008-2015 Mark Tyler

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



int be_clip_clear_selection (
	CuiFile		* const	file,
	CedSheet	* const	sheet,
	int		const	mode
	)
{
	int		r,
			c,
			rtot,
			ctot;


	if ( ! sheet )
	{
		return 1;
	}

	r = MIN ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	c = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	rtot = 1 + abs ( sheet->prefs.cursor_r1 - sheet->prefs.cursor_r2 );
	ctot = 1 + abs ( sheet->prefs.cursor_c1 - sheet->prefs.cursor_c2 );

	if ( r < 1 || c < 1 )
	{
		return 1;
	}

	return cui_sheet_clear_area ( file->cubook, sheet, r, c, rtot, ctot,
		mode );
}

CedSheet * be_clip_transform_start (
	CuiClip		* const	clip,
	int		const	mode
	)
{
	switch ( mode )
	{
	case 0:	return ced_sheet_transpose ( clip->sheet );
	case 1:	return ced_sheet_flip_horizontal ( clip->sheet );
	case 2:	return ced_sheet_flip_vertical ( clip->sheet );
	case 3:	return ced_sheet_rotate_clockwise ( clip->sheet );
	case 4:	return ced_sheet_rotate_anticlockwise ( clip->sheet );
	}

	return NULL;
}

int be_clip_transform_finish (
	CuiClip		* const	clip,
	CedSheet	* const	sheet,
	int		const	mode
	)
{
	if ( clip->sheet )
	{
		ced_sheet_destroy ( sheet );

		return 1;		// Fail
	}

	clip->sheet = sheet;

	if (	mode != 1 &&
		mode != 2
		)
	{
		int		tmp = clip->rows;


		// X/Y Geometry has been swapped

		clip->rows = clip->cols;
		clip->cols = tmp;
	}

	return 0;			// Success
}

static int copy_value_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	char		txt[256];


	switch ( cell->type )
	{
	case CED_CELL_TYPE_FORMULA:
	case CED_CELL_TYPE_FORMULA_EVAL:
		snprintf ( txt, sizeof ( txt ), CED_PRINTF_NUM, cell->value );
		if ( ! mtkit_strfreedup ( &cell->text, txt ) )
		{
			cell->type = CED_CELL_TYPE_VALUE;
		}
		else
		{
			return 1;
		}
		break;

	default:
		break;
	}

	return 0;
}

int be_clip_copy_values (
	CedSheet	* const	sheet
	)
{
	return ced_sheet_scan_area ( sheet, 1, 1, 0, 0, copy_value_scan, NULL );
}

static int copy_output_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	char		* txt;


	txt = ced_cell_create_output ( cell, NULL );
	free ( cell->text );

	cell->text = txt;
	cell->type = CED_CELL_TYPE_TEXT_EXPLICIT;
	cell->value = 0;

	return 0;
}

int be_clip_copy_output ( CedSheet * sheet )
{
	return ced_sheet_scan_area ( sheet, 1, 1, 0, 0, copy_output_scan,
		NULL );
}

int be_selection_row_extent (
	CedSheet	* const	sheet,
	int		* const	row,
	int		* const	rowtot
	)
{
	if ( ! sheet )
	{
		return 1;
	}

	if ( sheet->prefs.cursor_r1 > sheet->prefs.cursor_r2 )
	{
		row[0] = sheet->prefs.cursor_r2;
		rowtot[0] = sheet->prefs.cursor_r1 - sheet->prefs.cursor_r2 + 1;
	}
	else
	{
		row[0] = sheet->prefs.cursor_r1;
		rowtot[0] = sheet->prefs.cursor_r2 - sheet->prefs.cursor_r1 + 1;
	}

	return 0;
}

int be_selection_col_extent (
	CedSheet	* const	sheet,
	int		* const	col,
	int		* const	coltot
	)
{
	if ( ! sheet )
	{
		return 1;
	}

	if ( sheet->prefs.cursor_c1 > sheet->prefs.cursor_c2 )
	{
		col[0] = sheet->prefs.cursor_c2;
		coltot[0] = sheet->prefs.cursor_c1 - sheet->prefs.cursor_c2 + 1;
	}
	else
	{
		col[0] = sheet->prefs.cursor_c1;
		coltot[0] = sheet->prefs.cursor_c2 - sheet->prefs.cursor_c1 + 1;
	}

	return 0;
}

