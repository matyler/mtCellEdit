/*
	Copyright (C) 2008-2014 Mark Tyler

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



// global.cedview->ren.sheet must exist
static int visible_rows_total ( void )
{
	int		start,
			tot;


	start = global.cedview->ren.sheet->prefs.start_row;
	tot = cui_ren_row_from_y ( start, &global.cedview->ren,
		global.cedview->area[CEDVIEW_AREA_BR]->allocation.height )
			- start;

	if ( tot < 1 )
	{
		tot = 1;
	}

	return tot;
}



typedef struct
{
	int		changed,
			r,
			c;
} jumpSTATE;



static int jump_to_active_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	jumpSTATE	* const	jstate = user_data;


	jstate->r = row;
	jstate->c = col;

	return 1;			// STOP
}

gboolean key_event (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventKey	* const	event,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		row_change = 0,
			col_change = 0,
			home = 0,
			end = 0,
			key_shift,
			key_ctrl,
			key_alt,
			return_true = 0;
	uint32_t	unicode = gdk_keyval_to_unicode ( event->keyval );
	CedSheet	* sheet;
	jumpSTATE	jstate = { 0 };


	sheet = global.cedview->ren.sheet;
	if ( ! sheet )
	{
		return FALSE;
	}

	jstate.r = sheet->prefs.cursor_r2;
	jstate.c = sheet->prefs.cursor_c2;

	if ( event->keyval != GDK_Tab )
	{
		grab_focus_sheet ();

		// Hack to ensure Ctrl + Home/End works after Ctrl + Tab in the
		// cell ref entry.
	}

	key_shift = (event->state & GDK_SHIFT_MASK);
	key_ctrl = (event->state & GDK_CONTROL_MASK);
	key_alt = (event->state & GDK_MOD1_MASK);

	switch ( event->keyval )	// Handle any control keys
	{
	case GDK_KP_Up:
	case GDK_Up:
		if ( key_ctrl )
		{
			return_true = 1;

			row_change = 1 - sheet->prefs.cursor_r2; // Default
			if ( jstate.r > 1 )
			{
				if ( ced_sheet_scan_area_backwards ( sheet,
					jstate.r - 1, CED_MAX_COLUMN,
					jstate.r - 1, CED_MAX_COLUMN,
					jump_to_active_cb, &jstate ) == 2 )
				{
					row_change = jstate.r -
						sheet->prefs.cursor_r2;
				}
			}
		}
		else
		{
			row_change = -1;
		}
		break;

	case GDK_KP_Enter:
	case GDK_Return:
	case GDK_KP_Down:
	case GDK_Down:
		if ( key_ctrl )
		{
			return_true = 1;

			row_change = CED_MAX_ROW - sheet->prefs.cursor_r2;
					// Default

			if ( jstate.r < CED_MAX_ROW )
			{
				if ( ced_sheet_scan_area ( sheet,
					jstate.r + 1, 0, 0, 0,
					jump_to_active_cb, &jstate ) == 2 )
				{
					row_change = jstate.r -
						sheet->prefs.cursor_r2;
				}
			}
		}
		else
		{
			row_change = 1;
		}
		break;

	case GDK_KP_Left:
	case GDK_Left:
		if ( key_ctrl )
		{
			return_true = 1;

			col_change = 1 - sheet->prefs.cursor_c2; // Default
			if ( jstate.c > 1 )
			{
				if ( ced_sheet_scan_area_backwards ( sheet,
					jstate.r, jstate.c - 1,
					1, jstate.c - 1,
					jump_to_active_cb, &jstate ) == 2 )
				{
					col_change = jstate.c -
						sheet->prefs.cursor_c2;
				}
			}
		}
		else
		{
			col_change = -1;
		}
		break;

	case GDK_KP_Right:
	case GDK_Right:
		if ( key_ctrl )
		{
			return_true = 1;

			col_change = CED_MAX_COLUMN - sheet->prefs.cursor_c2;
					// Default

			if ( jstate.c < CED_MAX_COLUMN )
			{
				if ( ced_sheet_scan_area ( sheet,
					jstate.r, jstate.c + 1, 1, 0,
					jump_to_active_cb, &jstate ) == 2 )
				{
					col_change = jstate.c -
						sheet->prefs.cursor_c2;
				}
			}
		}
		else
		{
			col_change = 1;
		}
		break;

	case GDK_KP_Page_Up:
	case GDK_Page_Up:
		if ( key_alt )
		{
			// ALT key pressed so shift a page to the left
			col_change = cui_ren_column_from_x_backwards (
				sheet->prefs.cursor_c2,
				&global.cedview->ren,
				global.cedview->area[CEDVIEW_AREA_BR]->
					allocation.width
				) - sheet->prefs.cursor_c2;

			if ( col_change > -1 )
			{
				col_change = -1;
			}
		}
		else
		{
			row_change = -visible_rows_total ();
		}
		break;

	case GDK_KP_Page_Down:
	case GDK_Page_Down:
		if ( key_alt )
		{
			// ALT key pressed so shift a page to the right

			col_change = cui_ren_column_from_x (
				sheet->prefs.cursor_c2,
				&global.cedview->ren,
				global.cedview->area[CEDVIEW_AREA_BR]->
					allocation.width
				) - sheet->prefs.cursor_c2;

			if ( col_change < 1 )
			{
				col_change = 1;
			}
		}
		else
		{
			row_change = visible_rows_total ();
		}
		break;

	case GDK_Tab:
		if ( key_ctrl || key_shift )
		{
			col_change = -1;
		}
		else
		{
			col_change = 1;
		}
		break;

	case GDK_KP_Home:
	case GDK_Home:
		home = 1;
		col_change = 1 - sheet->prefs.cursor_c2;
		if ( key_ctrl )
		{
			row_change = 1 - sheet->prefs.cursor_r2;
		}
		break;

	case GDK_KP_End:
	case GDK_End:
		end = 1;
		col_change = global.cedview->sheet_cols -
			sheet->prefs.cursor_c2;

		if ( key_ctrl )
		{
			row_change = global.cedview->sheet_rows -
				sheet->prefs.cursor_r2;
		}
		break;
	}

	if ( row_change || col_change || home || end )
	{
		int		newrow = row_change,
				newcol = col_change;

		if ( key_shift )
		{
			newrow += sheet->prefs.cursor_r2;
			newcol += sheet->prefs.cursor_c2;

			if ( newrow < 1 )
			{
				newrow = 1;
			}

			if ( newcol < 1 )
			{
				newcol = 1;
			}

			set_cursor_range ( sheet->prefs.cursor_r1,
				sheet->prefs.cursor_c1,
				newrow, newcol, 0, 0 );
			ced_view_ensure_visible ( global.cedview,
				sheet->prefs.cursor_r2,
				sheet->prefs.cursor_c2 );
		}
		else
		{
			if ( key_ctrl || home || end )
			{
				newrow += sheet->prefs.cursor_r2;
				newcol += sheet->prefs.cursor_c2;
			}
			else
			{
				newrow += sheet->prefs.cursor_r1;
				newcol += sheet->prefs.cursor_c1;
			}

			if ( newrow < 1 )
			{
				newrow = 1;
			}

			if ( newcol < 1 )
			{
				newcol = 1;
			}

			set_cursor_range ( newrow, newcol, newrow, newcol,
				1, 0 );
		}

		return TRUE;
	}

	if ( return_true )
	{
		return TRUE;
	}

	if ( key_alt || key_ctrl )
	{
		// We must avoid interpreting Alt + F, Ctrl + O etc. as Unicode
		// keys.

		return FALSE;		// Not actioned
	}

	if ( unicode && sheet->prefs.locked )
	{
		mtgex_alert_box ( _("Error"), _("Sheet locked."), _("OK"),
			NULL, NULL, global.main_window );

		return TRUE;
	}

	if (	unicode &&
		GTK_WIDGET_SENSITIVE ( global.entry_celltext )
		)			// Is this a Unicode key?
	{
		int		bytes;
		char		newtxt[16];


		bytes = mtkit_utf8_from_utf32 ( (unsigned char *)newtxt,
			unicode );

		if ( bytes <= 0 )
		{
			return FALSE;	// Fail, so no action
		}

		gtk_entry_set_text ( GTK_ENTRY ( global.entry_celltext ),
			newtxt );
		grab_focus_entry ();

		return TRUE;		// Actioned
	}

	return FALSE;			// Not actioned -
					// no control key and no unicode input
}

gboolean scroll_wheel (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventScroll	* const	event,
	CedView		* const	view
	)
{
	double		val,
			page;
	GtkAdjustment	* adj;


	if ( ! view || ! view->ren.sheet )
	{
		return FALSE;
	}

	adj = GTK_ADJUSTMENT ( view->adj_vert );
	val = adj->value;
	page = visible_rows_total () / 8;

	if ( page < 1 )
	{
		page = 1;
	}

	switch ( event->direction )
	{
	case GDK_SCROLL_DOWN:
		val += page;
		break;

	case GDK_SCROLL_UP:
		val -= page;
		break;

	default:
		return FALSE;
	}

	if ( (val + page) > (adj->upper - adj->page_size) )
	{
		val = adj->upper - adj->page_size;
	}

	if ( val < adj->lower )
	{
		val = adj->lower;
	}

	gtk_adjustment_set_value ( adj, val );

	return TRUE;
}

gboolean button_press_header (
	GtkWidget	* const	widget,
	GdkEventButton	* const	event,
	gpointer	const	data
	)
{
	int		row_start = 0,
			col_start = 0,
			r1,
			c1,
			r2,
			c2;
	CedView		* const	view = (CedView *)data;
	CedSheet	* sheet;


	if ( ! view || ! event )
	{
		return FALSE;
	}

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return FALSE;
	}

	if (	event->button != 1 &&
		event->button != 3
		)
	{
		// We only look at left/right clicks

		return FALSE;
	}

	ced_view_grab_focus ( view->table );

	if ( widget == view->area[CEDVIEW_TITLE_C1] )
	{
		col_start = sheet->prefs.split_c1;
	}
	else if ( widget == view->area[CEDVIEW_TITLE_C2] )
	{
		col_start = sheet->prefs.start_col;
	}
	else if ( widget == view->area[CEDVIEW_TITLE_R1] )
	{
		row_start = sheet->prefs.split_r1;
	}
	else if ( widget == view->area[CEDVIEW_TITLE_R2] )
	{
		row_start = sheet->prefs.start_row;
	}
	else
	{
		return FALSE;
	}

	r1 = r2 = 1;
	c1 = c2 = 1;

	if ( event->button == 1 )
	{
		if ( col_start )
		{
			r2 = view->sheet_rows;
			c1 = c2 = cui_ren_column_from_x ( col_start, &view->ren,
				(int)(event->x) );
		}
		else if ( row_start )
		{
			r1 = r2 = cui_ren_row_from_y ( row_start, &view->ren,
				(int)(event->y) );
			c2 = view->sheet_cols;
		}
		else
		{
			return FALSE;
		}
	}

	if ( event->button == 3 ) // Right click - change second corner only
	{
		if ( col_start )
		{
			c1 = sheet->prefs.cursor_c1;
			r2 = view->sheet_rows;
			c2 = cui_ren_column_from_x ( col_start, &view->ren,
				(int)(event->x) );
		}
		else if ( row_start )
		{
			r1 = sheet->prefs.cursor_r1;
			r2 = cui_ren_row_from_y ( row_start, &view->ren,
				(int)(event->y) );
			c2 = view->sheet_cols;
		}
		else
		{
			return FALSE;
		}
	}

	if ( r1 < 1 ) { r1 = 1; }
	if ( r2 < 1 ) { r2 = 1; }
	if ( c1 < 1 ) { c1 = 1; }
	if ( c2 < 1 ) { c2 = 1; }

	set_cursor_range ( r1, c1, r2, c2, 0, 0 );

	return TRUE;
}

gboolean motion_notify_header (
	GtkWidget	* const	widget,
	GdkEventMotion	* const	event,
	gpointer	const	data
	)
{
	GdkModifierType	state;
	int		row_start = 0,
			col_start = 0,
			x,
			y,
			r1,
			c1,
			r2,
			c2,
			button;
	CedView		* view = (CedView *)data;
	CedSheet	* sheet;


	if ( GTK_WIDGET_HAS_FOCUS ( global.entry_celltext ) )
	{
		return FALSE;
	}

	if ( ! view || ! event )
	{
		return FALSE;
	}

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return FALSE;
	}

	if ( event->is_hint )
	{
		gdk_device_get_state ( event->device, event->window, NULL,
			&state );
	}

	x = (int)(event->x);
	y = (int)(event->y);
	state = event->state;

	button = 0;
	if ( state & GDK_BUTTON1_MASK )
	{
		button = 1;
	}

	if (	! button &&
		(state & GDK_BUTTON3_MASK)
		)
	{
		button = 3;
	}

	if ( ! button )
	{
		return FALSE;		// Only follow left/right button drags
	}

	// Allow left clicking in any of the 4 areas to set cursor position
	if ( widget == view->area[CEDVIEW_TITLE_C1] )
	{
		col_start = sheet->prefs.split_c1;
	}
	else if ( widget == view->area[CEDVIEW_TITLE_C2] )
	{
		col_start = sheet->prefs.start_col;
	}
	else if ( widget == view->area[CEDVIEW_TITLE_R1] )
	{
		row_start = sheet->prefs.split_r1;
	}
	else if ( widget == view->area[CEDVIEW_TITLE_R2] )
	{
		row_start = sheet->prefs.start_row;
	}
	else
	{
		return FALSE;
	}

	if ( col_start )
	{
		r1 = 1;
		c1 = sheet->prefs.cursor_c1;
		r2 = view->sheet_rows;
		c2 = cui_ren_column_from_x ( col_start, &view->ren, x );
	}
	else if ( row_start )
	{
		r1 = sheet->prefs.cursor_r1;
		c1 = 1;
		r2 = cui_ren_row_from_y ( row_start, &view->ren, y );
		c2 = view->sheet_cols;
	}
	else
	{
		return FALSE;
	}

	if ( r1 < 1 ) { r1 = 1; }
	if ( r2 < 1 ) { r2 = 1; }
	if ( c1 < 1 ) { c1 = 1; }
	if ( c2 < 1 ) { c2 = 1; }

	set_cursor_range ( r1, c1, r2, c2, 0, 0 );

	return TRUE;
}


gboolean button_press_canvas (
	GtkWidget	* const	widget,
	GdkEventButton	* const	event,
	gpointer	const	data
	)
{
	int		row_start,
			col_start,
			r,
			c;
	CedView		* view = (CedView *)data;
	CedSheet	* sheet;


	if ( ! view || ! event )
	{
		return FALSE;
	}

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return FALSE;
	}

	if ( event->button != 1 && event->button != 3 )
	{
		// We only look at left/right clicks

		return FALSE;
	}

	// Allow left clicking in any of the 4 areas to set cursor position
	if ( widget == view->area[CEDVIEW_AREA_BR] )
	{
		row_start = sheet->prefs.start_row;
		col_start = sheet->prefs.start_col;
	}
	else if ( widget == view->area[CEDVIEW_AREA_BL] )
	{
		row_start = sheet->prefs.start_row;
		col_start = sheet->prefs.split_c1;
	}
	else if ( widget == view->area[CEDVIEW_AREA_TL] )
	{
		row_start = sheet->prefs.split_r1;
		col_start = sheet->prefs.split_c1;
	}
	else if ( widget == view->area[CEDVIEW_AREA_TR] )
	{
		row_start = sheet->prefs.split_r1;
		col_start = sheet->prefs.start_col;
	}
	else
	{
		return FALSE;
	}

	r = cui_ren_row_from_y ( row_start, &view->ren, (int)(event->y) );
	c = cui_ren_column_from_x ( col_start, &view->ren, (int)(event->x) );

	if (	GTK_WIDGET_HAS_FOCUS ( global.entry_celltext ) &&
		GTK_WIDGET_SENSITIVE ( global.entry_celltext )
		)
	{
		char		txt[128] = {0},
				* tp = txt;
		CedCellRef	ref;
		gint		pos;


		// Get cell reference and put it into the cell formula

		if ( event->button == 3 )
		{
			*tp++ = ':';
		}

		if ( event->state & GDK_SHIFT_MASK )
		{
			// Absolute column
			ref.row_m = 0;
			ref.row_d = r;
		}
		else
		{
			// Relative row
			ref.row_m = 1;
			ref.row_d = r - sheet->prefs.cursor_r1;
		}

		if ( event->state & GDK_CONTROL_MASK )
		{
			// Absolute column
			ref.col_m = 0;
			ref.col_d = c;
		}
		else
		{
			// Relative column
			ref.col_m = 1;
			ref.col_d = c - sheet->prefs.cursor_c1;
		}

		ced_cellreftostr ( tp, &ref );

		if ( txt[0] != 0 )
		{
			pos = gtk_editable_get_position (
				GTK_EDITABLE ( global.entry_celltext ) );

			gtk_editable_insert_text (
				GTK_EDITABLE ( global.entry_celltext ),
				(const gchar *)txt, -1, &pos );

			gtk_editable_set_position (
				GTK_EDITABLE ( global.entry_celltext ), pos );
		}

		return TRUE;
	}

	ced_view_grab_focus ( view->table );

	if ( event->button == 1 )	// Left click - set first corner
	{
		set_cursor_range ( r, c, r, c, 1, 0 );
	}
	else if ( event->button == 3 )	// Right click - set second corner
	{
		set_cursor_range ( sheet->prefs.cursor_r1,
			sheet->prefs.cursor_c1, r, c, 0, 0 );
	}

	return TRUE;
}

gboolean motion_notify_canvas (
	GtkWidget	* const	widget,
	GdkEventMotion	* const	event,
	gpointer	const	data
	)
{
	GdkModifierType	state;
	int		row_start,
			col_start,
			x,
			y,
			r,
			c,
			button;
	CedView		* view = (CedView *)data;
	CedSheet	* sheet;

	if ( GTK_WIDGET_HAS_FOCUS ( global.entry_celltext ) )
	{
		return FALSE;
	}

	if ( ! view || ! event )
	{
		return FALSE;
	}

	sheet = view->ren.sheet;
	if ( ! sheet )
	{
		return FALSE;
	}

	if ( event->is_hint )
	{
		gdk_device_get_state ( event->device, event->window, NULL,
			&state);
	}

	x = (int)(event->x);
	y = (int)(event->y);
	state = event->state;

	button = 0;
	if ( state & GDK_BUTTON1_MASK )
	{
		button = 1;
	}

	if ( ! button && (state & GDK_BUTTON3_MASK) )
	{
		button = 3;
	}

	if ( ! button )
	{
		return FALSE;		// Only follow left/right button drags
	}

	// Allow left clicking in any of the 4 areas to set cursor position
	if ( widget == view->area[CEDVIEW_AREA_BR] )
	{
		row_start = sheet->prefs.start_row;
		col_start = sheet->prefs.start_col;
	}
	else if ( widget == view->area[CEDVIEW_AREA_BL] )
	{
		row_start = sheet->prefs.start_row;
		col_start = sheet->prefs.split_c1;
	}
	else if ( widget == view->area[CEDVIEW_AREA_TL] )
	{
		row_start = sheet->prefs.split_r1;
		col_start = sheet->prefs.split_c1;
	}
	else if ( widget == view->area[CEDVIEW_AREA_TR] )
	{
		row_start = sheet->prefs.split_r1;
		col_start = sheet->prefs.start_col;
	}
	else
	{
		return FALSE;
	}

	r = cui_ren_row_from_y ( row_start, &view->ren, y );
	c = cui_ren_column_from_x ( col_start, &view->ren, x );

	set_cursor_range ( sheet->prefs.cursor_r1, sheet->prefs.cursor_c1,
		r, c, 0, 0 );

	return TRUE;
}

static gboolean commit_input_and_move_cursor (
	int		const	row_delta,
	int		const	column_delta
	)
{
	int		res,
			r,
			c;
	char	const	* newtxt;
	CedSheet	* sheet;


	sheet = global.cedview->ren.sheet;
	if ( ! sheet )
	{
		return TRUE;
	}

	newtxt = gtk_entry_get_text ( GTK_ENTRY ( global.entry_celltext ) );

	if ( newtxt && newtxt[0] == 0 )
	{
		newtxt = NULL;
	}

	res = cui_sheet_set_cell ( global.file->cubook, sheet,
		sheet->prefs.cursor_r1, sheet->prefs.cursor_c1, newtxt );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return TRUE;
	}

	grab_focus_sheet ();

	/*
	Manually setting these limits here is safe because the scrollbars have
	already changed thanks to set_cursor_range ().  This is far more
	efficient than wasting time getting the new geometry from libmtcelledit.
	update_changes_chores () helpfully updates the rest of the GUI for us.
	*/

	global.cedview->sheet_rows = MAX ( global.cedview->sheet_rows,
		sheet->prefs.cursor_r1 );

	global.cedview->sheet_cols = MAX ( global.cedview->sheet_cols,
		sheet->prefs.cursor_c1 );

	r = sheet->prefs.cursor_r1 + row_delta;
	c = sheet->prefs.cursor_c1 + column_delta;

	set_cursor_range ( r, c, r, c, 1, 0 );

	update_changes_chores ( 0, 0 );

	return TRUE;
}

