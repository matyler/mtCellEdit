/*
	Copyright (C) 2011-2015 Mark Tyler

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
	GR_MENU_TOP,

	GR_MENU_DUPLICATE,
	GR_MENU_RENAME,
	GR_MENU_DELETE,
	GR_MENU_REDRAW,
	GR_MENU_EXPORT,
	GR_MENU_SELECTION,

	GR_MENU_TOTAL
};



static int		graph_total;
static GtkWidget	* graph_bmenu,
			* graph_image,
			* graph_menu_widgets[GR_MENU_TOTAL],
			* gredit_view;
static GtkTextBuffer	* gredit_buffer;



static void redraw_graph ( void )
{
	int		err	= -1;
	mtImage		* im;
	double		scale;
	CedBook		* book;


	scale = prefs_get_double ( GUI_INIFILE_GRAPH_SCALE );

	book = global.file->cubook->book;
	im = cui_graph_render_mtimage ( book, book->prefs.active_graph, &err,
		scale );

	mtgex_image_set_image ( graph_image, im, 1 );

	if ( err >= 0 )
	{
		GtkTextIter	start, end;


		gtk_text_buffer_get_start_iter ( gredit_buffer, &start );
		gtk_text_buffer_get_end_iter ( gredit_buffer, &end );
		gtk_text_iter_set_offset ( &start, err );

		gtk_text_buffer_select_range ( gredit_buffer, &start, &end );
	}
}

void graph_gui_store_changes ( void )
{
	// Checks for changes, if so save them to the current graph
	// (active_graph).

	if ( gtk_text_buffer_get_modified ( gredit_buffer ) )
	{
		char		* txt = NULL;
		GtkTextIter	start,
				end;
		CedBookFile	* bookfile;
		size_t		len;


		bookfile = cui_file_get_graph ( global.file );
		if ( ! bookfile )
		{
			return;
		}

		gtk_text_buffer_get_start_iter ( gredit_buffer, &start );
		gtk_text_buffer_get_end_iter ( gredit_buffer, &end );
		txt = gtk_text_buffer_get_text ( gredit_buffer, &start, &end,
			FALSE );

		len = strlen ( txt );
		if ( len > INT_MAX )
		{
			return;
		}

		// Free old memory and use newly allocated text
		free ( bookfile->mem );
		bookfile->mem = txt;
		bookfile->size = (int)len;
		ced_book_timestamp_file ( bookfile );

		update_changes_chores ( 0, 1 );
		gtk_text_buffer_set_modified ( gredit_buffer, FALSE );

		redraw_graph ();
	}
}

static void graph_bmenu_changed (
	GtkWidget	* const	widget,
	void		* const	ARG_UNUSED ( user_data ),
	int		const	new_value
	)
{
	char	const	* txt = "";
	char	const	* graph_name;
	int		txt_len = 0;
	CedBookFile	* bookfile;
	GtkTextIter	iter;
	CedBook		* book;


	graph_gui_store_changes ();

	graph_name = mtgex_bmenu_get_value_text ( widget, new_value );

	book = global.file->cubook->book;
	mtkit_strfreedup ( &book->prefs.active_graph, graph_name );

	redraw_graph ();		// Draw the new graph

	bookfile = cui_graph_get ( book, graph_name );
	if ( bookfile && bookfile->mem )
	{
		txt = bookfile->mem;
		txt_len = bookfile->size;
	}

	gtk_text_buffer_set_text ( GTK_TEXT_BUFFER ( gredit_buffer ), txt,
		txt_len );
	gtk_text_buffer_set_modified ( gredit_buffer, FALSE );

	// Move the cursor back to the top
	gtk_text_buffer_get_start_iter ( gredit_buffer, &iter );
	gtk_text_buffer_place_cursor ( gredit_buffer, &iter );
}

void show_hide_graphname ( void )
{
	int		graph_visible;


	graph_visible = GTK_WIDGET_VISIBLE ( global.pane_graph );

	if ( graph_visible )
	{
		gtk_widget_show ( graph_menu_widgets[GR_MENU_TOP] );
	}
	else
	{
		gtk_widget_hide ( graph_menu_widgets[GR_MENU_TOP] );
	}

	if ( graph_total > 0 && graph_visible )
	{
		gtk_widget_show ( graph_bmenu );
		gtk_widget_set_sensitive ( global.pane_graph, TRUE );
	}
	else
	{
		gtk_widget_hide ( graph_bmenu );
		gtk_widget_set_sensitive ( global.pane_graph, FALSE );
	}
}

void pressed_graph (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( GTK_WIDGET_VISIBLE ( global.pane_graph ) )
	{
		pane_pair_set ( PANE_PAIR_HIDE_GRAPH );
	}
	else
	{
		pane_pair_set ( PANE_PAIR_SHOW_GRAPH );
	}
}

static int populate_graph_list_cb (
	CedBook		* const	ARG_UNUSED ( book ),
	char	const	* const	graph_name,
	CedBookFile	* const	ARG_UNUSED ( bookfile ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	mtgex_bmenu_add_item ( graph_bmenu, graph_name );

	graph_total ++;

	return 0;			// Continue
}

void graph_gui_update (
	char	const	* const	graph_name
	)
{
	int		i;

	/*
	NOTE: this function is called after loading a new book/sheet so we can't
	save data at this point as we might be storing an old graph script to a
	new file with the same graph name.
	*/

	// Clear the bmenu, remove the current graph and its text
	mtgex_bmenu_add_item ( graph_bmenu, NULL );
	mtgex_image_set_image ( graph_image, NULL, 1 );
	gtk_text_buffer_set_text ( GTK_TEXT_BUFFER ( gredit_buffer  ), "", 0 );
	gtk_text_buffer_set_modified ( gredit_buffer, FALSE );
	graph_total = 0;

	// Turn off callbacks for now
	mtgex_bmenu_set_callback ( graph_bmenu, NULL, NULL );

	cui_graph_scan ( global.file->cubook->book, populate_graph_list_cb,
		NULL );

	// Turn on callbacks again
	mtgex_bmenu_set_callback ( graph_bmenu, graph_bmenu_changed, NULL );

	if ( graph_total > 0 )
	{
		int		item = 0;


		if ( graph_name )
		{
			item = mtgex_bmenu_get_value_from_text ( graph_bmenu,
				graph_name );

			if ( item < 0 )
			{
				// Not on list, so select first
				item = 0;
			}
		}

		mtgex_bmenu_set_value ( graph_bmenu, item );
	}

	for ( i = GR_MENU_TOP + 1; i < GR_MENU_TOTAL; i++ )
	{
		gtk_widget_set_sensitive ( graph_menu_widgets[i],
			graph_total ? TRUE : FALSE );
	}

	show_hide_graphname ();
}

