/*
	Copyright (C) 2012-2017 Mark Tyler

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



int jtf_delete_column (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	CedSheet	* const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return 2;
	}

	int		r1, c1, r2, c2;

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	int const res = cui_sheet_delete_column ( backend.file()->cubook, sheet,
		c1, c2 - c1 + 1 );

	backend.undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 2;
	}

	backend.update_changes_chores ();

	return 0;
}

int jtf_delete_row (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	CedSheet	* const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return 2;
	}

	int		r1, c1, r2, c2;

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	int const res = cui_sheet_delete_row ( backend.file()->cubook, sheet,
		r1, r2 - r1 + 1 );

	backend.undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 2;
	}

	backend.update_changes_chores ();

	return 0;
}

static int find_cb (
	CedSheet	* const	sheet,
	CedCell		* const	cell,
	int		const	r,
	int		const	c,
	void		* const	ud
	)
{
	char	const *		txt = "";
	char	const *		txt_sheet = "";
	int		* const	tot = (int *)ud;


	tot[0] ++;

	if ( cell->text )
	{
		txt = cell->text;
	}

	if ( sheet->book_tnode )
	{
		txt_sheet = (char const *)sheet->book_tnode->key;
	}

	printf ( "%5i %s r%ic%i = '%s'\n", tot[0], txt_sheet, r, c, txt );

	return 0;
}

int jtf_find (
	char	const * const * const	args
	)
{
	CedSheet	* const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return 2;
	}

	int		opts[4] = {0,0,0,0}, res;

	for ( int i = 1; args[i]; i++ )
	{
		mtKit::CharInt	const	chitab[] = {
			{ "wild",	0 },
			{ "case",	1 },
			{ "value",	2 },
			{ "all",	3 },
			{ NULL, 0 }
			};

		if (	mtKit::cli_parse_charint ( args[i], chitab, res ) ||
			res < 0 ||
			res > 4
			)
		{
			fprintf ( stderr, "Unable to match argument '%s'\n\n",
				args[i] );

			return 2;
		}

		opts[res] = 1;
	}

	int		find_mode = 0;

	if ( opts[0] )
	{
		find_mode |= CED_FIND_MODE_WILDCARD;
	}

	if ( opts[1] )
	{
		find_mode |= CED_FIND_MODE_CASE;
	}

	if ( opts[3] )
	{
		find_mode |= CED_FIND_MODE_ALL_SHEETS;
	}

	int	r1, c1, r2, c2;

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	if ( r1 == r2 && c1 == c2 )
	{
		// Search whole sheet
		r1 = 1;
		c1 = 1;
		r2 = 0;
		c2 = 0;
	}
	else
	{
		// Search current selection
		r2 = r2 - r1 + 1;
		c2 = c2 - c1 + 1;
	}

	int		tot_matches = 0;

	if ( opts[2] )
	{
		double		value;


		if ( mtkit_strtod ( args[0], &value, NULL, 1 ) )
		{
			fprintf ( stderr, "Bad value - '%s'\n\n", args[0] );

			return 2;
		}

		ced_sheet_find_value ( sheet, value, find_mode,
			r1, c1, r2, c2, find_cb, &tot_matches );
	}
	else
	{
		ced_sheet_find_text ( sheet, args[0], find_mode,
			r1, c1, r2, c2, find_cb, &tot_matches );
	}


	return 0;
}

int jtf_set_book (
	char	const * const * const	args
	)
{
	int		booknum = 0;


	if (	mtkit_strtoi ( args[0], &booknum, NULL, 1 )	||
		backend.set_file ( booknum )
		)
	{
		fprintf ( stderr, "Bad book number '%s'\n\n", args[0] );

		return 2;
	}

	return 0;
}

int jtf_set_cell (
	char	const * const * const	args
	)
{
	CedSheet * const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return 2;
	}

	int		r1, c1, r2, c2;

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	int const res = cui_sheet_set_cell ( backend.file()->cubook, sheet,
		sheet->prefs.cursor_r1, sheet->prefs.cursor_c1, args[0] );

	backend.undo_report_updates ( res );
	backend.update_changes_chores ();

	return 0;
}

int jtf_select (
	char	const * const * const	args
	)
{
	CedSheet * const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return 2;
	}

	if ( strcmp ( args[0], "all" ) == 0 )
	{
		int	row_max, col_max;


		// Select all

		if ( ced_sheet_get_geometry ( sheet, &row_max, &col_max ) )
		{
			fprintf ( stderr, "Unable to get sheet geometry.\n\n ");

			return 2;
		}

		sheet->prefs.cursor_r1 = 1;
		sheet->prefs.cursor_c1 = 1;
		sheet->prefs.cursor_r2 = MAX ( row_max, 1 );
		sheet->prefs.cursor_c2 = MAX ( col_max, 1 );
	}
	else if ( strchr ( args[0], ':' ) )
	{
		CedCellRef	r1, r2;


		// Cell range passed

		if ( ced_strtocellrange ( args[0], &r1, &r2, NULL, 1 ) ||
			r1.row_m != 0 ||
			r1.col_m != 0 ||
			r2.row_m != 0 ||
			r2.col_m != 0 ||
			r1.row_d < 1 ||
			r1.col_d < 1 ||
			r2.row_d < 1 ||
			r2.col_d < 1
			)
		{
			fprintf ( stderr, "Invalid cell range '%s'.\n\n",
				args[0] );

			return 2;
		}

		sheet->prefs.cursor_r1 = r1.row_d;
		sheet->prefs.cursor_c1 = r1.col_d;
		sheet->prefs.cursor_r2 = r2.row_d;
		sheet->prefs.cursor_c2 = r2.col_d;
	}
	else
	{
		CedCellRef	r1;


		// Cell reference passed

		if ( ced_strtocellref ( args[0], &r1, NULL, 1 ) ||
			r1.row_m != 0 ||
			r1.col_m != 0 ||
			r1.row_d < 1 ||
			r1.col_d < 1
			)
		{
			fprintf ( stderr, "Invalid cell reference '%s'.\n\n",
				args[0] );

			return 2;
		}

		sheet->prefs.cursor_r1 = r1.row_d;
		sheet->prefs.cursor_c1 = r1.col_d;
		sheet->prefs.cursor_r2 = r1.row_d;
		sheet->prefs.cursor_c2 = r1.col_d;
	}

	return 0;
}

int jtf_set_graph (
	char	const * const * const	args
	)
{
	if ( ! cui_graph_get ( backend.file()->cubook->book, args[0] ) )
	{
		fprintf ( stderr, "No such graph.\n\n" );

		return 2;
	}

	if ( mtkit_strfreedup ( &backend.file()->cubook->book->prefs.
		active_graph, args[0] ) )
	{
		fprintf ( stderr, "Unable to change graph.\n\n" );

		return 2;
	}

	return 0;
}

int jtf_set_sheet (
	char	const * const * const	args
	)
{
	if ( ! ced_book_get_sheet ( backend.file()->cubook->book, args[0] ) )
	{
		fprintf ( stderr, "No such sheet.\n\n" );

		return 2;
	}

	if ( mtkit_strfreedup ( &backend.file()->cubook->book->prefs.
		active_sheet, args[0] ) )
	{
		fprintf ( stderr, "Unable to change sheet.\n\n" );

		return 2;
	}

	return 0;
}

int jtf_set_width (
	char	const * const * const	args
	)
{
	CedSheet	* const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return 2;
	}

	int		r1, c1, r2, c2, res;

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	int	const	ctot = c2 - c1 + 1;

	if ( strcmp ( "auto", args[0] ) == 0 )
	{
		res = cui_sheet_set_column_width_auto ( backend.file()->cubook,
			sheet, c1, ctot );
	}
	else
	{
		int	w;


		if ( mtKit::cli_parse_int ( args[0], w,
			CED_MIN_COLUMN_WIDTH, CED_MAX_COLUMN_WIDTH )
			)
		{
			return 2;
		}

		res = cui_sheet_set_column_width( backend.file()->cubook, sheet,
			c1, ctot, w );
	}

	backend.undo_report_updates ( res );

	if ( res )
	{
		return 2;
	}

	return 0;
}

int jtf_undo (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( ! backend.file()->cubook->undo.undo_step )
	{
		fprintf ( stderr, "No undo available.\n\n" );

		return 2;
	}

	int const res = cui_book_undo_step ( backend.file()->cubook );

	backend.undo_report_updates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return 2;
	}

	backend.update_changes_chores ();

	return 0;
}

int jtf_redo (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( ! backend.file()->cubook->undo.redo_step )
	{
		fprintf ( stderr, "No redo available.\n\n" );

		return 2;
	}

	int const res = cui_book_redo_step ( backend.file()->cubook );

	backend.undo_report_updates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return 2;
	}

	backend.update_changes_chores ();

	return 0;
}

int jtf_recalc_book (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.recalc_book_core ();

	return 0;
}

int jtf_recalc_sheet (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.recalc_sheet_core ();

	return 0;
}

