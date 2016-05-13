/*
	Copyright (C) 2008-2016 Mark Tyler

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

#include "tk_gtk2.h"



static void error_message_file (
	char	const	* const	message,
	char	const	* const	filename
	)
{
	char		* msg,
			* f8;


	f8 = mtgex_gtkuncpy ( NULL, filename, 0 );
	msg = g_strdup_printf ( _("Unable to %s file: %s"), message, f8 );

	mtgex_alert_box ( _("Error"), msg, _("OK"), NULL, NULL,
		global.main_window );

	g_free ( msg );
	g_free ( f8 );
}



/*
	File operations - return 0 on success
*/

static void register_project ( void )
{
	if ( be_register_project ( global.file ) )
	{
		return;
	}

	update_titlebar ();
	update_recent_files ();
}

int project_load (
	char	const	* const	filename
	)
{
	CedSheet	* sheet;
	CedSheetPrefs	* tmp_sheet_prefs = NULL;


	sheet = global.cedview->ren.sheet;
	if ( sheet )
	{
		ced_view_set_sheet ( global.table, NULL );

		if ( ced_file_type_class ( global.file->type ) == 1 )
		{
			if ( prefs_get_int (
				GUI_INIFILE_SHEET_PREFS_PERSIST ) )
			{
				tmp_sheet_prefs = ced_sheet_prefs_new ();
				ced_sheet_prefs_copy ( tmp_sheet_prefs,
					&sheet->prefs );
			}
		}
	}

	if ( cui_file_load ( global.file, filename, be_get_force_tsvcsv () ) )
	{
		ced_view_set_sheet ( global.table, sheet );
		error_message_file ( _("load project"), filename );
		ced_sheet_prefs_free ( tmp_sheet_prefs );

		return 1;
	}

	cui_file_set_lock ( global.file,
		prefs_get_int ( GUI_INIFILE_FILE_LOCK_LOAD ) );

	if ( tmp_sheet_prefs )
	{
		if ( ced_file_type_class ( global.file->type ) == 1 )
		{
			sheet = cui_file_get_sheet ( global.file );
			if ( sheet )
			{
				ced_sheet_prefs_copy ( &sheet->prefs,
					tmp_sheet_prefs );
			}
		}

		ced_sheet_prefs_free ( tmp_sheet_prefs );
		tmp_sheet_prefs = NULL;
	}

	sheet_selector_populate ();
	graph_gui_update ( global.file->cubook->book->prefs.active_graph );

	if (	global.file->cubook->book->prefs.auto_recalc ||
		! ( global.file->type == CED_FILE_TYPE_TSV_VAL_BOOK ||
			global.file->type == CED_FILE_TYPE_LEDGER_VAL_BOOK )
		)
	{
		// Books with values don't need recalculating
		recalc_book_core ();
	}

	update_menus ();
	global.mem_changed = 0;
	register_project ();

	if ( global.file->cubook->book->prefs.disable_locks )
	{
		mtgex_alert_box ( _("Warning"),
			_("This book has the disable_locks flag set, "
			"which is potentially dangerous."),
			_("OK"), NULL, NULL, global.main_window );
	}

	return 0;
}

