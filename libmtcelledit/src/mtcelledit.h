/*
	Copyright (C) 2009-2016 Mark Tyler

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

#ifndef MTCELLEDIT_H_
#define MTCELLEDIT_H_



#include <mtkit.h>



typedef struct CedBook		CedBook;
typedef struct CedBookFile	CedBookFile;
typedef struct CedBookPrefs	CedBookPrefs;
typedef struct CedCell		CedCell;
typedef struct CedCellPrefs	CedCellPrefs;
typedef struct CedCellRef	CedCellRef;
typedef struct CedCellStack	CedCellStack;
typedef struct CedFuncArg	CedFuncArg;
typedef struct CedFuncState	CedFuncState;
typedef struct CedIndex		CedIndex;
typedef struct CedIndexItem	CedIndexItem;
typedef struct CedParser	CedParser;
typedef struct CedSheet		CedSheet;
typedef struct CedSheetPrefs	CedSheetPrefs;
typedef struct CedToken		CedToken;



enum
{
	CED_CELL_TYPE_NONE,

	CED_CELL_TYPE_TEXT,		// Parsed as text
	CED_CELL_TYPE_VALUE,
	CED_CELL_TYPE_FORMULA,		// Formula that only requires
					// evaluation on creation
	CED_CELL_TYPE_FORMULA_EVAL,	// Formula requires evaluation on
					// recalculation
					// i.e. it has a cell ref. OR a volatile
					// func

	CED_CELL_TYPE_ERROR,		// Error in formula
	CED_CELL_TYPE_DATE,

	CED_CELL_TYPE_TEXT_EXPLICIT,	// User/file has used a ' quote here

	CED_CELL_TYPE_TOTAL

// NOTE - New types must be added to query_cell ()

};

enum
{
	CED_FILE_TYPE_NONE,

	CED_FILE_TYPE_TSV_CONTENT,	// Save formula content e.g. "= 2 + 2"
	CED_FILE_TYPE_TSV_CONTENT_GZ,
	CED_FILE_TYPE_TSV_CONTENT_NOQ,	// Save without ' on text cells
	CED_FILE_TYPE_TSV_VALUE,	// Save formula resulting values
					// e.g. "4"
	CED_FILE_TYPE_TSV_VALUE_GZ,
	CED_FILE_TYPE_TSV_VALUE_NOQ,	// Save without ' on text cells

	CED_FILE_TYPE_CSV_CONTENT,	// Save formula content e.g. "= 2 + 2",
	CED_FILE_TYPE_CSV_CONTENT_NOQ,	// Save without ' on text cells
	CED_FILE_TYPE_CSV_VALUE,	// Save formula resulting values
					// e.g. "4",
	CED_FILE_TYPE_CSV_VALUE_NOQ,	// Save without ' on text cells

	CED_FILE_TYPE_LEDGER,		// 20 byte header:
					// ledger\t==!"$CED\tTSV\n
	CED_FILE_TYPE_LEDGER_GZ,	// Format here is one cell per line:
					// <ROW><\t><COL><\t><TEXT><\n>
	CED_FILE_TYPE_LEDGER_VAL,	// Ledger format saving formulas as
					// values
	CED_FILE_TYPE_LEDGER_VAL_GZ,

//	-------------- See below *

	CED_FILE_TYPE_TSV_BOOK,		// Save sheets in book as content TSV's
	CED_FILE_TYPE_TSV_VAL_BOOK,	// Save sheets in book as content TSV's
					// plus values in extra sheets
	CED_FILE_TYPE_LEDGER_BOOK,	// Save sheets in book as ledger's
	CED_FILE_TYPE_LEDGER_VAL_BOOK,	// Save sheets in book as ledger's plus
					// values in extra sheets
//	-------------- See below *

	CED_FILE_TYPE_OUTPUT_TSV,	// Save cell output
	CED_FILE_TYPE_OUTPUT_TSV_QUOTED, // Save cell output with ' quote to
					// begin each cell
	CED_FILE_TYPE_OUTPUT_HTML,	// Save sheet to HTML file

	CED_FILE_TYPE_TOTAL
};
//	-------------- Useful demarcations
#define CED_FILE_TYPE_TEXT	CED_FILE_TYPE_LEDGER_VAL_GZ
#define CED_FILE_TYPE_BOOK	CED_FILE_TYPE_LEDGER_VAL_BOOK



enum	// ced_file_type_detect()
{
	// Return values
	CED_FILE_DETECT_ERROR	= -1,
	CED_FILE_DETECT_TSV	= 0,
	CED_FILE_DETECT_CSV	= 1,
	CED_FILE_DETECT_BOOK	= 2,

	// force argument values
	CED_FILE_FORCE_NONE	= 0,	// Deduce text file type via CSV/TSV
					// extension or content of file.
	CED_FILE_FORCE_TSV	= 1,
	CED_FILE_FORCE_CSV	= 2
};

enum
{
	CED_INDEX_TYPE_NONE		= 0,

	CED_INDEX_TYPE_VALUE,
	CED_INDEX_TYPE_TEXT,

	CED_INDEX_TYPE_TOTAL
};

enum
{
	CED_ARGSET_VARIABLE		= -1,

	CED_ARGSET_NONE			= 0,

	CED_ARGSET_VOID,
	CED_ARGSET_NUM,
	CED_ARGSET_NUM_NUM,
	CED_ARGSET_NUM_NUM_NUM,
	CED_ARGSET_CELLRANGE,
	CED_ARGSET_CELLRANGE_NUM,
	CED_ARGSET_NUM_CELLRANGE_NUM,
	CED_ARGSET_CELLREF_NUM_NUM,
	CED_ARGSET_CELLRANGE_STR_NUM,
	CED_ARGSET_CELLRANGE_STR_NUM_CELLREF,
	CED_ARGSET_CELLREF_CELLRANGE_NUM,

	CED_ARGSET_TOTAL
};

enum
{
	// Bit fields
	CED_TEXT_STYLE_BOLD		= 1,
	CED_TEXT_STYLE_ITALIC		= 2,
	CED_TEXT_STYLE_UNDERLINE_SINGLE	= 4,
	CED_TEXT_STYLE_UNDERLINE_DOUBLE	= 8,
	CED_TEXT_STYLE_UNDERLINE_WAVY	= 8 | 4,
	CED_TEXT_STYLE_STRIKETHROUGH	= 16,

	// Special values
	CED_TEXT_STYLE_UNDERLINE_ANY	= 8 | 4,
	CED_TEXT_STYLE_CLEAR		= ~0
};

#define CED_TEXT_STYLE_IS_BOLD(X)	((X) & CED_TEXT_STYLE_BOLD)
#define CED_TEXT_STYLE_IS_ITALIC(X)	((X) & CED_TEXT_STYLE_ITALIC)
#define CED_TEXT_STYLE_IS_UNDERLINE(X)	((X) & CED_TEXT_STYLE_UNDERLINE_ANY)
#define CED_TEXT_STYLE_IS_STRIKETHROUGH(X) ((X) & CED_TEXT_STYLE_STRIKETHROUGH)

enum
{
	CED_CELL_BORDER_TOP_SHIFT	= 0,
	CED_CELL_BORDER_MIDDLE_SHIFT	= 4,
	CED_CELL_BORDER_BOTTOM_SHIFT	= 8,
//	12 Reserved

	CED_CELL_BORDER_LEFT_SHIFT	= 16,
	CED_CELL_BORDER_CENTER_SHIFT	= 20,
	CED_CELL_BORDER_RIGHT_SHIFT	= 24,
//	28 Reserved

	CED_CELL_BORDER_MASK		= 15,	// After shift this masks low 4
						// bits as follows:

	CED_CELL_BORDER_NONE		= 0,
	CED_CELL_BORDER_THIN		= 1,
	CED_CELL_BORDER_THICK		= 2,
	CED_CELL_BORDER_DOUBLE		= 3
//	Bits for 4 & 8 reserved
};

enum
{
	CED_CELL_JUSTIFY_NONE,

	CED_CELL_JUSTIFY_LEFT,
	CED_CELL_JUSTIFY_CENTER,
	CED_CELL_JUSTIFY_RIGHT,

	CED_CELL_JUSTIFY_TOTAL
};


/*
The TSV file spec used here is that \r or \n separate rows and \t separates
columns.  In other words cells cannot contain those 3 characters.

*_NOQ formats can lose value of certain data, e.g. text "'4" becomes numerical
"4".  These formats should be used only as a last resort when other programs
don't accept anything else.

CSV files saved by this lib can be read in by Gnumeric/OOcalc.  OOcalc needs to
read in the file as UTF-8, comma separator, with all columns read in as
standard.
*/

