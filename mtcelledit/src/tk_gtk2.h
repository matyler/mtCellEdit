/*
	Copyright (C) 2008-2015 Mark Tyler

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#pragma GCC diagnostic pop


#include <mtgex.h>

#include "be.h"



typedef struct	CedView		CedView;
typedef struct	GUI_Global	GUI_Global;



enum
{
	PANE_PAIR_STORE_POSITION,

	PANE_PAIR_HIDE_FIND,
	PANE_PAIR_HIDE_GRAPH,

	PANE_PAIR_SHOW_FIND,
	PANE_PAIR_SHOW_GRAPH
};

enum
{
	FS_LOAD_PROJECT		= 1,
	FS_SAVE_PROJECT,
	FS_IMPORT_PROJECT,
	FS_EXPORT_SHEET,
	FS_EXPORT_SHEET_OUTPUT,
	FS_EXPORT_GRAPH
};

enum		// Menu item widget references
{
	_MENU_REC_SEP,
	_MENU_REC_1, _MENU_REC_2, _MENU_REC_3, _MENU_REC_4, _MENU_REC_5,
	_MENU_REC_6, _MENU_REC_7, _MENU_REC_8, _MENU_REC_9, _MENU_REC_10,
	_MENU_REC_11, _MENU_REC_12, _MENU_REC_13, _MENU_REC_14, _MENU_REC_15,
	_MENU_REC_16, _MENU_REC_17, _MENU_REC_18, _MENU_REC_19, _MENU_REC_20,

	_MENU_UNDO, _MENU_REDO,

	_MENU_CUT, _MENU_COPY, _MENU_PASTE, _MENU_CLEAR, _MENU_SELECT_ALL,
	_MENU_CLEAR_CONTENT, _MENU_CLEAR_PREFS,
	_MENU_ROW_INSERT, _MENU_ROW_DELETE, _MENU_COL_INSERT, _MENU_COL_DELETE,

	_MENU_SHEET_LOCKED,
	_MENU_SHEET_FREEZE_PANES,

// Changes must be reflected in menu_refdef[]

	_MENU_TOTAL
};



#define PATHBUF				2048
#define	CEDVIEW_FRZ_PANE_SIZE		1
#define FS_SELECT_FILE			15
#define FS_SELECT_DIR			16

// Stops -pedantic warnings
#define GTKBUG __extension__



struct CedView
{
	GtkWidget	* table,	// This widget container
			* area[CEDVIEW_AREA_TOTAL]
					// Drawing areas: top left,
					// bottom right etc.
			;

	GtkAdjustment	* adj_hori,
			* adj_vert;	// Scrollbars

	int		sheet_rows,	// Cache of the current sheet geometry
			sheet_cols
			;

	int		bell[2];	// Visual bell setting 0..255,
					// [0] = current [1] = next

	CuiRender	ren;
};

struct GUI_Global
{
	int		mem_changed // Have changes to the project been made?
			;
	GtkWidget	* main_window,
			* table,
			* menu_widgets[_MENU_TOTAL],
			* entry_celltext,
			* entry_cellref,
			* bmenu_sheet,
			* label_sum,
			* pane_main,
			* pane_find,
			* pane_graph,
			* pane_pair
			;

	CedView		* cedview;
	mtFont		* font;

	int		sum_label_op;	// The current sum label operation

	CuiFile		* file;
	CuiClip		* clipboard;
};



extern GUI_Global	global;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

GtkWidget * ced_view_new ( void );

int ced_view_set_sheet (		// Set the sheet for the widget to show
	GtkWidget	* widget,	// Sets up scrollbars and refreshes
					// everything
	CedSheet	* sheet
	);

int ced_view_set_split (		// Set the split values
	GtkWidget	* widget,
	int		min_row,
	int		max_row,
	int		min_col,
	int		max_col
	);

int ced_view_set_cursor_area (		// Make a selection
	GtkWidget	* widget,
	int		r1,
	int		c1,
	int		r2,
	int		c2,
	int		cursor_visible,	// Move the scrollbars to ensure the
					// cursor is visible
	int		force_update	// 0 = checks for changes, if none,
					// do nothing. 1 = forces update
	);

int ced_view_set_font (			// Set the font
	GtkWidget	* widget,
	mtFont		* font
	);

int ced_view_grab_focus (		// Ensure that keypresses are caught by
					// CEDVIEW_AREA_CORNER
	GtkWidget	* widget
	);

int ced_view_get (
	GtkWidget	* widget,
	CedView		** cedview	// Put reference here
	);

int ced_view_redraw (
	GtkWidget	* widget
	);

int ced_view_redraw_view (
	CedView		* view
	);

int ced_view_update_geometry (		// Check current sheet geometry and set
					// up scrollbars
	CedView		* view
	);

int ced_view_bell_swap (		// Swap and update the visual bell
	CedView		* view
	);

int ced_view_ensure_visible (		// Ensure that this cell is visible
	CedView		* view,
	int		row,
	int		column
	);

void pressed_help (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

void pressed_about (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

void file_selector (
	int		action_type,
	GtkWidget	* main_window,
	char	const	* filename
	);

int project_load (
	char	const	* filename
	);

int project_import (
	char	const	* filename
	);

int project_save (
	char	const	* filename,
	int		filetype
	);

int project_export (
	char	const	* filename,
	int		filetype
	);

int project_export_output (
	char	const	* filename,
	int		filetype
	);

int project_export_graph (
	char	const	* filename,
	int		filetype
	);

void pressed_new_project (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_open_project (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_import_project (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_save_project_as (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_save_project (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_recent (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

void pressed_sheet_export (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_export_output (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_select_all (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_edit_cell (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_row_insert (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_row_delete (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_column_insert (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_column_delete (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_column_width (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_column_width_auto (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_edit_book_prefs (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_fix_2dyears (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_new (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_duplicate (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_rename (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_delete (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_previous (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_next (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_freeze_panes (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_sheet_locked (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

char * dialog_text_entry (
	char	const	* title,
	char	const	* entry_text
	);
	// Returns an allocated string. NULL = cancel

void update_main_area ( void );

void update_menus ( void );		// Update undo/redo menus

void update_titlebar ( void );		// Update filename in titlebar and the
					// bmenus
void update_entry_cellref ( void );

void update_entry_celltext ( void );

void update_recent_files ( void );	// Update the menu items

void update_quicksum_label ( void );

void update_set_changes_flag ( void );	// File changed + update titlebar

void update_changes_chores (
	int		new_geometry,	// Check for changed geometry
	int		block_sheet_recalcs // Never allow the sheet to be
					// recalculated
	);				// Done after any action that changes
					// the file in memory

void sheet_selector_populate ( void );

void disable_sheet_selector_cb ( void );

void enable_sheet_selector_cb ( void );

int check_for_changes ( void );
	// -10 = NOT CHECKED
	//   1 = STOP
	//   2 = IGNORE
	//  10 = ESCAPE

void grab_focus_sheet ( void );

void grab_focus_entry ( void );

void clear_all ( void );		// Remove file, set up empty book

void set_cursor_range (
	int		r1,
	int		c1,
	int		r2,
	int		c2,
	int		cursor_visible,
	int		force_update
	);

void pressed_recalculate (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_recalculate_book (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void recalc_book_core ( void );		// Update current book (do nothing else)

void recalc_sheet_core ( void );	// Update current sheet (do nothing
					// else)

gboolean view_focus_out (
	GtkWidget	* widget,
	GdkEventFocus	* event,
	gpointer	user_data
	);

gboolean view_focus_in (
	GtkWidget	* widget,
	GdkEventFocus	* event,
	gpointer	user_data
	);

gboolean cellref_entry_focus_in (
	GtkWidget	* widget,
	GdkEventFocus	* event,
	gpointer	user_data
	);

gboolean cellref_entry_focus_out (
	GtkWidget	* widget,
	GdkEventFocus	* event,
	gpointer	user_data
	);

gboolean celltext_entry_focus_in (
	GtkWidget	* widget,
	GdkEventFocus	* event,
	gpointer	user_data
	);

gboolean celltext_entry_focus_out (
	GtkWidget	* widget,
	GdkEventFocus	* event,
	gpointer	user_data
	);

void pressed_undo (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_redo (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void update_splits ( void );		// Update splits/scrollbars

int undo_report_updates (		// Reports to the user the results of
					// cuf_* (on error)
	int		error
	);

void pref_init_row_pad ( void );

void pressed_bold (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_change_color (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

void pressed_border_type (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

void pressed_preferences (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_edit_cell_prefs (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_set_fullscreen (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

gboolean key_event (
	GtkWidget	* widget,
	GdkEventKey	* event,
	gpointer	user_data
	);

gboolean scroll_wheel (
	GtkWidget	* widget,
	GdkEventScroll	* event,
	CedView		* view
	);

gboolean button_press_canvas (
	GtkWidget	* widget,
	GdkEventButton	* event,
	gpointer	data
	);

gboolean motion_notify_canvas (
	GtkWidget	* widget,
	GdkEventMotion	* event,
	gpointer	data
	);

gboolean button_press_header (
	GtkWidget	* widget,
	GdkEventButton	* event,
	gpointer	data
	);

gboolean motion_notify_header (
	GtkWidget	* widget,
	GdkEventMotion	* event,
	gpointer	data
	);

gboolean celltext_entry_key_event (
	GtkWidget	* widget,
	GdkEventKey	* event,
	gpointer	user_data
	);

gboolean cellref_entry_activate (
	GtkWidget	* widget,
	GdkEventKey	* event,
	gpointer	user_data
	);

gboolean key_event_escape (
	GtkWidget	* widget,
	GdkEventKey	* event,
	gpointer	user_data
	);

gboolean cellref_entry_key_event (
	GtkWidget	* widget,
	GdkEventKey	* event,
	gpointer	user_data
	);

void quicksum_bmenu_changed (
	GtkWidget	* widget,
	void		* user_data,
	int		new_value
	);

void pressed_copy (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_paste (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_cut (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_copy_values (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_copy_output (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_paste_content (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_paste_prefs (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_clear (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_clear_contents (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_clear_prefs (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_row_insert_paste_height (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_column_insert_paste_width (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_transform_clipboard (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

void find_gui_build (
	GtkWidget	* vbox
	);

void pressed_find (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_find_close (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void find_gui_save_settings ( void );

int pane_pair_set (
	int		action		// PANE_PAIR_*
	);

void pressed_sort (
	GtkMenuItem	* menu_item,
	gpointer	user_data,
	gint		item
	);

void graph_gui_build (
	GtkWidget	* vbox,
	GtkWidget	* hb,
	GtkItemFactory	* itemfactory
	);

void graph_gui_update (			// Force update of graph list bmenu
	char	const	* graph_name	// Select this graph, NULL = none
	);

void graph_gui_store_changes ( void );	// Store any changes to current graph
					// text

void pressed_graph (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void pressed_graph_redraw (
	GtkMenuItem	* menu_item,
	gpointer	user_data
	);

void show_hide_graphname ( void );

