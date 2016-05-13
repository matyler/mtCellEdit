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



#define PREFS_DATA_KEY "mtGEX.prefs_data"



enum
{
	PREFS_CLIST_COL_KEY,
	PREFS_CLIST_COL_STATUS,
	PREFS_CLIST_COL_TYPE,
	PREFS_CLIST_COL_VALUE,

	PREFS_CLIST_COLS
};



typedef struct
{
	mtPrefs		* prefs;
	GtkWidget	* clist,
			* entry,
			* main_window,
			* edit_item_window,
			* edit_entry,
			* prefs_window,
			* sort_arrows[PREFS_CLIST_COLS]	// Column sort arrows
			;

	mtPrefValue	* piv;		// Used when editing a list row

	mtPrefCB	closure_cb;	// Callback called when window is closed
	mtPrefValue	* closure_piv;
	int		closure_data;

	int		sort_column;
	GtkSortType	sort_direction;	// Sort direction of clist
} gexPrefsData;



static void prefs_close_window (
	GtkWidget	* const	window
	)
{
	int		i;
	char		txt[256];
	gexPrefsData	* pdata;
	GtkCListColumn	* col;


	pdata = (gexPrefsData *) gtk_object_get_data ( GTK_OBJECT ( window ),
		PREFS_DATA_KEY );
	col = GTK_CLIST ( pdata->clist )->column;

	for ( i = 0; i < PREFS_CLIST_COLS; i++ )
	{
		// Remember column widths

		snprintf ( txt, sizeof ( txt ), "prefs.col%i", i + 1 );
		mtkit_prefs_set_int ( pdata->prefs, txt, col[i].width );
	}

	mtgex_store_window_position ( window, pdata->prefs, "prefs.window" );

	if ( pdata->closure_cb )
	{
		pdata->closure_cb ( pdata->closure_piv, pdata->closure_data,
			NULL );
	}

	free ( pdata );
	mtgex_destroy_dialog ( window );
}

static void prefs_close_window_button (
	GtkButton	* const	ARG_UNUSED ( buttonwidget ),
	gpointer	const	user_data
	)
{
	prefs_close_window ( user_data );
}

static gboolean fs_destroy (
	GtkWidget	* const	fs
	)
{
	gexPrefsData	* pdata;


	pdata = (gexPrefsData *) gtk_object_get_data ( GTK_OBJECT ( fs ),
		PREFS_DATA_KEY );

	if ( ! pdata )
	{
		return FALSE;
	}

	mtgex_store_window_position ( fs, pdata->prefs, "prefs.file.window" );
	mtgex_fpick_destroy ( fs );

	return FALSE;
}

static void update_clist_status_value (
	GtkWidget	* const	clist,
	mtPrefValue	* const	piv,
	int			row
	)
{
	GList		* sel;
	char		* status_txt[2] = { _("default"), _("user set") },
			* st = "",
			buf[256],
			* nbuf,
			* bp;


	if ( row < 0 )
	{
		sel = GTK_CLIST ( clist )->selection;

		if ( ! sel )
		{
			return;
		}

		row = GPOINTER_TO_INT ( sel->data );
	}

	if ( piv->def )
	{
		if ( strcmp ( piv->value, piv->def ) == 0 )
		{
			st = status_txt[0];
		}
		else
		{
			st = status_txt[1];
		}
	}
	else		// Default is NULL
	{
		if ( piv->value[0] == 0 )
		{
			st = status_txt[0];
		}
		else
		{
			st = status_txt[1];
		}
	}

	mtkit_prefs_get_str_val ( piv, piv->value, buf, 256 );

	// Remove at any newlines
	while ( ( bp = strchr ( buf, '\n' ) ) )
	{
		bp[0] = ' ';
	}

	gtk_clist_set_text ( GTK_CLIST ( clist ), row, PREFS_CLIST_COL_STATUS,
		st );

	nbuf = mtgex_gtkuncpy ( NULL, buf, 0 );

	gtk_clist_set_text ( GTK_CLIST ( clist ), row, PREFS_CLIST_COL_VALUE,
		nbuf );

	free ( nbuf );
}

static void fs_ok (
	GtkWidget	* const	fs
	)
{
	gexPrefsData	* pdata;
	char		fname [ PATHBUF ];


	pdata = (gexPrefsData *) gtk_object_get_data ( GTK_OBJECT ( fs ),
		PREFS_DATA_KEY );

	if ( ! pdata )
	{
		return;
	}

	// Get filename the proper way (convert it from UTF8 in GTK2/Windows,
	// leave it in system filename encoding on Unix)

	mtkit_strnncpy ( fname, mtgex_fpick_get_filename ( fs, FALSE ),
		sizeof ( fname ) );

	mtkit_prefs_set_str ( pdata->prefs, pdata->piv->key, fname );
	update_clist_status_value ( pdata->clist, pdata->piv, -1 );

	fs_destroy ( fs );
}