enum			// errno values for parser state
{
	CED_ERROR_UNKNOWN,

	CED_ERROR_MEMORY_ALLOCATION		= 1,
	CED_ERROR_BAD_FUNCTION_NAME		= 2,
	CED_ERROR_BAD_FUNCTION_TOKEN		= 3,
	CED_ERROR_BAD_FUNCTION_ARGUMENTS	= 4,
	CED_ERROR_BAD_FUNCTION_OPERATION	= 5,
	CED_ERROR_BAD_NUMBER			= 6,
	CED_ERROR_BAD_SHEETREF			= 7,
	CED_ERROR_INFINITY			= 8,
	CED_ERROR_NAN				= 9,
	CED_ERROR_CELLREF			= 10,

	CED_ERROR_TOTAL
};

enum
{
	CED_FIND_MODE_NONE		= 0,

	CED_FIND_MODE_CASE		= 1<<0,	// Case sensitive
	CED_FIND_MODE_ALLCHARS		= 1<<1,	// Whole search string must be
						// whole cell string
	CED_FIND_MODE_WILDCARD		= 1<<2,
/*
Text to look for uses ? * \ special chars - as per mtkit_strmatch ().
CED_FIND_TEXT_MODE_ALLCHARS has no effect when using wildcards as the wildcard
matching string always relates to the whole text string in the cell.
*/

	CED_FIND_MODE_BACKWARDS		= 1<<3,	// Search from right to left,
						// bottom to top
	CED_FIND_MODE_IG_VAL		= 1<<4,	// Ignore value cells
	CED_FIND_MODE_IG_FORM		= 1<<5,	// Ignore formula cells
	CED_FIND_MODE_IG_ERROR		= 1<<6,	// Ignore error cells
	CED_FIND_MODE_IG_DATE		= 1<<7,	// Ignore date cells
	CED_FIND_MODE_IG_TEXT		= 1<<8,	// Ignore text cells
	CED_FIND_MODE_ALL_SHEETS	= 1<<9,	// Search all sheets in the book

