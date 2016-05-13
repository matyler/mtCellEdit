/*
	Copyright (C) 2010-2016 Mark Tyler

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



#define CEDVIEW_DATA_KEY "ced.cedview_data"



static gboolean destroy_cedview (
	GtkWidget	* const	widget,
	GdkEvent	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	CedView		* view;


	if ( ! ced_view_get ( widget, &view ) && view )
	{
		gtk_object_remove_data ( GTK_OBJECT ( view->table ),
			CEDVIEW_DATA_KEY );
		free ( view );
	}

	return FALSE;
}

static void expose_cb (
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	unsigned char	* const	rgb,
	void		* const	user_data
	)
{
	GtkWidget	* const	widget = user_data;


	gdk_draw_rgb_image ( widget->window, widget->style->black_gc,
		x, y, w, h, GDK_RGB_DITHER_NONE, rgb, w * 3 );
}

static gboolean expose_canvas (
	GtkWidget	* const	widget,
	GdkEventExpose	* const	event,
	gpointer	const	user_data
	)
{
	CedView		* view;
	CedSheet	* sheet;
	int		i,
			px,
			py,
			pw,
			ph,
			col = 0;


	if ( ! widget || ! event )
	{
		return TRUE;
	}

	view = (CedView *)user_data;
	if ( ! view )
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

	for ( i = 0; i < CEDVIEW_AREA_TOTAL; i++ )
	{
		if ( widget == view->area[i] )
		{
			break;
		}
	}

	if ( i >= CEDVIEW_AREA_TOTAL )
	{
		return TRUE;
	}

	sheet = view->ren.sheet;

	if ( ! view->ren.font || ! sheet )
	{
		// Don't try to render if anything is missing - leave as grey

		col = 256 + 182;
	}
	else switch ( i )
	{
	case CEDVIEW_AREA_TL:
		cui_ren_expose_main ( sheet->prefs.split_r1,
			sheet->prefs.split_c1, &view->ren,
			px, py, pw, ph, expose_cb, widget );
		break;

	case CEDVIEW_AREA_TR:
		cui_ren_expose_main ( sheet->prefs.split_r1,
			sheet->prefs.start_col, &view->ren,
			px, py, pw, ph, expose_cb, widget );
		break;

	case CEDVIEW_AREA_BL:
		cui_ren_expose_main ( sheet->prefs.start_row,
			sheet->prefs.split_c1, &view->ren,
			px, py, pw, ph, expose_cb, widget );
		break;

	case CEDVIEW_AREA_BR:
		cui_ren_expose_main ( sheet->prefs.start_row,
			sheet->prefs.start_col, &view->ren,
			px, py, pw, ph, expose_cb, widget );
		break;

	case CEDVIEW_TITLE_R1:
		cui_ren_expose_row_header ( sheet->prefs.split_r1,
			&view->ren, px, py, pw, ph,
			expose_cb, widget );
		break;

	case CEDVIEW_TITLE_R2:
		cui_ren_expose_row_header ( sheet->prefs.start_row,
			&view->ren, px, py, pw, ph,
			expose_cb, widget );
		break;

	case CEDVIEW_TITLE_C1:
		cui_ren_expose_column_header ( sheet->prefs.split_c1,
			&view->ren, px, py, pw, ph,
			expose_cb, widget );
		break;

	case CEDVIEW_TITLE_C2:
		cui_ren_expose_column_header ( sheet->prefs.start_col,
			&view->ren, px, py, pw, ph,
			expose_cb, widget );
		break;

	case CEDVIEW_AREA_CORNER:
		col = 256 + view->bell[0];
		break;

	default:
		col = 256;		// Frozen pane areas
	}

	if ( col )
	{
		unsigned char		* rgb = NULL;


		rgb = malloc ( (size_t)(pw * 3) );
		if ( ! rgb )
		{
			return FALSE;
		}

		memset ( rgb, col & 255, (size_t)(pw * 3) );

		gdk_draw_rgb_image ( widget->window, widget->style->black_gc,
			px, py, pw, ph, GDK_RGB_DITHER_NONE, rgb, 0 );

		free ( rgb );
	}

	return TRUE;
}

static void scrollbar_moved (
	GtkAdjustment	* const	adjustment,
	gpointer	const	user_data
	)
{
	int		val,
			delta;
	CedView		* view;
	CedSheet	* sheet;


	view = (CedView *)user_data;
	if ( ! view )
	{
		return;
	}

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return;
	}

	val = (int)lrint ( gtk_adjustment_get_value (
		GTK_ADJUSTMENT ( adjustment ) ) );

	if ( adjustment == GTK_ADJUSTMENT ( view->adj_vert ) )
	{
		int		tot_rows,
				osr = sheet->prefs.start_row;


		delta = val - osr;
		if ( delta == 0 )
		{
			return;		// Nothing to do
		}

		sheet->prefs.start_row = val;

		/*
		Find out how many rows fit on the screen.
		NOTE - This is slightly different to columns because rows are
		always of a fixed height and less work is needed.
		*/

		tot_rows = cui_ren_row_from_y ( osr, &view->ren,
			view->area[CEDVIEW_AREA_BR]->allocation.height )
			- osr;

		if ( abs ( delta ) < tot_rows )
		{
			int		ph;


			// We are optimizing by scrolling
			// (accelerated on most systems)

			ph = cui_ren_y_from_row ( osr, &view->ren, osr +
				abs ( delta ) );

			if ( delta > 0 )
			{
				ph = -ph;
			}

			gdk_window_scroll (
				view->area[CEDVIEW_TITLE_R2]->window, 0, ph );

			gdk_window_scroll (
				view->area[CEDVIEW_AREA_BR]->window, 0, ph );

			if ( GDK_IS_WINDOW ( view->area[CEDVIEW_AREA_BL]->window
				) )
			{
				gdk_window_scroll (
					view->area[CEDVIEW_AREA_BL]->window,
					0, ph );
			}
		}
		else
		{
			gtk_widget_queue_draw ( view->area[CEDVIEW_TITLE_R2] );
			gtk_widget_queue_draw ( view->area[CEDVIEW_AREA_BL] );
			gtk_widget_queue_draw ( view->area[CEDVIEW_AREA_BR] );
		}
	}
	else if ( adjustment == GTK_ADJUSTMENT ( view->adj_hori ) )
	{
		int	const	osc = sheet->prefs.start_col;
		int		max_col;
		int		pw = 0;


		delta = val - osc;
		if ( delta == 0 )
		{
			return;		// Nothing to do
		}

		sheet->prefs.start_col = val;

		if ( delta > 0 )
		{
			// Scrolling to the right
			max_col = cui_ren_column_from_x ( osc, &view->ren,
				view->area[CEDVIEW_AREA_BR]->allocation.width -
				1 );

			if ( max_col > val )
			{
				// Old & new area rectangles overlap, so scroll
				pw = -cui_ren_x_from_column ( osc, &view->ren,
					val );
			}
		}
		else	// delta < 0
		{
			// Scrolling to the left
			max_col = cui_ren_column_from_x ( val, &view->ren,
				view->area[CEDVIEW_AREA_BR]->allocation.width -
				1 );

			if ( max_col > osc )
			{
				// Old & new area rectangles overlap, so scroll
				pw = cui_ren_x_from_column ( val, &view->ren,
					osc );
			}
		}

		if ( pw != 0 )
		{
			gdk_window_scroll (
				view->area[CEDVIEW_TITLE_C2]->window, pw, 0 );
			gdk_window_scroll (
				view->area[CEDVIEW_AREA_BR]->window, pw, 0 );

			if ( GDK_IS_WINDOW ( view->area[CEDVIEW_AREA_TR]->window
				) )
			{
				gdk_window_scroll (
					view->area[CEDVIEW_AREA_TR]->window,
					pw, 0 );
			}
		}
		else
		{
			gtk_widget_queue_draw ( view->area[CEDVIEW_TITLE_C2] );
			gtk_widget_queue_draw ( view->area[CEDVIEW_AREA_TR] );
			gtk_widget_queue_draw ( view->area[CEDVIEW_AREA_BR] );
		}
	}
}

