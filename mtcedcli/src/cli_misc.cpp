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



int jtf_about (
	CedCli_STATE	* const	ARG_UNUSED ( state ),
	char		** const ARG_UNUSED ( args )
	)
{
	printf ( ABOUT_PRINTF );

	return 0;
}

int jtf_help (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	mtkit_cli_help ( state->clitab, args );

	return 0;
}

int jtf_info (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	char	const	* txt_sheet = "";
	char	const	* txt_graph = "";
	char	const	* txt_filename = "";
	char	const	* txt_filetype = "";
	char		txt_rc[128] = {0};
	CedSheet	* sheet;
	CedBookFile	* graph;


	sheet = cui_file_get_sheet ( CEDCLI_FILE );
	if ( sheet )
	{
		int	r1, r2, c1, c2;


		ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

		if ( r1 == r2 && c1 == c2 )
		{
			snprintf ( txt_rc, sizeof ( txt_rc ), "r%ic%i",
				r1, c1 );
		}
		else
		{
			snprintf ( txt_rc, sizeof ( txt_rc ), "r%ic%i:r%ic%i",
				r1, c1, r2, c2 );
		}

		txt_sheet = CEDCLI_FILE->cubook->book->prefs.active_sheet;
	}

	graph = cui_file_get_graph ( CEDCLI_FILE );
	if ( graph )
	{
		txt_graph = CEDCLI_FILE->cubook->book->prefs.active_graph;
	}

	if ( CEDCLI_FILE->name )
	{
		txt_filename = CEDCLI_FILE->name;
	}

	txt_filetype = ced_file_type_text ( CEDCLI_FILE->type );

	printf ( "Book #=%i "
		"Sheet Name='%s' "
		"Graph Name='%s' "
		"Cursor=%s "
		"Filename='%s' "
		"File Type=%s"
		"\n\n", state->booknum, txt_sheet, txt_graph, txt_rc,
		txt_filename, txt_filetype );

	return 0;
}

int jtf_quit (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	state->exit = CEDCLI_USER_QUIT;

	return 0;
}

static int cb_list_files (
	mtTreeNode	* const	node,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	CedBookFile	* bookfile = (CedBookFile *)node->data;


	printf ( "%8i  %04i-%02i-%02i %02i:%02i:%02i  %s\n", bookfile->size,
		bookfile->timestamp[0], bookfile->timestamp[1],
		bookfile->timestamp[2], bookfile->timestamp[3],
		bookfile->timestamp[4], bookfile->timestamp[5],
		(char *)node->key );

	return 0;		// Continue
}


int jtf_list_files (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	mtkit_tree_scan ( CEDCLI_FILE->cubook->book->files, cb_list_files,
		NULL, 0 );

	puts ( "" );

	return 0;
}