	CED_FIND_MODE_ALL		= CED_FIND_MODE_CASE	|
					CED_FIND_MODE_ALLCHARS	|
					CED_FIND_MODE_WILDCARD	|
					CED_FIND_MODE_BACKWARDS	|
					CED_FIND_MODE_IG_VAL	|
					CED_FIND_MODE_IG_FORM	|
					CED_FIND_MODE_IG_ERROR	|
					CED_FIND_MODE_IG_DATE	|
					CED_FIND_MODE_IG_TEXT	|
					CED_FIND_MODE_ALL_SHEETS
};

enum
{
	CED_SORT_MODE_ASCENDING		= 0,

	CED_SORT_MODE_DESCENDING	= 1<<0,
	CED_SORT_MODE_CASE		= 1<<1,	// Case sensitive text sort

	CED_SORT_MODE_ALL		= CED_SORT_MODE_DESCENDING |
					CED_SORT_MODE_CASE
};

enum
{
	CED_PASTE_ACTIVE_CELLS		= 1<<0,	// Paste only active cells,
						// 0 = clear area before paste
	CED_PASTE_CONTENT		= 1<<1,	// Only paste content
						// (i.e. NOT prefs)
	CED_PASTE_PREFS			= 1<<2	// Only paste prefs
						// (i.e. NOT content)
};

enum
{
	CED_PARSER_FLAG_ERROR		= 1 << 0,
	CED_PARSER_FLAG_VOLATILE	= 1 << 1,

	CED_PARSER_FLAG_ALL		= ~0
};

enum
{
	CED_TOKEN_FLAG_VOLATILE		= 1 << 0,

	CED_TOKEN_FLAG_ALL		= ~0
};

enum
{
	CED_FARG_TYPE_NONE		= 0,

	CED_FARG_TYPE_NUM,
	CED_FARG_TYPE_CELLRANGE,
	CED_FARG_TYPE_CELLREF,
	CED_FARG_TYPE_STRING,

	CED_FARG_TYPE_TOTAL
};



/*
These are the limits for cell references in a sheet.  When passing a row/column
that is greater, an API function will generate an error.
*/

#define CED_MAX_ROW		1000000000
#define CED_MAX_COLUMN		1000000000

#define CED_MIN_COLUMN_WIDTH	0
#define CED_MAX_COLUMN_WIDTH	250

#define CED_PRINTF_NUM		"%.15g"
/*
CED_PRINTF_NUM was "%.14g" until version 1.4.0 when it changed to
"%.17g" in order to avoid a loss of precision under certain conditions. This
was changed to "%.15g" in version 2.2 to be more portable across different
systems.
*/


/*
	These are the names that appear in the CED book files
	and can be re-used in mtkit prefs structures.
*/

/*
	----------
	BOOK PREFS
	----------
*/

#define CED_FILE_PREFS_BOOK			"book_prefs"

//	STRING
#define CED_FILE_PREFS_BOOK_AUTHOR		"author"
#define CED_FILE_PREFS_BOOK_COMMENT		"comment"
#define CED_FILE_PREFS_BOOK_ACTIVE_SHEET	"active_sheet"
#define CED_FILE_PREFS_BOOK_ACTIVE_GRAPH	"active_graph"

//	INT
#define CED_FILE_PREFS_BOOK_DISABLE_LOCKS	"disable_locks"
#define CED_FILE_PREFS_BOOK_AUTO_RECALC		"auto_recalc"


/*
	-----------
	SHEET PREFS
	-----------
*/

#define CED_FILE_PREFS_SHEET			"sheet_prefs"
#define CED_FILE_PREFS_SHEET_NAME		"name"

//	INT
#define CED_FILE_PREFS_SHEET_CURSOR_R1		"cursor_r1"
#define CED_FILE_PREFS_SHEET_CURSOR_C1		"cursor_c1"
#define CED_FILE_PREFS_SHEET_CURSOR_R2		"cursor_r2"
#define CED_FILE_PREFS_SHEET_CURSOR_C2		"cursor_c2"
#define CED_FILE_PREFS_SHEET_SPLIT_R1		"split_r1"
#define CED_FILE_PREFS_SHEET_SPLIT_C1		"split_c1"
#define CED_FILE_PREFS_SHEET_SPLIT_R2		"split_r2"
#define CED_FILE_PREFS_SHEET_SPLIT_C2		"split_c2"
#define CED_FILE_PREFS_SHEET_START_ROW		"start_row"
#define CED_FILE_PREFS_SHEET_START_COL		"start_col"
#define CED_FILE_PREFS_SHEET_LOCKED		"locked"

/*
	----------
	CELL PREFS
	----------
*/

#define CED_FILE_PREFS_CELL			"cell"
#define CED_FILE_PREFS_CELL_REF			"ref"