/*
NOTE: Thanks to the complexities of variable width columns we have to decide
at click-time how many columns we page to the left or the right.
* scroll_paged bypasses the user clicking in the troughs so we can manually
intervene and scroll around as the user expects.
---
MT 23 Jan 2012
*/

static gboolean vscroll_paged (
	GtkRange	* const	range,
	GtkScrollType	const	scroll,
	gdouble		const	ARG_UNUSED ( value ),
	gpointer	const	user_data
	)
{
	int		delta = 0,
			osr,
			val;
	CedView		* const	view = user_data;
	CedSheet	* sheet;


	if ( ! view )
	{
		return FALSE;
	}

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return FALSE;
	}

	// Get the current value
	val = (int)lrint ( gtk_range_get_value ( range ) );

	// Find out how many rows fit on the screen
	osr = sheet->prefs.start_row;
	delta = cui_ren_row_from_y ( osr, &view->ren,
		view->area[CEDVIEW_AREA_BR]->allocation.height ) - osr;

	switch ( scroll )
	{
	case GTK_SCROLL_PAGE_BACKWARD:
		delta = -delta;
		break;

	case GTK_SCROLL_PAGE_FORWARD:
		break;

	default:
		return FALSE;
	}

	gtk_range_set_value ( range, val + delta );

	return TRUE;			// Actioned
}

static gboolean hscroll_paged (
	GtkRange	* const	range,
	GtkScrollType	const	scroll,
	gdouble		const	ARG_UNUSED ( value ),
	gpointer	const	user_data
	)
{
	int		delta = 0,
			osc,
			val;
	CedView		* const	view = user_data;
	CedSheet	* sheet;


	if ( ! view )
	{
		return FALSE;
	}

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return FALSE;
	}

	// Get the current value
	val = (int)lrint ( gtk_range_get_value ( range ) );

	// Find out how many columns fit on the screen
	osc = sheet->prefs.start_col;

	switch ( scroll )
	{
	case GTK_SCROLL_PAGE_BACKWARD:
		delta = cui_ren_column_from_x_backwards ( osc, &view->ren,
			view->area[CEDVIEW_AREA_BR]->allocation.width - 1 ) -
			osc;

		if ( delta > -1 )
		{
			delta = -1;
		}
		break;

	case GTK_SCROLL_PAGE_FORWARD:
		delta = cui_ren_column_from_x ( osc, &view->ren,
			view->area[CEDVIEW_AREA_BR]->allocation.width - 1 ) -
			osc;

		if ( delta < 1 )
		{
			delta = 1;
		}
		break;

	default:
		return FALSE;
	}

	gtk_range_set_value ( range, val + delta );

	return TRUE;			// Actioned
}