static int cb_list_graphs (
	CedBook		* const	ARG_UNUSED ( book ),
	char	const	* const	graph_name,
	CedBookFile	* const	ARG_UNUSED ( bookfile ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	puts ( graph_name );

	return 0;		// Continue
}

int jtf_list_graphs (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	cui_graph_scan ( CEDCLI_FILE->cubook->book, cb_list_graphs, NULL );
	puts ( "" );

	return 0;
}

static int cb_list_sheets (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	char	const	* const	name,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	puts ( name );

	return 0;	// Continue
}

int jtf_list_sheets (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	ced_book_scan ( CEDCLI_FILE->cubook->book, cb_list_sheets, NULL );
	puts ( "" );

	return 0;
}


static void cb_space_output (
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	int		* const rc = (int *)user_data;


	if ( row > rc[0] )
	{
		while ( row > rc[0] )
		{
			putchar ( '\n' );
			rc[0] ++;
		}

		rc[1] = rc[2];
	}

	while ( col > rc[1] )
	{
		putchar ( '\t' );
		rc[1] ++;
	}
}

static int cb_scan_num (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	cb_space_output ( row, col, user_data );
	printf ( CED_PRINTF_NUM, cell->value );

	return 0;		// Continue
}

static int cb_scan_text (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	if ( ! cell->text )
	{
		return 0;
	}

	cb_space_output ( row, col, user_data );
	printf ( "%s", cell->text );

	return 0;		// Continue
}

static int cb_scan_type (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	cb_space_output ( row, col, user_data );
	printf ( "%i", (int)cell->type );

	return 0;		// Continue
}

static int cb_scan_output (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	char		* txt;


	txt = ced_cell_create_output ( cell, NULL );
	if ( ! txt )
	{
		return 0;
	}

	cb_space_output ( row, col, user_data );
	printf ( "%s", txt );

	free ( txt );

	return 0;		// Continue
}

static int cb_scan_prefs (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( cell->prefs )
	{
		CedCellPrefs const * const def = ced_cell_prefs_default ();
		int		tot,
				i,
				list_i_cell[] = {
				cell->prefs->align_horizontal,
				cell->prefs->color_background,
				cell->prefs->color_foreground,
				cell->prefs->format,
				cell->prefs->width,
				cell->prefs->num_zeros,
				cell->prefs->num_zeros,
				cell->prefs->text_style,
				cell->prefs->locked,
				cell->prefs->border_type,
				cell->prefs->border_color
				},
				list_i_def[] = {
				def->align_horizontal,
				def->color_background,
				def->color_foreground,
				def->format,
				def->width,
				def->num_decimal_places,
				def->num_zeros,
				def->text_style,
				def->locked,
				def->border_type,
				def->border_color
				}
				;
		char		* list_c_cell[] = {
				cell->prefs->format_datetime,
				cell->prefs->num_thousands,
				cell->prefs->text_prefix,
				cell->prefs->text_suffix
				}
				;
		static char const * const list_i_name[] = {
				"align_horizontal",
				"color_background",
				"color_foreground",
				"format",
				"width",
				"num_decimal_places",
				"num_zeros",
				"text_style",
				"locked",
				"border_type",
				"border_color"
				};
		static char const * const list_c_name[] = {
				"format_datetime",
				"num_thousands",
				"text_prefix",
				"text_suffix"
				}
				;


		printf ( "r%ic%i\n", row, col );

		tot = sizeof ( list_i_cell ) / sizeof ( list_i_cell[0] );
		for ( i = 0; i < tot; i++ )
		{
			if ( list_i_cell[i] != list_i_def[i] )
			{
				printf ( "\t%s = %i\n",
					list_i_name[i], list_i_cell[i] );
			}
		}

		tot = sizeof ( list_c_cell ) / sizeof ( list_c_cell[0] );
		for ( i = 0; i < tot; i++ )
		{
			if ( list_c_cell[i] )
			{
				printf ( "\t%s = '%s'\n",
					list_c_name[i], list_c_cell[i] );
			}
		}
	}

	return 0;		// Continue
}

static int cb_scan (
	CedCli_STATE	* const	state,
	CedFuncScanArea	const	callback
	)
{
	CedSheet	* sheet;
	int		res,
			rc[3],
			r1,
			r2,
			c1,
			c2;


	sheet = cui_file_get_sheet ( CEDCLI_FILE );
	if ( ! sheet )
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	rc[0] = r1;
	rc[1] = c1;
	rc[2] = c1;

	res = ced_sheet_scan_area ( sheet, r1, c1, r2 - r1 + 1, c2 - c1 + 1,
		callback, rc );

	if ( res )
	{
		fprintf ( stderr, "Problem scanning sheet.\n\n" );

		return 1;
	}

	putchar ( '\n' );

	return 0;
}

int jtf_print_cell_num (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return cb_scan ( state, cb_scan_num );
}

int jtf_print_cell_text (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return cb_scan ( state, cb_scan_text );
}

int jtf_print_cell_type (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return cb_scan ( state, cb_scan_type );
}

int jtf_print (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return cb_scan ( state, cb_scan_output );
}

int jtf_print_prefs_cell (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return cb_scan ( state, cb_scan_prefs );
}

