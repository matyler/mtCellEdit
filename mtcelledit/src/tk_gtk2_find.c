/*
	Copyright (C) 2010-2015 Mark Tyler

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



enum
{
	FIND_COL_MATCHES,
	FIND_COL_SHEET,
	FIND_COL_ROW,
	FIND_COL_COLUMN,
	FIND_COL_CONTENT,

	FIND_COL_TOTAL
};

enum
{
	TOGGLE_WILDCARDS,
	TOGGLE_CASE_SENSITIVE,
	TOGGLE_VALUE,
	TOGGLE_ALL_SHEETS,

	TOGGLE_TOTAL
};



static GtkWidget	* entry,
			* find_clist;
static int		toggle_state[TOGGLE_TOTAL],
			book_search,
			tot_matches;
static char	const	* toggle_prefs[TOGGLE_TOTAL] = {
				GUI_INIFILE_FIND_WILDCARDS,
				GUI_INIFILE_FIND_CASE_SENSITIVE,
				GUI_INIFILE_FIND_VALUE,
				GUI_INIFILE_FIND_ALL_SHEETS
			};



static int find_cb (
	CedSheet	* const	sheet,
	CedCell		* const	cell,
	int		const	r,
	int		const	c,
	void		* const	ARG_UNUSED ( ud )
	)
{
	static char	* txt[FIND_COL_TOTAL],
			buf_r[64],
			buf_c[64],
			buf_tot[64];


	tot_matches ++;

	txt[FIND_COL_MATCHES] = buf_tot;
	txt[FIND_COL_SHEET] = NULL;
	txt[FIND_COL_ROW] = buf_r;
	txt[FIND_COL_COLUMN] = buf_c;
	txt[FIND_COL_CONTENT] = cell->text;

	if ( ! txt[FIND_COL_CONTENT] )
	{
		txt[FIND_COL_CONTENT] = "";
	}

	if ( sheet->book_tnode )
	{
		txt[FIND_COL_SHEET] = sheet->book_tnode->key;
	}

	if ( ! txt[FIND_COL_SHEET] )
	{
		txt[FIND_COL_SHEET] = "?";
	}

	snprintf ( buf_tot, sizeof ( buf_tot ), "%i", tot_matches );
	snprintf ( buf_r, sizeof ( buf_r ), "%i", r );
	snprintf ( buf_c, sizeof ( buf_c ), "%i", c );

	gtk_clist_append ( GTK_CLIST ( find_clist ), txt );

	if ( tot_matches >= FIND_MAX_MATCHES )
	{
		return 1;
	}

	return 0;
}

static gboolean find_close (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	ARG_UNUSED ( data )
	)
{
	gtk_widget_hide ( global.pane_find );
	grab_focus_sheet ();

	if ( ! GTK_WIDGET_VISIBLE ( global.pane_graph ) )
	{
		gtk_widget_hide ( global.pane_pair );
	}

	return TRUE;
}

static gboolean find_ok (
	GtkWidget	* const	widget,
	gpointer	const	data
	)
{
	CedSheet	* sheet;
	int		i;
	char	const	* txt;


	if ( ! widget || ! data )
	{
		return FALSE;
	}

	sheet = global.cedview->ren.sheet;

	if ( ! sheet )
	{
		return FALSE;
	}

	book_search = toggle_state[TOGGLE_ALL_SHEETS];

	gtk_clist_set_column_visibility ( GTK_CLIST ( find_clist ),
		FIND_COL_SHEET, book_search );

	txt = gtk_entry_get_text ( GTK_ENTRY ( entry ) );

	if ( ! txt )
	{
		return FALSE;
	}

	tot_matches = 0;

	gtk_widget_set_sensitive ( global.main_window, FALSE );

	gtk_clist_freeze ( GTK_CLIST ( find_clist ) );
	gtk_clist_clear ( GTK_CLIST ( find_clist ) );

	for ( i = 0; i < FIND_COL_TOTAL; i++ )
	{
		gtk_clist_set_column_auto_resize ( GTK_CLIST ( find_clist ), i,
			TRUE );
	}

	be_find ( global.file, sheet, txt,
		toggle_state[TOGGLE_WILDCARDS],
		toggle_state[TOGGLE_CASE_SENSITIVE],
		toggle_state[TOGGLE_VALUE],
		book_search,
		find_cb, NULL
		);

	gtk_clist_thaw ( GTK_CLIST ( find_clist ) );

	for ( i = 0; i < FIND_COL_TOTAL; i++ )
	{
		gtk_clist_set_column_resizeable ( GTK_CLIST ( find_clist ),
			i, TRUE );
	}

	gtk_widget_set_sensitive ( global.main_window, TRUE );
	gtk_widget_grab_focus ( find_clist );

	return TRUE;
}

static void toggle_changed (
	GtkToggleButton	* const	togglebutton,
	gpointer	const	user_data
	)
{
	int		i = (int)(intptr_t)user_data;


	if ( i < 0 || i >= TOGGLE_TOTAL )
	{
		return;
	}

	toggle_state[i] = gtk_toggle_button_get_active ( togglebutton );
}

static void find_clist_select_row (
	GtkCList	* const	clist,
	gint		const	r,
	gint		const	ARG_UNUSED ( c ),
	GdkEventButton	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( ud )
	)
{
	char		* sheet_name,
			* txt_r,
			* txt_c;
	int		srow,
			scol;


	if ( ! gtk_clist_get_text ( clist, r, FIND_COL_SHEET, &sheet_name ) ||
		! gtk_clist_get_text ( clist, r, FIND_COL_ROW, &txt_r ) ||
		! gtk_clist_get_text ( clist, r, FIND_COL_COLUMN, &txt_c )
		)
	{
		return;
	}

	if (	! sheet_name ||
		! txt_r ||
		! txt_c
		)
	{
		return;
	}

	if ( mtkit_strtoi ( txt_r, &srow, NULL, 0 ) )
	{
		return;
	}

	if ( mtkit_strtoi ( txt_c, &scol, NULL, 0 ) )
	{
		return;
	}

	if ( book_search )
	{
		int		sheet_no;


		sheet_no = mtgex_bmenu_get_value_from_text (
			global.bmenu_sheet, sheet_name );

		if ( sheet_no < 0 )
		{
			// Sheet doesn't exist so list is old and needs clearing

			gtk_clist_clear ( GTK_CLIST ( find_clist ) );

			return;
		}

		mtgex_bmenu_set_value ( global.bmenu_sheet, sheet_no );

		// Needed to ensure that the setting cursor position next is
		// visible.

		while ( gtk_events_pending () )
		{
			gtk_main_iteration ();
		}
	}

	set_cursor_range ( srow, scol, srow, scol, 1, 0 );

	// Allow user to use up/down arrow in clist
	gtk_widget_grab_focus ( GTK_WIDGET ( clist ) );
}

int pane_pair_set (
	int	const	action
	)
{
	int		visible_find,
			visible_graph,
			pane_position,
			new_position = -1;


	visible_find  = GTK_WIDGET_VISIBLE ( global.pane_find );
	visible_graph = GTK_WIDGET_VISIBLE ( global.pane_graph );
	pane_position = gtk_paned_get_position ( GTK_PANED ( global.pane_main )
				);

	if ( visible_find )
	{
		prefs_set_int ( GUI_INIFILE_FIND_PANE_POS, pane_position );
	}
	else if ( visible_graph )
	{
		prefs_set_int ( GUI_INIFILE_GRAPH_PANE_POS, pane_position );
	}

	switch ( action )
	{
	case PANE_PAIR_STORE_POSITION:
		return 0;

	case PANE_PAIR_HIDE_FIND:
		gtk_widget_hide ( global.pane_pair );
		gtk_widget_hide ( global.pane_find );
		find_close ( NULL, NULL );
		break;

	case PANE_PAIR_HIDE_GRAPH:
		gtk_widget_hide ( global.pane_pair );
		gtk_widget_hide ( global.pane_graph );
		break;

	case PANE_PAIR_SHOW_FIND:
		gtk_widget_show ( global.pane_pair );
		gtk_widget_hide ( global.pane_graph );
		gtk_widget_show ( global.pane_find );

		gtk_widget_grab_focus ( entry );

		new_position = prefs_get_int ( GUI_INIFILE_FIND_PANE_POS );

		gtk_paned_set_position ( GTK_PANED ( global.pane_main ),
			new_position );
		break;

	case PANE_PAIR_SHOW_GRAPH:
		gtk_widget_show ( global.pane_pair );
		gtk_widget_hide ( global.pane_find );
		gtk_widget_show ( global.pane_graph );

		new_position = prefs_get_int ( GUI_INIFILE_GRAPH_PANE_POS );

		gtk_paned_set_position ( GTK_PANED ( global.pane_main ),
			new_position );
		break;
	}

	show_hide_graphname ();

	return 0;
}

void pressed_find (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	pane_pair_set ( PANE_PAIR_SHOW_FIND );
}

void pressed_find_close (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	pane_pair_set ( PANE_PAIR_HIDE_FIND );
}

void find_gui_build (
	GtkWidget	* const	vbox
	)
{
	GtkWidget	* button,
			* hbox,
			* toggle,
			* sw;
	static gchar		* clist_titles[] = {
				_("Match"), _("Sheet"), _("Row"),
				_("Column"), _("Content") };
	static gchar	const	* toggle_text[] = {
				_("? * Wildcards"),
				_("Case Sensitive"), _("Match Value"),
				_("All Sheets") };
			;
	int		i;
	char	const	* txt;


	// Populate toggle_states from prefs
	for ( i = 0; i < TOGGLE_TOTAL; i++ )
	{
		toggle_state[i] = prefs_get_int ( toggle_prefs[i] );
	}

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 2 );

	txt = prefs_get_string ( GUI_INIFILE_FIND_TEXT );

	entry = gtk_entry_new ();
	gtk_entry_set_text ( GTK_ENTRY ( entry ), txt );
	gtk_entry_set_max_length ( GTK_ENTRY ( entry ), 100 );
	gtk_widget_show ( entry );
	gtk_box_pack_start ( GTK_BOX ( hbox ), entry, TRUE, TRUE, 0 );

	button = gtk_button_new_with_label ( _("Find") );
	gtk_widget_show ( button );
	gtk_box_pack_start( GTK_BOX ( hbox ), button, TRUE, TRUE, 0 );
	g_signal_connect ( button, "clicked", G_CALLBACK ( find_ok ),
		(gpointer)entry );
	g_signal_connect ( entry, "activate", G_CALLBACK ( find_ok ),
		(gpointer)entry );

	button = gtk_button_new_with_label ( _("Close") );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, TRUE, TRUE, 0 );
	g_signal_connect ( button, "clicked", G_CALLBACK ( find_close ),
		(gpointer)entry );

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

	for ( i = 0; i < TOGGLE_TOTAL; i++ )
	{
		toggle = gtk_check_button_new_with_label ( toggle_text[i] );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( toggle ),
			toggle_state[i] );
		gtk_widget_show ( toggle );
		gtk_box_pack_start ( GTK_BOX ( hbox ), toggle, FALSE, FALSE,
			5 );

		g_signal_connect ( toggle, "toggled",
			G_CALLBACK ( toggle_changed ), (gpointer)(intptr_t)i );
	}

	sw = gtk_scrolled_window_new ( NULL, NULL );
	gtk_widget_show ( sw );
	gtk_box_pack_start ( GTK_BOX ( vbox ), sw, TRUE, TRUE, 0 );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	find_clist = gtk_clist_new_with_titles ( 5, clist_titles );
	gtk_widget_show ( find_clist );
	gtk_container_add ( GTK_CONTAINER ( sw ), find_clist );
	gtk_clist_column_titles_passive ( GTK_CLIST ( find_clist ) );
	gtk_clist_set_selection_mode ( GTK_CLIST ( find_clist ),
		GTK_SELECTION_BROWSE );

	g_signal_connect ( find_clist, "select_row",
		G_CALLBACK ( find_clist_select_row ), NULL );
	g_signal_connect ( find_clist, "key_press_event",
		G_CALLBACK ( key_event_escape ), NULL );
}

void find_gui_save_settings ( void )
{
	char	const	* txt;
	int		i;


	// Store toggle_states in prefs
	for ( i = 0; i < TOGGLE_TOTAL; i++ )
	{
		prefs_set_int ( toggle_prefs[i], toggle_state[i] );
	}

	txt = gtk_entry_get_text ( GTK_ENTRY ( entry ) );
	prefs_set_string ( GUI_INIFILE_FIND_TEXT, txt );
}