gboolean celltext_entry_key_event (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventKey	* const	event,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	int		row_d = 0,
			col_d = 0,
			key_ctrl,
			key_shift;


	key_ctrl = (event->state & GDK_CONTROL_MASK);
	key_shift = (event->state & GDK_SHIFT_MASK);

	switch ( event->keyval )
	{
	case GDK_Escape:
		update_entry_celltext ();
		grab_focus_sheet ();
		return TRUE;

	case GDK_Up:
	case GDK_KP_Up:
		row_d = -1;
		break;

	case GDK_Down:
	case GDK_KP_Down:
		row_d = 1;
		break;

	case GDK_Right:
	case GDK_KP_Right:
		// Pressing right with cursor on far right causes commit & move
		if (	GTK_ENTRY ( global.entry_celltext)->text_length ==
			GTK_ENTRY ( global.entry_celltext)->current_pos
			)
		{
			col_d = 1;
		}
		break;

	case GDK_KP_Enter:
	case GDK_Return:
		if ( key_ctrl )
		{
			col_d = 1;
		}
		else
		{
			row_d = 1;
		}
		break;

	case GDK_Tab:
		if ( key_ctrl || key_shift )
		{
			col_d = -1;
		}
		else
		{
			col_d = 1;
		}
		break;
	}

	if ( row_d || col_d )
	{
		commit_input_and_move_cursor ( row_d, col_d );

		return TRUE;
	}

	return FALSE;			// Not actioned
}

