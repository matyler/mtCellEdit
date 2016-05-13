/*
	Copyright (C) 2010-2014 Mark Tyler

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



static GtkWidget	* window,
			* sort_clist,
			* spin_rowcol,
			* toggle_direction,
			* toggle_case;
static int		sort_table[MAX_SORT][SORT_TABLE_TOTAL],
			sort_axis,
			sort_axis_total,
			sort_row,
			sort_col,
			sort_rowtot,
			sort_coltot,
			sort_origin;


static gboolean sort_close (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	ARG_UNUSED ( data )
	)
{
	gtk_widget_destroy ( window );

	return TRUE;
}

static gboolean sort_ok (
	GtkWidget	* const	widget,
	gpointer	const	data
	)
{
	int		i,
			res,
			mode_list[MAX_SORT] = {0},
			rowscols[MAX_SORT] = {0};


	sort_close ( widget, data );

	for ( i = 0; i < sort_axis_total; i++ )
	{
		rowscols[i] = sort_table[i][SORT_TABLE_ROWCOL];

		if ( sort_table[i][SORT_TABLE_DIRECTION] )
		{
			mode_list[i] |= CED_SORT_MODE_DESCENDING;
		}

		if ( sort_table[i][SORT_TABLE_CASE_SENSITIVE] )
		{
			mode_list[i] |= CED_SORT_MODE_CASE;
		}
	}

	if ( sort_axis )
	{
		res = cui_sheet_sort_columns ( global.file->cubook,
			global.cedview->ren.sheet,
			sort_col, sort_coltot, rowscols, 0, mode_list );
	}
	else
	{
		res = cui_sheet_sort_rows ( global.file->cubook,
			global.cedview->ren.sheet,
			sort_row, sort_rowtot, rowscols, 0, mode_list );
	}

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return TRUE;
	}

	update_changes_chores ( 1, 0 );

	return TRUE;
}

static void set_row_values (
	int	const	clist_row
	)
{
	char	const	* txt;
	char		buf[256] = {0};


	snprintf ( buf, sizeof ( buf ), "%i", sort_table[clist_row][SORT_TABLE_ROWCOL] );
	gtk_clist_set_text ( GTK_CLIST ( sort_clist ), clist_row,
		SORT_TABLE_ROWCOL, buf );

	if ( sort_table[clist_row][SORT_TABLE_CASE_SENSITIVE] )
	{
		txt = _("Yes");
	}
	else
	{
		txt = _("No");
	}

	gtk_clist_set_text ( GTK_CLIST ( sort_clist ), clist_row,
		SORT_TABLE_CASE_SENSITIVE, txt );

	if ( sort_table[clist_row][SORT_TABLE_DIRECTION] )
	{
		txt = "9 -> 1";
	}
	else
	{
		txt = "1 -> 9";
	}

	gtk_clist_set_text ( GTK_CLIST ( sort_clist ), clist_row,
		SORT_TABLE_DIRECTION, txt );
}

static void change_input ( void )
{
	int		crow = 0,
			rc,
			dir,
			cs;


	if ( GTK_CLIST ( sort_clist )->selection )
	{
		crow = (int)(intptr_t)GTK_CLIST ( sort_clist )->selection->data;
	}

	if ( crow < 0 || crow >= MAX_SORT )
	{
		crow = 0;
	}

	rc = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON ( spin_rowcol ) );

	dir = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON ( toggle_direction ) );

	cs = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( toggle_case ) );

	sort_table[crow][SORT_TABLE_ROWCOL] = rc;
	sort_table[crow][SORT_TABLE_DIRECTION] = dir;
	sort_table[crow][SORT_TABLE_CASE_SENSITIVE] = cs;

	set_row_values ( crow );
}

static void sort_clist_select_row (
	GtkCList	* const	ARG_UNUSED ( clist ),
	gint		const	r,
	gint		const	ARG_UNUSED ( c ),
	GdkEventButton	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( ud )
	)
{
	if ( r < 0 || r >= MAX_SORT )
	{
		return;
	}

GTKBUG	g_signal_handlers_block_by_func ( G_OBJECT ( spin_rowcol ),
		G_CALLBACK ( change_input ), NULL );
GTKBUG	g_signal_handlers_block_by_func ( G_OBJECT ( toggle_direction ),
		G_CALLBACK ( change_input ), NULL );
GTKBUG	g_signal_handlers_block_by_func ( G_OBJECT ( toggle_case ),
		G_CALLBACK ( change_input ), NULL );

	gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_rowcol ),
		sort_table[r][SORT_TABLE_ROWCOL] );
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( toggle_direction ),
		sort_table[r][SORT_TABLE_DIRECTION] );
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( toggle_case ),
		sort_table[r][SORT_TABLE_CASE_SENSITIVE] );

GTKBUG	g_signal_handlers_unblock_by_func ( G_OBJECT ( spin_rowcol ),
		(gpointer)G_CALLBACK ( change_input ), NULL );
GTKBUG	g_signal_handlers_unblock_by_func ( G_OBJECT ( toggle_direction ),
		(gpointer)G_CALLBACK ( change_input ), NULL );
GTKBUG	g_signal_handlers_unblock_by_func ( G_OBJECT ( toggle_case ),
		(gpointer)G_CALLBACK ( change_input ), NULL );
}

static void row_move (
	GtkCList	* const	ARG_UNUSED ( clist ),
	gint		const	row1,
	gint		const	row2,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		i,
			j,
			dir,
			temp[SORT_TABLE_TOTAL];


	if (	row1 == row2		||
		row1 < 0		||
		row2 < 0		||
		row1 >= MAX_SORT	||
		row2 >= MAX_SORT
		)
	{
		return;
	}

	if ( row1 < row2 )
	{
		dir = 1;
	}
	else
	{
		dir = -1;
	}

	// Store temp
	for ( j = 0; j < SORT_TABLE_TOTAL; j++ )
	{
		temp[j] = sort_table[row1][j];
	}

	// Shuffle rows up/down by one
	for ( i = row1; i != row2; i += dir )
	{
		for ( j = 0; j < SORT_TABLE_TOTAL; j++ )
		{
			sort_table[i][j] = sort_table[i + dir][j];
		}
	}

	// Restore temp
	for ( j = 0; j < SORT_TABLE_TOTAL; j++ )
	{
		sort_table[row2][j] = temp[j];
	}
}

static gboolean move_row (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	data
	)
{
	int		row,
			delta = (int)(intptr_t)data,
			row_new;


	if ( ! GTK_CLIST ( sort_clist )->selection )
	{
		return FALSE;
	}

	row = (int)(intptr_t)GTK_CLIST ( sort_clist )->selection->data;
	row_new = row + delta;

	if (	row < 0		||
		row >= MAX_SORT	||
		row_new < 0	||
		row_new >= MAX_SORT
		)
	{
		return FALSE;
	}

	gtk_clist_swap_rows ( GTK_CLIST ( sort_clist ), row, row_new );

	return TRUE;
}

void pressed_sort (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	item
	)
{
	CedSheet	* sheet;
	GtkWidget	* button,
			* vbox,
			* hbox,
			* vbox2;
	GtkAccelGroup	* ag;
	gchar		* clist_titles[] = {
				"",
				_("Direction"),
				_("Case Sensitive")
				};
	int		i,
			new_axis_tot,
			new_sort_origin;
	char		* title;


	sheet = global.cedview->ren.sheet;
	if ( ! sheet || item < 0 || item > 1 )
	{
		return;
	}

	sort_row = MIN ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	sort_col = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	sort_rowtot = MAX ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 ) -
		sort_row + 1;
	sort_coltot = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 ) -
		sort_col + 1;

	if ( item == 1 )		// Sort columns
	{
		if ( sort_coltot < 2 )
		{
			return;		// Nothing to do
		}

		new_axis_tot = MIN ( sort_rowtot, MAX_SORT );
		title = _("Sort Columns");
		clist_titles[SORT_TABLE_ROWCOL] = _("Row");
		new_sort_origin = sort_row;
	}
	else				// Sort rows
	{
		if ( sort_rowtot < 2 )
		{
			return;		// Nothing to do
		}

		new_axis_tot = MIN ( sort_coltot, MAX_SORT );
		title = _("Sort Rows");
		clist_titles[SORT_TABLE_ROWCOL] = _("Column");
		new_sort_origin = sort_col;
	}

	if (	sort_axis_total	!= new_axis_tot	||
		sort_axis != item		||
		sort_origin != new_sort_origin
		)
	{
		/*
		Only clear out old data if user changes anything.
		Otherwise its a re-sort of the same area with the same settings.
		*/

		sort_axis = item;
		sort_axis_total = new_axis_tot;
		sort_origin = new_sort_origin;

		for ( i = 0; i < MAX_SORT; i++ )
		{
			sort_table[i][SORT_TABLE_ROWCOL] = i + sort_origin;
			sort_table[i][SORT_TABLE_DIRECTION] = 0;
			sort_table[i][SORT_TABLE_CASE_SENSITIVE] = 0;
		}
	}

	window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL, title,
		GTK_WIN_POS_CENTER, TRUE );

	ag = gtk_accel_group_new ();

	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox );
	gtk_container_add ( GTK_CONTAINER ( window ), vbox );

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

	sort_clist = gtk_clist_new_with_titles ( SORT_TABLE_TOTAL,
		clist_titles );
	gtk_widget_show ( sort_clist );
	gtk_box_pack_start ( GTK_BOX ( hbox ), sort_clist, TRUE, TRUE, 5 );
	gtk_clist_column_titles_passive ( GTK_CLIST ( sort_clist ) );
	gtk_clist_set_selection_mode ( GTK_CLIST ( sort_clist ),
		GTK_SELECTION_BROWSE );

	mtgex_clist_enable_drag ( sort_clist );
	g_signal_connect ( sort_clist, "row-move", G_CALLBACK ( row_move ),
		NULL );

	for ( i = 0; i < sort_axis_total; i++ )
	{
		clist_titles[0] = "";
		clist_titles[1] = "";
		clist_titles[2] = "";
		gtk_clist_append ( GTK_CLIST ( sort_clist ), clist_titles );

		set_row_values ( i );
	}

	for ( i = 0; i < SORT_TABLE_TOTAL; i++ )
	{
		gtk_clist_set_column_auto_resize ( GTK_CLIST ( sort_clist ), i,
			TRUE );
	}

	vbox2 = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox2 );
	gtk_box_pack_start ( GTK_BOX ( hbox ), vbox2, FALSE, FALSE, 5 );

	spin_rowcol = gtk_spin_button_new_with_range ( 1, MIN ( CED_MAX_ROW,
		CED_MAX_COLUMN ), 1 );
	gtk_widget_show ( spin_rowcol );
	gtk_box_pack_start ( GTK_BOX ( vbox2 ), spin_rowcol, FALSE, FALSE, 5 );
	g_signal_connect ( spin_rowcol, "value-changed",
		G_CALLBACK ( change_input ), NULL );

	toggle_direction = gtk_check_button_new_with_label ( _("Direction") );
	gtk_widget_show ( toggle_direction );
	gtk_box_pack_start ( GTK_BOX ( vbox2 ), toggle_direction, FALSE, FALSE,
		5 );
	g_signal_connect ( toggle_direction, "toggled",
		G_CALLBACK ( change_input ), NULL );

	toggle_case = gtk_check_button_new_with_label ( _("Case Sensitive") );
	gtk_widget_show ( toggle_case );
	gtk_box_pack_start ( GTK_BOX ( vbox2 ), toggle_case, FALSE, FALSE, 5 );
	g_signal_connect ( toggle_case, "toggled", G_CALLBACK ( change_input ),
		NULL );

	button = gtk_button_new_with_label ( _("Move Up") );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( vbox2 ), button, TRUE, TRUE, 5 );
	g_signal_connect ( button, "clicked", G_CALLBACK ( move_row ),
		(gpointer) -1 );

	button = gtk_button_new_with_label ( _("Move Down") );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( vbox2 ), button, TRUE, TRUE, 5 );
	g_signal_connect ( button, "clicked", G_CALLBACK ( move_row ),
		(gpointer) 1 );

	// Must be done after spin/toggles set up
	g_signal_connect ( sort_clist, "select_row",
		G_CALLBACK ( sort_clist_select_row ), NULL );

	mtgex_add_hseparator ( vbox, -2, 10 );

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

	button = gtk_button_new_with_label ( _("Close") );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, TRUE, TRUE, 5 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Escape, 0,
		(GtkAccelFlags) 0 );
	g_signal_connect ( button, "clicked", G_CALLBACK ( sort_close ),
		(gpointer)window );

	button = gtk_button_new_with_label ( _("Sort") );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, TRUE, TRUE, 5 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_KP_Enter, 0,
		(GtkAccelFlags) 0 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Return, 0,
		(GtkAccelFlags) 0 );
	g_signal_connect ( button, "clicked", G_CALLBACK ( sort_ok ), NULL );

	g_signal_connect ( window, "delete_event", G_CALLBACK ( sort_close ),
		NULL );

	gtk_window_set_transient_for ( GTK_WINDOW ( window ),
		GTK_WINDOW ( global.main_window ) );
	gtk_widget_show ( window );
	gtk_window_add_accel_group ( GTK_WINDOW ( window ), ag );

	// Needed to set up values in spin/toggles
	gtk_clist_select_row ( GTK_CLIST ( sort_clist ), 0, 0 );
}