GtkWidget * ced_view_new ( void )
{
	int		r,
			c,
			i;
	int	const	rcs[CEDVIEW_AREA_TOTAL][2] = {
			{ 1, 1 }, { 1, 3 }, { 3, 1 }, { 3, 3 },
			{ 0, 0 },
			{ 0, 1 }, { 0, 3 }, { 1, 0 }, { 3, 0 },

			// Frozen pane areas
			{ 0, 2 }, { 2, 0 },
			{ 1, 2 }, { 3, 2 },
			{ 2, 1 }, { 2, 3 },
			{ 2, 2 }

			};
	int	const	fz_pane_sz[CEDVIEW_FRZ_PANE_TOT][2] = {
				{ CEDVIEW_FRZ_PANE_SIZE, -2 },
				{ -2, CEDVIEW_FRZ_PANE_SIZE },
				{ CEDVIEW_FRZ_PANE_SIZE, -2 },
				{ CEDVIEW_FRZ_PANE_SIZE, -2 },
				{ -2, CEDVIEW_FRZ_PANE_SIZE },
				{ -2,CEDVIEW_FRZ_PANE_SIZE },
				{ CEDVIEW_FRZ_PANE_SIZE, CEDVIEW_FRZ_PANE_SIZE }
				}
			;
	CedView		* view;
	GtkWidget	* area,
			* scroll;
	GtkObject	* adj;
	GtkAttachOptions attach;


	view = calloc ( 1, sizeof ( CedView ) );
	if ( ! view )
	{
		return NULL;
	}

	view->bell[0] = 230;
	view->bell[1] = 150;

	view->table = gtk_table_new ( 5, 5, FALSE );
	gtk_object_set_data ( GTK_OBJECT ( view->table ), CEDVIEW_DATA_KEY,
		view );
	gtk_widget_show ( view->table );

	gtk_signal_connect ( GTK_OBJECT ( view->table ), "destroy",
		GTK_SIGNAL_FUNC ( destroy_cedview ), view );

	for ( i = 0; i < CEDVIEW_AREA_TOTAL; i++ )
	{
		area = gtk_drawing_area_new ();
		mtgex_fix_darea ( area );
		gtk_widget_show ( area );

		g_signal_connect ( area, "expose_event",
			G_CALLBACK ( expose_canvas ), (gpointer)view );

		gtk_widget_set_events ( area, GDK_ALL_EVENTS_MASK );

		view->area[i] = area;

		r = rcs[i][0];
		c = rcs[i][1];

		if ( i == CEDVIEW_AREA_BR )
		{
			attach = GTK_FILL | GTK_EXPAND;
		}
		else
		{
			attach = GTK_FILL;
		}

		gtk_table_attach ( GTK_TABLE ( view->table ), area, (guint)(c),
			(guint)(c + 1), (guint)(r), (guint)(r + 1),
			attach, attach, 0, 0 );
	}

	for ( i = 0; i < CEDVIEW_FRZ_PANE_TOT; i++ )
	{
		gtk_drawing_area_size ( GTK_DRAWING_AREA (
			view->area[ CEDVIEW_FRZ_COL + i ] ),
			fz_pane_sz[i][0], fz_pane_sz[i][1] );
	}


	GTK_WIDGET_SET_FLAGS ( view->area[CEDVIEW_AREA_CORNER], GTK_CAN_FOCUS );

	adj = gtk_adjustment_new ( 1, 1, 2, 1, 1, 1 );
	g_signal_connect ( adj, "value-changed", G_CALLBACK ( scrollbar_moved ),
		(gpointer)view );

	scroll = gtk_vscrollbar_new ( GTK_ADJUSTMENT ( adj ) );
	gtk_widget_show ( scroll );
	gtk_table_attach ( GTK_TABLE ( view->table ), scroll, 4, 4 + 1,
		3, 3 + 1, GTK_FILL, GTK_FILL, 0, 0 );
	view->adj_vert = (GtkAdjustment *)adj;
	g_signal_connect ( GTK_RANGE ( scroll ), "change-value",
		G_CALLBACK ( vscroll_paged ), (gpointer)view );

	adj = gtk_adjustment_new ( 1, 1, 2, 1, 1, 1 );
	g_signal_connect ( adj, "value-changed", G_CALLBACK ( scrollbar_moved ),
		(gpointer)view );

	scroll = gtk_hscrollbar_new ( GTK_ADJUSTMENT ( adj ) );
	gtk_widget_show ( scroll );
	gtk_table_attach ( GTK_TABLE ( view->table ), scroll, 3, 3 + 1,
		4, 4 + 1, GTK_FILL, GTK_FILL, 0, 0 );
	view->adj_hori = (GtkAdjustment *)adj;
	g_signal_connect ( GTK_RANGE ( scroll ), "change-value",
		G_CALLBACK ( hscroll_paged ), (gpointer)view );

	// Start with only main bottom right area visible
	ced_view_set_split ( view->table, 0, 0, 0, 0 );

	return view->table;
}