static void prefs_edit_button_cancel (
	GtkButton	* const	ARG_UNUSED ( buttonwidget ),
	gpointer	const	user_data
	)
{
	gexPrefsData	* const	pdata = user_data;


	if ( ! pdata )
	{
		return;
	}

	if ( pdata->piv->type == MTKIT_PREF_TYPE_RGB )
	{
		mtgex_store_window_position ( pdata->edit_item_window,
			pdata->prefs, "prefs.color.window" );
	}
	else
	{
		mtgex_store_window_position ( pdata->edit_item_window,
			pdata->prefs, "prefs.edit.window" );
	}

	mtgex_destroy_dialog ( pdata->edit_item_window );
}

static void prefs_close_edit_window (
	GtkWidget	* const	window
	)
{
	gexPrefsData	* pdata;


	pdata = (gexPrefsData *) gtk_object_get_data ( GTK_OBJECT ( window ),
		PREFS_DATA_KEY );

	prefs_edit_button_cancel ( NULL, pdata );
}

static void prefs_edit_button_reset (
	GtkButton	* const	buttonwidget,
	gpointer	const	user_data
	)
{
	gexPrefsData	* pdata = user_data;


	if ( ! pdata )
	{
		return;
	}

	if (	pdata->piv->type == MTKIT_PREF_TYPE_DIR ||
		pdata->piv->type == MTKIT_PREF_TYPE_FILE
		)
	{
		fs_destroy ( pdata->edit_item_window );
	}
	else
	{
		prefs_edit_button_cancel ( buttonwidget, user_data );
	}

	mtkit_prefs_set_default ( pdata->prefs, pdata->piv->key );
	update_clist_status_value ( pdata->clist, pdata->piv, -1 );
}

static void prefs_edit_button_ok (
	GtkButton	* const	buttonwidget,
	gpointer	const	user_data
	)
{
	int		num = 0,
			update = MTKIT_PREF_TYPE_NONE;
	double		numd = 0;
	char	const	* astr = "";
	gexPrefsData	* const	pdata= user_data;
	GtkTextBuffer	* buffer;


	if ( ! pdata || ! pdata->edit_entry )
	{
		return;
	}

	switch ( pdata->piv->type )
	{
	case MTKIT_PREF_TYPE_INT:
		gtk_spin_button_update ( GTK_SPIN_BUTTON ( pdata->edit_entry )
			);
		num = gtk_spin_button_get_value_as_int (
			GTK_SPIN_BUTTON ( pdata->edit_entry ) );
		update = MTKIT_PREF_TYPE_INT;

		break;

	case MTKIT_PREF_TYPE_BOOL:
	case MTKIT_PREF_TYPE_OPTION:
		num = mtgex_bmenu_get_value ( pdata->edit_entry );

		if ( num < 0 )
		{
			num = 0;
		}

		update = MTKIT_PREF_TYPE_INT;

		break;

	case MTKIT_PREF_TYPE_DOUBLE:
		gtk_spin_button_update ( GTK_SPIN_BUTTON ( pdata->edit_entry )
			);
		numd = gtk_spin_button_get_value (
			GTK_SPIN_BUTTON ( pdata->edit_entry ) );
		update = MTKIT_PREF_TYPE_DOUBLE;

		break;

	case MTKIT_PREF_TYPE_STR:
		astr = gtk_entry_get_text ( GTK_ENTRY ( pdata->edit_entry ) );
		update = MTKIT_PREF_TYPE_STR;

		break;

	case MTKIT_PREF_TYPE_STR_MULTI:
		buffer = gtk_text_view_get_buffer (
			GTK_TEXT_VIEW ( pdata->edit_entry ) );

		if ( buffer )
		{
			gchar		* nfs;
			GtkTextIter	start, end;


			gtk_text_buffer_get_start_iter ( buffer, &start );
			gtk_text_buffer_get_end_iter ( buffer, &end );
			nfs = gtk_text_buffer_get_text ( buffer, &start,
				&end, FALSE );

			if ( nfs )
			{
				update = MTKIT_PREF_TYPE_STR_MULTI;
				mtkit_prefs_set_str ( pdata->prefs,
					pdata->piv->key, nfs );
				g_free ( nfs );
			}
		}

		break;

	case MTKIT_PREF_TYPE_RGB:
		num = mtgex_cpick_get_color ( pdata->edit_entry, NULL );
		update = MTKIT_PREF_TYPE_INT;

		break;

	default:
		break;
	}

	switch ( update )
	{
	case MTKIT_PREF_TYPE_INT:
		mtkit_prefs_set_int ( pdata->prefs, pdata->piv->key, num );
		break;

	case MTKIT_PREF_TYPE_DOUBLE:
		mtkit_prefs_set_double ( pdata->prefs, pdata->piv->key, numd );
		break;

	case MTKIT_PREF_TYPE_STR:
		mtkit_prefs_set_str ( pdata->prefs, pdata->piv->key, astr );
		break;

	default:
		break;
	}

	if ( update != MTKIT_PREF_TYPE_NONE )
	{
		update_clist_status_value ( pdata->clist, pdata->piv, -1 );
	}

	prefs_edit_button_cancel ( buttonwidget, user_data );
}

