/*
	Copyright (C) 2011-2016 Mark Tyler

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

#ifndef MTCEDUI_H_
#define MTCEDUI_H_



#include <mtcelledit.h>
#include <mtpixy.h>



typedef struct CuiRenPage	CuiRenPage;
typedef struct CuiRender	CuiRender;
typedef struct CuiClip		CuiClip;
typedef struct CuiFile		CuiFile;
typedef struct CuiBook		CuiBook;
typedef struct CuiUndo		CuiUndo;
typedef struct CuiUndoStep	CuiUndoStep;



enum
{
	CUI_FILE_LOCK_RW	= 0,	// Re-Writable
	CUI_FILE_LOCK_RWL	= 1,	// Re-Writable & Locked
	CUI_FILE_LOCK_RO	= 2	// Read Only
};

enum
{
	CUI_SHEET_EXPORT_EPS,
	CUI_SHEET_EXPORT_HTML,
	CUI_SHEET_EXPORT_PDF,
	CUI_SHEET_EXPORT_PDF_PAGED,
	CUI_SHEET_EXPORT_PNG,
	CUI_SHEET_EXPORT_PS,
	CUI_SHEET_EXPORT_SVG,
	CUI_SHEET_EXPORT_TSV,
	CUI_SHEET_EXPORT_TSV_QUOTED,

	CUI_SHEET_EXPORT_TOTAL
};

// Unless otherwise stated undo functions return these values:
enum
{
	CUI_ERROR_NONE		= 0,	// No errors, nothing to report

	CUI_ERROR_NO_CHANGES	= -1,	// The sheet/book was not changed
	CUI_ERROR_UNDO_OP	= -2,	// Error in undo system, operation not
					// completed
	CUI_ERROR_UNDO_LOST	= -3,	// Undo history lost, but operation
					// completed
	CUI_ERROR_CHANGES	= -4,	// The sheet/book may have changed
	CUI_ERROR_LOCKED_CELL	= -5,	// This operation was blocked as a cell
					// was locked
	CUI_ERROR_LOCKED_SHEET	= -6	// This operation was blocked as a
					// sheet was locked
};

enum
{
	CUI_GRAPH_TYPE_EPS,
	CUI_GRAPH_TYPE_PDF,
	CUI_GRAPH_TYPE_PNG,
	CUI_GRAPH_TYPE_PS,
	CUI_GRAPH_TYPE_SVG,

	CUI_GRAPH_TYPE_TOTAL
};

enum
{
	CUI_CELLPREFS_align_horizontal = 0,
	CUI_CELLPREFS_color_background,
	CUI_CELLPREFS_color_foreground,
	CUI_CELLPREFS_format,
	CUI_CELLPREFS_width,
	CUI_CELLPREFS_num_decimal_places,
	CUI_CELLPREFS_num_zeros,
	CUI_CELLPREFS_text_style,
	CUI_CELLPREFS_locked,
	CUI_CELLPREFS_border_type,
	CUI_CELLPREFS_border_color,
	CUI_CELLPREFS_format_datetime,
	CUI_CELLPREFS_num_thousands,
	CUI_CELLPREFS_text_prefix,
	CUI_CELLPREFS_text_suffix,

	CUI_CELLPREFS_TOTAL
};

enum
{
	CUI_CELLBORD_NONE =		-1,
	CUI_CELLBORD_OUT_THIN =		-2,
	CUI_CELLBORD_OUT_THICK =	-3,
	CUI_CELLBORD_OUT_DOUBLE =	-4,
	CUI_CELLBORD_TB_THIN =		-5,
	CUI_CELLBORD_TB_THICK =		-6,
	CUI_CELLBORD_TB_DOUBLE =	-7,

	CUI_CELLBORD_H_CLEAR_TOP =	0,
	CUI_CELLBORD_H_CLEAR_MID =	1,
	CUI_CELLBORD_H_CLEAR_BOT =	2,
	CUI_CELLBORD_H_THIN_TOP =	3,
	CUI_CELLBORD_H_THIN_MID =	4,
	CUI_CELLBORD_H_THIN_BOT =	5,
	CUI_CELLBORD_H_THICK_TOP =	6,
	CUI_CELLBORD_H_THICK_MID =	7,
	CUI_CELLBORD_H_THICK_BOT =	8,
	CUI_CELLBORD_H_DOUB_TOP =	9,
	CUI_CELLBORD_H_DOUB_MID =	10,
	CUI_CELLBORD_H_DOUB_BOT =	11,

	CUI_CELLBORD_V_CLEAR_L =	12,
	CUI_CELLBORD_V_CLEAR_C =	13,
	CUI_CELLBORD_V_CLEAR_R =	14,
	CUI_CELLBORD_V_THIN_L =		15,
	CUI_CELLBORD_V_THIN_C =		16,
	CUI_CELLBORD_V_THIN_R =		17,
	CUI_CELLBORD_V_THICK_L =	18,
	CUI_CELLBORD_V_THICK_C =	19,
	CUI_CELLBORD_V_THICK_R =	20,
	CUI_CELLBORD_V_DOUB_L =		21,
	CUI_CELLBORD_V_DOUB_C =		22,
	CUI_CELLBORD_V_DOUB_R =		23,

	CUI_CELLBORD_MIN =		-7,
	CUI_CELLBORD_MAX =		23
};



#define CUI_DEFAULT_CELLWIDTH_CHARS	10
#define CUI_ROWHEIGHT( REN )		(REN->font->get_height() + 2 * REN->row_pad)
#define CUI_DEFAULT_CELLWIDTH( REN )	(CUI_DEFAULT_CELLWIDTH_CHARS * REN->font_width)

#define CUI_INIFILE_PAGE_MARGIN_X	"page_margin_x"
#define CUI_INIFILE_PAGE_MARGIN_Y	"page_margin_y"
#define CUI_INIFILE_PAGE_FOOTER_LEFT	"page_footer_left"
#define CUI_INIFILE_PAGE_FOOTER_CENTRE	"page_footer_centre"
#define CUI_INIFILE_PAGE_FOOTER_RIGHT	"page_footer_right"
#define CUI_INIFILE_PAGE_HEADER_LEFT	"page_header_left"
#define CUI_INIFILE_PAGE_HEADER_CENTRE	"page_header_centre"
#define CUI_INIFILE_PAGE_HEADER_RIGHT	"page_header_right"
#define CUI_INIFILE_PAGE_WIDTH		"page_mm_width"
#define CUI_INIFILE_PAGE_HEIGHT		"page_mm_height"

#define CUI_HEADER_OPTIONS		"None	Filename (long)	Filename (short)	Sheet Name	Page #	Date	Date and Time"

// Default number of max undo/redo steps
#define CUI_DEFAULT_MAX_STEPS		100

// Sanity limits for the size of sheets when saving TSV/CSV

#define CUI_CLIPBOARD_TIMESTAMP_SIZE	32

#define CUI_SHEET_MAX_ROW		1000000
#define CUI_SHEET_MAX_COL		1000000
#define CUI_SHEET_MAX_AREA		1000000000
#define CUI_SHEET_MAX_NAME		10000

#define CUI_CLIPBOARD_SHEET_NAME	"clipboard"

#define CUI_GRAPH_NAME_PREFIX		"graph/"
#define CUI_GRAPH_NAME_PREFIX_LEN	6



struct CuiRenPage
{
	int		width;
	int		height;
	int		margin_x;
	int		margin_y;
	int		footer[3];
	int		header[3];
};

struct CuiRender
{
	CedSheet	* sheet;	// Sheet currently being edited

	mtPixy::Font	* font;
	int		font_width;	// Average width of font for numbers,
					// i.e. "01234567890"
	int		row_header_width; // Used to cetralize the
					// rendering of row numbers
	int		row_pad;	// Pixels above and below the row
};

struct CuiClip
{
	CedSheet	* sheet;	// Current clipboard
	char		* ced;		// Dynamic description for clip from/to
					// other instance
	char		* tsv;		// TSV data created for others
	char		timestamp[CUI_CLIPBOARD_TIMESTAMP_SIZE];
	int		temp;		// Flag 1 = temp file exists
	int		rows;		// Size of clipboard - needed in case of
					// blank.
	int		cols;		// trailing rows/cols.
};

struct CuiFile
{
	CuiBook		* cubook;
	char		* name;		// OS filename loaded/saved
	int		type;
	int		lock_state;	// CUI_FILE_LOCK_*
	mode_t		perm_mode;	// File permissions if
					// lock_state == CUI_FILE_PERM_RWL
};

struct CuiUndo
{
	int		max;		// Maximum allowable undo/redo steps
	int		redo_tot;	// Total steps currently stored
	int		undo_tot;	// Total steps currently stored
	CuiUndoStep	* redo_step;	// Next redo step
	CuiUndoStep	* undo_step;	// Next undo step
};

struct CuiBook
{
	CedBook		* book;
	CuiUndo		undo;
};



typedef void (* CuiRenCB) (
	int		x,
	int		y,
	int		w,
	int		h,
	unsigned char	* rgb,
	void		* user_data
	);

typedef int (* CuiGraphScan) (
	CedBook		* book,		// Don't add/remove any graph/file from
					// the book during this callback
	char	const	* graph_name,
	CedBookFile	* bookfile,
	void		* user_data
	);
	// 0 = continue
	// 1 = stop



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

/*
	GEOMETRY FUNCTIONS
*/

