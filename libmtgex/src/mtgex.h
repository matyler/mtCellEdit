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

#ifndef MTGEX_H_
#define MTGEX_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"

#include <gtk/gtk.h>

#pragma GCC diagnostic pop

#include <mtkit.h>



typedef struct
{
	mtImage		* image
			;
	GtkWidget	* drawing_area,
			* scrolled_window // As returned from mtgex_image_new ()
			;
	int		zoom,
			own,
			margin_x,
			margin_y,
			background	// Background grey shade 0..255
			;
} gexImage;

typedef struct
{
	GtkWidget	* drawing_area,	// ->allocation.width &
					// ->allocation.height to get visible
			* vscroll,
			* hscroll,
			* table;	// The widget that holds this structure
	GtkObject	* adj_hori,
			* adj_vert;	// Adjustments for 2 scrollbars
	int		x,
			y,		// Current visible origin
			w,		// User defined width/height of the
			h;		// drawing area
} gexSdraw;



typedef void (* gexCBmenu) (
	GtkWidget	* widget,
	void		* user_data,
	int		new_value
	);



#define MTGEX_FPICK_ENTRY	1
#define MTGEX_FPICK_LOAD	2
#define MTGEX_FPICK_DIRS_ONLY	4



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

char * mtgex_gtkuncpy (			// Wrapper for C->utf8 translation
	char		* dest,		// Buffer, or NULL => dynamic creation
	char	const	* src,
	size_t		cnt		// Bytes in dest buffer
	);

int mtgex_alert_box (
	char	const	* title,
	char	const	* message,
	char	const	* text1,
	char	const	* text2,
	char	const	* text3,
	GtkWidget	* main_window	// NULL = no main window to be transient
	);				// over
	//  1 = text1
	//  2 = text2
	//  3 = text3
	// 10 = WM closure

void mtgex_store_window_position (	// Store window position
	GtkWidget	* window,
	mtPrefs		* prefs,	// Must contain the 4 prefixed keys
	char	const	* key_prefix	// Prefix for _x, _y, _w, _h
	);				// (max 120 chars)

void mtgex_restore_window_position (	// Restore window position
	GtkWidget	* window,
	mtPrefs		* prefs,
	char	const	* key_prefix
	);

GtkWidget * mtgex_add_a_window (
	GtkWindowType	type,
	char	const	* title,
	GtkWindowPosition pos,
	gboolean	modal
	);

GtkWidget * mtgex_add_to_table (
	char	const	* text,
	GtkWidget	* table,
	int		row,
	int		column,
	int		hspacing,
	int		vspacing
	);

void mtgex_add_hseparator (
	GtkWidget	* widget,
	int		xs,
	int		ys
	);

// Most common use of boxes

GtkWidget * mtgex_pack (
	GtkWidget	* box,
	GtkWidget	* widget
	);

// Workaround for GtkCList reordering bug
// From mtPaint and the work of Dmitry Groshev

/* This bug is the favorite pet of GNOME developer Behdad Esfahbod
 * See http://bugzilla.gnome.org/show_bug.cgi?id=400249#c2 */

void mtgex_clist_enable_drag (
	GtkWidget	* clist
	);

GtkWidget * mtgex_fpick_new (
	char	const	* title,
	int		flags,
	GtkWidget	* main_window,
	mtPrefs		* prefs,
	char	const	* prefs_prefix	// Up to 64 chars. NULL = no prefix
	);

int mtgex_fpick_init_prefs (		// Initialize default prefs for file
					// picker window
	mtPrefs		* prefs,	// Call this before mtkit_prefs_load
	char	const	* prefs_prefix
	);