static void prefs_edit_item (
	GtkButton	* const	ARG_UNUSED ( buttonwidget ),
	gpointer	const	user_data
	)
{
	char	const	* window_title = _("Edit Preference");
	char	const	* table_titles[5] = {
				_("Key"),
				_("Description"),
				_("Type"),
				_("Default"),
				_("New Value")
				};
	char		buf[256],
			* comtxt
			;
	char	const	* txt;
	GtkWidget	* window,
			* button,
			* hbox,
			* vbox = NULL,
			* cs = NULL,
			* xtra,
			* entry = NULL,
			* focus_widget = NULL;
	GtkTextBuffer	* gtext_buffer = NULL;
	GtkAccelGroup	* ag;
	gexPrefsData	* pdata;
	int		fpick_flags = MTGEX_FPICK_LOAD,
			row,
			c = -1,
			i,
			j,
			spin_value_i = 0,
			max_chars = 0;
	double		spin_value_d = 0;
	GList		* sel;
	gdouble		spin_min = -1000000,
			spin_max = 1000000,
			spin_step = 0.001;


	pdata = user_data;
	if ( ! pdata )
	{
		return;
	}

	sel = GTK_CLIST ( pdata->clist )->selection;
	if ( ! sel )
	{
		return;
	}

	row = GPOINTER_TO_INT ( sel->data );

	pdata->piv = gtk_clist_get_row_data ( GTK_CLIST ( pdata->clist ), row );
	if ( ! pdata->piv )
	{
		return;
	}

	switch ( pdata->piv->type )
	{
	case MTKIT_PREF_TYPE_INT:
	case MTKIT_PREF_TYPE_BOOL:
	case MTKIT_PREF_TYPE_OPTION:
	case MTKIT_PREF_TYPE_DOUBLE:
	case MTKIT_PREF_TYPE_STR:
	case MTKIT_PREF_TYPE_STR_MULTI:
	case MTKIT_PREF_TYPE_RGB:
		window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL, window_title,
			GTK_WIN_POS_CENTER, TRUE );
		gtk_signal_connect_object ( GTK_OBJECT ( window ),
			"delete_event",
			GTK_SIGNAL_FUNC ( prefs_close_edit_window ),
			GTK_OBJECT ( window ) );

		vbox = gtk_vbox_new ( FALSE, 0 );
		gtk_widget_show ( vbox );
		gtk_container_add ( GTK_CONTAINER ( window ), vbox );

		if (	pdata->piv->type == MTKIT_PREF_TYPE_STR &&
			pdata->piv->opt
			)
		{
			if ( mtkit_strtoi ( pdata->piv->opt,
				&max_chars, NULL, 0 ) ||
				max_chars < 1
				)
			{
				max_chars = 0;
			}
		}

		if ( pdata->piv->type == MTKIT_PREF_TYPE_RGB )
		{
			hbox = gtk_hbox_new ( FALSE, 0 );
			gtk_widget_show ( hbox );
			gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE,
				FALSE, 5 );

			cs = mtgex_cpick_new ( pdata->main_window,
				pdata->prefs, "prefs." );
			pdata->edit_entry = cs;
			mtgex_cpick_set_opacity_visibility ( cs, FALSE );
			gtk_widget_show ( cs );
			c = 0;
			mtkit_strtoi ( pdata->piv->value, &c, NULL, 0 );

			gtk_box_pack_start ( GTK_BOX ( hbox ), cs, TRUE, FALSE,
				5 );
		}

		break;

	case MTKIT_PREF_TYPE_DIR:
		fpick_flags = MTGEX_FPICK_DIRS_ONLY;

	case MTKIT_PREF_TYPE_FILE:
		window = mtgex_fpick_new ( window_title, fpick_flags,
			pdata->main_window, pdata->prefs, "prefs." );

		if ( ! window )
		{
			return;
		}

		mtgex_fpick_set_filename ( window, pdata->piv->value, FALSE );

		break;

	default:
		return;
	}

	gtk_object_set_data ( GTK_OBJECT ( window ), PREFS_DATA_KEY,
		(gpointer)pdata );
	pdata->edit_item_window = window;

	xtra = gtk_table_new ( 5, 3, FALSE );
	gtk_widget_show ( xtra );

	j = 5;

	if (	pdata->piv->type == MTKIT_PREF_TYPE_DIR	||
		pdata->piv->type == MTKIT_PREF_TYPE_FILE	||
		pdata->piv->type == MTKIT_PREF_TYPE_RGB
		)
	{
		j = 4;
	}

	for ( i = 0; i < j; i++ )
	{
		mtgex_add_to_table ( table_titles[i], xtra, i, 0, 5, 5 );

		txt = NULL;

		switch ( i )
		{
		case 0:
			txt = pdata->piv->key;

			break;

		case 1:
			txt = pdata->piv->description;

			if ( ! txt )
			{
				txt = "";
			}

			break;

		case 2:
			txt = mtkit_prefs_type_text ( pdata->piv->type );
			break;

		case 3:
			mtkit_prefs_get_str_val ( pdata->piv, pdata->piv->def,
				buf, 256 );
			txt = buf;

			break;

		case 4:
			if (	pdata->piv->type == MTKIT_PREF_TYPE_STR ||
				pdata->piv->type == MTKIT_PREF_TYPE_STR_MULTI ||
				pdata->piv->type == MTKIT_PREF_TYPE_FILE ||
				pdata->piv->type == MTKIT_PREF_TYPE_DIR
				)
			{
				txt = pdata->piv->value;
			}

			break;
		}

		if ( pdata->piv->type == MTKIT_PREF_TYPE_STR_MULTI
			&& i >= 3 )
		{
			GtkWidget	* sw;
			int		exp = GTK_FILL;


			sw = gtk_scrolled_window_new ( NULL, NULL );
			gtk_widget_show ( sw );
			gtk_scrolled_window_set_policy (
				GTK_SCROLLED_WINDOW ( sw ),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

			gtk_container_set_border_width ( GTK_CONTAINER ( sw ),
				2 );

			entry = gtk_text_view_new ();
			gtext_buffer = gtk_text_view_get_buffer (
				GTK_TEXT_VIEW ( entry ) );
			gtk_container_add ( GTK_CONTAINER ( sw ), entry );

			if ( i == 3 )
			{
				gtk_text_view_set_editable (
					GTK_TEXT_VIEW ( entry ), FALSE );
			}
			else
			{
				focus_widget = entry;
				exp = exp | GTK_EXPAND;
			}

			gtk_table_attach ( GTK_TABLE ( xtra ), sw, 1, 1 + 1,
				(guint)(i), (guint)(i + 1),
				GTK_FILL | GTK_EXPAND, exp, 0, 0 );
			gtk_widget_show ( entry );
			gtk_text_buffer_set_text ( gtext_buffer, txt, -1 );
		}
		else if ( txt )
		{
			entry = gtk_entry_new ();
			if ( max_chars && i == 4 )
			{
				gtk_entry_set_max_length ( GTK_ENTRY ( entry ),
					max_chars );
			}

			if ( i < 4 )
			{
				gtk_entry_set_editable ( GTK_ENTRY ( entry ),
					FALSE );
			}
			else
			{
				focus_widget = entry;
			}

			gtk_table_attach ( GTK_TABLE ( xtra ), entry, 1, 1 + 1,
				(guint)i, (guint)(i + 1),
				GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );
			gtk_widget_show ( entry );
			gtk_entry_set_text ( GTK_ENTRY ( entry ), txt );
		}
		else if ( i == 4 )
		{
			// Add spin button (int/double) or option menu
			// (boolean/option)

			switch ( pdata->piv->type )
			{
			case MTKIT_PREF_TYPE_INT:
				if ( pdata->piv->opt )
				{
					mtkit_strtok_num ( pdata->piv->opt,
						"\t", 0, &spin_min );

					mtkit_strtok_num ( pdata->piv->opt,
						"\t", 1, &spin_max );
				}

				entry = gtk_spin_button_new_with_range (
					spin_min, spin_max, 1 );
				gtk_widget_show ( entry );
				focus_widget = entry;
				gtk_table_attach ( GTK_TABLE ( xtra ), entry,
					1, 1 + 1, (guint)i, (guint)(i + 1),
					GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );

				mtkit_prefs_get_int ( pdata->prefs,
					pdata->piv->key, &spin_value_i );

				gtk_spin_button_set_value (
					GTK_SPIN_BUTTON ( entry ),
					(gdouble)spin_value_i );

				break;

			case MTKIT_PREF_TYPE_DOUBLE:
				if ( pdata->piv->opt )
				{
					mtkit_strtok_num ( pdata->piv->opt,
						"\t", 0, &spin_min );
					mtkit_strtok_num ( pdata->piv->opt,
						"\t", 1, &spin_max );
					mtkit_strtok_num ( pdata->piv->opt,
						"\t", 2, &spin_step );
				}

				entry = gtk_spin_button_new_with_range (
					spin_min, spin_max, spin_step );
				gtk_widget_show ( entry );
				focus_widget = entry;
				gtk_table_attach ( GTK_TABLE ( xtra ), entry,
					1, 1 + 1, (guint)i, (guint)(i + 1),
					GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );

				mtkit_prefs_get_double ( pdata->prefs,
					pdata->piv->key, &spin_value_d );

				gtk_spin_button_set_value (
					GTK_SPIN_BUTTON ( entry ),
					(gdouble)spin_value_d );

				break;

			case MTKIT_PREF_TYPE_BOOL:
				entry = mtgex_bmenu_new ();
				focus_widget = entry;

				mtgex_bmenu_add_item ( entry, _("FALSE") );
				mtgex_bmenu_add_item ( entry, _("TRUE") );

				mtkit_prefs_get_int ( pdata->prefs,
					pdata->piv->key, &spin_value_i );
				mtgex_bmenu_set_value ( entry, spin_value_i );

				gtk_table_attach ( GTK_TABLE ( xtra ), entry,
					1, 1 + 1, (guint)i, (guint)(i + 1),
					GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );

				break;

			case MTKIT_PREF_TYPE_OPTION:
				entry = mtgex_bmenu_new ();
				focus_widget = entry;

				for ( spin_value_i = 0; ; spin_value_i ++ )
				{
					comtxt = mtkit_strtok ( pdata->piv->opt,
						"\t", spin_value_i );

					if ( ! comtxt )
					{
						break;	// End of list
					}

					snprintf ( buf, sizeof ( buf ),
						"( %i ) = %s", spin_value_i,
						comtxt );

					mtgex_bmenu_add_item ( entry, buf );
					free ( comtxt );
				}

				mtkit_prefs_get_int ( pdata->prefs,
					pdata->piv->key, &spin_value_i );
				mtgex_bmenu_set_value ( entry, spin_value_i );

				gtk_table_attach ( GTK_TABLE ( xtra ), entry,
					1, 1 + 1, (guint)i, (guint)(i + 1),
					GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0 );

				break;

			default:
				break;
			}
		}
	}

	button = gtk_button_new_with_label ( _("Reset") );
	gtk_widget_show ( button );
	gtk_table_attach ( GTK_TABLE ( xtra ), button,
		2, 2 + 1, 3, 3 + 1, GTK_FILL, GTK_FILL, 5, 5 );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( prefs_edit_button_reset ), pdata );

	switch ( pdata->piv->type )
	{
	case MTKIT_PREF_TYPE_DIR:
	case MTKIT_PREF_TYPE_FILE:
		mtgex_fpick_setup ( window, xtra, GTK_SIGNAL_FUNC ( fs_ok ),
			GTK_SIGNAL_FUNC ( fs_destroy ) );
		mtgex_restore_window_position ( window, pdata->prefs,
			"prefs.file.window" );
		break;

	default:
		if ( pdata->piv->type == MTKIT_PREF_TYPE_RGB )
		{
			mtgex_restore_window_position ( window, pdata->prefs,
				"prefs.color.window" );
		}
		else
		{
			mtgex_restore_window_position ( window, pdata->prefs,
				"prefs.edit.window" );
		}

		if ( xtra )
		{
			gtk_box_pack_start ( GTK_BOX ( vbox ), xtra, TRUE,
				TRUE, 5 );
		}

		mtgex_add_hseparator ( vbox, -2, 5 );
		ag = gtk_accel_group_new ();

		hbox = gtk_hbox_new ( FALSE, 0 );
		gtk_widget_show ( hbox );
		gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

		button = gtk_button_new_with_label ( _("OK") );
		gtk_widget_show ( button );
		gtk_box_pack_end ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );

		if ( pdata->piv->type != MTKIT_PREF_TYPE_STR_MULTI )
		{
			gtk_widget_add_accelerator ( button, "clicked", ag,
				GDK_KP_Enter, 0, (GtkAccelFlags) 0 );
			gtk_widget_add_accelerator ( button, "clicked", ag,
				GDK_Return, 0, (GtkAccelFlags) 0 );
		}

		gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			GTK_SIGNAL_FUNC ( prefs_edit_button_ok ), pdata );
		gtk_widget_set_usize ( button, 100, -2 );

		button = gtk_button_new_with_label ( _("Cancel") );
		gtk_widget_show ( button );
		gtk_box_pack_end ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
		gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Escape,
			0, (GtkAccelFlags) 0 );
		gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
			GTK_SIGNAL_FUNC ( prefs_edit_button_cancel ), pdata );
		gtk_widget_set_usize ( button, 100, -2 );

		gtk_window_add_accel_group ( GTK_WINDOW(window), ag );

		break;
	}

	gtk_window_set_transient_for ( GTK_WINDOW ( window ),
		GTK_WINDOW ( pdata->prefs_window ) );
	gtk_widget_show ( window );

	if ( focus_widget )
	{
		gtk_widget_grab_focus ( focus_widget );
	}

	pdata->edit_entry = entry;

	if ( gtext_buffer )
	{
		GtkTextIter		iter;


// This is an appalling hack to get the cursor visible from startup.

		while ( gtk_events_pending () )
		{
			gtk_main_iteration ();
		}

		gtk_text_buffer_get_end_iter ( gtext_buffer, &iter );
		gtk_text_buffer_place_cursor ( gtext_buffer, &iter );
		gtk_text_view_scroll_to_iter ( GTK_TEXT_VIEW ( entry ),
			&iter, 0, FALSE, 0, 0 );
	}

	if ( cs && c > -1 )
	{
		pdata->edit_entry = cs;
		mtgex_cpick_set_color_previous ( cs, c, 255 );
		mtgex_cpick_set_color ( cs, c, 255 );
	}
}