int cui_ren_x_from_column (
	int		col_start,
	CuiRender	* viewren,
	int		column
	);
	// X

int cui_ren_y_from_row (
	int		row_start,
	CuiRender	* viewren,
	int		row
	);
	// Y

int cui_ren_column_from_x_backwards (
	int		col_start,
	CuiRender	* viewren,
	int		x
	);
	// Column

int cui_ren_column_from_x (
	int		col_start,
	CuiRender	* viewren,
	int		x
	);
	// Column

int cui_ren_row_from_y (
	int		row_start,
	CuiRender	* viewren,
	int		y
	);
	// Row

/*
	PAGE EXPORT FUNCTIONS
*/

int cui_export_output (
	mtPrefs		* prefs_file,
	CedSheet	* sheet,
	char	const	* filename,
	char	const	* gui_filename,
	int		filetype,	// CUI_SHEET_EXPORT_*
	int		row_pad,
	char	const	* font_name,
	int		font_size
	);

/*
	PIXEL EXPOSURE FUNCTIONS


	Before calling these *_expose_* functions 'view' must contain validated
	'sheet' and 'font' data.
*/

int cui_ren_expose_main (
	int		row_start,
	int		col_start,
	CuiRender	* viewren,
	int		x,
	int		y,
	int		w,
	int		h,
	CuiRenCB	callback,
	void		* callback_data
	);

