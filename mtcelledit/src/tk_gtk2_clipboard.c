/*
	Copyright (C) 2008-2014 Mark Tyler

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



/*
Summary of clipboard actions

When copying data we initially just put it into (CedSheet *) global.clip_sheet;
When this is pasted by this instance of mtCellEdit, we recognise it as our
data by the owner object being the main window.

When another app pastes this data we create a TSV file and pass over the data
- this data is kept in memory just in case it is pasted again later.  This is
pure text and is in (char *)global.clip_tsv;

When another instance of mtCellEdit pastes this data a CED ZIP file is created
and a timestamp is created to identify it between processes.

When another program, e.g. Gnumeric, copies data to the system clipboard this
is always read in on demand as a TSV memory chunk.
*/



///	COPYING + XFERING DATA TO/FROM SYSTEM CLIPBOARD

static void flush_internal_clipboard ( void )
{
	GtkClipboard	* clip;
	GObject		* owner = NULL;


	clip = gtk_clipboard_get ( GDK_SELECTION_CLIPBOARD );
	if ( clip )
	{
		owner = gtk_clipboard_get_owner ( clip );
	}

	if ( owner && owner == (GObject *)(global.main_window) )
	{
		// We own the system clipboard so destroy it

		gtk_clipboard_clear ( clip );
	}
	else
	{
		// We don't own the clipboard so only flush our copy of it

		cui_clip_flush ( global.clipboard );
	}
}

static void get_clip_func (
	GtkClipboard	* const	ARG_UNUSED ( clipboard ),
	GtkSelectionData * const selection_data,
	guint		const	ARG_UNUSED ( info ),
	gpointer	const	ARG_UNUSED ( user_data_or_owner )
	)
{
	GdkAtom		ced_clip_atom;


	ced_clip_atom = gdk_atom_intern ( MTCELLEDIT_CLIP_NAME, FALSE );

	if ( selection_data->target == ced_clip_atom )
	{

		if ( cui_clip_save_temp ( global.clipboard ) )
		{
			goto error_temp;
		}

		// Pass the timestamp
		gtk_selection_data_set_text ( selection_data,
			global.clipboard->timestamp,
			CUI_CLIPBOARD_TIMESTAMP_SIZE );

		gtk_selection_data_set ( selection_data, ced_clip_atom, 8,
			(guchar *)global.clipboard->timestamp,
			CUI_CLIPBOARD_TIMESTAMP_SIZE );
	}
	else
	{

		if (	cui_clip_export_text ( global.clipboard ) ||
			! global.clipboard->tsv )
		{
			goto error;
		}

		gtk_selection_data_set_text ( selection_data,
			global.clipboard->tsv, -1 );
	}

	return;				// Success

error:
	gtk_selection_data_set_text ( selection_data,
		_("Error - get_clip_func: unable to create TSV data\n"), -1 );

	return;

error_temp:
	gtk_selection_data_set_text ( selection_data,
		_("Error - get_clip_func: unable to create temp file\n"), -1 );
}


static void get_clear_func (
	GtkClipboard	* const	ARG_UNUSED ( clipboard ),
	gpointer	const	ARG_UNUSED ( user_data_or_owner )
	)
{
	cui_clip_flush ( global.clipboard );
}

static void set_clip_owner ( void )
{
	GtkTargetList	* list;
	GtkTargetEntry	* targets;
	gint		n_targets;
	GtkClipboard	* clip;


	clip = gtk_clipboard_get ( GDK_SELECTION_CLIPBOARD );
	if ( ! clip )
	{
		return;
	}

	list = gtk_target_list_new ( NULL, 0 );
	gtk_target_list_add ( list, gdk_atom_intern ( MTCELLEDIT_CLIP_NAME,
		FALSE ), 0, MTCELLEDIT_CLIP_CODE );
	gtk_target_list_add_text_targets ( list, 0 );
	targets = gtk_target_table_new_from_list ( list, &n_targets );

	gtk_clipboard_set_with_owner ( clip, targets, (guint)n_targets,
		get_clip_func, get_clear_func,
		G_OBJECT ( global.main_window ) );

	gtk_target_table_free ( targets, n_targets );
	gtk_target_list_unref ( list );
}

static int copy_selection_to_clip ( void )
{
	flush_internal_clipboard ();
	if ( cui_clip_copy ( global.file, global.clipboard ) )
	{
		mtgex_alert_box ( _("Error"),
			_("Unable to copy selection to clipboard."),
			_("OK"), NULL, NULL, global.main_window );

		return 1;
	}

	set_clip_owner ();

	return 0;			// Success
}

static void copy_router ( int mode )
{
	int		res = 0;


	if ( copy_selection_to_clip () )
	{
		return;
	}

	switch ( mode )
	{
	case 1:	res = be_clip_copy_values ( global.clipboard->sheet );
		break;
	case 2:	res = be_clip_copy_output ( global.clipboard->sheet );
		break;
	}

	if ( res )
	{
		mtgex_alert_box ( _("Error"), _("Unable to create clipboard."),
			_("OK"), NULL, NULL, global.main_window );

		return;
	}

	ced_view_bell_swap ( global.cedview );
}

void pressed_copy (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	copy_router ( 0 );
}

void pressed_copy_values (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	copy_router ( 1 );
}

void pressed_copy_output (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	copy_router ( 2 );
}



///	PASTING CUTTING CLEARING