static gboolean clist_key_event (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventKey	* const	event,
	gpointer	const	user_data
	)
{
	switch ( event->keyval )
	{
		case GDK_Return:
		case GDK_KP_Enter:
			prefs_edit_item ( NULL, user_data );
			return TRUE;

		default:
			return FALSE;
	}

	return FALSE;
}

static void clist_select_row (
	GtkCList	* const	ARG_UNUSED ( clist ),
	gint		const	ARG_UNUSED ( row ),
	gint		const	ARG_UNUSED ( column ),
	GdkEventButton	* const	event,
	gpointer	const	user_data
	)
{
	if ( ! event || ! user_data )
	{
		return;
	}

	if ( event->type != GDK_2BUTTON_PRESS )
	{
		return;
	}

	prefs_edit_item ( NULL, user_data );
}

static void prefs_clist_sort (
	gexPrefsData	* const	pdata
	)
{
	gtk_clist_set_sort_type ( GTK_CLIST ( pdata->clist ),
		pdata->sort_direction );
	gtk_clist_set_sort_column ( GTK_CLIST ( pdata->clist ),
		pdata->sort_column );
	gtk_clist_sort ( GTK_CLIST ( pdata->clist ) );
}

static void clist_column_button (
	GtkCList	* const	ARG_UNUSED ( clist ),
	gint		const	column,
	gpointer	const	user_data
	)
{
	GtkSortType	direction;
	gexPrefsData	* const	pdata = user_data;


	if (	! pdata			||
		column < 0		||
		column >= PREFS_CLIST_COLS
		)
	{
		return;
	}

// reverse the sorting direction if the list is already sorted by this col

	if ( pdata->sort_column == column )
	{
		direction = ( pdata->sort_direction == GTK_SORT_ASCENDING ?
			GTK_SORT_DESCENDING : GTK_SORT_ASCENDING );
	}
	else
	{
		// Different column clicked so use default value for that column

		direction = GTK_SORT_ASCENDING;

		// Hide old arrow
		gtk_widget_hide ( pdata->sort_arrows[pdata->sort_column] );

		// Show new arrow
		gtk_widget_show ( pdata->sort_arrows[column] );

		pdata->sort_column = column;
	}

	gtk_arrow_set ( GTK_ARROW ( pdata->sort_arrows[column] ),
		direction == GTK_SORT_ASCENDING ? GTK_ARROW_DOWN : GTK_ARROW_UP,
		GTK_SHADOW_IN );

	pdata->sort_direction = direction;

	prefs_clist_sort ( pdata );
}