//	INT
#define CED_FILE_PREFS_CELL_ALIGN_HORIZONTAL	"align_horizontal"
#define CED_FILE_PREFS_CELL_COLOR_BACKGROUND	"color_background"
#define CED_FILE_PREFS_CELL_COLOR_FOREGROUND	"color_foreground"
#define CED_FILE_PREFS_CELL_FORMAT		"format"
#define CED_FILE_PREFS_CELL_WIDTH		"width"
#define CED_FILE_PREFS_CELL_NUM_DECIMAL_PLACES	"num_decimal_places"
#define CED_FILE_PREFS_CELL_NUM_ZEROS		"num_zeros"
#define CED_FILE_PREFS_CELL_TEXT_STYLE		"bold_text"
#define CED_FILE_PREFS_CELL_LOCKED		"locked"
#define CED_FILE_PREFS_CELL_BORDER_TYPE		"border_type"
#define CED_FILE_PREFS_CELL_BORDER_COLOR	"border_color"

//	STRING
#define CED_FILE_PREFS_CELL_FORMAT_DATETIME	"format_datetime"
#define CED_FILE_PREFS_CELL_NUM_THOUSANDS	"num_thousands"
#define CED_FILE_PREFS_CELL_TEXT_PREFIX		"text_prefix"
#define CED_FILE_PREFS_CELL_TEXT_SUFFIX		"text_suffix"

#define CED_FUNC_ARG_MAX		10



typedef int (* CedFuncScanArea) (
	CedSheet	* sheet,
	CedCell		* cell,
	int		row,
	int		col,
	void		* user_data
	);
	// 0 = continue
	// 1 = stop

typedef int (* CedFuncBookScan) (
	CedSheet	* sheet,
	char	const	* name,		// Name of the sheet
	void		* user_data
	);
	// 0 = continue scan else stop returning that int

typedef int (* CedFuncBookMerge) (
	CedBook		* book_dest,	// Destination book
	CedBook		* book_insert,	// Source to be moved into book_dest
	void		* item,		// Pointer to sheet/bookfile
	int		type,		// 0 = sheet, 1 = file
	char	const	* name,		// Name of sheet or file
	int		already_exists,	// 1 = This name already exists in
					// book_dest
	void		* user_data
	);
	// 0 = move (possibly deleting current name)
	// 1 = don't move
	// 2 = stop merging

typedef int (* CedFuncCall) (		// Function callback
	CedFuncState	* funcs		// Function state
	);
	// 0 = success
	// 1 = fail



struct CedCellPrefs
{
	int	align_horizontal;	// 0..3 = None/Left/Centre/Right
	int	color_background;	// default = 255,255,255
	int	color_foreground;	// default = 0,0,0

	int	format;		/*	Display the cell content as:
					0 = General (deduce from cell->type)
					1 = Text
					2 = Fixed Decimal
					3 = Hex
					4 = Binary
					5 = Scientific
					6 = Percentage
					7 = Datetime
				*/

	int	width;			// row 0 : Column width in chars
					// (0 = default,10 chars,100 = max)
	int	num_decimal_places;
	int	num_zeros;		// Leading zeros
	int	text_style;		// CED_TEXT_STYLE_* bit field
	int	locked;			// Used by the GUI to block changes to
					// this cell
	int	border_type;		// Bits mapped as CED_CELL_BORDER_*
					// (0 = none = default)
	int	border_color;		// Colour of the borders (0 = default)

	char	* format_datetime;
	char	* num_thousands;
	char	* text_prefix;
	char	* text_suffix;
};

struct CedCell
{
	char		* text;		// Cell content as text
	double		value;		// Cell numerical value (or error
					// number)
	int		type;
	CedCellPrefs	* prefs;
};

struct CedSheetPrefs
{
	int		cursor_r1;	// Current active cell cursor
	int		cursor_c1;
	int		cursor_r2;	// If the same as rc1, only one cell
					// selected, else an area
	int		cursor_c2;

	int		split_r1;	// Minimum split row
	int		split_r2;	// Maxumum split row
	int		split_c1;	// Minimum split column
	int		split_c2;	// Maxumum split column

	int		start_row;	// The current scrollbar origin of
					// main area
	int		start_col;

	int		locked;		// 1 = prevent changes
};

struct CedSheet
{
	mtTree		* rows;		// key  = row number
					// data = mtTree of cols
	CedBook		* book;		// The book that owns this sheet
	mtTreeNode	* book_tnode;	// tnode in book->tree tnode->key.
					// tnode->key = sheet name

	CedSheetPrefs	prefs;
};

struct CedCellRef
{
	int		row_m;		// Current row multiplier
					// 0 = row_d is absolute
					// 1 = row_d is relative
	int		col_m;		// Current col multiplier
					// 0 = col_d is absolute
					// 1 = col_d is relative
	int		row_d;		// Row delta
	int		col_d;		// Col delta
};

struct CedCellStack
{
	CedCellStack	* next;

	int		row;
	int		col;
};

struct CedParser
{
	CedParser ();

///	------------------------------------------------------------------------

	unsigned int	flag;		// As enum CED_PARSER_FLAG_*
	int		ced_errno;	// CED error number
	int		sp;		// String pointer (char offset if error
					// occurs)

	double		data;		// Result
	CedSheet	* sheet;	// Current sheet
	CedCell		* cell;		// Current cell
	int		row,		// Current row in sheet
			column;		// Current column in sheet
};

struct CedIndexItem
{
	int		row;
	int		col;
};

