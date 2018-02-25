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

#include <readline/readline.h>
#include <readline/history.h>



Backend::Backend ()
	:
	m_file_p	(),
	m_file		(),
	m_clipboard	(),
	m_prefs		()
{
	ced_init ();

	for ( int i = 0; i < FILE_TOTAL; i++ )
	{
		m_file[i] = cui_file_new ();

		if (	! m_file[i]			||
			cui_file_book_new ( m_file[i] )
			)
		{
			goto error;
		}
	}

	m_clipboard = cui_clip_new ();

	if (	! m_clipboard	||
		prefs_init ()	||
		set_file ( 0 )
		)
	{
		goto error;
	}

	return;

error:
	exit.set_value ( 1 );
	exit.abort ();

	fprintf ( stderr, "Error: Unable to initialize backend.\n");
}

Backend::~Backend ()
{
	cui_clip_free ( m_clipboard );
	m_clipboard = NULL;

	for ( int i = 0; i < FILE_TOTAL; i++ )
	{
		cui_file_free ( m_file[i] );
		m_file[i] = NULL;
	}

	prefs_free ();
}

static int error_func (
	int		const	error,
	int		const	arg,
	int		const	argc,
	char	const * const	argv[],
	void		* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "error_func: Argument ERROR! - num=%i arg=%i/%i",
		error, arg, argc );

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n" );

	return 0;		// Keep parsing
}

int Backend::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	int	show_version	= 0;
	int	show_about	= 1;
	int	tab_text	= 0;

	mtArg	const	arg_list[]	= {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ "q",		MTKIT_ARG_SWITCH, &show_about, 0, NULL },
		{ "t",		MTKIT_ARG_SWITCH, &tab_text, 1, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};


	mtkit_arg_parse ( argc, argv, arg_list, NULL, error_func, NULL );

	switch ( show_version )
	{
	case 1:
		printf ( "%s\n\n", VERSION );

		return 1;		// Quit program

	case 2:
		printf (
		"%s\n\n"
		"For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n", VERSION, BIN_NAME );

		return 1;		// Quit program
	}

	if ( show_about )
	{
		print_about ();
	}

	rl_variable_bind ( "expand-tilde", "on" );

	if ( tab_text )
	{
		rl_bind_key ( '\t', rl_insert );
	}

	return 0;			// Continue program
}

