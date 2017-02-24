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



int jtf_delete_graph (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	CedBook		* book = CEDCLI_FILE->cubook->book;


	if ( ! cedcli_get_graph ( state ) )
	{
		return 1;
	}

	if ( cui_graph_destroy ( book, book->prefs.active_graph ) )
	{
		fprintf ( stderr, "Unable to delete graph\n\n" );

		return 1;
	}

	cui_file_get_graph ( CEDCLI_FILE );	// Sets active_graph for us

	return 0;
}

int jtf_delete_sheet (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	int		res;
	CedSheet	* sheet;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	res = cui_book_destroy_sheet ( CEDCLI_FILE->cubook,
		(char const *)sheet->book_tnode->key );
	undo_report_updates ( res );

	if ( res )
	{
		return 1;
	}

	return 0;
}

int jtf_duplicate_sheet (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	int		res;
	CedSheet	* sheet,
			* ns;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	res = cui_book_duplicate_sheet ( CEDCLI_FILE->cubook, sheet, &ns );
	undo_report_updates ( res );

	if ( res )
	{
		return 1;
	}

	mtkit_strfreedup ( &CEDCLI_FILE->cubook->book->prefs.active_sheet,
		(char const *)ns->book_tnode->key );

	return 0;
}

static int prep_insert (
	CedCli_STATE	* const	state,
	char		** const args,
	CedSheet	* const	sheet,
	int		* const	r1,
	int		* const	c1,
	int		* const	r2,
	int		* const	c2
	)
{
	ced_sheet_cursor_max_min ( sheet, r1, c1, r2, c2 );

	if ( args[0] )
	{
		if ( strcmp ( "clip", args[0] ) != 0 )
		{
			fprintf ( stderr, "Argument not recognised.\n\n" );

			return 1;
		}

		if ( ! state->clipboard->sheet )
		{
			fprintf ( stderr, "No clipboard available.\n\n" );

			return 1;
		}

		r2[0] = r1[0] + state->clipboard->rows - 1;
		c2[0] = c1[0] + state->clipboard->cols - 1;
	}

	return 0;
}

int jtf_insert_column (
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

	if ( prep_insert ( state, args, sheet, &r1, &c1, &r2, &c2 ) )
	{
		return 1;
	}

	res = cui_sheet_insert_column ( CEDCLI_FILE->cubook, sheet,
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

int jtf_insert_row (
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

	if ( prep_insert ( state, args, sheet, &r1, &c1, &r2, &c2 ) )
	{
		return 1;
	}

	res = cui_sheet_insert_row ( CEDCLI_FILE->cubook, sheet,
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

int jtf_new_book (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( cui_file_book_new ( CEDCLI_FILE ) )
	{
		fprintf ( stderr, "Unable to create new book.\n\n" );

		return 1;
	}

	return 0;
}

int jtf_new_sheet (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	int		res;


	res = cui_file_sheet_add ( CEDCLI_FILE );
	if ( res == 1 )
	{
		fprintf ( stderr, "Unable to create a new sheet.\n\n" );
		return 1;
	}

	undo_report_updates ( res );

	if ( res )
	{
		return 1;
	}

	return 0;
}

int jtf_rename_graph (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedBookFile	* old;
	CedBook		* book = CEDCLI_FILE->cubook->book;


	old = cedcli_get_graph ( state );
	if ( ! old )
	{
		return 1;
	}

	if ( cui_graph_get ( book, args[0] ) )
	{
		fprintf ( stderr, "Graph name already exists.\n\n" );

		return 1;
	}

	if ( ! cui_graph_new ( book, old->mem, old->size, args[0] ) )
	{
		fprintf ( stderr, "Unable to rename graph" );

		return 1;
	}

	old->mem = NULL;
	old->size = 0;

	cui_graph_destroy ( book, book->prefs.active_graph );
	mtkit_strfreedup ( &book->prefs.active_graph, args[0] );

	return 0;
}

int jtf_rename_sheet (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	int		res;
	CedSheet	* sheet;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	res = cui_book_page_rename ( CEDCLI_FILE->cubook, sheet, args[0] );
	undo_report_updates ( res );

	if ( res )
	{
		return 1;
	}

	return 0;
}

int jtf_set_2dyear (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedSheet	* sheet;
	int		res,
			year;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	if ( args[0] )
	{
		if ( mtKit::cli_parse_int ( args[0], year,
			MTKIT_DDT_MIN_DATE_YEAR, MTKIT_DDT_MAX_DATE_YEAR ) )
		{
			return 1;
		}
	}
	else
	{
		struct tm	* tm_now;
		time_t		now;


		now = time ( NULL );
		tm_now = localtime ( &now );

		year = 1900 + tm_now->tm_year - 50;
		if (	year < MTKIT_DDT_MIN_DATE_YEAR ||
			year > MTKIT_DDT_MAX_DATE_YEAR
			)
		{
			// Should never happen, but just in case
			fprintf ( stderr, "Unexpected error in system date.\n"
				);

			return 1;
		}
	}

	res = cui_sheet_2dyear ( CEDCLI_FILE, year );

	if ( res > 0 )
	{
		fprintf ( stderr, "Unable to fix years.\n" );
	}

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL	||
		res == CUI_ERROR_LOCKED_SHEET	||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;			// Nothing changed
	}

	update_changes_chores ( state, sheet );

	return 0;
}

