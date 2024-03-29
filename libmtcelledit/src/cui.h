/*
	Copyright (C) 2010-2021 Mark Tyler

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

#ifndef CUI_H_
#define CUI_H_



#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <pango/pangocairo.h>

#include <cairo-pdf.h>
#if (CAIRO_VERSION_MAJOR > 1) || ((CAIRO_VERSION_MAJOR == 1) && (CAIRO_VERSION_MINOR >= 6))
	#define USE_CAIRO_EPS
#endif

#include <cairo-ps.h>
#include <cairo-svg.h>



#include "mtcedui.h"



#define CUI_CELL_MAX_PIXELS	5000
#define CUI_PANGO_SCALE		0.75



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif


#ifdef __cplusplus
extern "C" {
#endif

int cui_book_add_sheet (
	CuiBook		* ub,
	CedSheet	* new_sheet,
	char	const	* page
	);

int cui_book_destroy (			// Destroy book, undo, and the ubook
					// that holds them
	CuiBook		* ub
	);

CuiBook * cui_book_new (
	void
	);

int cui_sheet_paste_area (
	CuiBook		* ub,
	CedSheet	* sheet,
	CedSheet	* paste,
	int		row,
	int		column,
	int		rowtot,		// Must be >= paste_rowtot
	int		coltot,		// Must be >= paste_coltot
	int		paste_rowtot,
	int		paste_coltot,
	int		mode
	);

#ifdef __cplusplus
}
#endif



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// CUI_H_