int Backend::init_table ()
{
	if (	0
		|| m_clitab.add_item ( "about", jtf_about )
		|| m_clitab.add_item ( "clear", jtf_clear )
		|| m_clitab.add_item ( "clear content", jtf_clear_content )
		|| m_clitab.add_item ( "clear prefs", jtf_clear_prefs )
		|| m_clitab.add_item ( "clip flip_h", jtf_clip_flip_h )
		|| m_clitab.add_item ( "clip flip_v", jtf_clip_flip_v )
		|| m_clitab.add_item ( "clip load", jtf_clip_load, 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "clip save", jtf_clip_save, 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "clip rotate_a", jtf_clip_rotate_a )
		|| m_clitab.add_item ( "clip rotate_c", jtf_clip_rotate_c )
		|| m_clitab.add_item ( "clip transpose", jtf_clip_transpose )
		|| m_clitab.add_item ( "copy", jtf_copy )
		|| m_clitab.add_item ( "copy output", jtf_copy_output )
		|| m_clitab.add_item ( "copy values", jtf_copy_values )
		|| m_clitab.add_item ( "cut", jtf_cut )
		|| m_clitab.add_item ( "delete column", jtf_delete_column )
		|| m_clitab.add_item ( "delete graph", jtf_delete_graph )
		|| m_clitab.add_item ( "delete row", jtf_delete_row )
		|| m_clitab.add_item ( "delete sheet", jtf_delete_sheet )
		|| m_clitab.add_item ( "duplicate sheet", jtf_duplicate_sheet )
		|| m_clitab.add_item ( "export graph", jtf_export_graph, 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "export output graph",
			jtf_export_output_graph, 2, 2,
			"<OS FILENAME> <FILETYPE>" )
		|| m_clitab.add_item ( "export output sheet",
			jtf_export_output_sheet, 2, 2,
			"<OS FILENAME> <FILETYPE>" )
		|| m_clitab.add_item ( "export sheet", jtf_export_sheet, 2, 2,
			"<OS FILENAME> <FILETYPE>" )
		|| m_clitab.add_item ( "find", jtf_find, 1, 5,
			"<TEXT> [wild] [case] [value] [all]" )
		|| m_clitab.add_item ( "help", jtf_help, 0, 100, "[ARG]..." )
		|| m_clitab.add_item ( "import book", jtf_import_book, 1, 2,
			"<OS FILENAME> [csv | tsv]" )
		|| m_clitab.add_item ( "import graph", jtf_import_graph, 2, 2,
			"<GRAPH NAME> <OS FILENAME>" )
		|| m_clitab.add_item ( "info", jtf_info )
		|| m_clitab.add_item ( "insert column", jtf_insert_column, 0, 1,
			"[clip]" )
		|| m_clitab.add_item ( "insert row", jtf_insert_row, 0, 1,
			"[clip]" )
		|| m_clitab.add_item ( "list files", jtf_list_files )
		|| m_clitab.add_item ( "list graphs", jtf_list_graphs )
		|| m_clitab.add_item ( "list sheets", jtf_list_sheets )
		|| m_clitab.add_item ( "load", jtf_load, 1, 2,
			"<OS FILENAME> [csv | tsv]" )
		|| m_clitab.add_item ( "new", jtf_new_book )
		|| m_clitab.add_item ( "new book", jtf_new_book )
		|| m_clitab.add_item ( "new sheet", jtf_new_sheet )
		|| m_clitab.add_item ( "paste", jtf_paste )
		|| m_clitab.add_item ( "paste content", jtf_paste_content )
		|| m_clitab.add_item ( "paste prefs", jtf_paste_prefs )
		|| m_clitab.add_item ( "print", jtf_print )
		|| m_clitab.add_item ( "print cell num", jtf_print_cell_num )
		|| m_clitab.add_item ( "print cell text", jtf_print_cell_text )
		|| m_clitab.add_item ( "print cell type", jtf_print_cell_type )
		|| m_clitab.add_item ( "print prefs book", jtf_print_prefs_book)
		|| m_clitab.add_item ( "print prefs cell", jtf_print_prefs_cell)
		|| m_clitab.add_item ( "print prefs sheet",
			jtf_print_prefs_sheet )
		|| m_clitab.add_item ( "print prefs state",
			jtf_print_prefs_state )
		|| m_clitab.add_item ( "q", jtf_quit )
		|| m_clitab.add_item ( "quit", jtf_quit )
		|| m_clitab.add_item ( "recalc", jtf_recalc_sheet )
		|| m_clitab.add_item ( "recalc book", jtf_recalc_book )
		|| m_clitab.add_item ( "recalc sheet", jtf_recalc_sheet )
		|| m_clitab.add_item ( "redo", jtf_redo )
		|| m_clitab.add_item ( "rename graph", jtf_rename_graph, 1, 1,
			"<NEW NAME>" )
		|| m_clitab.add_item ( "rename sheet", jtf_rename_sheet, 1, 1,
			"<NEW NAME>" )
		|| m_clitab.add_item ( "save", jtf_save )
		|| m_clitab.add_item ( "save as", jtf_save_as, 1, 2,
			"<OS FILENAME> [FILETYPE]" )
		|| m_clitab.add_item ( "select", jtf_select, 1, 1,
			"< all | CELLREF[:CELLREF] >" )
		|| m_clitab.add_item ( "set 2dyear", jtf_set_2dyear, 0, 1,
			"[YEAR START]" )
		|| m_clitab.add_item ( "set book", jtf_set_book, 1, 1,
			"<INTEGER - BOOK NUMBER 0-4>" )
		|| m_clitab.add_item ( "set cell", jtf_set_cell, 1, 1,
			"<CELL CONTENT>" )
		|| m_clitab.add_item ( "set graph", jtf_set_graph, 1, 1,
			"<GRAPH NAME>" )
		|| m_clitab.add_item ( "set prefs book", jtf_set_prefs_book,
			2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set prefs cell", jtf_set_prefs_cell,
			2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set prefs cellborder",
			jtf_set_prefs_cellborder, 1, 1, "<DATA>" )
		|| m_clitab.add_item ( "set prefs sheet", jtf_set_prefs_sheet,
			2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set prefs state", jtf_set_prefs_state,
			2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set sheet", jtf_set_sheet, 1, 1,
			"<SHEET NAME>" )
		|| m_clitab.add_item ( "set width", jtf_set_width, 1, 1,
			"< auto | INTEGER >" )
		|| m_clitab.add_item ( "sort column", jtf_sort_column, 1, 1,
			"<EXPRESSION>" )
		|| m_clitab.add_item ( "sort row", jtf_sort_row, 1, 1,
			"<EXPRESSION>" )
		|| m_clitab.add_item ( "undo", jtf_undo )
		)
	{
		return 1;
	}

	return 0;
}

