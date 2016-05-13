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



void pressed_select_all (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		r,
			c;


	r = MAX ( 1, global.cedview->sheet_rows );
	c = MAX ( 1, global.cedview->sheet_cols );

	set_cursor_range ( 1, 1, r, c, 0, 0 );
}

void pressed_edit_cell (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	grab_focus_entry ();
}

void pressed_row_insert (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* sheet = global.cedview->ren.sheet;
	int		row,
			rowtot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( undo_report_updates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_row_extent ( sheet, &row, &rowtot ) )
	{
		return;
	}

	res = cui_sheet_insert_row ( global.file->cubook, sheet, row, rowtot );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	update_changes_chores ( 1, 0 );
}

void pressed_row_delete (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* sheet = global.cedview->ren.sheet;
	int		row,
			rowtot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( undo_report_updates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_row_extent ( sheet, &row, &rowtot ) )
	{
		return;
	}

	res = cui_sheet_delete_row ( global.file->cubook, sheet, row, rowtot );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	update_changes_chores ( 1, 0 );
}

void pressed_column_insert (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* sheet = global.cedview->ren.sheet;
	int		col,
			coltot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( undo_report_updates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_col_extent ( sheet, &col, &coltot ) )
	{
		return;
	}

	res = cui_sheet_insert_column ( global.file->cubook, sheet, col, coltot
		);
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

void pressed_column_delete (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* sheet = global.cedview->ren.sheet;
	int		col,
			coltot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( undo_report_updates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_col_extent ( sheet, &col, &coltot ) )
	{
		return;
	}

	res = cui_sheet_delete_column ( global.file->cubook, sheet, col, coltot
		);
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



//	SIMPLE DIALOGS

static gboolean simple_dialog_close (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	data
	)
{
	gtk_widget_destroy ( GTK_WIDGET ( data ) );

	return TRUE;
}



//	COLUMN WIDTH DIALOG

static gboolean column_width_ok (
	GtkWidget	* const	widget,
	gpointer	const	data
	)
{
	GtkWidget	* window;
	int		c,
			ctot,
			w,
			res;
	CedSheet	* sheet;


	sheet = global.cedview->ren.sheet;

	if (	! widget	||
		! data		||
		! sheet
		)
	{
		return FALSE;
	}

	gtk_spin_button_update ( GTK_SPIN_BUTTON ( data ) );

	window = gtk_object_get_user_data ( GTK_OBJECT ( widget ) );
	w = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( data ) );

	c = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	ctot = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 ) - c + 1;

	gtk_widget_destroy ( window );

	res = cui_sheet_set_column_width ( global.file->cubook, sheet, c,
		ctot, w );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return TRUE;
	}

	ced_view_set_split ( global.table, sheet->prefs.split_r1,
		sheet->prefs.split_r2, sheet->prefs.split_c1,
		sheet->prefs.split_c2 );

	update_changes_chores ( 1, 1 );

	return TRUE;
}

void pressed_column_width_auto (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res,
			c,
			ctot;
	CedSheet	* sheet;


	sheet = global.cedview->ren.sheet;
	if ( ! sheet )
	{
		return;
	}

	c = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	ctot = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 ) - c + 1;

	res = cui_sheet_set_column_width_auto ( global.file->cubook, sheet, c,
		ctot );

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	ced_view_set_split ( global.table, sheet->prefs.split_r1,
		sheet->prefs.split_r2, sheet->prefs.split_c1,
		sheet->prefs.split_c2 );

	update_changes_chores ( 0, 1 );
}

void pressed_column_width (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* sheet;
	CedCell		* cell;
	GtkWidget	* window,
			* spin,
			* button,
			* vbox,
			* hbox;
	GtkAccelGroup	* ag;
	int		w;


	sheet = global.cedview->ren.sheet;
	if ( ! sheet )
	{
		return;
	}

	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( undo_report_updates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL,
		_("Set Column Width"), GTK_WIN_POS_CENTER, TRUE );
	ag = gtk_accel_group_new ();

	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox );
	gtk_container_add ( GTK_CONTAINER ( window ), vbox );

	spin = gtk_spin_button_new_with_range ( 0, CED_MAX_COLUMN_WIDTH, 1 );
	gtk_widget_show ( spin );
	gtk_box_pack_start ( GTK_BOX ( vbox ), spin, FALSE, FALSE, 5 );

	cell = ced_sheet_get_cell ( sheet, 0, sheet->prefs.cursor_c1 );

	if ( cell && cell->prefs )
	{
		w = cell->prefs->width;
	}
	else
	{
		w = 0;
	}

	gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin ), w );

	mtgex_add_hseparator ( vbox, -2, 10 );

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

	button = gtk_button_new_with_label ( _("Close") );
	gtk_widget_set_usize ( button, 100, -2 );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, TRUE, TRUE, 5 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Escape, 0,
		(GtkAccelFlags) 0 );
	g_signal_connect ( button, "clicked",
		G_CALLBACK ( simple_dialog_close ), (gpointer)window );

	button = gtk_button_new_with_label ( _("OK") );
	gtk_object_set_user_data ( GTK_OBJECT ( button ), (gpointer)window );
	gtk_widget_set_usize ( button, 100, -2 );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, TRUE, TRUE, 5 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_KP_Enter, 0,
		(GtkAccelFlags) 0 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Return, 0,
		(GtkAccelFlags) 0 );
	g_signal_connect ( button, "clicked",
		G_CALLBACK ( column_width_ok ), (gpointer)spin );

	gtk_window_set_transient_for ( GTK_WINDOW ( window ),
		GTK_WINDOW ( global.main_window ) );
	gtk_widget_show ( window );
	gtk_window_add_accel_group ( GTK_WINDOW ( window ), ag );
}

static void book_prefs_closure (
	mtPrefValue	* const	piv,
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	mtPrefs		* prefs = (mtPrefs *)piv;


	fe_save_pref_window_prefs ( prefs );
	be_book_prefs_finish ( prefs, global.file->cubook->book );
	mtkit_prefs_destroy ( prefs );
}

void pressed_edit_book_prefs (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	mtPrefs		* prefs;


	prefs = be_book_prefs_init ( global.file->cubook->book );

	mtgex_prefs_window ( prefs, global.main_window, _("Book Preferences"),
		book_prefs_closure, (mtPrefValue *)prefs, 0 );
}

void pressed_fix_2dyears (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res;


	res = be_fix_years ( global.file );

	switch ( res )
	{
	case 1:
		mtgex_alert_box ( _("Error"),
			_("Unable to fix years."),
			_("OK"), NULL, NULL, global.main_window );
		break;

	case 2:
		mtgex_alert_box ( _("Error"),
			_("Unexpected error in system date."),
			_("OK"), NULL, NULL, global.main_window );
		return;
	}

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL	||
		res == CUI_ERROR_LOCKED_SHEET	||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;			// Nothing changed
	}

	update_changes_chores ( 1, 0 );
}

