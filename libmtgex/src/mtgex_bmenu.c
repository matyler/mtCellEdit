/*
	Copyright (C) 2009-2014 Mark Tyler

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



#define GEXBMENU_DATA_KEY "mtGEX.bmenu_data"



typedef struct gexBmenuItem	gexBmenuItem;



struct gexBmenuItem
{
	char		* text;
	gexBmenuItem	* next;
};

typedef struct
{
	GtkWidget	* button,	// Main button
			* label;	// Label widget

	gexCBmenu	callback;	// Called when value changes
	void		* user_data;

	int		value,		// Current item setting
			tot_items;	// Total number of items
	gexBmenuItem	* item_first,	// List of menu items
			* item_last;

} gexBmenu;



static int mtgex_bmenu_get (
	GtkWidget	*	const	widget,
	gexBmenu	**	const	gexbmenu
	)
{
	if ( ! widget || ! gexbmenu )
	{
		return 1;
	}

	gexbmenu[0] = gtk_object_get_data ( GTK_OBJECT ( widget ),
		GEXBMENU_DATA_KEY );

	return 0;
}

static void bmenu_size_alloc (
	GtkWidget	* const	widget,
	GtkAllocation	* const	alloc,
	gpointer	const	user_data
	)
{
	GtkWidget	* menu = user_data;
	GtkRequisition	rmenu;


	if ( ! menu )
	{
		return;
	}

	gtk_widget_size_request ( menu, &rmenu );

	if ( rmenu.width < alloc->width )
	{
		gtk_widget_set_usize ( menu, alloc->width +
			widget->style->xthickness, -2 );
	}

	// Only call this function once
GTKBUG	g_signal_handlers_disconnect_by_func ( widget,
		G_CALLBACK ( bmenu_size_alloc ), user_data );
}

static void set_menu_pos (
	GtkMenu		* const	menu,
	gint		* const	x,
	gint		* const	y,
	gboolean	* const	ARG_UNUSED ( push_in ),
	gpointer	const	user_data
	)
{
	gexBmenu	* bmenu = user_data;
	GtkRequisition	req;
	gint		wx,
			wy,
			wh,
			scrw,
			scrh;


	if ( ! bmenu )
	{
		return;
	}

	gdk_window_get_origin ( bmenu->button->window, &wx, &wy );
	wx += bmenu->button->allocation.x;
	wx -= bmenu->button->style->xthickness;
	wy += bmenu->button->allocation.y;
	wy -= bmenu->button->style->ythickness;

	gtk_widget_size_request ( bmenu->button, &req );
	wh = req.height;

	x[0] = wx;
	y[0] = wy + wh;

	// Ensure that this all sits inside the screen
	scrw = gdk_screen_width ();
	scrh = gdk_screen_height ();

	gtk_widget_size_request ( GTK_WIDGET ( menu ), &req );

	if ( x[0] + req.width > scrw )
	{
		x[0] = scrw - req.width - 1;
	}

	if ( y[0] + req.height > scrh )
	{
		y[0] = scrh - req.height - 1;
	}

	if ( x[0] < 0 )
	{
		x[0] = 0;
	}

	if ( y[0] < 0 )
	{
		y[0] = 0;
	}

	g_signal_connect ( G_OBJECT ( bmenu->button ), "size_allocate",
		G_CALLBACK ( bmenu_size_alloc ), (gpointer)menu );
}

static void pressed_popup_item (
	GtkWidget	* const	widget,
	gpointer	const	user_data
	)
{
	gexBmenu	* const	bmenu = user_data;
	int		item;


	if ( ! bmenu )
	{
		return;
	}


	gpointer gp = gtk_object_get_user_data ( GTK_OBJECT ( widget ) );

	item = (int)(intptr_t)gp;
	mtgex_bmenu_set_value ( bmenu->button, item );
}

static void clicked_bmenu (
	GtkButton	* const	ARG_UNUSED ( button ),
	gpointer	const	user_data
	)
{
	GtkWidget	* popup,
			* menu_item,
			* start = NULL;
	gexBmenu	* const	bmenu = user_data;
	gexBmenuItem	* bmi;
	int		i;


	if ( ! bmenu || bmenu->tot_items < 1 )
	{
		return;
	}

	popup = gtk_menu_new ();

	for (	bmi = bmenu->item_first,	i = 0;
		bmi;
		bmi = bmi->next,		i++
		)
	{
		menu_item = gtk_menu_item_new_with_label ( bmi->text );
		gtk_widget_show ( menu_item );

		gtk_menu_append ( GTK_MENU ( popup ), menu_item );
		g_signal_connect ( G_OBJECT ( menu_item ), "activate",
			G_CALLBACK ( pressed_popup_item ), bmenu );
		gtk_object_set_user_data ( GTK_OBJECT ( menu_item ),
			(gpointer)(intptr_t)i );

		if ( i == bmenu->value )
		{
			start = menu_item;
		}

		gtk_menu_shell_select_item ( GTK_MENU_SHELL ( popup ),
			menu_item );
	}

	if ( start )
	{
		gtk_menu_shell_select_item ( GTK_MENU_SHELL ( popup ), start );
	}

	gtk_menu_popup ( GTK_MENU ( popup ), NULL, NULL, set_menu_pos, bmenu, 0,
		0 );
}

static gboolean keypress_bmenu (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventKey	* const	event,
	gpointer	const	user_data
	)
{
	gexBmenu	* const	bmenu = user_data;


	if ( ! bmenu )
	{
		return FALSE;
	}

	switch ( event->keyval )	// Handle any control keys
	{
	case GDK_KP_Up:
	case GDK_Up:
		mtgex_bmenu_set_value ( bmenu->button, bmenu->value - 1 );
		break;

	case GDK_KP_Down:
	case GDK_Down:
		mtgex_bmenu_set_value ( bmenu->button, bmenu->value + 1 );
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

static void empty_bmenu (
	gexBmenu	* const	bmenu
	)
{
	gexBmenuItem	* it1,
			* it2;


	for ( it1 = bmenu->item_first; it1; it1 = it2 )
	{
		it2 = it1->next;
		free ( it1->text );
		free ( it1 );
	}

	bmenu->item_first = NULL;
	bmenu->item_last = NULL;
	bmenu->tot_items = 0;
	bmenu->value = -1;

	gtk_label_set_text ( GTK_LABEL ( bmenu->label ), "" );
}

static gboolean destroy_bmenu (
	GtkWidget	* const	widget,
	GdkEvent	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	gexBmenu	* bmenu;


	if ( mtgex_bmenu_get ( widget, &bmenu ) )
	{
		fprintf ( stderr,
			"mtGEX error: destroy_bmenu unable to find bmenu\n" );

		return FALSE;
	}

	if ( ! bmenu )
	{
		return FALSE;
	}

	empty_bmenu ( bmenu );

// Stop all callbacks to avoid any dangling references to to destroyed structure
GTKBUG	g_signal_handlers_disconnect_by_func ( bmenu->button,
		G_CALLBACK ( clicked_bmenu ), bmenu );
GTKBUG	g_signal_handlers_disconnect_by_func ( bmenu->button,
		G_CALLBACK (keypress_bmenu ), bmenu );

	gtk_object_remove_data ( GTK_OBJECT ( widget ), GEXBMENU_DATA_KEY );
	free ( bmenu );

	return FALSE;
}

GtkWidget * mtgex_bmenu_new ( void )
{
	GtkWidget	* button,
			* w,
			* hbox;
	gexBmenu	* bmenu;


	bmenu = calloc ( sizeof ( gexBmenu ), 1 );
	if ( ! bmenu )
	{
		return NULL;
	}

	bmenu->value = -1;	// An empty list should return -1 for value

	button = gtk_button_new ();
	gtk_object_set_data ( GTK_OBJECT ( button ), GEXBMENU_DATA_KEY, bmenu );
	gtk_widget_show ( button );
	bmenu->button = button;

	g_signal_connect ( G_OBJECT ( button ), "destroy",
		G_CALLBACK ( destroy_bmenu ), NULL );
	g_signal_connect ( G_OBJECT ( button ), "clicked",
		G_CALLBACK ( clicked_bmenu ), bmenu );
	g_signal_connect ( G_OBJECT ( button ), "key_press_event",
		G_CALLBACK ( keypress_bmenu ), bmenu );

	hbox = gtk_hbox_new ( FALSE, 5 );
	gtk_widget_show ( hbox );
	gtk_container_add ( GTK_CONTAINER ( button ), hbox );

	w = gtk_arrow_new ( GTK_ARROW_DOWN, GTK_SHADOW_IN );
	gtk_widget_show ( w );
	gtk_box_pack_end ( GTK_BOX ( hbox ), w, FALSE, FALSE, 0 );

	w = gtk_vseparator_new ();
	gtk_widget_show ( w );
	gtk_box_pack_end ( GTK_BOX ( hbox ), w, FALSE, FALSE, 0 );

	w = gtk_label_new ( "" );
	gtk_widget_show ( w );
	gtk_box_pack_start ( GTK_BOX ( hbox ), w, FALSE, FALSE, 4 );
	bmenu->label = w;

	return button;
}

int mtgex_bmenu_add_item (
	GtkWidget	* const	widget,
	char	const	* const	item
	)
{
	gexBmenu	* bmenu;
	gexBmenuItem	* it1;


	if (	mtgex_bmenu_get ( widget, &bmenu ) ||
		! bmenu
		)
	{
		return -1;
	}

	if ( ! item )
	{
		// Empty list requested
		empty_bmenu ( bmenu );

		return 0;
	}

	it1 = calloc ( sizeof ( gexBmenuItem ), 1 );
	if ( ! it1 )
	{
		return -1;
	}

	it1->text = strdup ( item );
	if ( ! it1->text )
	{
		free ( it1 );

		return -1;
	}

	bmenu->tot_items ++;

	if ( ! bmenu->item_last )
	{
		// First item entered

		bmenu->item_first = it1;
		bmenu->item_last = it1;
		mtgex_bmenu_set_value ( widget, 0 );
	}
	else
	{
		bmenu->item_last->next = it1;
		bmenu->item_last = it1;
	}

	return (bmenu->tot_items - 1);	// Index of new item
}

int mtgex_bmenu_set_value (
	GtkWidget	* const	widget,
	int		const	value
	)
{
	int		i;
	gexBmenu	* bmenu;
	gexBmenuItem	* it;


	if (	mtgex_bmenu_get ( widget, &bmenu )	||
		! bmenu					||
		value < 0				||
		value >= bmenu->tot_items
		)
	{
		return 1;
	}

	it = bmenu->item_first;

	for ( i = 0; i < value; i++ )
	{
		it = it->next;
	}

	gtk_label_set_text ( GTK_LABEL ( bmenu->label ), it->text );

	bmenu->value = value;

	if ( bmenu->callback && bmenu->value >= 0 )
	{
		bmenu->callback ( widget, bmenu->user_data, bmenu->value );
	}

	return 0;
}

int mtgex_bmenu_get_value (
	GtkWidget	* const	widget
	)
{
	gexBmenu	* bmenu;


	if (	mtgex_bmenu_get ( widget, &bmenu ) ||
		! bmenu
		)
	{
		return -1;
	}

	return bmenu->value;
}

int mtgex_bmenu_get_value_from_text (
	GtkWidget	* const	widget,
	char	const	* const	text
	)
{
	gexBmenu	* bmenu;
	gexBmenuItem	* bmi;
	int		i;


	if (	! text ||
		mtgex_bmenu_get ( widget, &bmenu ) ||
		! bmenu
		)
	{
		return -1;
	}

	for (	bmi = bmenu->item_first,	i = 0;
		bmi;
		bmi = bmi->next,		i++
		)
	{
		if ( ! strcmp ( bmi->text, text ) )
		{
			return i;	// Found
		}
	}

	return -1;			// Not found
}

char * mtgex_bmenu_get_value_text (
	GtkWidget	* const	widget,
	int		const	item
	)
{
	gexBmenu	* bmenu;
	gexBmenuItem	* bmi;
	int		i;


	if (	mtgex_bmenu_get ( widget, &bmenu )	||
		! bmenu					||
		item >= bmenu->tot_items
		)
	{
		return NULL;
	}

	for (	bmi = bmenu->item_first,	i = 0;
		bmi &&				i < item;
		bmi = bmi->next,		i++ )
	{
	}

	if ( ! bmi )
	{
		return NULL;
	}

	return bmi->text;
}

int mtgex_bmenu_set_callback (
	GtkWidget	* const	widget,
	gexCBmenu	const	callback,
	void		* const	user_data
	)
{
	gexBmenu	* bmenu;


	if ( mtgex_bmenu_get ( widget, &bmenu ) || ! bmenu )
	{
		return 1;
	}

	bmenu->callback = callback;
	bmenu->user_data = user_data;

	return 0;
}