gboolean cellref_entry_activate (
	GtkWidget	* const	widget,
	GdkEventKey	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	char	const	* txt;
	CedCellRef	r1,
			r2;


	txt = gtk_entry_get_text ( GTK_ENTRY ( widget ) );

	if ( ! ced_strtocellref ( txt, &r1, NULL, 1 ) )
	{
		if (	r1.row_m	||
			r1.col_m	||
			r1.row_d < 1	||
			r1.col_d < 1
			)
		{
			return TRUE;
		}

		set_cursor_range ( r1.row_d, r1.col_d, r1.row_d, r1.col_d,
			1, 1 );
	}
	else if ( ! ced_strtocellrange ( txt, &r1, &r2, NULL, 1 ) )
	{
		if (	r1.row_m	||
			r1.col_m	||
			r1.row_d < 1	||
			r1.col_d < 1	||
			r2.row_m	||
			r2.col_m	||
			r2.row_d < 1	||
			r2.col_d < 1
			)
		{
			return TRUE;
		}

		set_cursor_range ( r1.row_d, r1.col_d, r2.row_d, r2.col_d,
			1, 1 );
	}
	else
	{
		return TRUE;
	}

	grab_focus_sheet ();

	return TRUE;
}

gboolean key_event_escape (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventKey	* const	event,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( event->keyval == GDK_Escape )
	{
		grab_focus_sheet ();

		return TRUE;
	}

	return FALSE;			// Not actioned
}

gboolean cellref_entry_key_event (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkEventKey	* const	event,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	if ( event->keyval == GDK_Escape )
	{
		update_entry_cellref ();
		grab_focus_sheet ();

		return TRUE;
	}

	return FALSE;	// Not actioned - no control key and no unicode input
}

void quicksum_bmenu_changed (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	void		* const	ARG_UNUSED ( user_data ),
	int		const	new_value
	)
{
	global.sum_label_op = new_value;
	update_quicksum_label ();
}

