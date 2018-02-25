/*
	Copyright (C) 2011-2016 Mark Tyler

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

#include "private.h"



typedef struct
{
	int		rows;
	int		cols;
	CedSheet	* new_sheet;
	CedCell		* new_cell;
} tranSTATE;



static int set_cell (
	tranSTATE	* const	state,
	int		const	row,
	int		const	column,
	CedCell		* const	cell
	)
{
	state->new_cell = ced_sheet_set_cell_text ( state->new_sheet, row,
			column, "y" );

	if ( ! state->new_cell )
	{
		return 1;		// Failure
	}

	if ( ced_cell_paste ( state->new_cell, cell ) )
	{
		return 1;		// Failure
	}

	return 0;			// Success
}

static int cb_transpose (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	return set_cell ( (tranSTATE *)user_data, col, row, cell );
}

static int cb_flip_horizontal (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	tranSTATE	* const	state = (tranSTATE *)user_data;


	return set_cell ( state, row, state->cols - col + 1, cell );
}

static int cb_flip_vertical (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	tranSTATE	* const	state = (tranSTATE *)user_data;


	return set_cell ( state, state->rows - row + 1, col, cell );
}

static int cb_rotate_clock (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	tranSTATE	* const	state = (tranSTATE *)user_data;


	return set_cell ( state, col, state->rows - row + 1, cell );
}

static int cb_rotate_anticlock (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	tranSTATE	* const	state = (tranSTATE *)user_data;


	return set_cell ( state, state->cols - col + 1, row, cell );
}



static CedSheet * transform (
	CedSheet	* const	sheet,
	CedFuncScanArea	const	callback
	)
{
	tranSTATE	state = { 0, 0, NULL, NULL };


	if ( ! sheet )
	{
		return NULL;
	}

	state.new_sheet = ced_sheet_new ();
	if ( ! state.new_sheet )
	{
		return NULL;
	}

	if ( callback != cb_transpose )
	{
		if ( ced_sheet_get_geometry ( sheet, &state.rows, &state.cols )
			)
		{
			goto error;
		}
	}

	if ( ced_sheet_scan_area ( sheet, 1, 1, 0, 0, callback, &state ) != 0 )
	{
		goto error;
	}

	return state.new_sheet;

error:
	ced_sheet_destroy ( state.new_sheet );
	state.new_sheet = NULL;

	return NULL;
}

CedSheet * ced_sheet_transpose (
	CedSheet	* const	sheet
	)
{
	return transform ( sheet, cb_transpose );
}

CedSheet * ced_sheet_flip_horizontal (
	CedSheet	* const	sheet
	)
{
	return transform ( sheet, cb_flip_horizontal );
}

CedSheet * ced_sheet_flip_vertical (
	CedSheet	* const	sheet
	)
{
	return transform ( sheet, cb_flip_vertical );
}

CedSheet * ced_sheet_rotate_clockwise (
	CedSheet	* const	sheet
	)
{
	return transform ( sheet, cb_rotate_clock );
}

CedSheet * ced_sheet_rotate_anticlockwise (
	CedSheet	* const	sheet
	)
{
	return transform ( sheet, cb_rotate_anticlock );
}