static void configure_row_header_widths (
	CedView		* const	view
	)
{
	if (	view->adj_hori &&
		view->adj_vert &&
		view->ren.font
		)
	{
		be_cedrender_set_header_width ( &view->ren,
			(int)(view->adj_vert->upper + view->adj_vert->page_size)
			);

		gtk_drawing_area_size ( GTK_DRAWING_AREA (
			view->area [ CEDVIEW_TITLE_R1 ] ),
			view->ren.row_header_width,
			cui_font_get_height ( view->ren.font ) );

		gtk_drawing_area_size ( GTK_DRAWING_AREA (
			view->area [ CEDVIEW_TITLE_R2 ] ),
			view->ren.row_header_width,
			cui_font_get_height ( view->ren.font ) );
	}
}



#define ADJ_PAGE_SIZE 20



static void setup_scrollbars (
	CedView		* const	view
	)
{
	int		min,
			max;
	CedSheet	* const	sheet = view->ren.sheet;


	if ( ! sheet )
	{
		return;
	}

	// Vertical
	min = sheet->prefs.split_r2 + 1;

	max = MAX ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	max = MAX ( view->sheet_rows, max );
	max = MAX ( min, max );

	GTK_ADJUSTMENT ( view->adj_vert )->lower = min;
	GTK_ADJUSTMENT ( view->adj_vert )->upper = max + ADJ_PAGE_SIZE;
	GTK_ADJUSTMENT ( view->adj_vert )->page_size = ADJ_PAGE_SIZE;
	gtk_adjustment_changed ( GTK_ADJUSTMENT ( view->adj_vert ) );

	if ( GTK_ADJUSTMENT ( view->adj_vert )->value > max )
	{
		gtk_adjustment_set_value ( GTK_ADJUSTMENT ( view->adj_vert),
			max );
	}
	if ( GTK_ADJUSTMENT ( view->adj_vert )->value < min )
	{
		gtk_adjustment_set_value ( GTK_ADJUSTMENT ( view->adj_vert ),
			min );
	}

	// Horizontal
	min = sheet->prefs.split_c2 + 1;

	max = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	max = MAX ( view->sheet_cols, max );
	max = MAX ( min, max );

	GTK_ADJUSTMENT ( view->adj_hori )->lower = min;
	GTK_ADJUSTMENT ( view->adj_hori )->upper = max + ADJ_PAGE_SIZE;
	GTK_ADJUSTMENT ( view->adj_hori )->page_size = ADJ_PAGE_SIZE;
	gtk_adjustment_changed ( GTK_ADJUSTMENT ( view->adj_hori ) );

	if ( GTK_ADJUSTMENT ( view->adj_hori )->value > max )
	{
		gtk_adjustment_set_value ( GTK_ADJUSTMENT ( view->adj_hori ),
			max );
	}
	if ( GTK_ADJUSTMENT ( view->adj_hori )->value < min )
	{
		gtk_adjustment_set_value ( GTK_ADJUSTMENT ( view->adj_hori ),
			min );
	}

	configure_row_header_widths ( view );
}

int ced_view_update_geometry (
	CedView		* const	view
	)
{
	if ( ! view )
	{
		return 1;
	}

	view->sheet_rows = 0;
	view->sheet_cols = 0;
	ced_sheet_get_geometry ( view->ren.sheet, &view->sheet_rows,
		&view->sheet_cols );
	setup_scrollbars ( view );

	return 1;
}

int ced_view_set_sheet (
	GtkWidget	* const	widget,
	CedSheet	* const	sheet
	)
{
	CedView		* view;


	if (	ced_view_get ( widget, &view ) ||
		! view
		)
	{
		return 1;
	}

	if ( sheet )
	{
		if ( sheet->prefs.start_row < 1 )
		{
			sheet->prefs.start_row = 1;
		}

		if ( sheet->prefs.start_col < 1 )
		{
			sheet->prefs.start_col = 1;
		}
	}

	view->ren.sheet = sheet;

	// This stops any changes to internal values before we set them later
GTKBUG	g_signal_handlers_block_by_func ( G_OBJECT ( view->adj_vert ),
		G_CALLBACK ( scrollbar_moved ), (gpointer)view );
GTKBUG	g_signal_handlers_block_by_func ( G_OBJECT ( view->adj_hori ),
		G_CALLBACK ( scrollbar_moved ), (gpointer)view );

	ced_view_update_geometry ( view );

GTKBUG	g_signal_handlers_unblock_by_func ( G_OBJECT ( view->adj_vert ),
		G_CALLBACK ( scrollbar_moved ), (gpointer)view );
GTKBUG	g_signal_handlers_unblock_by_func ( G_OBJECT ( view->adj_hori ),
		G_CALLBACK ( scrollbar_moved ), (gpointer)view );

	if ( sheet )
	{
		ced_view_set_split ( widget, sheet->prefs.split_r1,
			sheet->prefs.split_r2,
			sheet->prefs.split_c1, sheet->prefs.split_c2 );

		if ( sheet->prefs.split_r2 + 1 > sheet->prefs.start_row )
		{
			sheet->prefs.start_row = sheet->prefs.split_r2 + 1;
		}

		if ( sheet->prefs.split_c2 + 1 > sheet->prefs.start_col )
		{
			sheet->prefs.start_col = sheet->prefs.split_c2 + 1;
		}

		gtk_adjustment_set_value ( view->adj_vert,
			sheet->prefs.start_row );
		gtk_adjustment_set_value ( view->adj_hori,
			sheet->prefs.start_col );

		ced_view_set_cursor_area ( widget, sheet->prefs.cursor_r1,
			sheet->prefs.cursor_c1,
			sheet->prefs.cursor_r2, sheet->prefs.cursor_c2, 0, 1 );
	}

	ced_view_redraw_view ( view );

	return 0;
}

