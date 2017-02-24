/*
	Copyright (C) 2012-2016 Mark Tyler

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



static void recalc_sheet_core (
	CedSheet	* const	sheet
	)
{
	ced_sheet_recalculate ( sheet, NULL, 0 );
	ced_sheet_recalculate ( sheet, NULL, 1 );
}

static void recalc_book_core (
	CedCli_STATE	* const	state
	)
{
	ced_book_recalculate ( CEDCLI_FILE->cubook->book, 0 );
	ced_book_recalculate ( CEDCLI_FILE->cubook->book, 1 );
}

int jtf_delete_column (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	CedSheet	* sheet;
	int		r1, c1, r2, c2,
			res;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	res = cui_sheet_delete_column ( CEDCLI_FILE->cubook, sheet,
		c1, c2 - c1 + 1 );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	update_changes_chores ( state, sheet );

	return 0;
}

int jtf_delete_row (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	CedSheet	* sheet;
	int		r1, c1, r2, c2,
			res;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	res = cui_sheet_delete_row ( CEDCLI_FILE->cubook, sheet,
		r1, r2 - r1 + 1 );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	update_changes_chores ( state, sheet );

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
	char	const	* txt = "";
	char	const	* txt_sheet = "";
	int		* tot = (int *)ud;


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
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedSheet	* sheet;
	int		opts[4] = {0,0,0,0},
			tot_matches, i, res,
			r1, c1, r2, c2, find_mode;

	mtKit::CharInt	const	chitab[] = {
			{ "wild",	0 },
			{ "case",	1 },
			{ "value",	2 },
			{ "all",	3 },
			{ NULL, 0 }
			};


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	for ( i = 1; args[i]; i++ )
	{
		if (	mtKit::cli_parse_charint ( args[i], chitab, res ) ||
			res < 0 ||
			res > 4
			)
		{
			fprintf ( stderr, "Unable to match argument '%s'\n\n",
				args[i] );

			return 1;
		}

		opts[res] = 1;
	}

	tot_matches = 0;
	find_mode = 0;

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

	if ( opts[2] )
	{
		double		value;


		if ( mtkit_strtod ( args[0], &value, NULL, 1 ) )
		{
			fprintf ( stderr, "Bad value - '%s'\n\n", args[0] );

			return 1;
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
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	int		booknum = 0;


	if (	mtkit_strtoi ( args[0], &booknum, NULL, 1 ) ||
		booknum < CEDCLI_FILE_MIN ||
		booknum > CEDCLI_FILE_MAX
		)
	{
		fprintf ( stderr, "Bad book number '%s'\n\n",
			args[0] );

		return 1;
	}

	state->booknum = booknum;

	return 0;
}

int jtf_set_cell (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedSheet	* sheet;
	int		r1,
			c1,
			r2,
			c2,
			res;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	res = cui_sheet_set_cell ( CEDCLI_FILE->cubook, sheet,
		sheet->prefs.cursor_r1, sheet->prefs.cursor_c1, args[0] );
	undo_report_updates ( res );

	update_changes_chores ( state, sheet );

	return 0;
}

int jtf_select (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedSheet	* sheet;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	if ( strcmp ( args[0], "all" ) == 0 )
	{
		int		row_max,
				col_max;


		// Select all

		if ( ced_sheet_get_geometry ( sheet, &row_max, &col_max ) )
		{
			fprintf ( stderr, "Unable to get sheet geometry.\n\n ");

			return 1;
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

			return 1;
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

			return 1;
		}

		sheet->prefs.cursor_r1 = r1.row_d;
		sheet->prefs.cursor_c1 = r1.col_d;
		sheet->prefs.cursor_r2 = r1.row_d;
		sheet->prefs.cursor_c2 = r1.col_d;
	}

	return 0;
}

int jtf_set_graph (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	if ( ! cui_graph_get ( CEDCLI_FILE->cubook->book, args[0] ) )
	{
		fprintf ( stderr, "No such graph.\n\n" );

		return 1;
	}

	if ( mtkit_strfreedup ( &CEDCLI_FILE->cubook->book->prefs.active_graph,
		args[0] ) )
	{
		fprintf ( stderr, "Unable to change graph.\n\n" );

		return 1;
	}

	return 0;
}

int jtf_set_sheet (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	if ( ! ced_book_get_sheet ( CEDCLI_FILE->cubook->book, args[0] ) )
	{
		fprintf ( stderr, "No such sheet.\n\n" );

		return 1;
	}

	if ( mtkit_strfreedup ( &CEDCLI_FILE->cubook->book->prefs.active_sheet,
		args[0] ) )
	{
		fprintf ( stderr, "Unable to change sheet.\n\n" );

		return 1;
	}

	return 0;
}

int jtf_set_width (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedSheet	* sheet;
	int		r1,
			c1,
			r2,
			c2,
			ctot,
			res;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );
	ctot = c2 - c1 + 1;

	if ( strcmp ( "auto", args[0] ) == 0 )
	{
		res = cui_sheet_set_column_width_auto ( CEDCLI_FILE->cubook,
			sheet, c1, ctot );
	}
	else
	{
		int	w;


		if ( mtKit::cli_parse_int ( args[0], w,
			CED_MIN_COLUMN_WIDTH, CED_MAX_COLUMN_WIDTH )
			)
		{
			return 1;
		}

		res = cui_sheet_set_column_width ( CEDCLI_FILE->cubook, sheet,
			c1, ctot, w );
	}

	undo_report_updates ( res );

	if ( res )
	{
		return 1;
	}

	return 0;
}

int jtf_undo (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	int		res;
	CedSheet	* sheet;


	if ( ! CEDCLI_FILE->cubook->undo.undo_step )
	{
		fprintf ( stderr, "No undo available.\n\n" );

		return 1;
	}

	res = cui_book_undo_step ( CEDCLI_FILE->cubook );
	undo_report_updates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return 1;
	}

	sheet = cui_file_get_sheet ( CEDCLI_FILE );
	update_changes_chores ( state, sheet );

	return 0;
}

int jtf_redo (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	int		res;
	CedSheet	* sheet;


	if ( ! CEDCLI_FILE->cubook->undo.redo_step )
	{
		fprintf ( stderr, "No redo available.\n\n" );

		return 1;
	}

	res = cui_book_redo_step ( CEDCLI_FILE->cubook );
	undo_report_updates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return 1;
	}

	sheet = cui_file_get_sheet ( CEDCLI_FILE );
	update_changes_chores ( state, sheet );

	return 0;
}

int jtf_recalc_book (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	recalc_book_core ( state );

	return 0;
}

int jtf_recalc_sheet (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	CedSheet	* sheet;


	sheet = cui_file_get_sheet ( CEDCLI_FILE );
	if ( ! sheet )
	{
		return 1;
	}

	recalc_sheet_core ( sheet );

	return 0;
}

int undo_report_updates (
	int		const	error
	)
{
	if ( error < 0 )
	{
		static char const * mes[] = { "?",
		"Error during operation.",
		"Unable to begin operation due to problem with undo system.",
		"Undo history lost.",
		"Undo history lost.  Possible data corruption.",
		"cell locked.  Operation aborted.",
		"Sheet locked.  Operation aborted."
				};

		char	const	* msg;
		size_t		mi;


		mi = ((size_t) -error );
		if ( mi >= sizeof ( mes ) / sizeof ( msg[0] ) )
		{
			mi = 0;
		}

		msg = mes[mi];

		if ( error == CUI_ERROR_LOCKED_CELL )
		{
			static char	buf[2048];


			snprintf ( buf, sizeof ( buf ), "%s %s",
				cui_error_str (), msg );

			msg = buf;
		}

		fprintf ( stderr, "Undo Error: %s\n\n", msg );
	}

	return error;
}

void update_changes_chores (
	CedCli_STATE	* const	state,
	CedSheet	* const	sheet
	)
{
	if ( sheet && CEDCLI_FILE->cubook->book->prefs.auto_recalc )
	{
		// Always update sheet (must be done before book updates)
		recalc_sheet_core ( sheet );

		switch ( CEDCLI_FILE->cubook->book->prefs.auto_recalc )
		{
		case 2:
			recalc_book_core ( state );
			break;
		}
	}
}

CedSheet * cedcli_get_sheet (
	CedCli_STATE	* const	state
	)
{
	CedSheet	* sheet;


	sheet = cui_file_get_sheet ( CEDCLI_FILE );
	if ( ! sheet )
	{
		fprintf ( stderr, "No sheet available.\n\n" );
	}

	return sheet;
}

CedBookFile * cedcli_get_graph (
	CedCli_STATE	* const	state
	)
{
	CedBookFile	* graph;


	graph = cui_file_get_graph ( CEDCLI_FILE );
	if ( ! graph )
	{
		fprintf ( stderr, "No graph available.\n\n" );
	}

	return graph;
}

