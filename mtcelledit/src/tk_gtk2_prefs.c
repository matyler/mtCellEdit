/*
	Copyright (C) 2008-2016 Mark Tyler

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



static int project_set_font (
	char	const	* const	name,
	int		const	size
	)
{
	mtFont		* newfont = NULL;


	newfont = cui_font_new_pango ( name, size );

	if ( ! newfont )
	{
		char		* msg,
				* f8;


		f8 = mtgex_gtkuncpy ( NULL, name, 0 );
		msg = g_strdup_printf (
			_("Unable to open font file %s size %i."), f8, size );

		mtgex_alert_box ( _("Error"), msg, _("OK"), NULL, NULL,
			global.main_window );

		g_free ( msg );
		g_free ( f8 );

		return 1;
	}

	ced_view_set_font ( global.table, newfont );
	cui_font_destroy ( global.font );
	global.font = newfont;


	CedSheet	* sheet;


	sheet = global.cedview->ren.sheet;
	if ( sheet )
	{
		ced_view_set_split ( global.table,
			sheet->prefs.split_r1, sheet->prefs.split_r2,
			sheet->prefs.split_c1, sheet->prefs.split_c2 );
	}

	return 0;
}

void pref_change_font (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	char	const	* name;
	int		size;


	name = prefs_get_string ( GUI_INIFILE_FONT_PANGO_NAME );
	size = prefs_get_int ( GUI_INIFILE_FONT_SIZE );

	if ( project_set_font ( name, size ) )
	{
		project_set_font ( "Sans", 16 );
	}
}

void pref_init_row_pad ( void )
{
	int		size;
	CedSheet	* sheet;


	size = prefs_get_int ( GUI_INIFILE_ROW_PAD );

	global.cedview->ren.row_pad = size;

	sheet = global.cedview->ren.sheet;
	if ( sheet )
	{
		ced_view_set_split ( global.table,
			sheet->prefs.split_r1, sheet->prefs.split_r2,
			sheet->prefs.split_c1, sheet->prefs.split_c2 );
	}

	ced_view_redraw_view ( global.cedview );
}

void pref_change_row_pad (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	pref_init_row_pad ();
}

void pref_change_graph_scale (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	pressed_graph_redraw ( NULL, NULL );
}

void pref_change_recent_filename_len (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	update_recent_files ();
}

void fe_save_pref_window_prefs (
	mtPrefs		* const	prefs
	)
{
	be_save_pref_window_prefs ( prefs );
	mtgex_prefs_window_mirror_prefs ( prefs_file (), prefs );
}

mtPrefs * fe_load_pref_window_prefs (
	mtPrefTable	const * const	table
	)
{
	mtPrefs		* prefs;


	prefs = mtkit_prefs_new ( table );
	mtgex_prefs_init_prefs ( prefs );

	if ( mtgex_prefs_window_mirror_prefs ( prefs, prefs_file () ) )
	{
		mtkit_prefs_destroy ( prefs );

		return NULL;
	}

	be_load_pref_window_prefs ( prefs );

	return prefs;
}

void pressed_preferences (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	mtgex_prefs_window ( prefs_file (), global.main_window,
		_("Program Preferences"), NULL, NULL, 0 );
}

void pressed_set_fullscreen(
	GtkMenuItem	* const	menu_item,
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	ARG_UNUSED ( item )
	)
{
	if ( gtk_check_menu_item_get_active (
		GTK_CHECK_MENU_ITEM ( menu_item ) )
		)
	{
		gtk_window_fullscreen ( GTK_WINDOW ( global.main_window ) );
	}
	else
	{
		gtk_window_unfullscreen ( GTK_WINDOW ( global.main_window ) );
	}
}

static void cell_prefs_closure (
	mtPrefValue	* const	piv,
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	if ( piv )
	{
		mtPrefs		* const	prefs = (mtPrefs *)piv;


		fe_save_pref_window_prefs ( prefs );
		mtkit_prefs_destroy ( prefs );
	}

	be_cellpref_cleanup ( global.cedview->ren.sheet );

	ced_view_redraw_view ( global.cedview );
}

void fe_commit_prefs_set (
	int		const	pref_id,
	int		const	pref_num,
	char	const	* const	pref_charp,
	int		const	change_cursor,
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	CedSheet	* const	sheet = global.cedview->ren.sheet;
	int		res;


	if ( ! sheet )
	{
		return;
	}

	res = be_commit_prefs_set ( sheet, global.file->cubook,
		pref_id, pref_num, pref_charp );

	undo_report_updates ( res );

	if ( be_commit_prefs_set_check ( res, sheet, change_cursor, pref_id ) )
	{
		return;
	}

	update_changes_chores ( 1, 1 );
}

void pressed_edit_cell_prefs (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedSheet	* const	sheet = global.cedview->ren.sheet;
	mtPrefs		* prefs;


	if ( undo_report_updates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	prefs = be_cellpref_init ( sheet, be_cellpref_changed, NULL );
	if ( ! prefs )
	{
		return;
	}

	mtgex_prefs_window ( prefs, global.main_window, _("Cell Preferences"),
		cell_prefs_closure, (mtPrefValue *)prefs, 0 );
}

void pressed_bold (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		res;


	if ( be_prepare_prefs_set ( global.cedview->ren.sheet ) )
	{
		return;
	}

	res = cui_cellprefs_text_style ( global.file, CED_TEXT_STYLE_BOLD );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	undo_report_updates ( res );
	update_changes_chores ( 1, 1 );
	be_cellpref_cleanup ( global.cedview->ren.sheet );
}



#define COLOR_SWATCH_DEFAULT	-100
#define COLOR_SWATCH_TERMINATE	-10



static int	swatch_result;



static gboolean click_swatch (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	data
	)
{
	if ( swatch_result != COLOR_SWATCH_DEFAULT )
	{
		// Already set once so take the first selection

		return TRUE;
	}


	int		i = (int)(intptr_t)data;

	if ( i == COLOR_SWATCH_TERMINATE )
	{
		// Termination signal
		swatch_result = COLOR_SWATCH_TERMINATE;

		return TRUE;
	}

	if ( i == -1 )
	{
		// Cancel button pressed
		swatch_result = -1;

		return TRUE;
	}

	swatch_result = be_color_swatch_get ( i );

	return TRUE;
}

static gboolean expose_swatch (
	GtkWidget	* const	widget,
	GdkEventExpose	* const	event,
	gpointer	const	user_data
	)
{
	int		i,
			px,
			py,
			pw,
			ph,
			col = 0;
	unsigned char	r,
			g,
			b,
			* rgb,
			* dest;


	if ( ! widget || ! event )
	{
		return TRUE;
	}

	px = event->area.x;
	py = event->area.y;
	pw = event->area.width;
	ph = event->area.height;

	if ( pw < 1 || ph < 1 )
	{
		return TRUE;
	}

	col = be_color_swatch_get ( (int)(intptr_t)user_data );
	r = (unsigned char)MTKIT_INT_2_R ( col );
	g = (unsigned char)MTKIT_INT_2_G ( col );
	b = (unsigned char)MTKIT_INT_2_B ( col );

	rgb = malloc ( (size_t)(pw * 3) );
	if ( ! rgb )
	{
		return FALSE;
	}

	for ( dest = rgb, i = 0; i < pw; i++ )
	{
		*dest++ = r;
		*dest++ = g;
		*dest++ = b;
	}

	gdk_draw_rgb_image ( widget->window, widget->style->black_gc,
		px, py, pw, ph, GDK_RGB_DITHER_NONE, rgb, 0 );

	free ( rgb );

	return TRUE;
}

static int swatch_color_dialog (
	char	const	* const	title
	)
{
	int		r,
			c,
			i;
	GtkWidget	* window,
			* table,
			* button,
			* vbox,
			* area;
	GtkAccelGroup	* ag;


	// Revert to default
	swatch_result = COLOR_SWATCH_DEFAULT;

	window = mtgex_add_a_window ( GTK_WINDOW_TOPLEVEL, title,
		GTK_WIN_POS_MOUSE, TRUE );
	ag = gtk_accel_group_new ();

	g_signal_connect ( window, "destroy", G_CALLBACK ( click_swatch ),
		(gpointer)COLOR_SWATCH_TERMINATE );

	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_widget_show ( vbox );
	gtk_container_add ( GTK_CONTAINER ( window ), vbox );

	table = gtk_table_new ( COLOR_SWATCH_ROWS, COLOR_SWATCH_COLS, FALSE );
	gtk_widget_show ( table );
	gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 0 );

	for ( r = 0; r < COLOR_SWATCH_ROWS; r++ )
	{
		for ( c = 0; c < COLOR_SWATCH_COLS; c++ )
		{
			i = r * COLOR_SWATCH_COLS + c;

			button = gtk_button_new ();
			gtk_widget_show ( button );

			g_signal_connect ( button, "clicked",
				G_CALLBACK ( click_swatch ),
				(gpointer)(intptr_t)i );

			gtk_table_attach ( GTK_TABLE ( table ), button,
				(guint)(c), (guint)(c + 1),
				(guint)(r), (guint)(r + 1),
				GTK_FILL, GTK_FILL, 0, 0 );

			area = gtk_drawing_area_new ();
			gtk_drawing_area_size ( GTK_DRAWING_AREA ( area ),
				COLOR_SWATCH_SIZE, COLOR_SWATCH_SIZE );
			mtgex_fix_darea ( area );
			gtk_widget_show ( area );
			g_signal_connect ( area, "expose_event",
				G_CALLBACK ( expose_swatch ),
				(gpointer)(intptr_t)i );

			gtk_container_add ( GTK_CONTAINER ( button ), area );
		}
	}

	button = gtk_button_new_with_label ( _("Cancel") );
	gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );
	gtk_widget_show ( button );
	g_signal_connect ( button, "clicked",
		G_CALLBACK ( click_swatch ), (gpointer)-1 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_Escape, 0,
		(GtkAccelFlags) 0 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_F8, 0,
		(GtkAccelFlags) 0 );
	gtk_widget_add_accelerator ( button, "clicked", ag, GDK_F9, 0,
		(GtkAccelFlags) 0 );

	gtk_widget_show ( window );
	gtk_window_set_transient_for ( GTK_WINDOW ( window ),
		GTK_WINDOW ( global.main_window ) );
	gtk_window_add_accel_group ( GTK_WINDOW ( window ), ag );

	while ( swatch_result == COLOR_SWATCH_DEFAULT )
	{
		gtk_main_iteration ();
	}

	if ( swatch_result != COLOR_SWATCH_TERMINATE )
	{
		gtk_widget_destroy ( window );
	}

	return swatch_result;
}

void pressed_change_color (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	item
	)
{
	char	const	* title = "?";
	int		pref_id,
			num;


	switch ( item )
	{
	case 0:
		pref_id = CUI_CELLPREFS_color_background;
		title = _("Background Colour");
		break;

	case 1:
		pref_id = CUI_CELLPREFS_color_foreground;
		title = _("Foreground Colour");
		break;

	case 2:
		pref_id = CUI_CELLPREFS_border_color;
		title = _("Border Colour");
		break;

	default:
		return;
	}

	if ( be_prepare_prefs_set ( global.cedview->ren.sheet ) )
	{
		return;
	}

	// Get the colour from the user driven swatch
	num = swatch_color_dialog ( title );

	if ( num >= 0 )
	{
		fe_commit_prefs_set ( pref_id, num, NULL, 0, NULL );
	}

	be_cellpref_cleanup ( global.cedview->ren.sheet );
}

void pressed_border_type (
	GtkMenuItem	* const	ARG_UNUSED ( menu_item ),
	gpointer	const	ARG_UNUSED ( user_data ),
	gint		const	item
	)
{
	int		res;


	if ( be_prepare_prefs_set ( global.cedview->ren.sheet ) )
	{
		return;
	}

	res = cui_cellprefs_border ( global.file, item );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	undo_report_updates ( res );
	update_changes_chores ( 1, 1 );
	be_cellpref_cleanup ( global.cedview->ren.sheet );
}

void fe_book_prefs_changed (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	update_set_changes_flag ();
}

