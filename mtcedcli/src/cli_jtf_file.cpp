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



static int get_force_arg (
	char	const * const	arg
	)
{
	if ( ! arg )
	{
		return CED_FILE_FORCE_NONE;
	}

	int res = CED_FILE_DETECT_ERROR;

	mtKit::CharInt	const	chitab[] = {
			{ "tsv",	CED_FILE_FORCE_TSV },
			{ "csv",	CED_FILE_FORCE_CSV },
			{ NULL, 0 }
			};

	mtKit::cli_parse_charint ( arg, chitab, res );

	return res;
}

int jtf_load (
	char	const * const * const	args
	)
{
	int	const	ft = get_force_arg ( args[1] );

	if ( ft == CED_FILE_DETECT_ERROR )
	{
		return 2;
	}

	if ( cui_file_load ( backend.file(), args[0], ft ) )
	{
		fprintf ( stderr, "Unable to load '%s'\n\n", args[0] );

		return 2;
	}

	return 0;
}

int jtf_save (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( backend.file()->name == NULL )
	{
		fprintf ( stderr, "Filename not set.\n\n" );

		return 2;
	}

	if ( cui_file_save ( backend.file(), backend.file()->name,
		backend.file()->type ) )
	{
		fprintf ( stderr, "Unable to save.\n\n" );

		return 2;
	}

	return 0;
}

static int get_file_type (
	char	const	* const	txt,
	int		* const	dest
	)
{
	mtKit::CharInt const types[] = {
		{ "tsv_book",		CED_FILE_TYPE_TSV_BOOK },
		{ "tsv_value_book",	CED_FILE_TYPE_TSV_VAL_BOOK },
		{ "ledger_book",	CED_FILE_TYPE_LEDGER_BOOK },
		{ "ledger_value_book",	CED_FILE_TYPE_LEDGER_VAL_BOOK },
		{ NULL, 0 }
		};

	int		res;


	if ( mtKit::cli_parse_charint ( txt, types, res ) )
	{
		return 1;
	}

	dest[0] = res;

	return 0;
}

int jtf_save_as (
	char	const * const * const	args
	)
{
	if ( args[1] )
	{
		if ( get_file_type ( args[1], &backend.file()->type ) )
		{
			return 2;
		}
	}

	if ( cui_file_save ( backend.file(), args[0], backend.file()->type ) )
	{
		fprintf ( stderr, "Unable to save.\n\n" );

		return 2;
	}

	return 0;
}

int jtf_import_book (
	char	const * const * const	args
	)
{
	int	const	ft = get_force_arg ( args[1] );

	if ( ft == CED_FILE_DETECT_ERROR )
	{
		return 2;
	}

	CuiFile * uifile = cui_file_new ();
	if (	! uifile				||
		cui_file_load ( uifile, args[0], ft )	||
		! uifile->cubook			||
		! uifile->cubook->book
		)
	{
		cui_file_free ( uifile );
		fprintf ( stderr, "Unable to load file.\n\n" );

		return 2;
	}

	int sheet_tot, sheet_fail, file_tot, file_fail;
	int const res = cui_book_merge ( backend.file()->cubook,
		uifile->cubook->book, &sheet_tot, &sheet_fail, &file_tot,
		&file_fail );

	cui_file_free ( uifile );
	uifile = NULL;

	backend.undo_report_updates ( res );

	printf ( "%i sheets imported.\n"
		"%i sheets not imported due to identical names.\n"
		"%i graphs/files imported.\n"
		"%i graphs/files not imported due to identical names.\n\n",
		sheet_tot, sheet_fail, file_tot, file_fail );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 2;
	}

	// Update sheet/graph names if they were empty before
	backend.sheet ();
	backend.graph ();
	backend.update_changes_chores ();

	return 0;			// Success
}

int jtf_import_graph (
	char	const * const * const	args
	)
{
	if ( cui_graph_get ( backend.file()->cubook->book, args[0] ) )
	{
		fprintf ( stderr, "Graph name already exists.\n\n" );

		return 2;
	}

	int	buf_size;
	char	* buf = mtkit_file_load ( args[1], &buf_size, 0, 0 );

	if ( ! buf )
	{
		fprintf ( stderr, "Unable to load '%s'.\n\n", args[1] );

		return 2;
	}

	if ( ! cui_graph_new ( backend.file()->cubook->book, buf, buf_size,
		args[0] )
		)
	{
		free ( buf );
		buf = NULL;
		fprintf ( stderr, "Unable to create graph.\n\n" );

		return 2;
	}

	buf = NULL;
	mtkit_strfreedup ( &backend.file()->cubook->book->prefs.active_graph,
		args[0] );

	return 0;
}

int jtf_export_graph (
	char	const * const * const	args
	)
{
	CedBookFile * const graph = backend.graph ();

	if ( ! graph )
	{
		return 2;
	}

	if ( mtkit_file_save ( args[0], graph->mem, graph->size, 0 ) )
	{
		fprintf ( stderr, "Error exporting graph.\n\n" );

		return 2;
	}

	return 0;
}