static void clist_populate_recurse (
	GtkWidget	* const	clist,
	mtTreeNode	* const	node,
	char	const	* const	filter
	)
{
	char	const	* t;
	int		row;
	gchar		* row_txt[PREFS_CLIST_COLS] = { NULL, NULL, NULL, NULL};
	mtPrefValue	* piv;


	if ( ! node )
	{
		return;
	}

	clist_populate_recurse ( clist, node->left, filter );
	piv = (mtPrefValue *)node->data;

	if (	piv &&
		strncmp ( piv->key, "prefs.", 6 ) &&
		( ! filter || mtkit_strcasestr ( piv->key, filter ) )
		)
	{
		t = mtkit_prefs_type_text ( piv->type );

		if ( ! t )
		{
			t = "?";
		}

		row = gtk_clist_append ( GTK_CLIST ( clist ), row_txt );

		gtk_clist_set_text ( GTK_CLIST ( clist ), row,
			PREFS_CLIST_COL_KEY, piv->key );

		gtk_clist_set_text ( GTK_CLIST ( clist ), row,
			PREFS_CLIST_COL_TYPE, t );

		gtk_clist_set_row_data ( GTK_CLIST ( clist ), row,
			(gpointer)piv );

		update_clist_status_value ( clist, piv, row );
	}

	clist_populate_recurse ( clist, node->right, filter );
}