int cui_ren_expose_row_header (
	int		row_start,
	CuiRender	* viewren,
	int		x,
	int		y,
	int		w,
	int		h,
	CuiRenCB	callback,
	void		* callback_data
	);

int cui_ren_expose_column_header (
	int		col_start,
	CuiRender	* viewren,
	int		x,
	int		y,
	int		w,
	int		h,
	CuiRenCB	callback,
	void		* callback_data
	);

/*
	BOOK FUNCTIONS
*/

int cui_book_undo_step (		// Undo the last step
	CuiBook		* ub
	);

int cui_book_redo_step (		// Redo the next step
	CuiBook		* ub
	);

int cui_check_sheet_lock (		// Is this sheet locked?
	CedSheet	* sheet
	);
	// 1 = locked

char const * cui_error_str (		// Get the error string to report to
					// user (when cells locked)
	void
	);

/*
	--------------------------------------------
	Undoable wrapper functions for libmtcelledit
	--------------------------------------------

All arguments are required to be valid on entry unless otherwise stated.
*/

int cui_book_destroy_sheet (
	CuiBook		* ub,
	char	const	* page
	);

int cui_book_page_rename (
	CuiBook		* ub,
	CedSheet	* a_sheet,
	char	const	* name
	);

int cui_book_merge (
	CuiBook		* ub,
	CedBook		* book,
	int		* sheet_tot,
	int		* sheet_fail,
	int		* file_tot,
	int		* file_fail
	);

int cui_sheet_set_cell (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		row,
	int		column,
	char	const	* text
	);

int cui_sheet_set_column_width (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		column,
	int		coltot,
	int		width
	);