/*
These entries are required in the prefs table (prefix before each key) if you
don't use mtgex_fpick_init_prefs:
(if you don't you will forever live with defaults!)

{ "fpick_case_insensitive", MTKIT_PREF_TYPE_INT, "1" },
{ "fpick_show_hidden",	MTKIT_PREF_TYPE_INT, "0" },

{ "fpick_window_x",	MTKIT_PREF_TYPE_INT, "0" },
{ "fpick_window_y",	MTKIT_PREF_TYPE_INT, "0" },
{ "fpick_window_w",	MTKIT_PREF_TYPE_INT, "600" },
{ "fpick_window_h",	MTKIT_PREF_TYPE_INT, "500" },

{ "fpick_dir_1",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_2",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_3",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_4",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_5",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_6",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_7",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_8",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_9",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_10",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_11",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_12",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_13",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_14",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_15",	MTKIT_PREF_TYPE_DIR },
{ "fpick_dir_16",	MTKIT_PREF_TYPE_DIR },

{ "fpick_col1",		MTKIT_PREF_TYPE_INT, "250" },
{ "fpick_col2",		MTKIT_PREF_TYPE_INT, "64" },
{ "fpick_col3",		MTKIT_PREF_TYPE_INT, "80" },
{ "fpick_col4",		MTKIT_PREF_TYPE_INT, "150" },

*/

void mtgex_fpick_destroy (
	GtkWidget	* fp
	);

void mtgex_fpick_setup (
	GtkWidget	* fp,
	GtkWidget	* xtra,
	GtkSignalFunc	ok_fn,
	GtkSignalFunc	cancel_fn
	);

char const * mtgex_fpick_get_filename (
	GtkWidget	* fp,
	int		raw
	);

void mtgex_fpick_set_filename (
	GtkWidget	* fp,
	char	const	* name,
	int		raw
	);

int mtgex_prefs_window (
	mtPrefs		* prefs,
	GtkWidget	* main_window,	// Transient
	char	const	* title,
	mtPrefCB	closure_cb,	// Called when window is closed
	mtPrefValue	* closure_piv,
	int		closure_data
	);

int mtgex_prefs_init_prefs (		// Initialize default prefs for
					// preferences window
	mtPrefs		* prefs		// Call this before mtkit_prefs_load
	);

int mtgex_prefs_window_mirror_prefs (	// Copy prefs window settings: prefs.*
	mtPrefs		* dest,
	mtPrefs		* src
	);