static void pressed_graph_new (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	char	const	* newname;


	graph_gui_store_changes ();

	newname = be_graph_new ( global.file->cubook->book );

	if ( newname )
	{
		graph_gui_update ( newname );
	}
	else
	{
		mtgex_alert_box ( _("Error"),
			_("Unable to create a new graph."), _("OK"),
			NULL, NULL, global.main_window );
	}

	update_changes_chores ( 0, 1 );
}

static void pressed_graph_duplicate (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	char		* newname = NULL;


	graph_gui_store_changes ();

	newname = be_graph_duplicate ( global.file->cubook );

	if ( newname )
	{
		graph_gui_update ( newname );
		free ( newname );
	}
	else
	{
		mtgex_alert_box( _("Error"),
			_("Unable to duplicate the current graph."),
			_("OK"), NULL, NULL, global.main_window );
	}

	update_changes_chores ( 0, 1 );
}

static int rename_graph (
	char	const	* const	old_name,
	char	const	* const	new_name
	)
{
	char	const	* msg = NULL;
	CedBookFile	* old;
	CedBook		* book = global.file->cubook->book;


	if ( ! new_name || new_name[0] == 0 )
	{
		msg = _("Bad graph name");
	}
	else if ( cui_graph_get ( book, new_name ) )
	{
		msg = _("Graph name already exists");
	}

	if ( msg )
	{
		mtgex_alert_box ( _("Error"), msg, _("OK"), NULL, NULL,
			global.main_window );

		return 1;
	}

	old = cui_graph_get ( book, old_name );
	if (	! old ||
		! cui_graph_new ( book, old->mem, old->size, new_name )
		)
	{
		msg = _("Unable to rename graph");
		mtgex_alert_box ( _("Error"), msg, _("OK"), NULL, NULL,
			global.main_window );

		return 1;
	}
	else
	{
		old->mem = NULL;
		cui_graph_destroy ( book, old_name );
	}

	graph_gui_update ( new_name );
	update_changes_chores ( 0, 1 );

	return 0;
}

static void pressed_graph_rename (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res;
	char	const	* old_name;
	char		* new_name;


	old_name = global.file->cubook->book->prefs.active_graph;

	if ( ! old_name )
	{
		return;
	}

	graph_gui_store_changes ();

	for ( res = 1; res; )
	{
		new_name = dialog_text_entry ( _("Rename Graph"), old_name );

		if ( new_name )
		{
			res = rename_graph ( old_name, new_name );
			free ( new_name );
		}
		else
		{
			res = 0;
		}
	}
}

static void pressed_graph_delete (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	char		* graph_name;
	int		i,
			res;
	CedBook		* book;


	res = mtgex_alert_box ( _("Question"),
		_("Do you really want to delete this graph?"),
		_("No"), _("Yes"), NULL, global.main_window );

	if ( res != 2 )
	{
		return;
	}

	i = mtgex_bmenu_get_value ( graph_bmenu ) - 1;
	if ( i < 0 )
	{
		i = 0;
	}

	book = global.file->cubook->book;
	if ( cui_graph_destroy ( book, book->prefs.active_graph ) )
	{
		mtgex_alert_box ( _("Error"), _("Unable to delete this graph"),
			_("OK"), NULL, NULL, global.main_window );

		return;
	}

	graph_name = mtgex_bmenu_get_value_text ( graph_bmenu, i );
	if ( graph_name )
	{
		graph_name = strdup ( graph_name );

		/*
		NOTE: we need to strdup here because the original
		graph_name is free'd in graph_gui_update ()
		*/
	}

	graph_gui_update ( graph_name );
	update_changes_chores ( 0, 1 );
	free ( graph_name );
}

