/*
	Copyright (C) 2004-2015 Mark Tyler
	Copyright (C) 2006-2008 Mark Tyler and Dmitry Groshev

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



char * mtgex_gtkuncpy (
	char		* const	dest,
	char	const	* const	src,
	size_t		const	cnt
	)
{
	char		* c;


	if ( ! src )
	{
		return NULL;
	}

	c = mtkit_utf8_from_cstring ( src );

	if ( ! dest )
	{
		return c;
	}

	if ( c )
	{
		mtkit_strnncpy ( dest, c, cnt );
		free ( c );
	}
	else
	{
		// Miserable failure so return an empty string
		dest[0] = 0;
	}

	return dest;
}



////	ALERT BOX

static int	alert_result;



static gboolean alert_reply (
	GtkWidget	* const	widget,
	gpointer	const	data
	)
{
	if ( alert_result == 0 )
	{
		alert_result = (int)(intptr_t)data;
	}

	if ( alert_result == 10 )
	{
		gtk_widget_destroy ( widget );
	}

	return FALSE;
}

int mtgex_alert_box (
	char	const	* const	title,
	char	const	* const	message,
	char	const	* const	text1,
	char	const	* const	text2,
	char	const	* const	text3,
	GtkWidget	* const	main_window
	)
{
	GtkWidget	* alert,
			* buttons[3],
			* label;
	char	const	* butxt[3] = { text1, text2, text3 };
	gint		i;
	GtkAccelGroup	* ag;


	if ( ! text1 )
	{
		return 1;
	}

	ag = gtk_accel_group_new ();

	alert = gtk_dialog_new ();
	gtk_window_set_title ( GTK_WINDOW ( alert ), title );
	gtk_window_set_modal ( GTK_WINDOW ( alert ), TRUE );
	gtk_window_set_position ( GTK_WINDOW ( alert ), GTK_WIN_POS_CENTER );
	gtk_container_set_border_width ( GTK_CONTAINER ( alert ), 6 );
	gtk_signal_connect ( GTK_OBJECT ( alert ), "destroy",
			GTK_SIGNAL_FUNC ( alert_reply ), (gpointer) 10 );

	label = gtk_label_new ( message );
	gtk_label_set_line_wrap ( GTK_LABEL ( label ), TRUE );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( alert )->vbox), label,
		TRUE, FALSE, 8 );
	gtk_widget_show ( label );

	for ( i = 0; i <= 2; i++ )
	{
		if ( ! butxt[i] )
		{
			continue;
		}

		buttons[i] = mtgex_add_a_button ( butxt[i], 2,
			GTK_DIALOG ( alert )->action_area, TRUE );
		gtk_signal_connect ( GTK_OBJECT ( buttons[i] ), "clicked",
			GTK_SIGNAL_FUNC ( alert_reply ),
			(gpointer)(intptr_t)(i + 1));
	}

	gtk_widget_add_accelerator ( buttons[0], "clicked", ag, GDK_Escape, 0,
		(GtkAccelFlags) 0);

	if ( main_window )
	{
		gtk_window_set_transient_for ( GTK_WINDOW ( alert ),
			GTK_WINDOW ( main_window ) );
	}

	gtk_widget_show ( alert );
	gdk_window_raise ( alert->window );
	alert_result = 0;
	gtk_window_add_accel_group ( GTK_WINDOW ( alert ), ag );

	while ( alert_result == 0 )
	{
		gtk_main_iteration ();
	}

	if ( alert_result != 10 )
	{
		gtk_widget_destroy ( alert );
	}

	return alert_result;
}

void mtgex_store_window_position (
	GtkWidget	* const	window,
	mtPrefs		* const	prefs,
	char	const	* const	key_prefix
	)
{
	char		name[128];
	gint		xywh[4];
	int		i;
	size_t		len;


	len = strlen ( key_prefix );

	if ( len > 120 )
	{
		return;
	}

	memcpy ( name, key_prefix, len );
	name[len++] = '_';
	name[len + 1] = '\0';
	gdk_window_get_size ( window->window, xywh + 2, xywh + 3 );
	gdk_window_get_root_origin ( window->window, xywh + 0, xywh + 1 );

	for ( i = 0; i < 4; i++ )
	{
		name [ len ] = "xywh"[i];
		mtkit_prefs_set_int ( prefs, name, xywh[i] );
	}
}

void mtgex_restore_window_position (
	GtkWidget	* const	window,
	mtPrefs		* const	prefs,
	char	const	* const	key_prefix
	)
{
	char		name[128];
	gint		xywh[4] = {0};
	int		i;
	size_t		len;


	len = strlen ( key_prefix );

	if ( len > 120 )
	{
		return;
	}

	memcpy ( name, key_prefix, len );
	name[len++] = '_';
	name[len + 1] = '\0';

	for ( i = 0; i < 4; i++ )
	{
		name [ len ] = "xywh"[i];
		mtkit_prefs_get_int ( prefs, name, &xywh[i] );
	}

	gtk_window_set_default_size ( GTK_WINDOW ( window ), xywh[2], xywh[3]);
	gtk_widget_set_uposition ( window, xywh[0], xywh[1] );
}

GtkWidget * mtgex_add_a_window (
	GtkWindowType	const	type,
	char	const	* const	title,
	GtkWindowPosition const	pos,
	gboolean	const	modal
	)
{
	GtkWidget	* win;


	win = gtk_window_new ( type );

	gtk_window_set_title ( GTK_WINDOW ( win ), title );
	gtk_window_set_position ( GTK_WINDOW ( win ), pos );
	gtk_window_set_modal ( GTK_WINDOW ( win ), modal );

	return win;
}

void mtgex_destroy_dialog (
	GtkWidget	* const	window
	)
{
	/* Needed in Windows to stop GTK+ lowering the main window */
	gtk_window_set_transient_for ( GTK_WINDOW ( window ), NULL );
	gtk_widget_destroy ( window );
}