static void parse_as_formula (
	char		const * input,
	CedSheet	* const	sheet
	)
{
	CedParser	parser_state;


	while ( input[0] == '=' )
	{
		input ++;
	}

	parser_state = ced_sheet_parse_text ( sheet, 1, 1, input, NULL );

	if (	parser_state.ced_errno ||
		( parser_state.flag & CED_PARSER_FLAG_ERROR )
		)
	{
		printf ( "errno=%i flag=%i sp=%i\n%s = %f\n",
			parser_state.ced_errno, parser_state.flag,
			parser_state.sp, input, parser_state.data );

		for ( int i = 0; i < (parser_state.sp - 1); i++ )
		{
			printf ( " " );
		}

		printf ( "^\n\n" );
	}
	else
	{
		printf ( CED_PRINTF_NUM"\n", parser_state.data );
	}
}

void Backend::main_loop ()
{
	if ( init_table () )
	{
		exit.abort ();
		exit.set_value ( 1 );

		fprintf ( stderr, "Error: main_loop().init_table()\n" );

		return;
	}

	while ( false == exit.aborted () )
	{
		char * line = readline ( BIN_NAME" > " );

		if ( ! line )
		{
			break;
		}

		if ( line[0] )
		{
			add_history ( line );
		}

		if (	isdigit ( line[0] ) ||
			isspace ( line[0] ) ||
			line[0] == '='
			)
		{
			parse_as_formula ( line, cui_file_get_sheet ( file ()));
		}
		else if ( line[0] == '#' )
		{
			// Comment
		}
		else
		{
			m_clitab.parse ( line );
		}

		free ( line );
	}

	clear_history ();
}

int Backend::get_help (
	char	const * const * const	argv
	) const
{
	return m_clitab.print_help ( argv );
}

int Backend::undo_report_updates (
	int	const	error
	)
{
	if ( error < 0 )
	{
		char const * mes[] = { "?",
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

void Backend::recalc_sheet_core ()
{
	CedSheet	* const	s = sheet ();

	if ( s )
	{
		ced_sheet_recalculate ( s, NULL, 0 );
		ced_sheet_recalculate ( s, NULL, 1 );
	}
}

void Backend::recalc_book_core ()
{
	ced_book_recalculate ( m_file_p->cubook->book, 0 );
	ced_book_recalculate ( m_file_p->cubook->book, 1 );
}

void Backend::update_changes_chores ()
{
	if ( sheet () && m_file_p->cubook->book->prefs.auto_recalc )
	{
		// Always update sheet (must be done before book updates)
		recalc_sheet_core ();

		switch ( m_file_p->cubook->book->prefs.auto_recalc )
		{
		case 2:
			recalc_book_core ();
			break;
		}
	}
}

CedSheet * Backend::sheet (
	bool	const	print_error
	)
{
	CedSheet	* s;


	s = cui_file_get_sheet ( m_file_p );
	if ( ! s && print_error )
	{
		fprintf ( stderr, "No sheet available.\n\n" );
	}

	return s;
}

CedBookFile * Backend::graph (
	bool	const	print_error
	)
{
	CedBookFile	* g;


	g = cui_file_get_graph ( m_file_p );
	if ( ! g && print_error )
	{
		fprintf ( stderr, "No graph available.\n\n" );
	}

	return g;
}

int Backend::set_file (
	int	const	n
	)
{
	if ( n < 0 || n >= FILE_TOTAL )
	{
		return 1;
	}

	m_file_p = m_file[n];

	return 0;
}

int Backend::get_file_number () const
{
	for ( int n = 0; n < FILE_TOTAL; n++ )
	{
		if ( m_file_p == m_file[n] )
		{
			return n;
		}
	}

	return -1;
}

void Backend::print_about ()
{
	printf ( "%s\n"
		"Copyright (C) " MT_COPYRIGHT_YEARS " Mark Tyler\n"
		"Type 'help' for command hints.  Read the manual for more "
		"specific info.\n\n", VERSION );
}