/*
You must set these up in the static lists somewhere if you don't use
mtgex_prefs_init_prefs:
(if you don't you will forever live with defaults!)
NOTE: These do not appear in the editable list.

{ "prefs.col1",		MTKIT_PREF_TYPE_INT, "150" },
{ "prefs.col2",		MTKIT_PREF_TYPE_INT, "50" },
{ "prefs.col3",		MTKIT_PREF_TYPE_INT, "50" },
{ "prefs.col4",		MTKIT_PREF_TYPE_INT, "50" },

{ "prefs.window_x",	MTKIT_PREF_TYPE_INT, "259" },
{ "prefs.window_y",	MTKIT_PREF_TYPE_INT, "259" },
{ "prefs.window_w",	MTKIT_PREF_TYPE_INT, "693" },
{ "prefs.window_h",	MTKIT_PREF_TYPE_INT, "651" },

{ "prefs.file.window_x",	MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.file.window_y",	MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.file.window_w",	MTKIT_PREF_TYPE_INT, "600" },
{ "prefs.file.window_h",	MTKIT_PREF_TYPE_INT, "500" },

{ "prefs.color.window_x",	MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.color.window_y",	MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.color.window_w",	MTKIT_PREF_TYPE_INT, "100" },
{ "prefs.color.window_h",	MTKIT_PREF_TYPE_INT, "100" },

{ "prefs.edit.window_x",	MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.edit.window_y",	MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.edit.window_w",	MTKIT_PREF_TYPE_INT, "400" },
{ "prefs.edit.window_h",	MTKIT_PREF_TYPE_INT, "100" },

{ "prefs.cpick_pal_1",	MTKIT_PREF_TYPE_RGB, "0" },
{ "prefs.cpick_pal_2",	MTKIT_PREF_TYPE_RGB, "16711680" },
{ "prefs.cpick_pal_3",	MTKIT_PREF_TYPE_RGB, "16776960" },
{ "prefs.cpick_pal_4",	MTKIT_PREF_TYPE_RGB, "65280" },
{ "prefs.cpick_pal_5",	MTKIT_PREF_TYPE_RGB, "255" },
{ "prefs.cpick_pal_6",	MTKIT_PREF_TYPE_RGB, "16711935" },
{ "prefs.cpick_pal_7",	MTKIT_PREF_TYPE_RGB, "65535" },
{ "prefs.cpick_pal_8",	MTKIT_PREF_TYPE_RGB, "16777215" },
{ "prefs.cpick_pal_9",	MTKIT_PREF_TYPE_RGB, "14408667" },
{ "prefs.cpick_pal_10",	MTKIT_PREF_TYPE_RGB, "11974326" },
{ "prefs.cpick_pal_11",	MTKIT_PREF_TYPE_RGB, "9605778" },
{ "prefs.cpick_pal_12",	MTKIT_PREF_TYPE_RGB, "7171437" },
{ "prefs.cpick_pal_13",	MTKIT_PREF_TYPE_RGB, "4802889" },
{ "prefs.cpick_pal_14",	MTKIT_PREF_TYPE_RGB, "2368548" },
{ "prefs.cpick_pal_15",	MTKIT_PREF_TYPE_RGB, "14389760" },
{ "prefs.cpick_pal_16",	MTKIT_PREF_TYPE_RGB, "7190089" },

{ "prefs.fpick_dir_1",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_2",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_3",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_4",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_5",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_6",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_7",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_8",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_9",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_10",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_11",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_12",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_13",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_14",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_15",	MTKIT_PREF_TYPE_DIR },
{ "prefs.fpick_dir_16",	MTKIT_PREF_TYPE_DIR },

{ "prefs.fpick_case_insensitive", MTKIT_PREF_TYPE_INT, "1" },
{ "prefs.fpick_show_hidden", MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.fpick_col1",	MTKIT_PREF_TYPE_INT, "250" },
{ "prefs.fpick_col2",	MTKIT_PREF_TYPE_INT, "64" },
{ "prefs.fpick_col3",	MTKIT_PREF_TYPE_INT, "80" },
{ "prefs.fpick_col4",	MTKIT_PREF_TYPE_INT, "150" },

*/

GtkWidget * mtgex_image_new (		// Set up new scrollable/zoomable image
	void
	);

int mtgex_image_set_image (
	GtkWidget	* widget,
	mtImage		* image,	// NULL = No image
	int		own		// 1 = destroy this image when widget
					// dies or image changes
	);

int mtgex_image_get_geximage (
	GtkWidget	* widget,
	gexImage	** geximage	// Put image here
	);

void mtgex_fix_darea (			// Stop jerking and flickering on a
					// drawing area
	GtkWidget	* darea
	);

GtkWidget * mtgex_bmenu_new (		// Set up a new button menu
	void
	);

int mtgex_bmenu_add_item (		// Add an item to the list
	GtkWidget	* widget,
	char	const	* item		// UTF-8 text.  NULL = clear the list
	);
	// -1 = error, else return new item index
	// if item = NULL then 0 = success

int mtgex_bmenu_set_value (		// Set the current button menu item
	GtkWidget	* widget,
	int		value
	);

int mtgex_bmenu_get_value (		// Return the item number that is
					// currently selected
	GtkWidget	* widget
	);
	// >= 0 : success
	//  < 0 : error

int mtgex_bmenu_get_value_from_text (	// Get the item number from the text in
					// it
	GtkWidget	* widget,
	char	const	* text		// Match this and select the item
	);
	// -1   = fail
	// >= 0 = success

char * mtgex_bmenu_get_value_text (	// Get the text for a given menu item
	GtkWidget	* widget,
	int		item
	);

int mtgex_bmenu_set_callback (		// Set the current button menu callback
					// for value changes
	GtkWidget	* widget,
	gexCBmenu	callback,	// Called when value changes
	void		* user_data
	);



#endif		// MTGEX_H_