struct CedIndex
{
	mtTree		* tree;		// Key  = (CedCell *)
					// Data = (CedIndexItem *)
	int		type;
};

struct CedBookPrefs
{
// All allocated C strings.  Change using:
//	mtkit_strfreedup ( &book->prefs.xxx, newtxt );

	char		* active_sheet;
	char		* active_graph;
	char		* author;
	char		* comment;

	int		disable_locks;	// 1 = disable all locks in this book
	int		auto_recalc;	// 1 = Recalc sheet/book when changes
					// occur
};

struct CedBookFile
{
	char		* mem;		// NULL = empty
	int		size;		// Number of bytes stored at 'mem'
	int		timestamp[6];	// year, month, day, hour, minute,
					// second
};

struct CedBook
{
	mtTree		* sheets;	// Key = C string (page name),
					// data = pointer to CedSheet

	mtTree		* files;	// Key = C string (file name),
					// data = pointer to CedBookFile

	CedBookPrefs	prefs;
};

struct CedFuncArg
{
	int		type;		// CED_FARG_TYPE_*

	CedSheet	* sheet;

	union
	{
	double		val;
	CedCellRef	cellref[2];	// [0] for cellref, both for cellrange
	char	const	* str;
	} u;
};

struct CedFuncState
{
	CedToken	* token;	// Token being actioned
	CedParser	* parser;	// Parser state
	double		* result;	// Populated by caller: Result of
					// function is put here
	CedFuncArg	* arg;		// Points to array of
					// [CED_FUNC_ARG_MAX + 1] or NULL for
					// no arguments
};

struct CedToken
{
	char		const * const	name;
	int			const	type;
	CedFuncCall		const	func;
	int			const	flag;
};



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

CedSheet * ced_sheet_new (		// Create a new empty sheet
	void
	);

int ced_sheet_destroy (			// Destroy a sheet
	CedSheet	* sheet
	);

CedSheet * ced_sheet_duplicate (
	CedSheet	* sheet
	);

CedSheet * ced_sheet_load (		// Load TSV from file
	char	const	* filename,
	char	const	* encoding,	// Force all input as utf8 if the file
					// is encoded as this
	int		* filetype	// Put filetype found here, NULL = don't
	);

CedSheet * ced_sheet_load_mem (		// Load TSV from memory
	char		* mem,		// NUL terminated
	size_t		bytes,
	char	const	* encoding,	// Force all input as utf8 if the file
					// is encoded as this
	int		* filetype	// Put filetype found here, NULL = don't
	);

CedSheet * ced_sheet_load_csv (		// Load CSV from file
	char	const	* filename,
	char	const	* encoding	// Force all input as utf8 if the file
					// is encoded as this
	);

CedSheet * ced_sheet_load_csv_mem (	// Load CSV from memory
	char	*	mem,		// NUL terminated
	size_t		bytes,
	char	const *	encoding	// Force all input as utf8 if the file
					// is encoded as this
	);

int ced_sheet_save (			// Save sheet to a file
	CedSheet	* sheet,
	char	const	* filename,
	int		filetype
	);

mtFile * ced_sheet_save_mem (		// Save sheet to a memory file
	CedSheet	* sheet,
	int		filetype
	);

CedCell * ced_sheet_get_cell (		// Get a cell from the spreadsheet
	CedSheet	* sheet,
	int		row,
	int		column
	);
	// NULL = no such cell exists

double ced_sheet_get_cell_value (	// Get a cell value from the spreadsheet
	CedSheet	* sheet,
	int		row,
	int		column
	);
	// If cell doesn't exist return 0.0

CedCell * ced_sheet_set_cell (		// Set cell text + parse/calculate
					// contents
	CedSheet	* sheet,
	int		row,		// 1..
	int		column,		// 1..
	char	const	* text		// If NULL, "", "'" delete the cell.
					// Must not contain \n \r \t
	);

CedCell * ced_sheet_set_cell_value (	// Set cell value (text set via printf)
	CedSheet	* sheet,
	int		row,		// 1..
	int		column,		// 1..
	double		value
	);

CedCell * ced_sheet_set_cell_text (	// Set cell text (type to
					// CED_CELL_TYPE_TEXT_EXPLICIT)
	CedSheet	* sheet,
	int		row,		// 1..
	int		column,		// 1..
	char	const	* text		// Must not contain \n \r \t
	);

CedSheetPrefs * ced_sheet_prefs_new ( void );

int ced_sheet_prefs_free (
	CedSheetPrefs	* prefs
	);

int ced_sheet_prefs_copy (		// Copy contents: dest = src
	CedSheetPrefs	* dest,
	CedSheetPrefs	* src
	);

int ced_sheet_set_cell_prefs (		// Set cell prefs - create empty new
					// cell if required
	CedSheet	* sheet,
	int		row,		// 1..
	int		column,		// 1..
	CedCellPrefs	* prefs,	// NULL = clear to default
	CedCell		** cellp	// Put cell pointer here on change,
					// NULL = don't
	);
	// 0 = success
	// 1 = fail
	// 2 = possible changes to sheet

int ced_sheet_set_column_width (	// Set column widths
	CedSheet	* sheet,
	int		column,		// 1..
	int		coltot,		// 1.. Number of columns to set
	int		width		// New width setting
	);