static void prefs_populate (
	gexPrefsData	* const	pdata,
	char	const	* const	filter
	)
{
	mtTree		* tree;


	gtk_clist_clear ( GTK_CLIST ( pdata->clist ) );
	gtk_clist_freeze ( GTK_CLIST ( pdata->clist ) );

	tree = mtkit_prefs_get_tree ( pdata->prefs );

	if ( tree )
	{
		clist_populate_recurse ( pdata->clist, tree->root,
			filter );
	}

	gtk_clist_select_row ( GTK_CLIST ( pdata->clist ), 0, 0 );
	gtk_clist_thaw ( GTK_CLIST ( pdata->clist ) );
	prefs_clist_sort ( pdata );
}

static void prefs_filter_button (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gexPrefsData	* const	pdata
	)
{
	char	const	* filter;


	if ( ! pdata )
	{
		return;
	}

	filter = gtk_entry_get_text ( GTK_ENTRY ( pdata->entry ) );

	if ( filter && filter[0] == 0 )
	{
		filter = NULL;
	}

	prefs_populate ( pdata, filter );
}

int mtgex_prefs_window (
	mtPrefs		* const	prefs,
	GtkWidget	* const	main_window,
	char	const	*	title,
	mtPrefCB	const	closure_cb,
	mtPrefValue	* const	closure_piv,
	int		const	closure_data
	)
{
	gexPrefsData	* pdata;
	GtkWidget	* window,
			* button,
			* hbox,
			* vbox,
			* sw,
			* temp_hbox;
	GtkAccelGroup	* ag;
	char	const	* col_titles[PREFS_CLIST_COLS];
	char		txt[256];
	int		i,
			j;


	ag = gtk_accel_group_new ();

	col_titles[PREFS_CLIST_COL_KEY] =	_("Key");
	col_titles[PREFS_CLIST_COL_STATUS] =	_("Status");
	col_titles[PREFS_CLIST_COL_TYPE] =	_("Type");
	col_titles[PREFS_CLIST_COL_VALUE] =	_("Value");

	pdata = calloc ( sizeof ( gexPrefsData ), 1 );

	if ( ! pdata )
	{
		return 1;
	}

	pdata->prefs = prefs;
	pdata->main_window  = main_window;
	pdata->closure_cb   = closure_cb;
	pdata->closure_piv  = closure_piv;
	pdata->closure_data = closure_data;

	if ( ! title )
	{
		title = _("Preferences");
	}

	window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL, title,
		GTK_WIN_POS_CENTER, TRUE );
	gtk_object_set_data ( GTK_OBJECT ( window ), PREFS_DATA_KEY, pdata );
	pdata->prefs_window = window;

	gtk_window_set_default_size ( GTK_WINDOW ( window ), 512, 400 );

	// Set minimum width/height
	gtk_widget_set_usize ( window, 100, 100 );

	mtgex_restore_window_position ( window, prefs, "prefs.window" );

	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox );
	gtk_container_add ( GTK_CONTAINER ( window ), vbox );

