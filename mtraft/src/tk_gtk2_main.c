/*
	Copyright (C) 2004-2015 Mark Tyler

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
#include "icon_xpm.xpm"



#define MAX_TABS	10
#define TOT_COLS	9
				// 9th column = empty to keep 8th column nicely
				// spaced.


typedef struct
{
	char		* new_path;
	GtkListStore	* store;
	int		sort_column;
	GtkSortType	sort_direction;
	CedSheet	* sheet;
	GtkWidget	* treeview,
			* entryText;
} Tab_node;



static GtkWidget	* main_window,
			* progressbar,
			* notebook,
			* button_copy;
static int		stop_flag;
static double		progress = -1.0;
static Tab_node		tabarr[ MAX_TABS ];



static void raft_gui_analyse (
	char	const	* directory
	);



static gboolean delete_event (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEvent	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( data )
	)
{
	gint		x,
			y,
			width,
			height;
	int		i;


	if ( progress > 0 )
	{
		// In the middle of analysing so signal a bail out
		stop_flag = TRUE;

		return TRUE;
	}

	gtk_window_get_size ( GTK_WINDOW ( main_window ), &width, &height );
	gdk_window_get_root_origin ( main_window->window, &x, &y );

	prefs_set_int ( PREFS_WINDOW_X, x );
	prefs_set_int ( PREFS_WINDOW_Y, y );
	prefs_set_int ( PREFS_WINDOW_W, width );
	prefs_set_int ( PREFS_WINDOW_H, height );

	gtk_main_quit ();

	for ( i = 0; i < MAX_TABS; i++ )
	{
		ced_sheet_destroy ( tabarr[ i ].sheet );
		tabarr[ i ].sheet = NULL;
	}

	return FALSE;
}

static int get_page ( void )
{
	int		page;


	page = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook ) );
	if ( page < 0 || page >= MAX_TABS )
	{
		// Trap stupidity
		return -1;
	}

	return page;
}

static void copy_button (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	ARG_UNUSED ( data )
	)
{
	int		page;
	char		* txt;


	page = get_page ();
	if ( page < 0 )
	{
		return;
	}

	txt = raft_get_clipboard ( tabarr[ page ].sheet );
	if ( ! txt )
	{
		return;
	}

	gtk_clipboard_set_text ( gtk_clipboard_get ( GDK_SELECTION_CLIPBOARD ),
		txt, -1 );

	free ( txt );
}

static void notebook_tab_up (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	ARG_UNUSED ( data )
	)
{
	int		page;


	page = get_page () - 1;
	if ( page < 0 )
	{
		return;
	}

	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), page );
}

static void notebook_tab_down (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	ARG_UNUSED ( data )
	)
{
	int		page,
			max;


	max = gtk_notebook_get_n_pages ( GTK_NOTEBOOK ( notebook ) );

	page = get_page () + 1;
	if ( page < 1 || page >= max )
	{
		return;
	}

	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), page );
}

static void raft_gui_init ( void )
{
	GtkWidget	* vbox,
			* hbox,
			* button;
	GtkAccelGroup	* ag;


	gtk_init ( NULL, NULL );

	main_window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	ag = gtk_accel_group_new ();

	// Set minimum width/height
	gtk_widget_set_usize ( main_window, 400, 300);

	gtk_window_set_default_size ( GTK_WINDOW ( main_window ),
		prefs_get_int ( PREFS_WINDOW_W ),
		prefs_get_int ( PREFS_WINDOW_H )
		);
	gtk_widget_set_uposition ( main_window,
		prefs_get_int ( PREFS_WINDOW_X ),
		prefs_get_int ( PREFS_WINDOW_Y )
		);

	gtk_window_set_title ( GTK_WINDOW ( main_window ), VERSION );

	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox );
	gtk_container_add ( GTK_CONTAINER ( main_window ), vbox );

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

/////////	TOP LINE BUTTONS

	button = gtk_button_new_with_label ( "Quit" );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0);
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( delete_event ), NULL );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Q,
		GDK_CONTROL_MASK, (GtkAccelFlags) 0 );

	progressbar = gtk_progress_bar_new ();
	gtk_widget_show ( progressbar );
	gtk_box_pack_start ( GTK_BOX ( hbox ), progressbar, TRUE, TRUE, 0 );
	gtk_progress_set_percentage ( GTK_PROGRESS ( progressbar ), 0.0 );
	gtk_progress_set_format_string ( GTK_PROGRESS ( progressbar ),
		"Status: Idle" );
	gtk_progress_set_show_text ( GTK_PROGRESS ( progressbar ), TRUE );

	button = gtk_button_new_with_label ( "Copy to Clipboard" );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( copy_button ), NULL );
	button_copy = button;
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_C,
		GDK_CONTROL_MASK, (GtkAccelFlags) 0 );

	gtk_widget_set_sensitive ( button_copy, FALSE );

/////////	NOTEBOOK

	notebook = gtk_notebook_new ();
	gtk_box_pack_start ( GTK_BOX ( vbox ), notebook, TRUE, TRUE, 0 );
	gtk_notebook_set_scrollable ( GTK_NOTEBOOK ( notebook ), TRUE );
	gtk_notebook_set_tab_pos ( GTK_NOTEBOOK ( notebook ), GTK_POS_TOP );

	g_signal_new ( "tab_up", gtk_widget_get_type (), G_SIGNAL_ACTION, 0,
		NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0 );
	gtk_widget_add_accelerator ( notebook, "tab_up", ag,
		GDK_Page_Up, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE );
	g_signal_connect ( G_OBJECT ( notebook ), "tab_up",
		G_CALLBACK ( notebook_tab_up ), NULL );

	g_signal_new ( "tab_down", gtk_widget_get_type (), G_SIGNAL_ACTION, 0,
		NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0 );
	gtk_widget_add_accelerator ( notebook, "tab_down", ag,
		GDK_Page_Down, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE );
	g_signal_connect ( G_OBJECT ( notebook ), "tab_down",
		G_CALLBACK ( notebook_tab_down ), NULL );

/////////	End of main window widget setup

	g_signal_connect ( main_window, "delete_event",
		G_CALLBACK ( delete_event ), NULL );
	gtk_container_set_border_width ( GTK_CONTAINER ( main_window ), 5 );

	gtk_widget_show_all ( main_window );
	gtk_window_add_accel_group ( GTK_WINDOW ( main_window ), ag );

	gtk_window_set_icon ( (GtkWindow *)main_window,
		gdk_pixbuf_new_from_xpm_data ( icon_xpm ) );
}

static void raft_gui_start ( void )
{
	gtk_main ();
}

static int scan_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	int		const	ARG_UNUSED ( row ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	progress = progress + 0.01;

	if ( progress > 1.0 )
	{
		progress = 0;
	}

	gtk_progress_set_percentage ( GTK_PROGRESS ( progressbar ), progress );

	while ( gtk_events_pending () )
	{
		gtk_main_iteration ();	// Keep GUI interactive
	}

	if ( stop_flag )
	{
		return 1;		// User wants to stop
	}

	return 0;			// Keep scanning
}

static void sort_and_populate (
	int		const	tab
	)
{
	gchar		* rtext[ RAFT_COL_TOTAL ] = { NULL };
	int		cols[] = { 1, 0 },
			r,
			c;
	CedSheet	* sheet;
	GtkTreeIter	iter;
	CedCell		* cell;
	char		* tmp;


	sheet = tabarr[ tab ].sheet;

	if ( tabarr[ tab ].sort_direction == GTK_SORT_ASCENDING )
	{
		r = CED_SORT_MODE_DESCENDING;
	}
	else
	{
		r = 0;
	}

	cols[0] = tabarr[ tab ].sort_column + 1;
	ced_sheet_sort_rows ( sheet, 1, 0, cols, r, NULL );

	gtk_list_store_clear ( tabarr[ tab ].store );

	// Populate clist from sheet
	for ( r = 1; ; r++ )
	{
		for ( c = 1; c < RAFT_COL_TOTAL; c++ )
		{
			cell = ced_sheet_get_cell ( sheet, r, c );
			if ( ! cell )
			{
				// Should never happen
				return;
			}

			tmp = ced_cell_create_output ( cell, NULL );

			rtext[ c - 1 ] = mtkit_utf8_from_cstring ( tmp );

			free ( tmp );
		}

		gtk_list_store_append ( tabarr[ tab ].store, &iter );
		gtk_list_store_set ( tabarr[ tab ].store, &iter,
			0, rtext [ 0 ],
			1, rtext [ 1 ],
			2, rtext [ 2 ],
			3, rtext [ 3 ],
			4, rtext [ 4 ],
			5, rtext [ 5 ],
			6, rtext [ 6 ],
			7, rtext [ 7 ],
			8, rtext [ 8 ],
			-1 );

		for ( c = 1; c < RAFT_COL_TOTAL; c++ )
		{
			free ( rtext [ c - 1 ] );
		}
	}
}

static void column_click (
	GtkTreeViewColumn * const	treeviewcolumn,
	gpointer	const		user_data
	)
{
	int		tab = (int)(intptr_t)user_data;


	if ( tab < 0 || tab >= MAX_TABS )
	{
		// Detect bogus tab
		return;
	}

	tabarr[ tab ].sort_column = gtk_tree_view_column_get_sort_column_id (
		treeviewcolumn );

	tabarr[ tab ].sort_direction = gtk_tree_view_column_get_sort_order (
		treeviewcolumn );

	sort_and_populate ( tab );
}

static int get_tree_row (
	GtkTreeView	* const	tree_view
	)
{
	GtkTreeSelection * sel;
	GtkTreeModel	* model;
	GList		* list;
	gint		* gpi;
	int		res = -1;


	sel = gtk_tree_view_get_selection ( tree_view );
	if ( ! sel )
	{
		return -1;
	}

	list = gtk_tree_selection_get_selected_rows ( sel, &model );
	if ( ! list )
	{
		return -1;
	}

	gpi = gtk_tree_path_get_indices ( (GtkTreePath *) ( list->data ) );
	if ( gpi )
	{
		res = gpi[0];
	}

	g_list_foreach ( list, (GFunc) gtk_tree_path_free, NULL );
	g_list_free ( list );

	return res;
}

static void row_activate (
	GtkTreeView	* const	tree_view,
	GtkTreePath	* const	ARG_UNUSED ( path ),
	GtkTreeViewColumn * const	ARG_UNUSED ( column ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		page, row;


	if ( progress >= 0 )
	{
		// Program is busy so do nothing
		return;
	}

	page = get_page ();
	if ( page < 0 )
	{
		return;
	}

	row = get_tree_row ( tree_view );
	if ( row >= 0 )
	{
		char		* new_path;


		new_path = raft_path_merge ( tabarr[ page ].new_path,
			tabarr[ page ].sheet, row + 1 );

		if ( new_path )
		{
			raft_gui_analyse ( new_path );
			free ( new_path );
		}
	}
}

static gint sort_func (
	GtkTreeModel	* const	ARG_UNUSED ( model ),
	GtkTreeIter	* const	ARG_UNUSED ( a ),
	GtkTreeIter	* const	ARG_UNUSED ( b ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	// Always return 0 (a == b)
	return 0;
}

static void tab_add (
	int		const	tab
	)
{
	GtkWidget	* sw,
			* vbox,
			* label,
			* treeview;
	GtkListStore	* store;
	GtkCellRenderer	* renderer;
	GtkTreeViewColumn * column;
	char		* col_titles[] = {
				"Name",
				"Files",
				"%",
				"Bytes",
				"MB",
				"%",
				"Subdirs",
				"Other",
				" "
				};
	char		tab_text[32],
			* tmp
			;
	int		i;


	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox );

	tabarr[tab].entryText = gtk_entry_new ();
	gtk_editable_set_editable ( GTK_EDITABLE ( tabarr[tab].entryText ),
		FALSE );
	gtk_box_pack_start ( GTK_BOX ( vbox ), tabarr[tab].entryText, FALSE,
		FALSE, 0);

	tmp = mtkit_utf8_from_cstring ( tabarr[tab].new_path );
	gtk_entry_set_text ( GTK_ENTRY ( tabarr[tab].entryText ), tmp );
	free ( tmp );

	sw = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW ( sw ),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show ( sw );
	gtk_box_pack_start ( GTK_BOX ( vbox ), sw, TRUE, TRUE, 0);

/////////	Analysis results area

	store = gtk_list_store_new ( TOT_COLS,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING
		);

	tabarr [ tab ].sort_direction = GTK_SORT_ASCENDING;
	tabarr [ tab ].sort_column = 3;
	tabarr [ tab ].store = store;

	treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ) );
	gtk_container_add ( GTK_CONTAINER ( sw ), treeview );
	tabarr[ tab ].treeview = treeview;

	for ( i = 0; i < TOT_COLS; i++ )
	{
		renderer = gtk_cell_renderer_text_new ();

#if GTK_MINOR_VERSION >= 18
		if ( i == 0 )
		{
			gtk_cell_renderer_set_alignment ( renderer, 0, 0 );
		}
		else
		{
			gtk_cell_renderer_set_alignment ( renderer, 1, 0 );
		}

		gtk_cell_renderer_set_padding ( renderer, 10, 0 );
#endif

		column = gtk_tree_view_column_new_with_attributes
			( col_titles[i], renderer, "text", i, NULL );

		gtk_tree_view_column_set_sort_column_id ( column, i );
		gtk_tree_view_append_column ( GTK_TREE_VIEW ( treeview ),
			column );

		g_signal_connect ( column, "clicked",
			G_CALLBACK ( column_click ), (gpointer)(intptr_t)tab );

		gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( store ),
			i, sort_func, NULL, NULL );

		if ( i == (RAFT_COL_BYTES - 1) )
		{
			gtk_tree_view_column_set_sort_order ( column,
				GTK_SORT_ASCENDING );
			gtk_tree_view_column_set_sort_indicator ( column,
				TRUE );
			gtk_tree_view_column_set_sort_column_id ( column, i );
		}
	}

	g_signal_connect ( treeview, "row_activated",
		G_CALLBACK ( row_activate ), NULL );
	snprintf ( tab_text, sizeof ( tab_text ), "%i", tab + 1 );

	label = gtk_label_new ( tab_text );
	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), vbox, label );

	sort_and_populate ( tab );

	gtk_widget_set_sensitive ( button_copy, TRUE );
	gtk_widget_show_all ( vbox );
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), tab );
}

static void raft_gui_analyse (
	char	const * const	directory
	)
{
	int		pages;
	char		* tmp;


	if ( progress != -1.0 )
	{
		return;			// Program is already busy
	}

	pages = gtk_notebook_get_n_pages ( GTK_NOTEBOOK ( notebook ) );

	if (	! directory		||
		! directory[0]		||
		pages < 0		||
		pages >= MAX_TABS
		)
	{
		return;
	}

	tmp = raft_path_check ( directory );
	if ( ! tmp )
	{
		return;
	}

	gtk_progress_set_percentage ( GTK_PROGRESS ( progressbar ), 0.0 );
	gtk_progress_set_format_string ( GTK_PROGRESS ( progressbar ),
		"Status: Working" );

	progress = 0.0;
	stop_flag = 0;

	gtk_widget_set_sensitive ( notebook, FALSE );

	if ( raft_scan_sheet ( tmp, &tabarr[ pages ].sheet, scan_cb, NULL ) )
	{
		goto finish;
	}

	tabarr[pages].new_path = tmp;
	tmp = NULL;

	tab_add ( pages );

finish:
	gtk_progress_set_percentage ( GTK_PROGRESS ( progressbar ), 0.0 );
	gtk_progress_set_format_string ( GTK_PROGRESS ( progressbar ),
		"Status: Idle" );

	progress = -1.0;		// Signal that we are done
	gtk_widget_set_sensitive ( notebook, TRUE );

	if ( tabarr[ pages ].treeview )
	{
		GtkTreePath	* treepath;


		// GTK+2 doesn't automatically select the first item
		treepath = gtk_tree_path_new_from_indices ( 0, -1 );
		gtk_tree_view_set_cursor ( GTK_TREE_VIEW (
			tabarr[ pages ].treeview ), treepath, NULL, FALSE );
		gtk_tree_path_free ( treepath );

		gtk_widget_grab_focus ( tabarr[ pages ].treeview );
	}
	else
	{
		int		page;


		page = get_page ();
		if ( page >= 0 && tabarr[ page ].treeview )
		{
			gtk_widget_grab_focus ( tabarr[ page ].treeview );
		}
	}

	free ( tmp );
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	int			i;
	char	const *	const	scan_directory = raft_cline ( argc, argv );


	prefs_init ();
	prefs_load ();

	raft_gui_init ();

	if ( scan_directory )
	{
		raft_gui_analyse ( scan_directory );
	}

	raft_gui_start ();

	prefs_save ();
	prefs_close ();

	for ( i = 0; i < MAX_TABS; i++ )
	{
		free ( tabarr[ i ].new_path );
	}
	memset ( tabarr, 0, sizeof ( tabarr ) );

	return 0;
}