int ced_sheet_get_column_width_list (	// Collect maximum column widths into an
					// array
	CedSheet	* sheet,
	int		col,		// First column
	int		coltot,		// Total columns
	int		** width_list	// Put array of widths here
	);

int ced_sheet_set_column_width_list (
	CedSheet	* sheet,
	int		col,		// First column
	int		coltot,		// Total columns
	int		* width_list
	);

CedCellPrefs const * ced_cell_prefs_default (
	void
	);
	// Return a pointer to the default cell prefs

int ced_sheet_recalculate (		// Do a complete recalculation of all
					// cell types CED_CELL_TYPE_FORMULA_EVAL
	CedSheet	* sheet,
	int		* updates,	// Number of cell values that changed,
					// NULL = don't
	int		mode		// 0 = Top left to bottom right
					// 1 = Bottom right to top left
	);

int ced_sheet_get_geometry (		// Get the maximum row/column indexes
	CedSheet	* sheet,
	int		* row_max,	// NULL = Don't get
	int		* col_max	// NULL = Don't get
	);

int ced_sheet_tsvmem_geometry (		// Get the geometry of a TSV chunk of
					// memory
	char	const	* mem,		// MUST be zero terminated
	size_t		bytes,		// Bytes used including zero terminator
					// in final byte of mem
	int		* rows,		// Result, NULL = don't use
	int		* cols		// Result, NULL = don't use
	);

char const * ced_file_type_text (	// Return a text representation of a
					// file type
	int		filetype
	);

int ced_file_type_class (
	int		type		// CED_FILE_TYPE_*
	);
	// 0 = Invalid type
	// 1 = Text
	// 2 = Book
	// 3 = Output

int ced_file_type_detect (		// Detect general file type
	char	const	* filename,
	int		force		// CED_FILE_FORCE_*
	);
	// CED_FILE_DETECT_*

int ced_strtocellref (			// Covert a string to a cell reference
	char	const	* input,	// Input string to parse NUL terminated
	CedCellRef	* result,	// Put result here
	char	const	** next,	// Next un-parsed character is put here
					// NULL = don't use
	int		strict		// 1 = Force whole string to be a cell
					// reference (except whitespaces)
	);

int ced_cellreftostr (			// Covert a cell reference to a string
	char		* output,	// Output buffer - at least 128 bytes
	CedCellRef	* cellref	// Cell reference
	);

int ced_strtocellrange (		// Covert a string to a cell range
	char	const	* input,	// Input string to parse NUL terminated
	CedCellRef	* r1,		// Put first ref here
	CedCellRef	* r2,		// Put second ref here
	char	const	** next,	// Next un-parsed character is put here,
					// NULL = don't use
	int		strict		// 1 = Force whole string to be a cell
					// reference (except whitespaces)
	);

CedParser ced_sheet_parse_text (	// Parse a string that contains an infix
					// formula
	CedSheet	* sheet,	// Sheet to parse from
	int		row,		// Cell to parse from
	int		column,		// Cell to parse from
	char	const	* text,		// Text string to parse
	CedCell		* cell		// Put result here if '=' is first char
					// in text, NULL = don't
	);
	// Return parser state


/*
	Low level functions
*/

CedCell * ced_cell_new (
	void
	);

int ced_cell_destroy (
	CedCell		* cell
	);

CedCell * ced_cell_duplicate (
	CedCell		* cell
	);

CedCellPrefs * ced_cell_prefs_new (
	void
	);

int ced_cell_prefs_destroy (
	CedCellPrefs	* prefs
	);

char * ced_cell_create_output (		// Create new string for output text in
					// this cell
	CedCell		* cell,
	int		* hjustify	// NULL = Don't use else contains
					// CED_CELL_JUSTIFY_*
	);
	// NULL = nothing/error

void ced_sheet_cursor_max_min (		// All args must be valid !
	CedSheet	* sheet,	// r1,c1,r2,c2 can be
					// sheet->prefs.cursor_*
	int		* r1,		// Minimum row in cursor
	int		* c1,		// Minimum column in cursor
	int		* r2,		// Maximum row in cursor
	int		* c2		// Maximum column in cursor
	);

/*
	FIND
*/

int ced_sheet_find_text (
	CedSheet	* sheet,
	char	const	* text,		// Text to look for in cell content
	int		mode,		// CED_FIND_MODE_*
	int		row,		// First cell
	int		col,		// First cell
	int		rowtot,		// 1..max, 0 = all after row
	int		coltot,		// 1..max, 0 = all after col
	CedFuncScanArea	callback,	// Call this when a match is found
	void		* user_data	// Passed to callback
	);
	// 0 = success
	// 1 = error
	// 2 = termination

int ced_sheet_find_value (
	CedSheet	* sheet,
	double		value,		// Value to look for
	int		mode,		// Only backwards & ignore flags valid
	int		row,		// First cell
	int		col,		// First cell
	int		rowtot,		// 1..max, 0 = all after row
	int		coltot,		// 1..max, 0 = all after col
	CedFuncScanArea	callback,	// Call this when a match is found
	void		* user_data	// Passed to callback
	);
	// 0 = success
	// 1 = error
	// 2 = termination