//	FILTER

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

	button = gtk_button_new_with_label ( _("Filter") );
	gtk_widget_show ( button );
	gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( prefs_filter_button ), pdata );

	pdata->entry = gtk_entry_new_with_max_length ( 256 );
	gtk_widget_show ( pdata->entry );
	gtk_box_pack_start ( GTK_BOX ( hbox ), pdata->entry, TRUE, TRUE, 5 );

	gtk_signal_connect ( GTK_OBJECT ( pdata->entry ), "activate",
		GTK_SIGNAL_FUNC ( prefs_filter_button ), pdata );

	gtk_widget_grab_focus ( pdata->entry );

//	LIST

	sw = mtgex_xpack5 ( vbox, gtk_scrolled_window_new ( NULL, NULL ) );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_widget_show ( sw );

	pdata->clist = gtk_clist_new ( PREFS_CLIST_COLS );

	for ( i = 0; i < PREFS_CLIST_COLS; i++ )
	{
		temp_hbox = gtk_hbox_new ( FALSE, 0 );
		mtgex_pack ( temp_hbox, gtk_label_new ( col_titles[i] ) );
		gtk_widget_show_all ( temp_hbox );

		pdata->sort_arrows[i] = mtgex_pack_end ( temp_hbox,
			gtk_arrow_new ( GTK_ARROW_DOWN, GTK_SHADOW_IN ) );

		gtk_clist_set_column_widget ( GTK_CLIST ( pdata->clist ), i,
			temp_hbox );
		GTK_WIDGET_UNSET_FLAGS (
			GTK_CLIST ( pdata->clist )->column[i].button,
			(guint)GTK_CAN_FOCUS );
	}

	gtk_widget_show ( pdata->sort_arrows[0] );	// Show first arrow

	for ( i = 0; i < PREFS_CLIST_COLS; i++ )
	{
		snprintf ( txt, sizeof ( txt ), "prefs.col%i", i + 1 );

		if ( mtkit_prefs_get_int ( prefs, txt, &j ) )
		{
			continue;
		}

		gtk_clist_set_column_width ( GTK_CLIST ( pdata->clist ), i, j );
	}

	gtk_clist_column_titles_show ( GTK_CLIST ( pdata->clist ) );
	gtk_clist_set_selection_mode ( GTK_CLIST ( pdata->clist ),
		GTK_SELECTION_BROWSE );

	gtk_container_add ( GTK_CONTAINER ( sw ), pdata->clist );
	gtk_widget_show ( pdata->clist );

//	Populate the list

	prefs_populate ( pdata, NULL );

	gtk_signal_connect ( GTK_OBJECT ( pdata->clist ), "click_column",
		GTK_SIGNAL_FUNC ( clist_column_button ), pdata );

	gtk_signal_connect ( GTK_OBJECT ( pdata->clist ), "select_row",
		GTK_SIGNAL_FUNC ( clist_select_row ), pdata );

	gtk_signal_connect ( GTK_OBJECT ( pdata->clist ), "key_press_event",
		GTK_SIGNAL_FUNC ( clist_key_event ), pdata );