int ced_view_set_split (
	GtkWidget	* const	widget,
	int		const	min_row,
	int		const	max_row,
	int		const	min_col,
	int		const	max_col
	)
{
	CedView		* view;
	CedSheet	* sheet;

	if (	ced_view_get ( widget, &view ) ||
		! view
		)
	{
		return 1;
	}

	if (	min_row > max_row ||
		min_col > max_col
		)
	{
		return 1;
	}

	if (	min_col == 0 ||
		min_row == 0
		)
	{
		gtk_widget_hide ( view->area[ CEDVIEW_AREA_TL ] );
	}
	else
	{
		gtk_widget_show ( view->area[ CEDVIEW_AREA_TL ] );
	}

	if (	min_col == 0 &&
		min_row == 0
		)
	{
		gtk_widget_hide ( view->area[ CEDVIEW_FRZ_ROW ] );
		gtk_widget_hide ( view->area[ CEDVIEW_FRZ_COL ] );
		gtk_widget_hide ( view->area[ CEDVIEW_FRZ_V_TOP ] );
		gtk_widget_hide ( view->area[ CEDVIEW_FRZ_V_BOTTOM ] );
		gtk_widget_hide ( view->area[ CEDVIEW_FRZ_H_LEFT ] );
		gtk_widget_hide ( view->area[ CEDVIEW_FRZ_H_RIGHT ] );
		gtk_widget_hide ( view->area[ CEDVIEW_FRZ_CORNER ] );
	}
	else
	{
		int		w,
				h;


		if ( min_col == 0 )
		{
			gtk_widget_hide ( view->area[ CEDVIEW_FRZ_H_LEFT ] );
		}
		else
		{
			w = cui_ren_x_from_column ( min_col, &view->ren,
				max_col + 1 );
			gtk_drawing_area_size ( GTK_DRAWING_AREA (
				view->area[CEDVIEW_FRZ_H_LEFT] ),
				w, -2 );
			gtk_widget_show ( view->area[ CEDVIEW_FRZ_H_LEFT ] );
		}

		if ( min_row == 0 )
		{
			gtk_widget_hide ( view->area[ CEDVIEW_FRZ_V_TOP ] );
		}
		else
		{
			h = (max_row - min_row + 1) * CUI_ROWHEIGHT (
				(&view->ren) );
			gtk_drawing_area_size ( GTK_DRAWING_AREA (
				view->area[CEDVIEW_FRZ_V_TOP] ),
				-2, h );
			gtk_widget_show ( view->area[ CEDVIEW_FRZ_V_TOP ] );
		}

		gtk_widget_show ( view->area[ CEDVIEW_FRZ_ROW ] );
		gtk_widget_show ( view->area[ CEDVIEW_FRZ_COL ] );
		gtk_widget_show ( view->area[ CEDVIEW_FRZ_V_BOTTOM ] );
		gtk_widget_show ( view->area[ CEDVIEW_FRZ_H_RIGHT ] );
		gtk_widget_show ( view->area[ CEDVIEW_FRZ_CORNER ] );
	}

	if ( min_col == 0 )
	{
		gtk_widget_hide ( view->area[ CEDVIEW_AREA_BL ] );
		gtk_widget_hide ( view->area[ CEDVIEW_TITLE_C1 ] );
	}
	else
	{
		gtk_widget_show ( view->area[ CEDVIEW_AREA_BL ] );
		gtk_widget_show ( view->area[ CEDVIEW_TITLE_C1 ] );
	}

	if ( min_row == 0 )
	{
		gtk_widget_hide ( view->area[ CEDVIEW_AREA_TR ] );
		gtk_widget_hide ( view->area[ CEDVIEW_TITLE_R1 ] );
	}
	else
	{
		gtk_widget_show ( view->area[ CEDVIEW_AREA_TR ] );
		gtk_widget_show ( view->area[ CEDVIEW_TITLE_R1 ] );
	}

	sheet = view->ren.sheet;
	if ( sheet )
	{
		sheet->prefs.split_r1 = min_row;
		sheet->prefs.split_r2 = max_row;
		sheet->prefs.split_c1 = min_col;
		sheet->prefs.split_c2 = max_col;
	}

	setup_scrollbars ( view );

	return 0;
}