int ced_sheet_scan_area (		// Find all non-empty cells left->right
					// top->bottom
	CedSheet	* sheet,
	int		row,		// Start cell
	int		col,
	int		rowtot,		// Number of cells to scan, 0 = all
	int		coltot,
	CedFuncScanArea	callback,
	void		* user_data	// Passed to callback
	);
	// 0 = success
	// 1 = error
	// 2 = termination

int ced_sheet_scan_area_backwards (	// Find all non-empty cells
					// right->left bottom->top
	CedSheet	* sheet,
	int		row,		// Start cell
	int		col,
	int		rowtot,		// Number of cells to scan, 0 = row 0
	int		coltot,		// Number of cells to scan, 0 = col 0
	CedFuncScanArea	callback,
	void		* user_data	// Passed to callback
	);
	// 0 = success
	// 1 = error
	// 2 = termination

/*
NOTE

You must never add or remove anything from the sheet that you are scanning in
the callback. If removals or additions are to be made put these into a list or
stack to be actioned once the scan is finished.

*/

int ced_sheet_sort_rows (		// Sort rows in this sheet
	CedSheet	* sheet,
	int		row,		// 1.. First row to sort
	int		rowtot,		// 2.. Number of rows to sort
					// 0 = sort all rows from 'row' onwards
	int	const	* cols,		// 0 terminated list of columns to sort
					// by.
	int		mode,		// CED_SORT_MODE_*
	int	const	* mode_list	// Mode per column. NULL = use mode for
					// all cols
	);

int ced_sheet_sort_columns (		// Sort columns in this sheet
	CedSheet	* sheet,
	int		column,		// 1.. First col to sort
	int		coltot,		// 2.. Number of cols to sort 0 = sort
					// all cols from 'column' onwards
	int	const	* rows,		// 0 terminated list of rows to sort by
	int		mode,		// CED_SORT_MODE_*
	int	const	* mode_list	// Mode per row. NULL = use mode for all
					// rows
	);

/*
	-----
	Notes
	-----

Empty cells/rows/cols are always placed at the end whatever the sort order.
3 Cell type order:
Number Value/Formula/Date < Text < Error/Unknown.

Text is sorted by strcmp, number sorted by < >, error/unknown cells always
equal to each other.

Sorting in descending order reverses all ordering (except empty cells which
always go to end).

*/

int ced_init (				// Set up C locale & random number
	void				// seeder
	);

CedSheet * ced_sheet_copy_area (	// Copy area to a new sheet
	CedSheet	* sheet,
	int		row,		// 1..
	int		column,		// 1..
	int		rowtot,		// 1.. or 0 = rest of row
	int		coltot		// 1.. or 0 = rest of column
	);

/*
NOTE

ced_sheet_copy_area ( sheet, 1, 1, 0, 0 ) is functionally the same as
ced_sheet_duplicate ( sheet ) but about 15-20% slower.

*/

CedSheet * ced_sheet_copy_selection (	// Copy the current cursor area to a
					// new sheet
	CedSheet	* sheet,
	int		* rowtot,	// Store total rows here
	int		* coltot	// Store total cols here
	);

int ced_sheet_paste_area (		// Paste 'paste' cells onto 'sheet' at
					// position row, column
	CedSheet	* sheet,	// dest
	CedSheet	* paste,	// src
	int		row,		// 1..
	int		column,		// 1..
	int		rowtot,		// 1.. to fill specific area in sheet
					// (repeat/truncate).  0 = whole of
					// 'paste'
	int		coltot,		// 1.. to fill specific area in sheet
					// (repeat/truncate).  0 = whole of
					// 'paste'
	int		paste_rowtot,	// 1.. Rows in 'paste'.  0 = autodetect
	int		paste_coltot,	// 1.. Cols in 'paste'.  0 = autodetect
	int		mode		// Flags as per CED_PASTE_*
	);
	// 0 = success
	// 1 = error (no changes to sheet)
	// 2 = error (possible changes to sheet)

int ced_sheet_clear_area (		// Clear an area on the sheet (remove
					// all cells in that range)
	CedSheet	* sheet,
	int		row,		// 1..
	int		column,		// 1..
	int		rowtot,		// 1.. or 0 = rest of row
	int		coltot,		// 1.. or 0 = rest of column
	int		mode		// 0 = clear whole cell, else
					// CED_PASTE_CONTENT or CED_PASTE_PREFS
	);
	// 0 = success
	// 1 = error (no changes to sheet)
	// 2 = error (possible changes to sheet)

int ced_sheet_insert_row (
	CedSheet	* sheet,
	int		row,		// 1.. Insert rows before this row
					// (re-indexing following rows by
					// rowtot)
	int		rowtot		// 1.. Number of rows to insert
	);
	// 0 = success
	// 1 = error (no changes to sheet)
	// 2 = error (possible changes to sheet)

int ced_sheet_delete_row (
	CedSheet	* sheet,
	int		row,		// 1.. Delete this row (re-indexing
					// following rows by rowtot)
	int		rowtot		// 1.. Number of rows to delete
	);
	// 0 = success
	// 1 = error (no changes to sheet)
	// 2 = error (possible changes to sheet)

int ced_sheet_insert_column (
	CedSheet	* sheet,
	int		col,		// 1.. Insert columns before this column
					// (re-indexing following cols by
					// coltot)
	int		coltot		// 1.. Number of cols to insert
	);
	// 0 = success
	// 1 = error (no changes to sheet)
	// 2 = error (possible changes to sheet)