//	BUTTONS

	mtgex_add_hseparator ( vbox, -2, 10 );

	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_widget_show ( hbox );
	gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5 );

	button = gtk_button_new_with_label ( _("Edit") );
	gtk_widget_set_usize ( button, 100, -2 );
	gtk_widget_show ( button );
	gtk_box_pack_end ( GTK_BOX(hbox), button, FALSE, FALSE, 5 );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( prefs_edit_item ), (gpointer)pdata );

	button = gtk_button_new_with_label( _("Close") );
	gtk_widget_set_usize ( button, 100, -2 );
	gtk_widget_show ( button );
	gtk_box_pack_end ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Escape, 0,
		(GtkAccelFlags) 0 );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( prefs_close_window_button ), window );

	gtk_signal_connect_object ( GTK_OBJECT ( window ), "delete_event",
		GTK_SIGNAL_FUNC ( prefs_close_window ), GTK_OBJECT ( window ) );

	gtk_window_set_transient_for ( GTK_WINDOW ( window ),
		GTK_WINDOW ( main_window ) );
	gtk_widget_show ( window );
	gtk_window_add_accel_group ( GTK_WINDOW ( window ), ag );

	return 0;
}



static mtPrefTable const default_prefs_table[] = {
{ "prefs.col1",		MTKIT_PREF_TYPE_INT, "150", NULL, NULL, 0, NULL, NULL },
{ "prefs.col2",		MTKIT_PREF_TYPE_INT, "70", NULL, NULL, 0, NULL, NULL },
{ "prefs.col3",		MTKIT_PREF_TYPE_INT, "70", NULL, NULL, 0, NULL, NULL },
{ "prefs.col4",		MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },

{ "prefs.window_x",	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_y",	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_w",	MTKIT_PREF_TYPE_INT, "600", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_h",	MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },

{ "prefs.file.window_x", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.file.window_y", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.file.window_w", MTKIT_PREF_TYPE_INT, "600", NULL, NULL, 0, NULL, NULL },
{ "prefs.file.window_h", MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },

{ "prefs.color.window_x", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.color.window_y", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.color.window_w", MTKIT_PREF_TYPE_INT, "100", NULL, NULL, 0, NULL, NULL },
{ "prefs.color.window_h", MTKIT_PREF_TYPE_INT, "100", NULL, NULL, 0, NULL, NULL },

{ "prefs.edit.window_x", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.edit.window_y", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.edit.window_w", MTKIT_PREF_TYPE_INT, "400", NULL, NULL, 0, NULL, NULL },
{ "prefs.edit.window_h", MTKIT_PREF_TYPE_INT, "100", NULL, NULL, 0, NULL, NULL },

{ "prefs.cpick_pal_1",	MTKIT_PREF_TYPE_RGB, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_2",	MTKIT_PREF_TYPE_RGB, "16711680", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_3",	MTKIT_PREF_TYPE_RGB, "16776960", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_4",	MTKIT_PREF_TYPE_RGB, "65280", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_5",	MTKIT_PREF_TYPE_RGB, "255", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_6",	MTKIT_PREF_TYPE_RGB, "16711935", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_7",	MTKIT_PREF_TYPE_RGB, "65535", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_8",	MTKIT_PREF_TYPE_RGB, "16777215", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_9",	MTKIT_PREF_TYPE_RGB, "14408667", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_10",	MTKIT_PREF_TYPE_RGB, "11974326", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_11",	MTKIT_PREF_TYPE_RGB, "9605778", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_12",	MTKIT_PREF_TYPE_RGB, "7171437", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_13",	MTKIT_PREF_TYPE_RGB, "4802889", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_14",	MTKIT_PREF_TYPE_RGB, "2368548", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_15",	MTKIT_PREF_TYPE_RGB, "14389760", NULL, NULL, 0, NULL, NULL },
{ "prefs.cpick_pal_16",	MTKIT_PREF_TYPE_RGB, "7190089", NULL, NULL, 0, NULL, NULL },

{ "prefs.fpick_dir_1",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_2",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_3",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_4",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_5",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_6",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_7",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_8",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_9",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_10",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_11",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_12",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_13",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_14",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_15",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_dir_16",	MTKIT_PREF_TYPE_DIR, NULL, NULL, NULL, 0, NULL, NULL },

{ "prefs.fpick_case_insensitive", MTKIT_PREF_TYPE_INT, "1", NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_show_hidden", MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_col1",	MTKIT_PREF_TYPE_INT, "250", NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_col2",	MTKIT_PREF_TYPE_INT, "64", NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_col3",	MTKIT_PREF_TYPE_INT, "80", NULL, NULL, 0, NULL, NULL },
{ "prefs.fpick_col4",	MTKIT_PREF_TYPE_INT, "150", NULL, NULL, 0, NULL, NULL },

{ NULL }

	};



int mtgex_prefs_init_prefs (
	mtPrefs		* const	prefs
	)
{
	return mtkit_prefs_add ( prefs, default_prefs_table, 0 );
}

int mtgex_prefs_window_mirror_prefs (
	mtPrefs		* const	dest,
	mtPrefs		* const	src
	)
{
	return mtkit_prefs_value_mirror ( dest, src, default_prefs_table );
}

