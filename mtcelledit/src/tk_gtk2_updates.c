/*
	Copyright (C) 2008-2015 Mark Tyler

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



void update_main_area ( void )
{
	ced_view_redraw ( global.table );
}

void update_menus ( void )		// Update undo/redo menus
{
	char	const	* txt;
	int	const	menu_list[] = {
			_MENU_CUT,
			_MENU_COPY,
			_MENU_PASTE,
			_MENU_CLEAR,
			_MENU_SELECT_ALL,
			_MENU_CLEAR_CONTENT,
			_MENU_CLEAR_PREFS,
			_MENU_ROW_INSERT,
			_MENU_ROW_DELETE,
			_MENU_COL_INSERT,
			_MENU_COL_DELETE,
			-1
			};
	int		i;
	gboolean	on;
	GtkWidget	* focus;
	CedSheet	* sheet;


	sheet = global.cedview->ren.sheet;

	focus = GTK_WINDOW ( global.main_window )->focus_widget;

	if (	GTK_WIDGET_HAS_FOCUS ( global.entry_celltext ) ||
		GTK_WIDGET_HAS_FOCUS ( global.entry_cellref ) ||
		! focus ||
		gtk_widget_is_ancestor ( focus, global.pane_find ) ||
		gtk_widget_is_ancestor ( focus, global.pane_graph )
		)
	{
		on = FALSE;
	}
	else
	{
		on = TRUE;
	}

	// This stops the shortcuts interfering with the entries when they
	// have focus.

	for ( i = 0; menu_list[i] >= 0; i++ )
	{
		gtk_widget_set_sensitive ( global.menu_widgets[ menu_list[i] ],
			on );
	}

	if ( ! global.file->cubook->book )
	{
		return;
	}

	// Enable/disable undo/redo according to their availability
	if ( global.file->cubook->undo.undo_step )
	{
		gtk_widget_set_sensitive ( global.menu_widgets[_MENU_UNDO],
			TRUE );
	}
	else
	{
		gtk_widget_set_sensitive ( global.menu_widgets[_MENU_UNDO],
			FALSE );
	}

	if ( global.file->cubook->undo.redo_step )
	{
		gtk_widget_set_sensitive ( global.menu_widgets[_MENU_REDO],
			TRUE );
	}
	else
	{
		gtk_widget_set_sensitive ( global.menu_widgets[_MENU_REDO],
			FALSE );
	}

	// Set the lock/unlock option for the sheet
	if ( sheet && sheet->prefs.locked )
	{
		txt = _("Unlock");
	}
	else
	{
		txt = _("Lock");
	}

	gtk_label_set_text ( GTK_LABEL ( GTK_MENU_ITEM (
		global.menu_widgets[_MENU_SHEET_LOCKED] )->item.bin.child ),
		txt );

	if (	sheet &&
		(sheet->prefs.split_r1 != 0 ||
			sheet->prefs.split_c1 != 0)
		)
	{
		txt = _("Unfreeze Panes");
	}
	else
	{
		txt = _("Freeze Panes");
	}

	gtk_label_set_text ( GTK_LABEL ( GTK_MENU_ITEM (
		global.menu_widgets[_MENU_SHEET_FREEZE_PANES] )->item.bin.child
		), txt );
}

void update_titlebar ( void )
{
	char		txt [ PATHBUF ];
	int		book;


	book = be_titlebar_text ( global.file, txt, sizeof ( txt ),
		global.mem_changed );

	gtk_window_set_title ( GTK_WINDOW ( global.main_window ), txt );

	if ( book )
	{
		gtk_widget_show ( global.bmenu_sheet );
	}
	else
	{
		gtk_widget_hide ( global.bmenu_sheet );
	}
}

void update_entry_celltext ( void )
{
	CedCell		* cell = NULL;
	GtkEntry	* entry;
	CedSheet	* sheet;


	sheet = global.cedview->ren.sheet;

	if ( sheet )
	{
		cell = ced_sheet_get_cell ( sheet, sheet->prefs.cursor_r1,
			sheet->prefs.cursor_c1 );
	}

	entry = GTK_ENTRY ( global.entry_celltext );

	gtk_entry_set_text ( entry, "" );

	if ( cell && cell->text )
	{
		if ( cell->type == CED_CELL_TYPE_TEXT_EXPLICIT )
		{
			gtk_entry_append_text ( entry, "'" );
		}

		gtk_entry_append_text ( entry, cell->text );

		if ( cell->type == CED_CELL_TYPE_ERROR )
		{
			int		e = (int)(cell->value / 1000);


			gtk_editable_select_region ( GTK_EDITABLE ( entry ),
				0, e );
		}
	}

	if (	sheet &&
		( sheet->prefs.locked ||
			( cell && cell->prefs && cell->prefs->locked ) ) &&
		! global.file->cubook->book->prefs.disable_locks
		)
	{
		gtk_widget_set_sensitive ( GTK_WIDGET ( entry ), FALSE );
	}
	else
	{
		gtk_widget_set_sensitive ( GTK_WIDGET ( entry ), TRUE );
	}
}

void update_entry_cellref ( void )
{
	char		txt [ 128 ];


	be_sheet_ref ( global.cedview->ren.sheet, txt, sizeof ( txt ) );

	gtk_entry_set_text ( GTK_ENTRY ( global.entry_cellref ), txt );
}

void update_recent_files ( void )	// Update the menu items
{
	char		buf [ PATHBUF ];
	int		i,
			count = 0
			;


	for ( i = 1; i <= RECENT_MENU_TOTAL; i++ )
	{
		if ( 1 == be_snip_filename ( i, buf, sizeof ( buf ) ) )
		{
			// Hide if empty
			gtk_widget_hide ( global.menu_widgets[i + _MENU_REC_SEP]
				);

			continue;
		}

		gtk_label_set_text ( GTK_LABEL ( GTK_MENU_ITEM (
			global.menu_widgets[i + _MENU_REC_SEP]
			)->item.bin.child), buf );
		gtk_widget_show ( global.menu_widgets[i + _MENU_REC_SEP] );
		count ++;
	}

	// Hide separator if not needed
	if ( count )
	{
		gtk_widget_show ( global.menu_widgets[_MENU_REC_SEP] );
	}
	else
	{
		gtk_widget_hide ( global.menu_widgets[_MENU_REC_SEP] );
	}
}

static void set_sheet_cb (
	GtkWidget	* const	widget,
	void		* const	ARG_UNUSED ( user_data ),
	int		const	new_value
	)
{
	char	const	* name;
	CedSheet	* sheet;
	CedBook		* book = global.file->cubook->book;


	name = mtgex_bmenu_get_value_text ( widget, new_value );
	sheet = ced_book_get_sheet ( book, name );
	ced_view_set_sheet ( global.table, sheet );

	mtkit_strfreedup ( &book->prefs.active_sheet, name );

	update_entry_celltext ();
	update_entry_cellref ();
	update_quicksum_label ();
	update_menus ();
	grab_focus_sheet ();
}

void enable_sheet_selector_cb ( void )
{
	mtgex_bmenu_set_callback ( global.bmenu_sheet, set_sheet_cb, NULL );
}

void disable_sheet_selector_cb ( void )
{
	mtgex_bmenu_set_callback ( global.bmenu_sheet, NULL, NULL );
}

static int sselpop_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	char	const	* const	name,
	void		* const	user_data
	)
{
	int		ni,
			* check = user_data;


	ni = mtgex_bmenu_add_item ( global.bmenu_sheet, name );
	if ( check[0] )
	{
		// Select this item from the list if its the current sheet
		if ( ! strcmp ( name,
			global.file->cubook->book->prefs.active_sheet ) )
		{
			check[0] = 0;
			enable_sheet_selector_cb ();
			mtgex_bmenu_set_value ( global.bmenu_sheet, ni );
		}
	}

	return 0;			// Continue
}

void sheet_selector_populate ( void )
{
	int		check = 1;


	if ( ! global.file->cubook->book->prefs.active_sheet )
	{
		check = 0;
	}
	else
	{
		disable_sheet_selector_cb ();
	}

	mtgex_bmenu_add_item ( global.bmenu_sheet, NULL );
	ced_book_scan ( global.file->cubook->book, sselpop_cb, &check );

	if ( check )
	{
		enable_sheet_selector_cb ();
		mtgex_bmenu_set_value ( global.bmenu_sheet, 0 );
	}
}

int check_for_changes ( void )
{
	int		i = -10;


	graph_gui_store_changes ();

	if ( global.mem_changed )
	{
		i = mtgex_alert_box ( _("Warning"),
			_("This project contains changes that have not been "
			"saved.  Do you really want to lose these changes?"),
			_("Cancel Operation"), _("Lose Changes"), NULL,
			global.main_window );
	}

	return i;
}

void grab_focus_sheet ( void )
{
	ced_view_grab_focus ( global.table );
}

void grab_focus_entry ( void )
{
	gtk_widget_grab_focus ( global.entry_celltext );
	gtk_entry_set_position ( GTK_ENTRY ( global.entry_celltext ), -1 );
}

void update_quicksum_label ( void )
{
	char		txt [ 256 ];


	if ( global.sum_label_op == 0 )
	{
		gtk_widget_hide ( global.label_sum );

		return;
	}

	be_quicksum_label ( global.cedview->ren.sheet, txt, sizeof ( txt ),
		global.sum_label_op );

	gtk_label_set_text ( GTK_LABEL ( global.label_sum ), txt );
	gtk_widget_show ( global.label_sum );
}

void set_cursor_range (
	int	const	r1,
	int	const	c1,
	int	const	r2,
	int	const	c2,
	int	const	cursor_visible,
	int	const	force_update
	)
{
	if ( ced_view_set_cursor_area ( global.table, r1, c1, r2, c2,
		cursor_visible, force_update ) )
	{
		return;
	}

	update_entry_cellref ();
	update_entry_celltext ();
	update_quicksum_label ();
}

void clear_all ( void )
{
	ced_view_set_sheet ( global.table, NULL );

	cui_file_book_new ( global.file );

	update_menus ();
	global.mem_changed = 0;
	sheet_selector_populate ();
	update_titlebar ();
	update_entry_celltext ();
	set_cursor_range ( 1, 1, 1, 1, 1, 1 );
	graph_gui_update ( NULL );
}

void recalc_sheet_core ( void )
{
	ced_sheet_recalculate ( global.cedview->ren.sheet, NULL, 0 );
	ced_sheet_recalculate ( global.cedview->ren.sheet, NULL, 1 );
}

void pressed_recalculate (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	recalc_sheet_core ();
	update_changes_chores ( 0, 1 );
}

void recalc_book_core ( void )
{
	ced_book_recalculate ( global.file->cubook->book, 0 );
	ced_book_recalculate ( global.file->cubook->book, 1 );
}

void pressed_recalculate_book (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	recalc_book_core ();
	update_changes_chores ( 0, 1 );
}

gboolean view_focus_out (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventFocus	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	update_menus ();

	return FALSE;
}

gboolean view_focus_in (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventFocus	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	update_menus ();

	return FALSE;
}

gboolean cellref_entry_focus_in (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventFocus	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	update_menus ();

	return FALSE;
}

gboolean cellref_entry_focus_out (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventFocus	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	update_entry_cellref ();
	update_menus ();

	return FALSE;
}

gboolean celltext_entry_focus_in (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventFocus	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	update_menus ();

	return FALSE;
}

gboolean celltext_entry_focus_out (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventFocus	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	update_menus ();

	return FALSE;
}

void pressed_undo (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res;


	// This is needed in case we remove the last sheet
	ced_view_set_sheet ( global.table, NULL );

	res = cui_book_undo_step ( global.file->cubook );
	undo_report_updates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return;
	}

	sheet_selector_populate ();
	update_splits ();
	update_changes_chores ( 1, 0 );	// Update menus/titles
}

void pressed_redo (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res;


	// This is needed in case we remove the last sheet
	ced_view_set_sheet ( global.table, NULL );

	res = cui_book_redo_step ( global.file->cubook );
	undo_report_updates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return;
	}

	sheet_selector_populate ();
	update_splits ();
	update_changes_chores ( 1, 0 );	// Update menus/titles
}

void update_set_changes_flag ( void )
{
	global.mem_changed = 1;
	update_titlebar ();
}

void update_changes_chores (
	int	const	new_geometry,
	int	const	block_sheet_recalcs
	)
{
	if (	! block_sheet_recalcs &&
		global.cedview->ren.sheet &&
		global.file->cubook->book->prefs.auto_recalc
		)
	{
		// Always update sheet (must be done before book updates)
		recalc_sheet_core ();

		switch ( global.file->cubook->book->prefs.auto_recalc )
		{
		case 2:
			recalc_book_core ();
			break;
		}
	}

	update_set_changes_flag ();
	update_main_area ();
	update_menus ();
	update_entry_celltext ();
	update_quicksum_label ();

	if ( new_geometry )
	{
		ced_view_update_geometry ( global.cedview );
	}
}

void update_splits ( void )
{
	ced_view_set_sheet ( global.table, global.cedview->ren.sheet );
	// Update splits/scrollbars
}

int undo_report_updates (
	int	const	error
	)
{
	char	const	* msg;
	char		buf [ 2048 ];


	msg = be_get_error_update_text ( error, buf, sizeof ( buf ) );

	if ( ! msg )
	{
		return 0;
	}

	// Report this error to the user
	mtgex_alert_box ( _("Error"), msg, _("OK"), NULL, NULL,
		global.main_window );

	return error;
}

