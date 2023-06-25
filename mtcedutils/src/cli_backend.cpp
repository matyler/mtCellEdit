/*
	Copyright (C) 2012-2022 Mark Tyler

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

#include "cli.h"



Backend::Backend ()
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
	if ( ! m_clipboard )
	{
		goto error;
	}

	prefs_init ();

	if ( set_file ( 0 ) )
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
}

int Backend::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	int	show_version	= 0;
	int	show_about	= 1;
	int	tab_text	= 0;

	mtKit::Arg args;

	args.add ( "-help",	show_version, 2 );
	args.add ( "-version",	show_version, 1 );
	args.add ( "q",		show_about, 0 );
	args.add ( "t",		tab_text, 1 );

	args.parse ( argc, argv );

	if ( show_version )
	{
		printf ( "%s (part of %s)\n", BIN_NAME, VERSION );

		if ( 2 == show_version )
		{
			printf (
				"For further information consult the man page "
				"%s(1) or the mtCellEdit Handbook.\n"
				"\n", BIN_NAME );
		}

		return 1;		// Quit program
	}

	if ( show_about )
	{
		jtf_about ( NULL );
	}

	m_clishell.bind_tilde ();

	if ( tab_text )
	{
		m_clishell.bind_tab ();
	}

	return 0;			// Continue program
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



#define JTFUNC( X ) [this](char const * const * f ) { return jtf_ ## X (f); }



void Backend::main_loop ()
{
	if (	0
		|| m_clitab.add_item ( "about", JTFUNC(about) )
		|| m_clitab.add_item ( "clear", JTFUNC(clear) )
		|| m_clitab.add_item ( "clear content", JTFUNC(clear_content) )
		|| m_clitab.add_item ( "clear prefs", JTFUNC(clear_prefs) )
		|| m_clitab.add_item ( "clip flip_h", JTFUNC(clip_flip_h) )
		|| m_clitab.add_item ( "clip flip_v", JTFUNC(clip_flip_v) )
		|| m_clitab.add_item ( "clip load", JTFUNC(clip_load), 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "clip save", JTFUNC(clip_save), 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "clip rotate_a", JTFUNC(clip_rotate_a) )
		|| m_clitab.add_item ( "clip rotate_c", JTFUNC(clip_rotate_c) )
		|| m_clitab.add_item ( "clip transpose", JTFUNC(clip_transpose))
		|| m_clitab.add_item ( "copy", JTFUNC(copy) )
		|| m_clitab.add_item ( "copy output", JTFUNC(copy_output) )
		|| m_clitab.add_item ( "copy values", JTFUNC(copy_values) )
		|| m_clitab.add_item ( "cut", JTFUNC(cut) )
		|| m_clitab.add_item ( "delete column", JTFUNC(delete_column) )
		|| m_clitab.add_item ( "delete graph", JTFUNC(delete_graph) )
		|| m_clitab.add_item ( "delete row", JTFUNC(delete_row) )
		|| m_clitab.add_item ( "delete sheet", JTFUNC(delete_sheet) )
		|| m_clitab.add_item ( "duplicate sheet",
			JTFUNC(duplicate_sheet) )
		|| m_clitab.add_item ( "export graph", JTFUNC(export_graph),
			1, 1, "<OS FILENAME>" )
		|| m_clitab.add_item ( "export output graph",
			JTFUNC(export_output_graph), 2, 2,
			"<OS FILENAME> <FILETYPE>" )
		|| m_clitab.add_item ( "export output sheet",
			JTFUNC(export_output_sheet), 2, 2,
			"<OS FILENAME> <FILETYPE>" )
		|| m_clitab.add_item ( "export sheet", JTFUNC(export_sheet),
			2, 2, "<OS FILENAME> <FILETYPE>" )
		|| m_clitab.add_item ( "find", JTFUNC(find), 1, 5,
			"<TEXT> [wild] [case] [value] [all]" )
		|| m_clitab.add_item ( "help", JTFUNC(help), 0, 100, "[ARG]...")
		|| m_clitab.add_item ( "import book", JTFUNC(import_book), 1, 2,
			"<OS FILENAME> [csv | tsv]" )
		|| m_clitab.add_item ( "import graph", JTFUNC(import_graph),
			2, 2, "<GRAPH NAME> <OS FILENAME>" )
		|| m_clitab.add_item ( "info", JTFUNC(info) )
		|| m_clitab.add_item ( "insert column", JTFUNC(insert_column),
			0, 1, "[clip]" )
		|| m_clitab.add_item ( "insert row", JTFUNC(insert_row), 0, 1,
			"[clip]" )
		|| m_clitab.add_item ( "list files", JTFUNC(list_files) )
		|| m_clitab.add_item ( "list graphs", JTFUNC(list_graphs) )
		|| m_clitab.add_item ( "list sheets", JTFUNC(list_sheets) )
		|| m_clitab.add_item ( "load", JTFUNC(load), 1, 2,
			"<OS FILENAME> [csv | tsv]" )
		|| m_clitab.add_item ( "new", JTFUNC(new_book) )
		|| m_clitab.add_item ( "new book", JTFUNC(new_book) )
		|| m_clitab.add_item ( "new sheet", JTFUNC(new_sheet) )
		|| m_clitab.add_item ( "paste", JTFUNC(paste) )
		|| m_clitab.add_item ( "paste content", JTFUNC(paste_content) )
		|| m_clitab.add_item ( "paste prefs", JTFUNC(paste_prefs) )
		|| m_clitab.add_item ( "print", JTFUNC(print) )
		|| m_clitab.add_item ( "print cell num",
			JTFUNC(print_cell_num) )
		|| m_clitab.add_item ( "print cell text",
			JTFUNC(print_cell_text) )
		|| m_clitab.add_item ( "print cell type",
			JTFUNC(print_cell_type) )
		|| m_clitab.add_item ( "print prefs book",
			JTFUNC(print_prefs_book) )
		|| m_clitab.add_item ( "print prefs cell",
			JTFUNC(print_prefs_cell) )
		|| m_clitab.add_item ( "print prefs sheet",
			JTFUNC(print_prefs_sheet) )
		|| m_clitab.add_item ( "print prefs state",
			JTFUNC(print_prefs_state) )
		|| m_clitab.add_item ( "q", JTFUNC(quit) )
		|| m_clitab.add_item ( "quit", JTFUNC(quit) )
		|| m_clitab.add_item ( "recalc", JTFUNC(recalc_sheet) )
		|| m_clitab.add_item ( "recalc book", JTFUNC(recalc_book) )
		|| m_clitab.add_item ( "recalc sheet", JTFUNC(recalc_sheet) )
		|| m_clitab.add_item ( "redo", JTFUNC(redo) )
		|| m_clitab.add_item ( "rename graph", JTFUNC(rename_graph),
			1, 1, "<NEW NAME>" )
		|| m_clitab.add_item ( "rename sheet", JTFUNC(rename_sheet),
			1, 1, "<NEW NAME>" )
		|| m_clitab.add_item ( "save", JTFUNC(save) )
		|| m_clitab.add_item ( "save as", JTFUNC(save_as), 1, 2,
			"<OS FILENAME> [FILETYPE]" )
		|| m_clitab.add_item ( "select", JTFUNC(select), 1, 1,
			"< all | CELLREF[:CELLREF] >" )
		|| m_clitab.add_item ( "set 2dyear", JTFUNC(set_2dyear), 0, 1,
			"[YEAR START]" )
		|| m_clitab.add_item ( "set book", JTFUNC(set_book), 1, 1,
			"<INTEGER - BOOK NUMBER 0-4>" )
		|| m_clitab.add_item ( "set cell", JTFUNC(set_cell), 1, 1,
			"<CELL CONTENT>" )
		|| m_clitab.add_item ( "set graph", JTFUNC(set_graph), 1, 1,
			"<GRAPH NAME>" )
		|| m_clitab.add_item ( "set prefs book", JTFUNC(set_prefs_book),
			2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set prefs cell", JTFUNC(set_prefs_cell),
			2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set prefs cellborder",
			JTFUNC(set_prefs_cellborder), 1, 1, "<DATA>" )
		|| m_clitab.add_item ( "set prefs sheet",
			JTFUNC(set_prefs_sheet), 2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set prefs state",
			JTFUNC(set_prefs_state), 2, 2, "<KEY> <DATA>" )
		|| m_clitab.add_item ( "set sheet", JTFUNC(set_sheet), 1, 1,
			"<SHEET NAME>" )
		|| m_clitab.add_item ( "set width", JTFUNC(set_width), 1, 1,
			"< auto | INTEGER >" )
		|| m_clitab.add_item ( "sort column", JTFUNC(sort_column), 1, 1,
			"<EXPRESSION>" )
		|| m_clitab.add_item ( "sort row", JTFUNC(sort_row), 1, 1,
			"<EXPRESSION>" )
		|| m_clitab.add_item ( "undo", JTFUNC(undo) )
		)
	{
		exit.abort ();
		exit.set_value ( 1 );

		fprintf ( stderr, "Error: main_loop().init_table()\n" );

		return;
	}

	while ( false == exit.aborted () )
	{
		std::string const & line = m_clishell.read_line( BIN_NAME" > ");

		if ( m_clishell.finished () )
		{
			break;
		}
		else if ( line[0] == '#' )
		{
			// Comment
		}
		else if ( line[0] )
		{
			m_clishell.add_history ();

			if (	isdigit ( line[0] ) ||
				isspace ( line[0] ) ||
				line[0] == '='
				)
			{
				parse_as_formula ( line.c_str(),
					cui_file_get_sheet ( file ()));
			}
			else
			{
				m_clitab.parse ( line.c_str() );
			}
		}
		else
		{
			// Quietly ignore empty lines
		}
	}
}

int Backend::get_help (
	char	const * const * const	argv
	) const
{
	return m_clitab.print_help ( argv );
}

int Backend::undo_report_updates ( int const error )
{
	if ( error < 0 )
	{
		char const * const mes[] = { "?",
		"Error during operation.",
		"Unable to begin operation due to problem with undo system.",
		"Undo history lost.",
		"Undo history lost.  Possible data corruption.",
		"Cell locked.  Operation aborted.",
		"Sheet locked.  Operation aborted."
				};

		size_t		mi = ((size_t) -error );

		if ( mi >= sizeof ( mes ) / sizeof ( mes[0] ) )
		{
			mi = 0;
		}

		fprintf ( stderr, "Undo Error: %s\n\n", mes[mi] );
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

CedSheet * Backend::sheet ( bool const print_error )
{
	CedSheet	* const s = cui_file_get_sheet ( m_file_p );

	if ( ! s && print_error )
	{
		fprintf ( stderr, "No sheet available.\n\n" );
	}

	return s;
}

CedBookFile * Backend::graph ( bool const print_error )
{
	CedBookFile	* const g = cui_file_get_graph ( m_file_p );

	if ( ! g && print_error )
	{
		fprintf ( stderr, "No graph available.\n\n" );
	}

	return g;
}

int Backend::set_file ( int const n )
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

