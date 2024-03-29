/*
	Copyright (C) 2015-2020 Mark Tyler

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

#include "ced.h"



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
	printf ( "%s", prefix );

	CedCell const * const cell = ced_sheet_get_cell ( sheet, row, col );
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

int Global::ced_diff ()
{
	int		res		= 0;
	CedSheet	* sheet_delta	= NULL;	// Delta sheet cells


	CedSheet * const sheet_a = m_sheet;
	m_sheet = NULL;

	if ( load_file () || m_sheet == NULL )
	{
		res = ERROR_LOAD_FILE;	// Fail, and tell user load failed

		goto finish;
	}

	m_filename_a = m_filename_b;
	m_filename_b = s_arg;

	if ( ! m_filename_a )
	{
		// This is the first file arg so nothing to do
		goto finish;
	}

	printf ( "--- %s\n", m_filename_a );
	printf ( "+++ %s\n", m_filename_b );

	sheet_delta = ced_sheet_new ();
	if ( ! sheet_delta )
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

	if (	cmp_sheet_cells ( sheet_a, m_sheet, sheet_delta ) ||
		cmp_sheet_cells ( m_sheet, sheet_a, sheet_delta )
		)
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}


	{
		scanState state = { sheet_a, m_sheet, sheet_delta };


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

