/*
	Copyright (C) 2008-2014 Mark Tyler

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



void be_find (
	CuiFile		* const	file,
	CedSheet	* const	sheet,
	char	const	* const	txt,
	int		const	wildc,
	int		const	casen,
	int		const	mval,
	int		const	allsheets,
	CedFuncScanArea	const	callback,
	void		* const	user_data
	)
{
	int		find_mode,
			find_all_sheets,
			find_row,
			find_col,
			find_rowtot,
			find_coltot
			;


	if ( ! file || ! sheet || ! txt || ! txt[0] )
	{
		return;
	}

	find_all_sheets = allsheets;

	if (	sheet->prefs.cursor_r1 == sheet->prefs.cursor_r2 &&
		sheet->prefs.cursor_c1 == sheet->prefs.cursor_c2
		)
	{
		// Just one cell selected so search the whole sheet,
		// i.e. 1,1,0,0

		find_row = 1;
		find_col = 1;
		find_rowtot = 0;
		find_coltot = 0;
	}
	else
	{
		ced_sheet_cursor_max_min ( sheet, &find_row, &find_col,
			&find_rowtot, &find_coltot );

		find_rowtot += 1 - find_row;
		find_coltot += 1 - find_col;
	}

	if (	file->type == CED_FILE_TYPE_TSV_BOOK		||
		file->type == CED_FILE_TYPE_TSV_VAL_BOOK	||
		file->type == CED_FILE_TYPE_LEDGER_BOOK		||
		file->type == CED_FILE_TYPE_LEDGER_VAL_BOOK
		)
	{
	}
	else
	{
		find_all_sheets = 0;
	}

	find_mode = 0;

	if ( casen )
	{
		find_mode |= CED_FIND_MODE_CASE;
	}

	if ( wildc )
	{
		find_mode |= CED_FIND_MODE_WILDCARD;
	}

	if ( find_all_sheets )
	{
		find_mode |= CED_FIND_MODE_ALL_SHEETS;
	}

	if ( mval )
	{
		double		value = 0;


		mtkit_strtod ( txt, &value, NULL, 0 );

		ced_sheet_find_value ( sheet, value, find_mode,
			find_row, find_col, find_rowtot, find_coltot,
			callback, user_data );
	}
	else
	{
		ced_sheet_find_text ( sheet, txt, find_mode,
			find_row, find_col, find_rowtot, find_coltot,
			callback, user_data );
	}
}