int cui_sheet_set_column_width_auto (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		column,
	int		coltot
	);

int cui_sheet_sort_rows (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		row,
	int		rowtot,
	int	const	* cols,
	int		mode,
	int	const	* mode_list
	);

int cui_sheet_sort_columns (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		column,
	int		coltot,
	int	const	* rows,
	int		mode,
	int	const	* mode_list
	);

int cui_sheet_insert_row (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		row,
	int		rowtot
	);

int cui_sheet_delete_row (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		row,
	int		rowtot
	);

int cui_sheet_insert_column (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		col,
	int		coltot
	);

int cui_sheet_delete_column (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		col,
	int		coltot
	);

int cui_sheet_clear_area (
	CuiBook		* ub,
	CedSheet	* sheet,
	int		row,
	int		column,
	int		rowtot,
	int		coltot,
	int		mode
	);

int cui_book_duplicate_sheet (
	CuiBook		* cubook,
	CedSheet	* sheet,
	CedSheet	** new_sh	// Optional: new sheet
	);

/*
	FILE FUNCTIONS

The general idea is that a UI will have access to at least one file and
at least one clipboard.  Exotic implementations may have several of each.
*/

CuiFile * cui_file_new (		// Create new empty file state
	void
	);

int cui_file_free (			// Release this structure, and its
					// contents
	CuiFile		* file
	);

int cui_file_load (			// Load a sheet or book as a new CuBook
	CuiFile		* file,		// On success set filetype and real
					// filename in state
	char	const	* filename,	// Filename to load
	int		force		// CED_FILE_FORCE_*
	);
	// 0 = success
	// 1 = arg error
	// 2 = file error

int cui_file_save (
/*
	Save a sheet or book using a temp file to avoid destroying file data
	if a fail occurs mid-way. TSV/CSV sheets must be within limits
	outlined above.
*/
	CuiFile		* file,		// On success set filetype and
					// real filename in state
	char	const	* filename,
	int		filetype
	);
	// -1 = error
	//  0 = success
	//  1 = TSV/CSV sheet too large

int cui_file_set_lock (
	CuiFile		* file,
	int		new_lock	// CUI_FILE_LOCK_*
	);
	// 0 = Set as requested (or unchanged)
	// 1 = Unable to set as requested
	// NOTE: Cannot change current RO file to RW or RWL

int cui_file_sheet_add (		// Add a new sheet "Sheet 1", 2, 3, etc.
					// via undo system
	CuiFile		* file
	);
	// 0 = success
	// 1 = error creating sheet, else CUI_ERROR_*

int cui_file_book_new (			// Remove old book, filename, type, and
					// start afresh
	CuiFile		* file
	);

CedSheet * cui_file_get_sheet (
/*
	Get the currently active sheet (if there is one)
	If active name is dangling, it gets cleared.
	If active name is empty but sheets exist, root is chosen.
*/
	CuiFile		* file
	);
	// NULL = fail or no active sheet

CedBookFile * cui_file_get_graph (
/*
	Get the currently active graph (if there is one)
	If active name is dangling, it gets cleared.
	If active name is empty but graphs exist, 1st is chosen.
*/
	CuiFile		* file
	);
	// NULL = fail or no active graph

int cui_sheet_check_geometry (		// Is this sheet too large to save?
	CedSheet	* sheet
	);
	// 0 = ok to save
	// 1 = too large
	// 2 = error

int cui_sheet_2dyear (			// Change century of 2 digit years
	CuiFile		* file,
	int		year_start	// MTKIT_DDT_MIN_DATE_YEAR..
					// (MTKIT_DDT_MAX_DATE_YEAR - 99)
	);
	// 1 = Arg Error, else CUI_ERROR_*

/*
	CLIPBOARD FUNCTIONS
*/

CuiClip * cui_clip_new (		// Create new empty clipboard
	void
	);

int cui_clip_free (			// Release this structure, and its
					// contents
	CuiClip		* clipboard
	);

int cui_clip_flush (			// Empty clipboard contents
	CuiClip		* clipboard
	);

int cui_clip_import_text (		// Import text from another program via
					// TSV text block
	CuiClip		* clipboard,
	char		* text		// NUL terminated text to import as a
					// sheet
	);
	// 0 = success