static void ensure_visible_row (
	CedView		* const	view,
	CedSheet	* const	sheet,
	int		const	row
	)
{
	// Move the scrollbars if the cursor is not visible
	if ( row < sheet->prefs.start_row )
	{
		gtk_adjustment_set_value ( view->adj_vert, row );
	}
	else
	{
		int	last_row,	// Last row visible
			new_row;


		last_row = cui_ren_row_from_y ( sheet->prefs.start_row,
			&view->ren,
			view->area[CEDVIEW_AREA_BR]->allocation.height );

		if ( last_row <= row )
		{
			for ( new_row = row - 1; ; new_row -- )
			{
				last_row = cui_ren_row_from_y ( new_row,
					&view->ren, view->area[ CEDVIEW_AREA_BR
					]->allocation.height );

				if ( last_row <= row )
				{
					break;
				}
			}

			new_row ++;

			gtk_adjustment_set_value ( view->adj_vert, new_row );
		}
	}
}

static void ensure_visible_column (
	CedView		* const	view,
	CedSheet	* const	sheet,
	int		const	column
	)
{
	if ( column < sheet->prefs.start_col )
	{
		gtk_adjustment_set_value ( view->adj_hori, column );
	}
	else
	{
		int		last_col,	// Last row visible
				new_col;


		last_col = cui_ren_column_from_x ( sheet->prefs.start_col,
			&view->ren,
			view->area[CEDVIEW_AREA_BR]->allocation.width );

		if ( last_col <= column )
		{
			for ( new_col = column - 1; ; new_col -- )
			{
				last_col = cui_ren_column_from_x ( new_col,
					&view->ren, view->area[ CEDVIEW_AREA_BR
					]->allocation.width );

				if ( last_col <= column )
				{
					break;
				}
			}

			new_col ++;

			gtk_adjustment_set_value ( view->adj_hori, new_col );
		}
	}
}

static void ensure_visible (
	CedView		* const	view,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column
	)
{
	if ( row > sheet->prefs.split_r2 )
	{
		ensure_visible_row ( view, sheet, row );
	}

	if ( column > sheet->prefs.split_c2 )
	{
		ensure_visible_column ( view, sheet, column );
	}
}

int ced_view_ensure_visible (
	CedView		* const	view,
	int		const	row,
	int		const	column
	)
{
	if (	! view			||
		! view->ren.sheet	||
		row < 1			||
		column < 1
		)
	{
		return 1;
	}

	ensure_visible ( view, view->ren.sheet, row, column );

	return 0;
}

