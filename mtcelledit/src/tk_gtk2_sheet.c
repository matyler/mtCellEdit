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



#define FIRST_DUPLICATE_ID	2



void pressed_sheet_new (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res;


	res = cui_file_sheet_add ( global.file );
	if ( res == 1 )
	{
		mtgex_alert_box ( _("Error"),
			_("Unable to create a new sheet."), _("OK"),
			NULL, NULL, global.main_window );

		return;
	}

	undo_report_updates ( res );

	if (	res == CUI_ERROR_UNDO_OP ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		// Nothing more to report or do

		return;
	}

	sheet_selector_populate ();
	be_update_file_to_book ( global.file );

	update_changes_chores ( 1, 1 );
}

void pressed_sheet_duplicate (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* const	sheet = global.cedview->ren.sheet;
	CedSheet	* newsheet;
	int		res;


	if ( ! sheet || ! sheet->book_tnode )
	{
		return;
	}

	res = cui_book_duplicate_sheet ( global.file->cubook, sheet,
		&newsheet );
	undo_report_updates ( res );

	if ( res )
	{
		mtgex_alert_box ( _("Error"),
			_("Unable to duplicate the current sheet."),
			_("OK"), NULL, NULL, global.main_window );

		return;
	}

	sheet_selector_populate ();
	res = mtgex_bmenu_get_value_from_text ( global.bmenu_sheet,
		(char *)newsheet->book_tnode->key );
	if ( res >= 0 )
	{
		mtgex_bmenu_set_value ( global.bmenu_sheet, res );
	}

	be_update_file_to_book ( global.file );

	update_changes_chores ( 1, 0 );
}



///	DIALOG - Rename/text entry

static char	* dialog_text_result;
static int	dialog_text_finished;



static gboolean dialog_name_close (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	data
	)
{
	gtk_widget_destroy ( GTK_WIDGET ( data ) );
	dialog_text_finished = 1;

	return TRUE;
}

static gboolean dialog_name_ok (
	GtkWidget	* const	widget,
	gpointer	const	data
	)
{
	GtkWidget	* window;
	char	const	* txt;


	window = gtk_object_get_user_data ( GTK_OBJECT ( widget ) );
	txt = gtk_entry_get_text ( GTK_ENTRY ( data ) );
	if ( ! txt )
	{
		txt = "";
	}

	dialog_text_result = strdup ( txt );
	dialog_text_finished = 1;

	gtk_widget_destroy ( window );	// txt destroyed here

	return TRUE;
}

char * dialog_text_entry (
	char	const	* const	title,
	char	const	* const	entry_text
	)
{
	GtkWidget	* window,
			* entry,
			* button,
			* vbox,
			* hbox;
	GtkAccelGroup	* ag;


	dialog_text_result = NULL;
	dialog_text_finished = 0;

	window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL, title,
		GTK_WIN_POS_CENTER, TRUE );
	ag = gtk_accel_group_new ();

	gtk_signal_connect ( GTK_OBJECT ( window ), "destroy",
		GTK_SIGNAL_FUNC ( dialog_name_close ), window );

	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox );
	gtk_container_add ( GTK_CONTAINER ( window ), vbox );

	entry = gtk_entry_new ();
	gtk_widget_show ( entry );
	gtk_box_pack_start ( GTK_BOX ( vbox ), entry, FALSE, FALSE, 5 );

	if ( entry_text )
	{
		gtk_entry_set_text ( GTK_ENTRY ( entry ), entry_text );
	}

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
		G_CALLBACK ( dialog_name_close ), (gpointer)window );

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
		G_CALLBACK ( dialog_name_ok ), (gpointer)entry );

	gtk_window_set_transient_for ( GTK_WINDOW ( window ),
		GTK_WINDOW ( global.main_window ) );
	gtk_widget_show ( window );
	gtk_window_add_accel_group ( GTK_WINDOW ( window ), ag );

	while ( dialog_text_finished == 0 )
	{
		gtk_main_iteration ();
	}

	return dialog_text_result;
}

static int rename_sheet (
	char	const	* const	new_name
	)
{
	int		res;
	char	const	* msg = NULL;


	if ( ! new_name || new_name[0] == 0 )
	{
		msg = _("Bad sheet name");
	}
	else if ( ced_book_get_sheet ( global.file->cubook->book, new_name ) )
	{
		msg = _("Sheet name already exists");
	}

	if ( msg )
	{
		mtgex_alert_box ( _("Error"), msg, _("OK"), NULL, NULL,
			global.main_window );

		return 1;		// Try again
	}

	res = cui_book_page_rename ( global.file->cubook,
		global.cedview->ren.sheet, new_name );
	undo_report_updates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return 1;		// Try again
	}

	sheet_selector_populate ();
	update_changes_chores ( 0, 1 );

	return 0;			// Success
}