int jtf_export_output_graph (
	char	const * const * const	args
	)
{
	CedBookFile * const graph = backend.graph ();
	if ( ! graph )
	{
		return 2;
	}

	int		filetype;
	mtKit::CharInt	const graph_types[] = {
			{ "eps",	CUI_GRAPH_TYPE_EPS },
			{ "pdf",	CUI_GRAPH_TYPE_PDF },
			{ "png",	CUI_GRAPH_TYPE_PNG },
			{ "ps",		CUI_GRAPH_TYPE_PS },
			{ "svg",	CUI_GRAPH_TYPE_SVG },
			{ NULL, 0 }
			};

	if ( mtKit::cli_parse_charint ( args[1], graph_types, filetype ) )
	{
		fprintf ( stderr, "Unknown graph type.\n\n" );

		return 2;
	}

	if ( cui_graph_render_file ( backend.file()->cubook->book,
		backend.file()->cubook->book->prefs.active_graph,
		args[0], filetype, NULL, 1 )
		)
	{
		fprintf ( stderr, "Error exporting graph output.\n\n" );

		return 2;
	}

	return 0;
}

int jtf_export_output_sheet (
	char	const * const * const	args
	)
{
	CedSheet * const sheet = backend.sheet ();
	if ( ! sheet )
	{
		return 2;
	}

	int		filetype;
	mtKit::CharInt	const sheet_types[] = {
			{ "eps",	CUI_SHEET_EXPORT_EPS },
			{ "html",	CUI_SHEET_EXPORT_HTML },
			{ "pdf",	CUI_SHEET_EXPORT_PDF },
			{ "pdf_paged",	CUI_SHEET_EXPORT_PDF_PAGED },
			{ "png",	CUI_SHEET_EXPORT_PNG },
			{ "ps",		CUI_SHEET_EXPORT_PS },
			{ "svg",	CUI_SHEET_EXPORT_SVG },
			{ "tsv",	CUI_SHEET_EXPORT_TSV },
			{ "tsv_q",	CUI_SHEET_EXPORT_TSV_QUOTED },
			{ NULL, 0 }
			};

	if ( mtKit::cli_parse_charint ( args[1], sheet_types, filetype ) )
	{
		fprintf ( stderr, "Unknown sheet type.\n\n" );

		return 2;
	}

	int		font_size = 12;
	char	const	* font_name = "Sans";
	int		row_pad = 1;

	mtkit_prefs_get_str ( backend.prefs(), MAIN_FONT_NAME, &font_name );
	mtkit_prefs_get_int ( backend.prefs(), MAIN_FONT_SIZE, &font_size );
	mtkit_prefs_get_int ( backend.prefs(), MAIN_ROW_PAD, &row_pad );

	if ( cui_export_output ( backend.prefs(), sheet, args[0],
		backend.file()->name, filetype, row_pad, font_name, font_size ))
	{
		fprintf ( stderr, "Error exporting sheet output.\n\n" );

		return 2;
	}

	return 0;
}

int jtf_export_sheet (
	char	const * const * const	args
	)
{
	CedSheet * const sheet = backend.sheet ();
	if ( ! sheet )
	{
		return 2;
	}

	int		filetype;
	mtKit::CharInt	const sheet_types[] = {
		{ "tsv",		CED_FILE_TYPE_TSV_CONTENT },
		{ "tsv_gz",		CED_FILE_TYPE_TSV_CONTENT_GZ },
		{ "tsv_noq",		CED_FILE_TYPE_TSV_CONTENT_NOQ },
		{ "tsv_val",		CED_FILE_TYPE_TSV_VALUE },
		{ "tsv_val_gz",		CED_FILE_TYPE_TSV_VALUE_GZ },
		{ "tsv_val_noq",	CED_FILE_TYPE_TSV_VALUE_NOQ },
		{ "csv",		CED_FILE_TYPE_CSV_CONTENT },
		{ "csv_noq",		CED_FILE_TYPE_CSV_CONTENT_NOQ },
		{ "csv_val",		CED_FILE_TYPE_CSV_VALUE },
		{ "csv_val_noq",	CED_FILE_TYPE_CSV_VALUE_NOQ },
		{ "ledger",		CED_FILE_TYPE_LEDGER },
		{ "ledger_gz",		CED_FILE_TYPE_LEDGER_GZ },
		{ "ledger_val",		CED_FILE_TYPE_LEDGER_VAL },
		{ "ledger_val_gz",	CED_FILE_TYPE_LEDGER_VAL_GZ },
		{ NULL, 0 }
		};

	if ( mtKit::cli_parse_charint ( args[1], sheet_types, filetype ) )
	{
		fprintf ( stderr, "Unknown sheet type.\n\n" );

		return 2;
	}

	if ( ced_sheet_save ( sheet, args[0], filetype ) )
	{
		fprintf ( stderr, "Error exporting sheet.\n\n" );

		return 2;
	}

	return 0;
}