// MUST have valid view & sheet
static int ced_view_redraw_view_cursor (
	CedView		* const	view,
	CedSheet	* const	sheet,
	int		const	nr1,
	int		const	nc1,
	int		const	nr2,
	int		const	nc2
	)
{
	int		r1,
			r2,
			c1,
			c2,

			r1_top = -1,
			r1_bot = -1,	// Row min for top/bottom area

			r2_top,
			r2_bot,		// Row max for top/bottom area

			c1_left = -1,
			c1_right = -1,
			c2_left,
			c2_right,
			max_col,
			max_row,
			px1_left,
			pw2_left,
			px1_right,
			pw2_right,
			py1_top,
			ph2_top,
			py1_bot,
			ph2_bot
			;
	GtkWidget	* w;



	/*
	These variable inits stop warnings in gcc (e.g. 4.6.2) when optimizing.
	Its a bug in gcc because these vars are always set early in the func
	if they are used later on.  I have done a manual trace to check.
	---
	MT 10th Jan 2012
	*/

	px1_left = 0;
	pw2_left = 0;
	px1_right = 0;
	pw2_right = 0;
	py1_top = 0;
	ph2_top = 0;
	py1_bot = 0;
	ph2_bot = 0;



	// Ensure r1, c1 = min r2, c2 = max
	r1 = MIN ( nr1, nr2 );
	r2 = MAX ( nr2, nr1 );
	c1 = MIN ( nc1, nc2 );
	c2 = MAX ( nc2, nc1 );

	// If area is poorly set, update all
	if (	r1 < 1 ||
		c1 < 1 ||
		r2 < 1 ||
		c2 < 1
		)
	{
		ced_view_redraw_view ( view );

		return 0;
	}

	// Calculate maximum visible row/column
	max_col = sheet->prefs.start_col;
	max_row = sheet->prefs.start_row;
	w = view->area[ CEDVIEW_AREA_BR ];

	if ( w->allocation.width > 0 )
	{
		max_col = cui_ren_column_from_x ( max_col, &view->ren,
			w->allocation.width - 1 );
	}

	if ( w->allocation.height > 0 )
	{
		max_row = cui_ren_row_from_y ( max_row, &view->ren,
			w->allocation.height - 1 );
	}

	// Calculate extent of cursor on visible areas (updating headers)

	if (	c1 <= sheet->prefs.split_c2 &&
		c2 >= sheet->prefs.split_c1
		)
	{
		// Cursor overlaps left vertical area

		c1_left = MAX ( c1, sheet->prefs.split_c1 );
		c2_left = MIN ( c2, sheet->prefs.split_c2 );

		px1_left = cui_ren_x_from_column ( sheet->prefs.split_c1,
			&view->ren, c1_left );
		pw2_left = cui_ren_x_from_column ( sheet->prefs.split_c1,
			&view->ren, c2_left + 1 ) - px1_left;

		w = view->area[ CEDVIEW_TITLE_C1 ];
		gtk_widget_queue_draw_area ( w, px1_left, 0, pw2_left,
			w->allocation.height );
	}

	if (	c1 <= max_col &&
		c2 >= sheet->prefs.start_col
		)
	{
		// Cursor overlaps right vertical area

		c1_right = MAX ( c1, sheet->prefs.start_col );
		c2_right = MIN ( c2, max_col );

		px1_right = cui_ren_x_from_column ( sheet->prefs.start_col,
			&view->ren, c1_right );
		pw2_right = cui_ren_x_from_column ( sheet->prefs.start_col,
			&view->ren, c2_right + 1 ) - px1_right;

		w = view->area[ CEDVIEW_TITLE_C2 ];
		gtk_widget_queue_draw_area ( w, px1_right, 0, pw2_right,
			w->allocation.height );
	}

	if (	r1 <= sheet->prefs.split_r2 &&
		r2 >= sheet->prefs.split_r1
		)
	{
		// Cursor overlaps top horizontal area

		r1_top = MAX ( r1, sheet->prefs.split_r1 );
		r2_top = MIN ( r2, sheet->prefs.split_r2 );

		py1_top = cui_ren_y_from_row ( sheet->prefs.split_r1,
			&view->ren, r1_top );
		ph2_top = cui_ren_y_from_row ( sheet->prefs.split_r1,
			&view->ren, r2_top + 1 ) - py1_top;

		w = view->area[ CEDVIEW_TITLE_R1 ];
		gtk_widget_queue_draw_area ( w, 0, py1_top,
			w->allocation.width, ph2_top );
	}

	if (	r1 <= max_row &&
		r2 >= sheet->prefs.start_row
		)
	{
		// Cursor overlaps bottom horizontal area

		r1_bot = MAX ( r1, sheet->prefs.start_row );
		r2_bot = MIN ( r2, max_row );

		py1_bot = cui_ren_y_from_row ( sheet->prefs.start_row,
			&view->ren, r1_bot );
		ph2_bot = cui_ren_y_from_row ( sheet->prefs.start_row,
			&view->ren, r2_bot + 1 ) - py1_bot;

		w = view->area[ CEDVIEW_TITLE_R2 ];
		gtk_widget_queue_draw_area ( w, 0, py1_bot,
			w->allocation.width, ph2_bot );
	}

	if (	r1_bot != -1 &&
		c1_right != -1
		)
	{
		w = view->area[ CEDVIEW_AREA_BR ];
		gtk_widget_queue_draw_area ( w, px1_right, py1_bot, pw2_right,
			ph2_bot );
	}

	if (	r1_top != -1 &&
		c1_right != -1
		)
	{
		w = view->area[ CEDVIEW_AREA_TR ];
		gtk_widget_queue_draw_area ( w, px1_right, py1_top, pw2_right,
			ph2_top );
	}

	if (	r1_bot != -1 &&
		c1_left != -1
		)
	{
		w = view->area[ CEDVIEW_AREA_BL ];
		gtk_widget_queue_draw_area ( w, px1_left, py1_bot, pw2_left,
			ph2_bot );
	}

	if (	r1_top != -1 &&
		c1_left != -1
		)
	{
		w = view->area[ CEDVIEW_AREA_TL ];
		gtk_widget_queue_draw_area ( w, px1_left, py1_top, pw2_left,
			ph2_top );
	}

	return 0;
}

int ced_view_set_cursor_area (
	GtkWidget	* const	widget,
	int			r1,
	int			c1,
	int			r2,
	int			c2,
	int		const	cursor_visible,
	int		const	force_update
	)
{
	CedView		* view;
	CedSheet	* sheet;
	int		mr1 = -1,
			mr2,
			mc1,
			mc2;


	if ( ced_view_get ( widget, &view ) )
	{
		return 1;
	}

	if ( r1 < 1 ) r1 = 1;
	if ( r2 < 1 ) r2 = 1;
	if ( c1 < 1 ) c1 = 1;
	if ( c2 < 1 ) c2 = 1;
	if ( r1 > CED_MAX_ROW ) r1 = CED_MAX_ROW;
	if ( r2 > CED_MAX_ROW ) r2 = CED_MAX_ROW;
	if ( c1 > CED_MAX_COLUMN ) c1 = CED_MAX_COLUMN;
	if ( c2 > CED_MAX_COLUMN ) c2 = CED_MAX_COLUMN;

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return 1;
	}

	if (	! force_update &&
		sheet->prefs.cursor_r1 == r1 &&
		sheet->prefs.cursor_c1 == c1 &&
		sheet->prefs.cursor_r2 == r2 &&
		sheet->prefs.cursor_c2 == c2
		)
	{
		return 1;	// Nothing to do
	}