GtkWidget * mtgex_add_a_button (
	char	const	* const	text,
	int		const	bord,
	GtkWidget	* const	box,
	gboolean	const	filler
	)
{
	GtkWidget	* button;


	button = gtk_button_new_with_label ( text );

	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( box ), button, filler, filler, 0 );
	gtk_container_set_border_width ( GTK_CONTAINER ( button), (guint)bord );

	return button;
}

static GtkWidget * spin_new_x (
	GtkObject	* const	adj,
	int		const	fpart
	)
{
	GtkWidget	* spin;


	spin = gtk_spin_button_new ( GTK_ADJUSTMENT ( adj ), 1, (guint)fpart );
	gtk_widget_show ( spin );
	gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON ( spin ), TRUE );

	return spin;
}

GtkWidget * mtgex_add_a_spin (
	int		const	value,
	int		const	min,
	int		const	max
	)
{
	return spin_new_x ( gtk_adjustment_new ( value, min, max, 1, 10, 0 ),
		0 );
}

GtkWidget * mtgex_add_to_table (
	char	const	* const	text,
	GtkWidget	* const	table,
	int		const	row,
	int		const	column,
	int		const	hspacing,
	int		const	vspacing
	)
{
	GtkWidget	* label;


	label = gtk_label_new ( text );
	gtk_widget_show ( label );
	gtk_table_attach ( GTK_TABLE ( table ), label,
		(guint)column, (guint)(column + 1),
		(guint)row, (guint)(row + 1), (GtkAttachOptions) GTK_FILL,
		(GtkAttachOptions) 0, (guint)hspacing, (guint)vspacing );

	gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );

	return label;
}

void mtgex_add_hseparator (
	GtkWidget	* const	widget,
	int		const	xs,
	int		const	ys
	)
{
	GtkWidget	* sep;


	sep = mtgex_pack ( widget, gtk_hseparator_new () );

	gtk_widget_show ( sep );
	gtk_widget_set_usize ( sep, xs, ys );
}

GtkWidget * mtgex_pack (
	GtkWidget	* const	box,
	GtkWidget	* const	widget
	)
{
	gtk_box_pack_start ( GTK_BOX ( box ), widget, FALSE, FALSE, 0 );

	return widget;
}

GtkWidget * mtgex_pack_end (
	GtkWidget	* const	box,
	GtkWidget	* const	widget
	)
{
	gtk_box_pack_end ( GTK_BOX ( box ), widget, FALSE, FALSE, 0 );

	return widget;
}

GtkWidget * mtgex_xpack5 (
	GtkWidget	* const	box,
	GtkWidget	* const	widget
	)
{
	gtk_box_pack_start ( GTK_BOX ( box ), widget, TRUE, TRUE, 5 );

	return widget;
}



// Workaround for GtkCList reordering bug
// From mtPaint and the work of Dmitry Groshev

/* This bug is the favorite pet of GNOME developer Behdad Esfahbod
 * See http://bugzilla.gnome.org/show_bug.cgi?id=400249#c2 */

static void clist_drag_fix (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkDragContext	* const	drag_context,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	g_dataset_remove_data ( drag_context, "gtk-clist-drag-source" );
}

void mtgex_clist_enable_drag (
	GtkWidget	* const	clist
	)
{
	gtk_signal_connect ( GTK_OBJECT ( clist ), "drag_begin",
		GTK_SIGNAL_FUNC ( clist_drag_fix ), NULL );
	gtk_signal_connect ( GTK_OBJECT ( clist ), "drag_end",
		GTK_SIGNAL_FUNC ( clist_drag_fix ), NULL );
	gtk_clist_set_reorderable ( GTK_CLIST ( clist ), TRUE );
}

