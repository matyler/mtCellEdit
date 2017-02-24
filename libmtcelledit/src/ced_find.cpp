/*
	Copyright (C) 2009-2016 Mark Tyler

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
	CedSheet	* sheet;	// Sheet to scan
	char	const	* text;		// Text to match
	double		value;		// Value to match
	int		mode;		// CED_FIND_MODE_*
	int		row,		// First Row to search
			col,		// First Column to search
			rowtot,		// 1.. Rows to look in
					//	0 = all after start
			coltot		// 1.. Columns to look in
					//	0 = all after start
			;

	CedFuncScanArea	callback;	// Called when a cell is matched
	void		* user_data;	// Passed to callback

	int		find_value,	// 1 = value, 0 = text
			res;		// Result -1 = error, 1 = found
} findSTATE;



static void query_cell (
	CedCell		* const	cell,
	findSTATE	* const	state
	)
{
	state->res = 0;

	switch ( cell->type )
	{
	case CED_CELL_TYPE_TEXT:
	case CED_CELL_TYPE_TEXT_EXPLICIT:
		if ( state->mode & CED_FIND_MODE_IG_TEXT )
		{
			return;
		}
		break;

	case CED_CELL_TYPE_VALUE:
		if ( state->mode & CED_FIND_MODE_IG_VAL )
		{
			return;
		}
		break;

	case CED_CELL_TYPE_FORMULA:
	case CED_CELL_TYPE_FORMULA_EVAL:
		if ( state->mode & CED_FIND_MODE_IG_FORM )
		{
			return;
		}
		break;

	case CED_CELL_TYPE_ERROR:
		if ( state->mode & CED_FIND_MODE_IG_ERROR )
		{
			return;
		}
		break;

	case CED_CELL_TYPE_DATE:
		if ( state->mode & CED_FIND_MODE_IG_DATE )
		{
			return;
		}
		break;

	default:
		break;
	}

	if ( state->find_value )
	{
		if ( cell->value == state->value )
		{
			state->res = 1;
		}
	}
	else if ( cell->text )
	{
		// Must have text
		switch ( state->mode &
				( CED_FIND_MODE_CASE |
				CED_FIND_MODE_ALLCHARS |
				CED_FIND_MODE_WILDCARD )
				)
		{
		case CED_FIND_MODE_NONE:
			if ( mtkit_strcasestr ( cell->text, state->text ) )
			{
				state->res = 1;
			}
			break;

		case CED_FIND_MODE_CASE:
			if ( strstr ( cell->text, state->text ) )
			{
				state->res = 1;
			}
			break;

		case CED_FIND_MODE_ALLCHARS:
			if ( ! strcasecmp ( cell->text, state->text ) )
			{
				state->res = 1;
			}
			break;

		case CED_FIND_MODE_CASE | CED_FIND_MODE_ALLCHARS:
			if ( ! strcmp ( cell->text, state->text ) )
			{
				state->res = 1;
			}
			break;

		case CED_FIND_MODE_WILDCARD:
		case CED_FIND_MODE_WILDCARD | CED_FIND_MODE_ALLCHARS:
			if ( mtkit_strmatch ( cell->text, state->text, 0 ) >= 0
				)
			{
				state->res = 1;
			}
			break;

		case CED_FIND_MODE_WILDCARD | CED_FIND_MODE_CASE:
		case CED_FIND_MODE_WILDCARD |
			CED_FIND_MODE_CASE |
			CED_FIND_MODE_ALLCHARS :
			if ( mtkit_strmatch ( cell->text, state->text, 1 ) >= 0
				)
			{
				state->res = 1;
			}
			break;

		default:
			state->res = -1;	// Should never happen
		}
	}
}


static int find_real_scan_cb (
	CedSheet	* const	sheet,
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	findSTATE	* const	state = (findSTATE *)user_data;


	query_cell ( cell, state );

	if ( state->res == 1 )
	{
		// Text/Value found so do callback
		return state->callback ( sheet, cell, row, col,
			state->user_data );
	}

	return 0;		// Not matched, so continue
}

static int action_find (
	findSTATE	* const	state
	)
{
	if ( ! state->sheet->rows || ! state->sheet->rows->root )
	{
		return 0;		// Empty sheet so text isn't here
	}

	if ( state->mode & CED_FIND_MODE_BACKWARDS )
	{
		return ced_sheet_scan_area_backwards ( state->sheet, state->row,
			state->col, state->rowtot, state->coltot,
			find_real_scan_cb, state );
	}

	return ced_sheet_scan_area ( state->sheet, state->row, state->col,
		state->rowtot, state->coltot, find_real_scan_cb, state );
}

static int book_scan_cb (
	CedSheet	* const	sheet,
	char	const	* const	ARG_UNUSED ( name ),
	void		* const	user_data
	)
{
	findSTATE	* const	state = (findSTATE *)user_data;


	state->sheet = sheet;

	return action_find ( state );
}

static int find_real (
	findSTATE	* const	state
	)
{
	if ( ! state->callback || ! state->sheet )
	{
		return 1;		// Argument error
	}

	state->mode &= CED_FIND_MODE_ALL;

	if ( state->mode & CED_FIND_MODE_ALL_SHEETS )
	{
		if ( ! state->sheet->book )
		{
			return 1;	// No book to scan
		}

		return ced_book_scan ( state->sheet->book, book_scan_cb,
			state );
	}

	return action_find ( state );
}

int ced_sheet_find_text (
	CedSheet		* const	sheet,
	char		const	* const	text,
	int			const	mode,
	int			const	row,
	int			const	col,
	int			const	rowtot,
	int			const	coltot,
	CedFuncScanArea		const	callback,
	void			* const	user_data
	)
{
	findSTATE	state = { sheet, text, 0, mode, row, col, rowtot,
				coltot, callback, user_data, 0, 0 };

	if ( ! text )
	{
		return 1;
	}

	return find_real ( &state );
}

int ced_sheet_find_value (
	CedSheet	* const	sheet,
	double		const	value,
	int		const	mode,
	int		const	row,
	int		const	col,
	int		const	rowtot,
	int		const	coltot,
	CedFuncScanArea	const	callback,
	void		* const	user_data
	)
{
	findSTATE	state = { sheet, NULL, value, mode, row, col, rowtot,
				coltot, callback, user_data, 1, 0 };

	return find_real ( &state );
}
