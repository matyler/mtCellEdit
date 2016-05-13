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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>

#include "mtgex.h"

#include <gdk/gdkkeysyms.h>



#define PATHBUF		2048



#ifdef U_NLS
	#include <libintl.h>
	#define _(text) gettext(text)
	#define __(text) gettext(text)
#else
	#define _(text) text
	#define __(text) text
#endif



// Stops -pedantic warnings
#define GTKBUG __extension__



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif


GtkWidget * mtgex_add_a_button (
	char	const	* text,
	int		bord,
	GtkWidget	* box,
	gboolean	filler
	);

GtkWidget * mtgex_add_a_spin (
	int		value,
	int		min,
	int		max
	);

void mtgex_destroy_dialog (		// Properly destroy transient window
	GtkWidget	* window
	);

GtkWidget * mtgex_pack_end (
	GtkWidget	* box,
	GtkWidget	* widget
	);

GtkWidget * mtgex_xpack5 (
	GtkWidget	* box,
	GtkWidget	* widget
	);



// NOTE - only one colour picker can be used at a time by the app

int mtgex_cpick_get_color (
	GtkWidget	* w,
	int		* opacity
	);

GtkWidget * mtgex_cpick_new (
	GtkWidget	* main_window,
	mtPrefs		* prefs,
	char	const	* prefs_prefix	// Up to 64 chars. NULL = no prefix
	);

void mtgex_cpick_set_color (
	GtkWidget	* w,
	int		rgb,
	int		opacity
	);

void mtgex_cpick_set_color_previous (
	GtkWidget	* w,
	int		rgb,
	int		opacity
	);

void mtgex_cpick_set_opacity_visibility (
	GtkWidget	* w,
	int		visible
	);



#ifndef DEBUG
#pragma GCC visibility pop
#endif