void pressed_graph_redraw (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( gtk_text_buffer_get_modified ( gredit_buffer ) )
	{
		graph_gui_store_changes ();
	}
	else
	{
		redraw_graph ();
	}
}

static void pressed_graph_export (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	file_selector ( FS_EXPORT_GRAPH, global.main_window, NULL );
}

static void pressed_graph_plot_sel (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	GtkClipboard	* clip;
	char		txt [ 2000 ] = { 0 };


	if ( be_graph_selection_clip ( global.cedview->ren.sheet, txt,
		sizeof ( txt ) ) )
	{
		mtkit_strnncpy ( txt, _("No sheet available"), sizeof ( txt ) );
	}

	clip = gtk_clipboard_get ( GDK_SELECTION_CLIPBOARD );
	gtk_clipboard_set_text ( clip, txt, -1 );
}

void graph_gui_build (
	GtkWidget	* const	vbox,
	GtkWidget	* const	hb,
	GtkItemFactory	* const	itemfactory
	)
{
	int		i;
	char		* menu_text[GR_MENU_TOTAL] = {
			_("/Graph"),
			_("/Graph/Duplicate"),
			_("/Graph/Rename ..."),
			_("/Graph/Delete ..."),
			_("/Graph/Redraw"),
			_("/Graph/Export ..."),
			_("/Graph/Sheet Selection to Clipboard")
			};
	GtkWidget	* pane,
			* sw;
	GtkItemFactoryEntry menu_items[] = {
{ _("/_Graph"),			NULL, NULL, 0, "<Branch>", NULL },
{ _("/Graph/tear"),		NULL, NULL, 0, "<Tearoff>", NULL },
{ _("/Graph/New"),		"", pressed_graph_new, 0, "<StockItem>", GTK_STOCK_NEW },
{ menu_text[GR_MENU_DUPLICATE],	"", pressed_graph_duplicate, 0, "<StockItem>", GTK_STOCK_COPY },
{ _("/Graph/sep1"),		NULL, NULL, 0, "<Separator>", NULL },
{ menu_text[GR_MENU_RENAME],	NULL, pressed_graph_rename, 0, "<StockItem>", GTK_STOCK_EDIT },
{ menu_text[GR_MENU_DELETE],	NULL, pressed_graph_delete, 0, "<StockItem>", GTK_STOCK_DELETE },
{ _("/Graph/sep2"),		NULL, NULL, 0, "<Separator>", NULL },
{ menu_text[GR_MENU_REDRAW],	"<control>F5", pressed_graph_redraw, 0, "<StockItem>", GTK_STOCK_REFRESH },
{ menu_text[GR_MENU_EXPORT],	NULL, pressed_graph_export, 0, "<StockItem>", GTK_STOCK_SAVE_AS },
{ menu_text[GR_MENU_SELECTION],	NULL, pressed_graph_plot_sel, 0, NULL, NULL }
	};


///	MENU

	gtk_item_factory_create_items_ac ( itemfactory,
		sizeof ( menu_items ) / sizeof ( ( menu_items[0] ) ),
		menu_items, NULL, 2 );

	for ( i = 0; i < GR_MENU_TOTAL; i++ )
	{
		graph_menu_widgets[i] = gtk_item_factory_get_item (
			itemfactory, menu_text[i] );
	}

	graph_bmenu = mtgex_bmenu_new ();
	gtk_box_pack_start ( GTK_BOX ( hb ), graph_bmenu, TRUE, FALSE, 0 );
	mtgex_bmenu_set_callback ( graph_bmenu, graph_bmenu_changed, NULL );

	g_signal_connect ( G_OBJECT ( vbox ), "key_press_event",
		G_CALLBACK ( key_event_escape ), NULL );

	pane = gtk_vpaned_new ();
	gtk_widget_show ( pane );
	gtk_box_pack_start ( GTK_BOX ( vbox ), pane, TRUE, TRUE, 0 );

	graph_image = mtgex_image_new ();
	gtk_paned_pack1 ( GTK_PANED ( pane ), graph_image, FALSE, TRUE );

	sw = gtk_scrolled_window_new ( NULL, NULL );
	gtk_widget_show ( sw );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

	gtk_paned_pack2 ( GTK_PANED ( pane ), sw, FALSE, TRUE );

	gredit_view = gtk_text_view_new ();
	gtk_widget_show ( gredit_view );
	gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW ( gredit_view ),
		GTK_WRAP_WORD );
	gtk_container_add ( GTK_CONTAINER ( sw ), gredit_view );

	gredit_buffer = gtk_text_view_get_buffer (
		GTK_TEXT_VIEW ( gredit_view ) );

	gtk_widget_hide ( graph_menu_widgets[GR_MENU_TOP] );
}