void pressed_sheet_rename (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res;
	CedSheet	* sheet;
	char		* new_name;


	sheet = global.cedview->ren.sheet;
	if (	! sheet			||
		undo_report_updates ( cui_check_sheet_lock ( sheet ) ) ||
		! ( global.file->type == CED_FILE_TYPE_TSV_BOOK ||
			global.file->type == CED_FILE_TYPE_TSV_VAL_BOOK ||
			global.file->type == CED_FILE_TYPE_LEDGER_BOOK ||
			global.file->type == CED_FILE_TYPE_LEDGER_VAL_BOOK
			)		||
		! sheet->book_tnode	||
		! sheet->book_tnode->key
		)
	{
		return;
	}

	for ( res = 1; res; )
	{
		new_name = dialog_text_entry ( _("Rename Sheet"),
			(char *)sheet->book_tnode->key );

		if ( new_name )
		{
			res = rename_sheet ( new_name );
			free ( new_name );
		}
		else
		{
			res = 0;
		}
	}
}

void pressed_sheet_delete (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* const	sheet = global.cedview->ren.sheet;
	int		page_num, res;


	if ( ! sheet || ! sheet->book_tnode )
	{
		return;
	}

	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( undo_report_updates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	page_num = mtgex_bmenu_get_value ( global.bmenu_sheet );

	// This is needed in case we remove the last sheet
	ced_view_set_sheet ( global.table, NULL );

	res = cui_book_destroy_sheet ( global.file->cubook,
		sheet->book_tnode->key );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		mtgex_bmenu_set_value ( global.bmenu_sheet, page_num );

		return;
	}

	sheet_selector_populate ();

	if ( page_num > 0 )
	{
		page_num --;
	}

	mtgex_bmenu_set_value ( global.bmenu_sheet, page_num );

	update_changes_chores ( 1, 1 );
}

void pressed_sheet_previous (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		i;


	i = mtgex_bmenu_get_value ( global.bmenu_sheet );
	if ( i < 1 )
	{
		return;
	}

	i--;
	mtgex_bmenu_set_value ( global.bmenu_sheet, i );
}

void pressed_sheet_next (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		i;


	i = mtgex_bmenu_get_value ( global.bmenu_sheet );
	if ( i < 0 )
	{
		return;
	}

	i++;
	mtgex_bmenu_set_value ( global.bmenu_sheet, i );
}

void pressed_sheet_freeze_panes (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* const	sheet = global.cedview->ren.sheet;
	int		r1,
			c1,
			r2,
			c2,
			srow,
			scol,
			set_pos = 0;


	if ( ! sheet )
	{
		return;
	}

	if (	sheet->prefs.split_r1 != 0 ||
		sheet->prefs.split_c1 != 0
		)
	{
		// Split currently in force so turn current one off

		srow = sheet->prefs.split_r1;
		scol = sheet->prefs.split_c1;

		r1 = 0;
		c1 = 0;
		r2 = 0;
		c2 = 0;

		set_pos = 1;
	}
	else
	{
		// No split yet so set one up

		r1 = sheet->prefs.start_row;
		c1 = sheet->prefs.start_col;
		r2 = MAX ( sheet->prefs.cursor_r1 - 1, r1 );
		c2 = MAX ( sheet->prefs.cursor_c1 - 1, c1 );

		if ( sheet->prefs.cursor_r1 == 1 )
		{
			r1 = 0;
			r2 = 0;
		}

		if ( sheet->prefs.cursor_c1 == 1 )
		{
			c1 = 0;
			c2 = 0;
		}
	}

	ced_view_set_split ( global.table, r1, r2, c1, c2 );

	if ( set_pos )
	{
		gtk_adjustment_set_value (
			GTK_ADJUSTMENT ( global.cedview->adj_hori ), scol );
		gtk_adjustment_set_value (
			GTK_ADJUSTMENT ( global.cedview->adj_vert ), srow );
	}

	update_changes_chores ( 0, 1 );
}

void pressed_sheet_locked (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* const	sheet = global.cedview->ren.sheet;


	if ( ! sheet )
	{
		return;
	}

	sheet->prefs.locked = ! sheet->prefs.locked;

	// Update the lock/unlock menu and the other gubbins
	update_changes_chores ( 0, 1 );
}