int cui_clip_export_text (		// Create the file->tsv text for export
	CuiClip		* clipboard
	);

int cui_clip_save_temp (		// Save current clipboard to filesystem
	CuiClip		* clipboard
	);

int cui_clip_save_temp_file (		// Save current clipboard to filesystem
	CuiClip		* clipboard,
	char	const	* filename
	);

int cui_clip_load_temp (		// Load clipboard from filesystem
	CuiClip		* clipboard
	);
	// 0 = success

int cui_clip_load_temp_file (		// Load clipboard from filesystem
	CuiClip		* clipboard,
	char	const	* filename
	);

int cui_clip_set_timestamp (		// Set the timestamp
	CuiClip		* clipboard,
	char	const	* txt		// If NULL set to current time,
					// else use this NUL terminate string
	);

int cui_clip_paste (			// Paste the clipboard onto the current
					// cursor area
	CuiFile		* file,
	CuiClip		* clipboard,
	int		paste_mode
	);
	// 1 = arg error else CUI_ERROR_*

int cui_clip_copy (			// Copy current selection to the
					// clipboard
	CuiFile		* file,
	CuiClip		* clipboard
	);

/*
	GRAPH FUNCTIONS
*/

CedBookFile * cui_graph_new (
/*
	Create a new graph in this book (possibly destroying same named graph)
*/
	CedBook		* book,
	char		* mem,
	int		memsize,
	char	const	* graph_name
	);

CedBookFile * cui_graph_get (
	CedBook		* book,
	char	const	* graph_name
	);

int cui_graph_destroy (
	CedBook		* book,
	char	const	* graph_name
	);

int cui_graph_scan (			// Scan a book and use callback for each
					// graph found
	CedBook		* book,
	CuiGraphScan	callback,
	void		* user_data
	);
	// 0 = success
	// 1 = error
	// 2 = user termination

mtPixy::Image * cui_graph_render_image (
	CedBook		* book,
	char	const	* graph_name,
	int		* breakpoint,
/*
	Store point of error here (i.e. valid characters) (NULL = don't)
	-1 = Error during rendering (not parsing)
*/
	double		scale		// 1 = 100% 2 = 200% (10..1000%)
	);

int cui_graph_render_file (
	CedBook		* book,
	char	const	* graph_name,
	char	const	* filename,
	int		filetype,	// CUI_GRAPH_TYPE_*
	int		* breakpoint,
/*
	Store point of error here (i.e. valid characters) (NULL = don't)
	-1 = Error during rendering (not parsing)
*/
	double		scale		// 1 = 100% 2 = 200% (10..1000%)
					// Vector files use scale * 2.
	);

int cui_graph_duplicate (
	CuiBook		* cubook,
	char	const	* graph_name,
	char	*	* new_name	// Optional - put allocated string to
					// new name here
	);

/*
	CELLPREFS FUNCTIONS

All function returns: 0 = success, 1 = arg error, CUI_ERROR_*
*/

int cui_cellprefs_init (
	CedSheet	* sheet,
	int		* r1,
	int		* c1,
	int		* r2,
	int		* c2,
	CedSheet	** tmp_sheet	// Put temp sheet here
	);

int cui_cellprefs_change (
	CuiBook		* cubook,
	CedSheet	* sheet,
	int		r1,		// Must be same values as sent to
	int		c1,		// cedui_cellprefs_init ()
	int		r2,
	int		c2,
	CedSheet	* tmp_sheet,	// From cedui_cellprefs_init ()
	int		pref_id,	// Pref to change CUI_CELLPREF_*
	char	const	* pref_charp,	// Points to new value (if string)
	int		pref_int	// Used if pref_id requires it
	);
	// After final use, caller must destroy tmp_sheet

// The following functions don't require cedui_cellprefs_init ()

int cui_cellprefs_text_style (		// Toggles text style in selected cells
	CuiFile		* uifile,
	int		style		// CED_TEXT_STYLE_*
	);

int cui_cellprefs_border (		// Adds a border type to selected cells.
	CuiFile		* uifile,
	int		border_type	// CUI_CELLBORD_*
	);



#endif		// MTCEDUI_H_

