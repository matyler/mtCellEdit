/*
	Copyright (C) 2015-2016 Mark Tyler

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
	CedSheet	* a;	// First sheet
	CedSheet	* b;	// Second sheet
	CedSheet	* d;	// Delta sheet
} scanState;



static void cell_check (
	char	const * const	prefix,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	col
	)
{
	CedCell		* cell;


	printf ( "%s", prefix );

	cell = ced_sheet_get_cell ( sheet, row, col );
	if ( cell && cell->text )
	{
		puts ( cell->text );
	}
	else
	{
		puts ( "" );
	}
}

static int scan_delta_output (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	scanState	* state = (scanState *) user_data;


	printf ( "@@ r%ic%i @@\n", row, col );

	cell_check ( "---", state->a, row, col );
	cell_check ( "+++", state->b, row, col );

	return 0;	// Continue
}

static int scan_check_a_b (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	scanState	* state = (scanState *) user_data;


	// Only bother checking if A has any text in it
	if ( cell->text )
	{
		CedCell		* cell_b;


		cell_b = ced_sheet_get_cell ( state->b, row, col );

		if (	! cell_b				||
			! cell_b->text				||
			0 != strcmp ( cell_b->text, cell->text )
			)
		{
			// Cell text is different so mark as delta cell

			if ( ! ced_sheet_set_cell_text ( state->d, row, col,
				"q" ) )
			{
				// libmtcelledit error
				return 1;
			}
		}
	}

	return 0;	// Continue
}

static int cmp_sheet_cells (
	CedSheet	* const	a,
	CedSheet	* const	b,
	CedSheet	* const	d
	)
{
	scanState	state = { a, b, d };


	if ( ced_sheet_scan_area ( a, 1, 1, 0, 0, scan_check_a_b, &state ) )
	{
		return 1;
	}

	return 0;
}

int cedut_diff ( void )
{
	static char const	* filename_a;	// Previous filename in args
	static char const	* filename_b;	// Latest filename in args

	int		res		= 0;
	CedSheet	* sheet_a	= NULL;	// Previous sheet
	CedSheet	* sheet_delta	= NULL;	// Delta sheet cells


	sheet_a = global.sheet;
	global.sheet = NULL;

	if ( ut_load_file () || global.sheet == NULL )
	{
		res = ERROR_LOAD_FILE;	// Fail, and tell user load failed

		goto finish;
	}

	filename_a = filename_b;
	filename_b = global.s_arg;

	if ( ! filename_a )
	{
		// This is the first file arg so nothing to do
		goto finish;
	}

	printf ( "--- %s\n", filename_a );
	printf ( "+++ %s\n", filename_b );

	sheet_delta = ced_sheet_new ();
	if ( ! sheet_delta )
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

	if (	cmp_sheet_cells ( sheet_a, global.sheet, sheet_delta )	||
		cmp_sheet_cells ( global.sheet, sheet_a, sheet_delta )
		)
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}


	{
		scanState	state = { sheet_a, global.sheet, sheet_delta };


		if ( ced_sheet_scan_area ( sheet_delta, 1, 1, 0, 0,
			scan_delta_output, &state ) )
		{
			res = ERROR_LIBMTCELLEDIT;
			goto finish;
		}
	}

finish:
	ced_sheet_destroy ( sheet_a );
	ced_sheet_destroy ( sheet_delta );

	return res;
}

