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



/*
	The general pattern of a command line is:

	<VERB> [NOUN]... [ARG]...	|
	<NOUN> [VERB]... [ARG]...
*/

static mtCliItem const jump_table[] =
{
	{ "about",		jtf_about, 0, NULL },
	{ "clear content",	jtf_clear_content, 0, NULL },
	{ "clear prefs",	jtf_clear_prefs, 0, NULL },
	{ "clear",		jtf_clear, 0, NULL },
	{ "clip flip_h",	jtf_clip_flip_h, 0, NULL },
	{ "clip flip_v",	jtf_clip_flip_v, 0, NULL },
	{ "clip load",		jtf_clip_load, 1, "<OS FILENAME>" },
	{ "clip save",		jtf_clip_save, 1, "<OS FILENAME>" },
	{ "clip rotate_a",	jtf_clip_rotate_a, 0, NULL },
	{ "clip rotate_c",	jtf_clip_rotate_c, 0, NULL },
	{ "clip transpose",	jtf_clip_transpose, 0, NULL },
	{ "copy output",	jtf_copy_output, 0, NULL },
	{ "copy values",	jtf_copy_values, 0, NULL },
	{ "copy",		jtf_copy, 0, NULL },
	{ "cut",		jtf_cut, 0, NULL },
	{ "delete column",	jtf_delete_column, 0, NULL },
	{ "delete graph",	jtf_delete_graph, 0, NULL },
	{ "delete row",		jtf_delete_row, 0, NULL },
	{ "delete sheet",	jtf_delete_sheet, 0, NULL },
	{ "duplicate sheet",	jtf_duplicate_sheet, 0, NULL },
	{ "export graph",	jtf_export_graph, 1, "<OS FILENAME>" },
	{ "export output graph", jtf_export_output_graph, 2, "<OS FILENAME> <FILETYPE>" },
	{ "export output sheet", jtf_export_output_sheet, 2, "<OS FILENAME> <FILETYPE>" },
	{ "export sheet",	jtf_export_sheet, 2, "<OS FILENAME> <FILETYPE>" },
	{ "find",		jtf_find, -2, "<TEXT> [wild] [case] [value] [all]" },
	{ "help",		jtf_help, -1, "[ARG]..." },
	{ "import book",	jtf_import_book, -2, "<OS FILENAME> [csv | tsv]" },
	{ "import graph",	jtf_import_graph, 2, "<GRAPH NAME> <OS FILENAME>" },
	{ "info",		jtf_info, 0, NULL },
	{ "insert column",	jtf_insert_column, -1, "[clip]" },
	{ "insert row",		jtf_insert_row, -1, "[clip]" },
	{ "list files",		jtf_list_files, 0, NULL },
	{ "list graphs",	jtf_list_graphs, 0, NULL },
	{ "list sheets",	jtf_list_sheets, 0, NULL },
	{ "load",		jtf_load, -2, "<OS FILENAME> [csv | tsv]" },
	{ "new book",		jtf_new_book, 0, NULL },
	{ "new sheet",		jtf_new_sheet, 0, NULL },
	{ "new",		jtf_new_book, 0, NULL },
	{ "paste content",	jtf_paste_content, 0, NULL },
	{ "paste prefs",	jtf_paste_prefs, 0, NULL },
	{ "paste",		jtf_paste, 0, NULL },
	{ "print cell num",	jtf_print_cell_num, 0, NULL },
	{ "print cell text",	jtf_print_cell_text, 0, NULL },
	{ "print cell type",	jtf_print_cell_type, 0, NULL },
	{ "print prefs book",	jtf_print_prefs_book, 0, NULL },
	{ "print prefs cell",	jtf_print_prefs_cell, 0, NULL },
	{ "print prefs sheet",	jtf_print_prefs_sheet, 0, NULL },
	{ "print prefs state",	jtf_print_prefs_state, 0, NULL },
	{ "print",		jtf_print, 0, NULL },
	{ "q",			jtf_quit, 0, NULL },
	{ "quit",		jtf_quit, 0, NULL },
	{ "recalc book",	jtf_recalc_book, 0, NULL },
	{ "recalc sheet",	jtf_recalc_sheet, 0, NULL },
	{ "recalc",		jtf_recalc_sheet, 0, NULL },
	{ "redo",		jtf_redo, 0, NULL },
	{ "rename graph",	jtf_rename_graph, 1, "<NEW NAME>" },
	{ "rename sheet",	jtf_rename_sheet, 1, "<NEW NAME>" },
	{ "save",		jtf_save, 0, NULL },
	{ "save as",		jtf_save_as, -2, "<OS FILENAME> [FILETYPE]" },
	{ "select",		jtf_select, 1, "< all | CELLREF[:CELLREF] >" },
	{ "set 2dyear",		jtf_set_2dyear, -1, "[YEAR START]" },
	{ "set book",		jtf_set_book, 1, "<INTEGER - BOOK NUMBER 0-4>" },
	{ "set cell",		jtf_set_cell, 1, "<CELL CONTENT>" },
	{ "set graph",		jtf_set_graph, 1, "<GRAPH NAME>" },
	{ "set prefs book",	jtf_set_prefs_book, 2, "<KEY> <DATA>" },
	{ "set prefs cell",	jtf_set_prefs_cell, 2, "<KEY> <DATA>" },
	{ "set prefs cellborder", jtf_set_prefs_cellborder, 1, "<DATA>" },
	{ "set prefs sheet",	jtf_set_prefs_sheet, 2, "<KEY> <DATA>" },
	{ "set prefs state",	jtf_set_prefs_state, 2, "<KEY> <DATA>" },
	{ "set sheet",		jtf_set_sheet, 1, "<SHEET NAME>" },
	{ "set width",		jtf_set_width, 1, "< auto | INTEGER >" },
	{ "sort column",	jtf_sort_column, 1, "<EXPRESSION>" },
	{ "sort row",		jtf_sort_row, 1, "<EXPRESSION>" },
	{ "undo",		jtf_undo, 0, NULL },
	{ NULL, NULL, 0, NULL }
};