static int request_mtcelledit_content (
	GtkClipboard	* const	clip
	)
{
	GtkSelectionData * selection;


	selection = gtk_clipboard_wait_for_contents ( clip,
			gdk_atom_intern ( MTCELLEDIT_CLIP_NAME, FALSE ) );

	if (	! selection ||
		selection->length != CUI_CLIPBOARD_TIMESTAMP_SIZE
		)
	{
		return 0;		// Not sent by mtCellEdit
	}

	if (	! global.clipboard->sheet ||
		memcmp( global.clipboard->timestamp, selection->data,
			CUI_CLIPBOARD_TIMESTAMP_SIZE )
		)
	{
		if ( cui_clip_load_temp ( global.clipboard ) )
		{
			return 0;	// Unable to load clipboard - error
		}

		// Set the new timestamp
		cui_clip_set_timestamp ( global.clipboard,
			(char *)selection->data );
	}

	gtk_selection_data_free ( selection );

	return 1;			// mtCellEdit clipboard is available
}

static int read_system_clipboard (
	GtkClipboard	* const	clip
	)
{
	char		* txt;
	GObject		* owner;


	owner = gtk_clipboard_get_owner ( clip );

	if ( owner && owner == (GObject *)(global.main_window) )
	{
		// If we created this clipboard ourselves we can stop here

		return 0;
	}

	if ( request_mtcelledit_content ( clip ) )
	{
		// This clipboard was created by another mtCellEdit instance

		return 0;
	}

	// This is not mtCellEdit clipboard data so load it in as a TSV text
	// chunk.

	txt = gtk_clipboard_wait_for_text ( clip );

	if ( cui_clip_import_text ( global.clipboard, txt ) )
	{
		g_free ( txt );

		return 1;		// Error importing
	}

	g_free ( txt );

	return 0;			// Success
}

static int obtain_paste ( void )
{
	CedSheet	* sheet;
	GtkClipboard	* clip;


	sheet = global.cedview->ren.sheet;
	if ( ! sheet )
	{
		return 0;
	}

	clip = gtk_clipboard_get ( GDK_SELECTION_CLIPBOARD );
	if ( ! clip )
	{
		return 0;
	}

	if ( read_system_clipboard ( clip ) )
	{
		return 0;		// Nothing read in
	}

	if ( ! global.clipboard->sheet )
	{
		return 0;		// Nothing to paste
	}

	return 1;	// Successfully transferred something to clipboard
}

static int paste_clipboard_at_cursor (
	int		const	mode
	)
{
	int		res;


	if ( ! obtain_paste () )
	{
		return 1;		// No paste found
	}

	res = cui_clip_paste ( global.file, global.clipboard, mode );
	if ( res == 1 )
	{
		return 1;
	}

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_LOCKED_SHEET ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;		// Nothing changed
	}

	update_changes_chores ( 1, 0 );

	return 0;			// Paste committed
}

void pressed_paste (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	paste_clipboard_at_cursor ( 0 );
}

void pressed_paste_content (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	paste_clipboard_at_cursor ( CED_PASTE_CONTENT );
}

void pressed_paste_prefs (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	paste_clipboard_at_cursor ( CED_PASTE_PREFS );
}

static int clear_selection (
	int		const	mode
	)
{
	int		res;


	res = be_clip_clear_selection ( global.file, global.cedview->ren.sheet,
		mode );

	if ( res > 0 )
	{
		return 1;
	}

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	update_changes_chores ( 1, 0 );

	return 0;			// Success
}

void pressed_cut (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( copy_selection_to_clip () )
	{
		return;
	}

	if ( clear_selection ( 0 ) )
	{
		return;
	}
}

void pressed_clear (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( clear_selection ( 0 ) )
	{
		return;
	}
}

void pressed_clear_contents (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( clear_selection ( CED_PASTE_CONTENT ) )
	{
		return;
	}
}

void pressed_clear_prefs (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( clear_selection ( CED_PASTE_PREFS ) )
	{
		return;
	}
}

static int calc_selection_position (
	int		* const	row,
	int		* const	col
	)
{
	CedSheet	* sheet = global.cedview->ren.sheet;


	if ( ! sheet )
	{
		return 1;
	}

	if ( row )
	{
		row[0] = sheet->prefs.cursor_r1;
	}

	if ( col )
	{
		col[0] = sheet->prefs.cursor_c1;
	}

	if ( ! obtain_paste () )
	{
		return 1;		// No paste found
	}

	return 0;
}

void pressed_row_insert_paste_height (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		row,
			res;


	if ( undo_report_updates ( cui_check_sheet_lock (
		global.cedview->ren.sheet ) ) )
	{
		return;
	}

	if (	calc_selection_position ( &row, NULL ) ||
		row < 1 )
	{
		return;
	}

	res = cui_sheet_insert_row ( global.file->cubook,
		global.cedview->ren.sheet, row, global.clipboard->rows );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	update_changes_chores ( 1, 0 );
}

void pressed_column_insert_paste_width (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		col,
			res;


	if ( undo_report_updates ( cui_check_sheet_lock (
		global.cedview->ren.sheet ) ) )
	{
		return;
	}

	if (	calc_selection_position ( NULL, &col ) ||
		col < 1
		)
	{
		return;
	}

	res = cui_sheet_insert_column ( global.file->cubook,
		global.cedview->ren.sheet, col, global.clipboard->cols );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	update_splits ();
	update_changes_chores ( 1, 0 );
}

void pressed_transform_clipboard (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	item
	)
{
	if ( ! obtain_paste () )
	{
		return;			// No paste to act on
	}

	CedSheet * sheet = be_clip_transform_start ( global.clipboard, item );

	if ( ! sheet )
	{
		goto error;
	}

	// Remove old, install new
	flush_internal_clipboard ();

	if ( be_clip_transform_finish ( global.clipboard, sheet, item ) )
	{
		// be_* clears up sheet on error
		goto error;
	}

	set_clip_owner ();		// Claim ownership

	return;				// Success

error:
	mtgex_alert_box ( _("Error"), _("Unable to transform clipboard."),
		_("OK"), NULL, NULL, global.main_window );
}