/*
For ultra optimization spot when rows or cols are the same in old & new and only
one row/col is different.  Then just update the difference in rows or cols
between the old and new.  e.g. when the user is enlarging or shrinking the
selection area with the mouse or the arrow keys.
*/

/*
	if (	sheet->prefs.cursor_r1 == r1 &&
		sheet->prefs.cursor_r2 == r2 )
	{
		if ( sheet->prefs.cursor_c1 == c1 )
		{
			mr1 = r1;
			mr2 = r2;
			mc1 = c2;
			mc2 = sheet->prefs.cursor_c2;
		}
		else if ( sheet->prefs.cursor_c2 == c2 )
		{
			mr1 = r1;
			mr2 = r2;
			mc1 = c1;
			mc2 = sheet->prefs.cursor_c1;
		}
	}

NOTE - we disallow horizontal optimization because sometimes long text lines
need to be fully redrawn rather than partially by a single column.  For example
in the test suite there is the long line test which doesn't redraw properly when
the user expands the cursor selection in a particular way.  This is because of
the different text colour that is rendered if the text cell is selected, or
painted over if the text cell isn't selected.
*/

	if (	sheet->prefs.cursor_c1 == c1 &&
		sheet->prefs.cursor_c2 == c2
		)
	{
		if ( sheet->prefs.cursor_r1 == r1 )
		{
			mr1 = sheet->prefs.cursor_r2;
			mr2 = r2;
			mc1 = c1;
			mc2 = c2;
		}
		else if ( sheet->prefs.cursor_r2 == r2 )
		{
			mr1 = sheet->prefs.cursor_r1;
			mr2 = r1;
			mc1 = c1;
			mc2 = c2;
		}
	}

	if ( mr1 == -1 )
	{
		ced_view_redraw_view_cursor ( view, sheet,
			sheet->prefs.cursor_r1, sheet->prefs.cursor_c1,
			sheet->prefs.cursor_r2, sheet->prefs.cursor_c2
			);
	}

	sheet->prefs.cursor_r1 = r1;
	sheet->prefs.cursor_c1 = c1;
	sheet->prefs.cursor_r2 = r2;
	sheet->prefs.cursor_c2 = c2;

	if ( mr1 == -1 )
	{
		ced_view_redraw_view_cursor ( view, sheet, r1, c1, r2, c2 );
	}
	else
	{
		ced_view_redraw_view_cursor ( view, sheet, mr1, mc1, mr2, mc2 );
	}

	setup_scrollbars ( view );

	if ( cursor_visible )
	{
		ensure_visible ( view, sheet, r1, c1 );
	}

	return 0;
}

int ced_view_set_font (
	GtkWidget	* const	widget,
	mtFont		* const	font
	)
{
	CedView		* view;


	if (	ced_view_get ( widget, &view ) ||
		! view )
	{
		return 1;
	}

	view->ren.font = font;
	if ( ! font )
	{
		return 0;
	}

	gtk_drawing_area_size ( GTK_DRAWING_AREA ( view->area[CEDVIEW_TITLE_C1]
		), -2, cui_font_get_height ( font ) );
	gtk_drawing_area_size ( GTK_DRAWING_AREA ( view->area[CEDVIEW_TITLE_C2]
		), -2, cui_font_get_height ( font ) );

	be_cedrender_set_font_width ( &view->ren );

	configure_row_header_widths ( view );
	ced_view_redraw ( widget );

	return 0;
}

int ced_view_grab_focus (
	GtkWidget	* const	widget
	)
{
	CedView		* view;

	if (	ced_view_get ( widget, &view ) ||
		! view
		)
	{
		return 1;
	}

	gtk_widget_grab_focus ( view->area[CEDVIEW_AREA_CORNER] );

	return 0;
}

int ced_view_get (
	GtkWidget	* const widget,
	CedView		** const cedview
	)
{
	if ( ! widget || ! cedview )
	{
		return 1;
	}

	cedview[0] = gtk_object_get_data ( GTK_OBJECT ( widget ),
		CEDVIEW_DATA_KEY );

	return 0;
}

int ced_view_redraw (
	GtkWidget	* const	widget
	)
{
	CedView		* view;


	if (	ced_view_get ( widget, &view ) ||
		ced_view_redraw_view ( view )
		)
	{
		return 1;
	}

	return 0;
}

int ced_view_redraw_view (
	CedView		* const	view
	)
{
	int		i;


	if ( ! view )
	{
		return 1;
	}

	for ( i = 0; i < CEDVIEW_AREA_TOTAL; i++ )
	{
		gtk_widget_queue_draw ( view->area[i] );
	}

	return 0;
}

int ced_view_bell_swap (
	CedView		* const	view
	)
{
	int		i;

	if ( ! view )
	{
		return 1;
	}

	i = view->bell[0];
	view->bell[0] = view->bell[1];
	view->bell[1] = i;

	gtk_widget_queue_draw ( view->area[CEDVIEW_AREA_CORNER] );

	return 0;
}