int project_import (
	char	const	* const	filename
	)
{
	CuiFile		* uifile;
	int		sheet_tot,
			sheet_fail,
			file_tot,
			file_fail,
			res;
	char		buf [ PATHBUF ];


	uifile = cui_file_new ();
	if ( ! uifile )
	{
		goto fail;
	}

	if (	cui_file_load ( uifile, filename, be_get_force_tsvcsv () ) ||
		! uifile->cubook			||
		! uifile->cubook->book
		)
	{
		cui_file_free ( uifile );

		goto fail;
	}

	res = cui_book_merge ( global.file->cubook, uifile->cubook->book,
		&sheet_tot, &sheet_fail, &file_tot, &file_fail );

	cui_file_free ( uifile );

	undo_report_updates ( res );

	sheet_selector_populate ();	// Done here as need for success/fail

	// Update graph selector
	graph_gui_update ( global.file->cubook->book->prefs.active_graph );

	snprintf ( buf, sizeof ( buf ),
		"%i sheets imported.  "
		"%i sheets not imported due to identical names.\n"
		"%i graphs/files imported.  "
		"%i graphs/files not imported due to identical names.",
		sheet_tot, sheet_fail, file_tot, file_fail );

	mtgex_alert_box ( _("Information"), buf, _("OK"), NULL, NULL,
		global.main_window );

	if (	res == CUI_ERROR_LOCKED_CELL	||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	if ( sheet_tot > 0 )
	{
		be_update_file_to_book ( global.file );
	}

	update_changes_chores ( 0, 1 );

	return 0;			// Success

fail:
	error_message_file ( _("load project (for import)"), filename );

	return 1;
}

static void report_large_tsv ( void )
{
	mtgex_alert_box ( _("Error"),
		_("The sheet geometry is too large to save in this file"
			" format."),
		_("OK"), NULL, NULL, global.main_window );
}

int project_save (
	char	const	* const	filename,
	int		const	filetype
	)
{
	int		res;


	if (	filetype == CED_FILE_TYPE_TSV_BOOK	||
		filetype == CED_FILE_TYPE_TSV_VAL_BOOK	||
		filetype == CED_FILE_TYPE_LEDGER_BOOK	||
		filetype == CED_FILE_TYPE_LEDGER_VAL_BOOK
		)
	{
		graph_gui_store_changes ();
	}

	res = cui_file_save ( global.file, filename, filetype );
	if ( res == 1 )
	{
		report_large_tsv ();

		return 1;
	}

	if ( res )
	{
		error_message_file ( _("save project"), filename );

		return 1;
	}

	cui_file_set_lock ( global.file,
		prefs_get_int ( GUI_INIFILE_FILE_LOCK_SAVE ) );

	global.mem_changed = 0;
	register_project ();

	return 0;
}

int project_export (
	char	const	* const	filename,
	int		const	filetype
	)
{
	if ( be_export_sheet ( global.cedview->ren.sheet, filename, filetype ) )
	{
		error_message_file ( _("export sheet"), filename );

		return 1;
	}

	register_project ();

	return 0;
}

int project_export_output (
	char	const	* const	filename,
	int		const	filetype
	)
{
	if ( cui_export_output ( prefs_file (), global.cedview->ren.sheet,
		filename, global.file->name, filetype,
		prefs_get_int ( GUI_INIFILE_ROW_PAD ),
		prefs_get_string ( GUI_INIFILE_FONT_PANGO_NAME ),
		prefs_get_int ( GUI_INIFILE_FONT_SIZE )
		) )
	{
		error_message_file ( _("export sheet output"), filename );

		return 1;
	}

	be_remember_last_dir ( filename );

	return 0;
}

int project_export_graph (
	char	const	* const	filename,
	int		const	filetype
	)
{
	double		scale;


	scale = prefs_get_double ( GUI_INIFILE_GRAPH_SCALE );

	if ( cui_graph_render_file ( global.file->cubook->book,
		global.file->cubook->book->prefs.active_graph,
		filename, filetype, NULL, scale ) )
	{
		error_message_file ( _("export graph"), filename );

		return 1;
	}

	be_remember_last_dir ( filename );

	return 0;
}

void pressed_new_project (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		i;


	i = check_for_changes ();

	if ( i != -10 && i != 2 )
	{
		return;
	}

	clear_all ();
}

void pressed_open_project (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		i;


	i = check_for_changes ();
	if ( i != -10 && i != 2 )
	{
		return;
	}

	file_selector ( FS_LOAD_PROJECT, global.main_window,
		global.file->name );
}

void pressed_save_project_as (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	file_selector ( FS_SAVE_PROJECT, global.main_window,
		global.file->name );
}

void pressed_save_project (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if (	! global.file->name		||
		global.file->name[0] == 0
		)
	{
		file_selector ( FS_SAVE_PROJECT, global.main_window,
			global.file->name );
	}
	else
	{
		project_save ( global.file->name, global.file->type );
	}
}

void pressed_recent (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	item
	)
{
	int		i;


	i = check_for_changes ();
	if ( i != -10 && i != 2 )
	{
		return;
	}

	project_load ( be_get_recent_filename ( item ) );
}

void pressed_sheet_export (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( ! global.cedview->ren.sheet )
	{
		return;			// Nothing to export
	}

	file_selector ( FS_EXPORT_SHEET, global.main_window, NULL );
}

void pressed_sheet_export_output (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( ! global.cedview->ren.sheet )
	{
		return;			// Nothing to export
	}

	if ( cui_sheet_check_geometry ( global.cedview->ren.sheet ) )
	{
		report_large_tsv ();

		return;
	}

	file_selector ( FS_EXPORT_SHEET_OUTPUT, global.main_window, NULL );
}

void pressed_import_project (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	file_selector ( FS_IMPORT_PROJECT, global.main_window, NULL );
}

