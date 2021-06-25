/*
	Copyright (C) 2012-2020 Mark Tyler

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



int Backend::jtf_delete_graph (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( ! graph () )
	{
		return 2;
	}

	CedBook * const book = file()->cubook->book;

	if ( cui_graph_destroy ( book, book->prefs.active_graph ) )
	{
		fprintf ( stderr, "Unable to delete graph\n\n" );

		return 2;
	}

	graph ();		// Sets active_graph for us

	return 0;
}


int Backend::jtf_delete_sheet (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	CedSheet * const sh = this->sheet ();

	if ( ! sh )
	{
		return 2;
	}

	int const res = cui_book_destroy_sheet ( file()->cubook,
		(char const *)sh->book_tnode->key );

	undo_report_updates ( res );

	if ( res )
	{
		return 2;
	}

	return 0;
}

int Backend::jtf_duplicate_sheet (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	CedSheet * const sh = this->sheet ();

	if ( ! sh )
	{
		return 2;
	}

	CedSheet	* ns;
	int	const	res = cui_book_duplicate_sheet ( file()->cubook,
				sh, &ns );

	undo_report_updates ( res );

	if ( res )
	{
		return 2;
	}

	mtkit_strfreedup ( &file()->cubook->book->prefs.active_sheet,
		(char const *)ns->book_tnode->key );

	return 0;
}

CedSheet * Backend::prep_insert (
	char	const * const * const	args,
	int				&r1,
	int				&c1,
	int				&r2,
	int				&c2
	)
{
	CedSheet * const sh = this->sheet ();

	if ( ! sh )
	{
		return NULL;
	}

	ced_sheet_cursor_max_min ( sh, &r1, &c1, &r2, &c2 );

	if ( args[0] )
	{
		if ( strcmp ( "clip", args[0] ) != 0 )
		{
			fprintf ( stderr, "Argument not recognised.\n\n" );

			return NULL;
		}

		if ( ! clipboard()->sheet )
		{
			fprintf ( stderr, "No clipboard available.\n\n" );

			return NULL;
		}

		r2 = r1 + clipboard()->rows - 1;
		c2 = c1 + clipboard()->cols - 1;
	}

	return sh;
}

int Backend::jtf_insert_column (
	char	const * const * const	args
	)
{
	int		r1, c1, r2, c2;
	CedSheet * const sh = prep_insert ( args, r1, c1, r2, c2 );

	if ( ! sh )
	{
		return 2;
	}

	int const res = cui_sheet_insert_column ( file()->cubook, sh,
		c1, c2 - c1 + 1 );

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 2;
	}

	update_changes_chores ();

	return 0;
}

int Backend::jtf_insert_row (
	char	const * const * const	args
	)
{
	int		r1, c1, r2, c2;
	CedSheet * const sh = prep_insert ( args, r1, c1, r2, c2 );

	if ( ! sh )
	{
		return 2;
	}

	int const res = cui_sheet_insert_row ( file()->cubook, sh,
		r1, r2 - r1 + 1 );

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 2;
	}

	update_changes_chores ();

	return 0;
}

int Backend::jtf_new_book (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( cui_file_book_new ( file() ) )
	{
		fprintf ( stderr, "Unable to create new book.\n\n" );

		return 2;
	}

	return 0;
}

int Backend::jtf_new_sheet (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	int	const	res = cui_file_sheet_add ( file() );

	if ( res == 1 )
	{
		fprintf ( stderr, "Unable to create a new sheet.\n\n" );
		return 2;
	}

	undo_report_updates ( res );

	if ( res )
	{
		return 2;
	}

	return 0;
}

int Backend::jtf_rename_graph (
	char	const * const * const	args
	)
{
	CedBook		* const book = file()->cubook->book;
	CedBookFile	* const old = graph ();

	if ( ! old )
	{
		return 2;
	}

	if ( cui_graph_get ( book, args[0] ) )
	{
		fprintf ( stderr, "Graph name already exists.\n\n" );

		return 2;
	}

	if ( ! cui_graph_new ( book, old->mem, old->size, args[0] ) )
	{
		fprintf ( stderr, "Unable to rename graph" );

		return 2;
	}

	old->mem = NULL;
	old->size = 0;

	cui_graph_destroy ( book, book->prefs.active_graph );
	mtkit_strfreedup ( &book->prefs.active_graph, args[0] );

	return 0;
}

int Backend::jtf_rename_sheet (
	char	const * const * const	args
	)
{
	CedSheet * const sh = sheet ();

	if ( ! sh )
	{
		return 2;
	}

	int const res = cui_book_page_rename ( file()->cubook, sh, args[0] );

	undo_report_updates ( res );

	if ( res )
	{
		return 2;
	}

	return 0;
}

int Backend::jtf_set_2dyear (
	char	const * const * const	args
	)
{
	if ( ! sheet() )
	{
		return 2;
	}

	int		year;

	if ( args[0] )
	{
		if ( mtKit::cli_parse_int ( args[0], year,
			MTKIT_DDT_MIN_DATE_YEAR, MTKIT_DDT_MAX_DATE_YEAR ) )
		{
			return 2;
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

			return 2;
		}
	}

	int const res = cui_sheet_2dyear ( file(), year );

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
		return 2;			// Nothing changed
	}

	update_changes_chores ();

	return 0;
}