int ced_sheet_delete_column (
	CedSheet	* sheet,
	int		col,		// 1.. Delete this column (re-indexing
					// following cols by coltot)
	int		coltot		// 1.. Number of cols to delete
	);
	// 0 = success
	// 1 = error (no changes to sheet)
	// 2 = error (possible changes to sheet)

int ced_cell_set_2dyear (		// If this cell has a 2 digit date
					// change the text so that the century
	CedCell		* cell,		// is more recent.
	int		year_start	// 0.. (MTKIT_DDT_MAX_DATE_YEAR - 99)
	);
	// -2 = Error
	// -1 = Arg Error
	//  0 = No change
	//  1 = Changed

/*
The index contains references to cells so the index must be destroyed if
the sheet is modified to avoid problems of dangling references.
*/

CedIndex * ced_index_new (		// Create a new empty index
	int		type		// CED_INDEX_TYPE_*
	);

int ced_index_destroy (
	CedIndex	* index
	);

int ced_index_add_items (		// Add items to an index
	CedIndex	* index,
	CedSheet	* sheet,	// Sheet to index
	int		row,		// First cell
	int		col,		// First cell
	int		rowtot,		// Total to index 0 = all remaining
	int		coltot		// Total to index 0 = all remaining
	);

int ced_index_query (
	CedIndex	* index,
	double		value,		// Value to find as primary key
	char	const	* text,		// Text to find as primary key
					// NULL = use value
	CedIndexItem	** item		// Put index item here if found
	);
	// -1 = error
	//  0 = not found
	//  1 = found

CedBook * ced_book_new (		// Create a new empty book
	void
	);

int ced_book_destroy (			// Destroy a book and its contents
	CedBook		* book
	);

int ced_book_add_sheet (
	CedBook		* book,
	CedSheet	* sheet,	// Sheet must not already be in a book
	char	const	* page
	);

int ced_book_detach_sheet (		// Take a sheet out of a book
	CedSheet	* sheet
	);

int ced_book_destroy_sheet (
	CedBook		* book,
	char	const	* page
	);

CedSheet * ced_book_get_sheet (
	CedBook		* book,
	char	const	* page
	);

int ced_book_page_rename (
	CedSheet	* sheet,	// This sheet must be in a book
	char	const	* name		// New page name
	);
	// -1 = sheet detached, but not put back into book
	//  0 = success
	//  1 = fail
	//  2 = name already taken in sheet

CedBookFile * ced_book_add_file (
	CedBook		* book,
	char		* mem,
	int		memsize,
	char	const	* filename
	);

int ced_book_destroy_file (
	CedBook		* book,
	char	const	* filename
	);

CedBookFile * ced_book_get_file (
	CedBook		* book,
	char	const	* filename
	);

int ced_book_timestamp_file (		// Mark file with the current date/time
	CedBookFile	* bookfile
	);

int ced_book_scan (			// Find all sheets in this book
	CedBook		* book,
	CedFuncBookScan	callback,
	void		* user_data
	);
	// 0 = success
	// 1 = fail else termination code from callback
/*
NOTE

While scanning you must not add or remove a sheet to/from this book.
If this is required you must put the sheet names into the stack as I have
done in ced_book_merge ().

*/

int ced_book_recalculate (		// Recalculate all sheets in this book
	CedBook		* book,
	int		mode		// As per ced_sheet_recalculate ()
	);

int ced_book_save (
	CedBook		* book,
	char	const	* filename,
	int		type		// CED_FILE_TYPE_*_BOOK
	);

CedBook * ced_book_load (
	char	const	* filename,
	int		* type,		// Optional: put type here
	char	const	* encoding	// Optional: Force prefs/sheets into
					// UTF-8 if not already.
					// Convert from this encoding
	);

/*
ZIP archive conventions

Sheets are stored as TSV content files under the name "filename/sheet/name"
where the filename is what is passed to ced_book_save (), 'sheet' is a literal
string, and name is the name of the page.

When saving, a temporary file is created: "filename_000" or 001, or until an
unused filename is found.  This file is removed before ced_book_save ()
terminates.

*/

int ced_book_merge (
	CedBook		* book_dest,	// Destination book
	CedBook		* book_insert,	// Sheets from here are to be moved
					// into book_dest
	CedFuncBookMerge callback,	// Call this to see if a sheet is to be
					// moved or not
	void		* user_data	// Passed to callback
	);
	// 0 = success
	// 1 = error (no changes to dest)
	// 2 = error (possible changes to dest/insert books)
	// 3 = user stopped

CedSheet * ced_sheet_transpose (	// Transpose this sheet
	CedSheet	* sheet
	);

CedSheet * ced_sheet_flip_horizontal (	// Flip this sheet
	CedSheet	* sheet
	);

CedSheet * ced_sheet_flip_vertical (	// Flip this sheet
	CedSheet	* sheet
	);

CedSheet * ced_sheet_rotate_clockwise (	// Rotate this sheet
	CedSheet	* sheet
	);

CedSheet * ced_sheet_rotate_anticlockwise ( // Rotate this sheet
	CedSheet	* sheet
	);



#endif		// MTCELLEDIT_H_