int cedcli_jumptable_init (
	CedCli_STATE	* const	state
	)
{
	state->clitab = mtkit_cli_new ( jump_table );
	if ( ! state->clitab )
	{
		state->exit = 1;
		fprintf ( stderr, "Unable to set up jump table\n\n" );

		return 1;
	}

	return 0;
}

int cedcli_jumptable_free (
	CedCli_STATE	* const	state
	)
{
	mtkit_cli_free ( state->clitab );
	state->clitab = NULL;

	return 0;
}

static int parse_as_formula (
	char	const	*	input,
	CedCli_STATE	* const	state
	)
{
	CedParser	parser_state;
	CedSheet	* sheet;



	while ( input[0] == '=' )
	{
		input ++;
	}

	sheet = cui_file_get_sheet ( CEDCLI_FILE );
	parser_state = ced_sheet_parse_text ( sheet, 1, 1, input, NULL );

	if (	parser_state.ced_errno ||
		( parser_state.flag & CED_PARSER_FLAG_ERROR )
		)
	{
		int		i;


		printf ( "errno=%i flag=%i sp=%i\n%s = %f\n",
			parser_state.ced_errno, parser_state.flag,
			parser_state.sp, input, parser_state.data );

		for ( i = 0; i < (parser_state.sp - 1); i++ )
		{
			printf ( " " );
		}

		printf ( "^\n\n" );
	}
	else
	{
		printf ( CED_PRINTF_NUM"\n", parser_state.data );
	}

	return 0;
}

int cedcli_parse (
	char	const	* const	input,
	CedCli_STATE	* const	state
	)
{
	int		res = 0;
	char		** args;


	if ( ! input || ! state )
	{
		return 1;
	}

	if (	isdigit ( input[0] ) ||
		isspace ( input[0] ) ||
		input[0] == '='
		)
	{
		return parse_as_formula ( input, state );
	}
	else if ( input[0] == '#' )
	{
		return 0;
	}

	args = mtkit_string_argv ( input );
	if ( ! args )
	{
		fprintf ( stderr,
			"cedcli_parse failure: unable to create argv\n\n" );

		return 1;
	}

	if ( args[0] )
	{
		int		err = 0,
				ncarg = 0;
		mtCliItem const	* match;


		match = mtkit_cli_match ( state->clitab, args, &err, &ncarg );

		if ( ! match )
		{
			if ( err >= 0 )
			{
				fprintf ( stderr,
					"Unknown command: %s\n\n",
					args[err - 1] );
			}
			else if ( err == -1 )
			{
				fprintf ( stderr, "Too few commands.\n\n" );
			}
			else
			{
				fprintf ( stderr,
					"Unknown error after no command match"
					" : %i.\n\n", err );
			}
		}
		else
		{
			// Match was made
			if ( err == 0 )
			{
				jtFunc		func;


				func = (jtFunc)match->func;
				res = func ( state, args + ncarg );
			}
			else if ( err == -1 )
			{
				fprintf ( stderr, "Too few arguments.\n\n" );
			}
			else if ( err == -2 )
			{
				fprintf ( stderr, "Too many arguments.\n\n" );
			}
			else
			{
				fprintf ( stderr, "Unknown error after"
					" matching command : %i.\n\n", err );
			}
		}
	}

	mtkit_string_argv_free ( args );

	return res;
}

