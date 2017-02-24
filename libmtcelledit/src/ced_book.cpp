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

#include "private.h"



static int key_cmp (
	void	const * const	k1,
	void	const * const	k2
	)
{
	return strcmp ( (char const *)k1, (char const *)k2 );
}

static void sheet_delete (
	mtTreeNode	* const	node
	)
{
	CedSheet	* const sheet = (CedSheet *)node->data;


	free ( node->key );
	if ( sheet )
	{
		sheet->book = NULL;		// Delink sheet and book
		ced_sheet_destroy ( sheet );
	}
}

static void file_delete (
	mtTreeNode	* const node
	)
{
	CedBookFile	* const bookfile = (CedBookFile *)node->data;


	free ( node->key );
	if ( bookfile )
	{
		free ( bookfile->mem );
		free ( bookfile );
	}
}



#define BOOK_AUTO_RECALC_DEFAULT	1



static CedBook * ced_book_new_real ( void )
{
	CedBook		* book;


	book = (CedBook *)calloc ( 1, sizeof ( CedBook ) );
	if ( ! book )
	{
		return NULL;
	}

	// Sheet recalc by default
	book->prefs.auto_recalc = BOOK_AUTO_RECALC_DEFAULT;

	book->sheets = mtkit_tree_new ( key_cmp, sheet_delete );
	if ( ! book->sheets )
	{
		goto error;
	}

	book->files = mtkit_tree_new ( key_cmp, file_delete );
	if ( ! book->files )
	{
		goto error;
	}

	return book;

error:
	ced_book_destroy ( book );
	return NULL;
}

CedBook * ced_book_new ( void )
{
	CedBook		* book;


	book = ced_book_new_real ();

	if ( book )
	{
		char		txt[128],
				* username = NULL;
		time_t		now;
		struct tm	* now_tm;
		struct passwd	* p;


		now = time ( NULL );
		now_tm = localtime ( &now );

		snprintf ( txt, sizeof ( txt ),
			"Book created %i-%i-%i %02i:%02i:%02i",
			now_tm->tm_year + 1900,
			now_tm->tm_mon + 1,
			now_tm->tm_mday,
			now_tm->tm_hour,
			now_tm->tm_min,
			now_tm->tm_sec );

		mtkit_strfreedup ( &book->prefs.comment, txt );

		p = getpwuid ( getuid () );
		if ( p )
		{
			if ( p->pw_gecos && p->pw_gecos[0] )
			{
				username = p->pw_gecos;
			}
			else if ( p->pw_name && p->pw_name[0] )
			{
				username = p->pw_name;
			}
		}

		if (	username &&
			! mtkit_strfreedup ( &book->prefs.author, username ) &&
			book->prefs.author
			)
		{
			char	* s;


			// Lose all the trailing ',' characters
			s = strchr ( book->prefs.author, ',' );
			if ( s )
			{
				* s = 0;
			}
		}
	}

	return book;
}

int ced_book_destroy (
	CedBook		* const	book
	)
{
	if ( ! book )
	{
		return 1;
	}

	mtkit_tree_destroy ( book->sheets );
	mtkit_tree_destroy ( book->files );

	free ( book->prefs.active_sheet );
	free ( book->prefs.active_graph );
	free ( book->prefs.author );
	free ( book->prefs.comment );
	free ( book );

	return 0;
}

int ced_book_add_sheet (
	CedBook			* const	book,
	CedSheet		* const	sheet,
	char		const	* const	page
	)
{
	char		* key;


	if ( ! book || ! sheet || sheet->book || ! page )
	{
		return 1;
	}

	key = strdup ( page );
	if ( ! key )
	{
		return 1;
	}

	if ( ! mtkit_tree_node_add ( book->sheets, key, sheet ) )
	{
		free ( key );
		return 1;
	}

	sheet->book = book;
	sheet->book_tnode = mtkit_tree_node_find ( book->sheets, page );

	return 0;
}

int ced_book_detach_sheet (
	CedSheet	* const	sheet
	)
{
	if ( ! sheet || ! sheet->book || ! sheet->book_tnode )
	{
		return 1;
	}

	// Remove the link to this sheet in the book;
	sheet->book_tnode->data = NULL;

	ced_book_destroy_sheet ( sheet->book,
		(char const *)sheet->book_tnode->key );

	sheet->book = NULL;
	sheet->book_tnode = NULL;

	return 0;
}

int ced_book_destroy_sheet (
	CedBook		* const	book,
	char	const	* const	page
	)
{
	if ( ! book || ! page ) return 1;

	return ! mtkit_tree_node_remove ( book->sheets, page );
}

CedSheet * ced_book_get_sheet (
	CedBook			* const	book,
	char		const	* const	page
	)
{
	mtTreeNode	* node;


	if ( ! book || ! page )
	{
		return NULL;
	}

	node = mtkit_tree_node_find ( book->sheets, page );
	if ( ! node )
	{
		return NULL;
	}

	return (CedSheet *)node->data;
}

int ced_book_page_rename (
	CedSheet	* const sheet,
	char	const	* const name
	)
{
	CedBook		* book;


	if ( ! sheet || ! name || ! sheet->book )
	{
		return 1;
	}

	book = sheet->book;

	if ( ced_book_get_sheet ( book, name ) )
	{
		// Name already taken
		return 2;
	}

	ced_book_detach_sheet ( sheet );

	if ( ced_book_add_sheet ( book, sheet, name ) )
	{
		// Unable to re-attach
		return -1;
	}

	return 0;
}

CedBookFile * ced_book_add_file (
	CedBook			* const	book,
	char			* const	mem,
	int			const	memsize,
	char		const	* const	filename
	)
{
	char		* key = NULL;
	CedBookFile	* bookfile = NULL;


	if (	! book		||
		! filename	||
		memsize < 0	||
		memsize > MTKIT_FILESIZE_MAX
		)
	{
		return NULL;
	}

	key = strdup ( filename );
	if ( ! key )
	{
		return NULL;
	}

	bookfile = (CedBookFile *)calloc ( sizeof ( CedBookFile ), 1 );
	if ( ! bookfile )
	{
		free ( key );

		return NULL;
	}

	if ( ! mtkit_tree_node_add ( book->files, key, bookfile ) )
	{
		free ( key );
		free ( bookfile );

		return NULL;
	}

	bookfile->mem = mem;
	bookfile->size = memsize;

	ced_book_timestamp_file ( bookfile );

	return bookfile;
}

int ced_book_destroy_file (
	CedBook		* const	book,
	char	const	* const	filename
	)
{
	if ( ! book || ! filename )
	{
		return 1;
	}

	return ! mtkit_tree_node_remove ( book->files, filename );
}

CedBookFile * ced_book_get_file (
	CedBook		* const	book,
	char	const	* const	filename
	)
{
	mtTreeNode	* node;


	if ( ! book || ! filename )
	{
		return NULL;
	}

	node = mtkit_tree_node_find ( book->files, filename );
	if ( ! node )
	{
		return NULL;
	}

	return (CedBookFile *)node->data;
}

int ced_book_timestamp_file (
	CedBookFile	* const	bookfile
	)
{
	time_t		now;
	struct tm	* now_tm;


	now = time ( NULL );
	now_tm = localtime ( &now );

	bookfile->timestamp[0] = now_tm->tm_year + 1900;
	bookfile->timestamp[1] = now_tm->tm_mon + 1;
	bookfile->timestamp[2] = now_tm->tm_mday;
	bookfile->timestamp[3] = now_tm->tm_hour;
	bookfile->timestamp[4] = now_tm->tm_min;
	bookfile->timestamp[5] = now_tm->tm_sec;

	return 0;
}

typedef struct
{
	CedBook		* book;
	char		* buf;
	char	const	* encoding;
	char		* tmp_fname;
	char	const	* txt_dir;
	int		buf_size;
	int		pages_tot;
	mtUtreeNode	* book_prefs;
	int		filetype;
	int		save_values;

	// For loading a book
	CedBook		* book_values;
	CedSheet	* book_sheet;
	CedCell		* book_cell;

	// For saving a book
	mtZip		* zip;
	mtUtreeNode	* book_prefs_node;

	int32_t		mod_year;
	int8_t		mod_month;
	int8_t		mod_day;
	int8_t		mod_hour;
	int8_t		mod_minute;
	int8_t		mod_second;
} bookSTATE;



static int book_prefs_utree (
	mtUtreeNode	* const	unode,
	CedBook		* const	book,
	int		const	write
	)
{
	mtBulkStr book_list_str[] = {
	{ CED_FILE_PREFS_BOOK_AUTHOR,		&book->prefs.author },
	{ CED_FILE_PREFS_BOOK_COMMENT,		&book->prefs.comment },
	{ CED_FILE_PREFS_BOOK_ACTIVE_SHEET,	&book->prefs.active_sheet },
	{ CED_FILE_PREFS_BOOK_ACTIVE_GRAPH,	&book->prefs.active_graph },
	{ NULL, NULL }
	};

	mtBulkInt book_list_int[] = {
	{ CED_FILE_PREFS_BOOK_DISABLE_LOCKS,	&book->prefs.disable_locks },
	{ CED_FILE_PREFS_BOOK_AUTO_RECALC,	&book->prefs.auto_recalc },
	{ NULL, NULL }
	};


	if ( write )
	{
		mtBulkStr	* bs;


		// Clear empty string references to NULL
		for ( bs = book_list_str; bs->name; bs++ )
		{
			if ( ! bs->var[0] )
			{
				bs->var = NULL;
			}
		}

		// Clear default int's to NULL so that nothing is generated
		if ( book->prefs.disable_locks == 0 )
		{
			book_list_int[0].var = NULL;
		}

		if ( book->prefs.auto_recalc == BOOK_AUTO_RECALC_DEFAULT )
		{
			book_list_int[1].var = NULL;
		}

		if ( mtkit_utree_bulk_set ( unode, book_list_int, NULL,
			book_list_str ) )
		{
			return 1;
		}
	}
	else
	{
		if ( mtkit_utree_bulk_get ( unode, book_list_int, NULL,
			book_list_str ) )
		{
			return 1;
		}
	}

	return 0;
}

typedef struct
{
	mtUtreeNode		* unode;
	CedSheet		* sheet;
	CedCellPrefs	const	* dpref;
	char			buf[2048];
} cprefSTATE;




// Note - at the entry point, cell->prefs must exist
static int cell_prefs_utree (
	mtUtreeNode		* const	unode,
	int			const	row,
	int			const	col,
	CedCell			* const	cell,
	int			const	write,
	CedCellPrefs	const * const	dprefs,
	char				buf[2048] // Save only (load can = NULL)
	)
{
	mtBulkInt	cell_list_int[] = {
	{ CED_FILE_PREFS_CELL_ALIGN_HORIZONTAL,	&cell->prefs->align_horizontal },
	{ CED_FILE_PREFS_CELL_COLOR_BACKGROUND,	&cell->prefs->color_background },
	{ CED_FILE_PREFS_CELL_COLOR_FOREGROUND,	&cell->prefs->color_foreground },
	{ CED_FILE_PREFS_CELL_FORMAT,		&cell->prefs->format },
	{ CED_FILE_PREFS_CELL_WIDTH,		&cell->prefs->width },
	{ CED_FILE_PREFS_CELL_NUM_DECIMAL_PLACES, &cell->prefs->num_decimal_places },
	{ CED_FILE_PREFS_CELL_NUM_ZEROS,	&cell->prefs->num_zeros },
	{ CED_FILE_PREFS_CELL_TEXT_STYLE,	&cell->prefs->text_style },
	{ CED_FILE_PREFS_CELL_LOCKED,		&cell->prefs->locked },
	{ CED_FILE_PREFS_CELL_BORDER_TYPE,	&cell->prefs->border_type },
	{ CED_FILE_PREFS_CELL_BORDER_COLOR,	&cell->prefs->border_color },
	{ NULL, NULL }
	};

	mtBulkStr	cell_list_str[] = {
	{ CED_FILE_PREFS_CELL_FORMAT_DATETIME,	&cell->prefs->format_datetime },
	{ CED_FILE_PREFS_CELL_NUM_THOUSANDS,	&cell->prefs->num_thousands },
	{ CED_FILE_PREFS_CELL_TEXT_PREFIX,	&cell->prefs->text_prefix },
	{ CED_FILE_PREFS_CELL_TEXT_SUFFIX,	&cell->prefs->text_suffix },
	{ NULL, NULL }
	};


	if ( write )
	{
		mtUtreeNode	* child;
		mtBulkStr	* bs;
		mtBulkInt	* bi;
		int	const	** di;
		int	const	* def_int[] =
				{	// Order as cell_list_int
				&dprefs->align_horizontal,
				&dprefs->color_background,
				&dprefs->color_foreground,
				&dprefs->format,
				&dprefs->width,
				&dprefs->num_decimal_places,
				&dprefs->num_zeros,
				&dprefs->text_style,
				&dprefs->locked,
				&dprefs->border_type,
				&dprefs->border_color
				};


		// Clear empty string references to NULL
		for ( bs = cell_list_str; bs->name; bs++ )
		{
			if ( ! bs->var[0] )
			{
				bs->var = NULL;
			}
		}

		// Clear default int's to NULL so that nothing is generated
		for (	bi = cell_list_int, di = def_int;
			bi->name;
			bi++ , di++
			)
		{
			if ( bi->var[0] == di[0][0] )
			{
				bi->var = NULL;
			}
		}

		child = mtkit_utree_new_element ( unode, CED_FILE_PREFS_CELL );
		if ( ! child )
		{
			return 1;
		}

		snprintf ( buf, 2048, "r%ic%i", row, col );

		if ( mtkit_utree_set_attribute_str ( child,
			CED_FILE_PREFS_CELL_REF, buf ) )
		{
			return 1;
		}

		if ( mtkit_utree_bulk_set ( child, cell_list_int, NULL,
			cell_list_str ) )
		{
			return 1;
		}
	}
	else
	{
		if ( mtkit_utree_bulk_get ( unode, cell_list_int, NULL,
			cell_list_str ) )
		{
			return 1;
		}

		if ( cell->prefs->width > CED_MAX_COLUMN_WIDTH )
		{
			cell->prefs->width = CED_MAX_COLUMN_WIDTH;
		}

		if ( cell->prefs->width < CED_MIN_COLUMN_WIDTH )
		{
			cell->prefs->width = CED_MIN_COLUMN_WIDTH;
		}
	}


	return 0;
}

static int cell_prefs_save_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	cprefSTATE	* const	state = (cprefSTATE *)user_data;


	if (	cell->prefs &&
		memcmp ( &cell->prefs, state->dpref, sizeof ( CedCellPrefs ) )
		)
	{
		cell_prefs_utree ( state->unode, row, col, cell, 1,
			state->dpref, state->buf );
	}

	return 0;	// continue
}

static int cell_prefs_load (
	mtUtreeNode	* const	unode,
	CedSheet	* const	sheet
	)
{
	char		const	* txt;
	CedCellPrefs	const	* dprefs = ced_cell_prefs_default ();
	mtUtreeNode		* node;
	CedCellRef		cref;
	CedCell			* cell;


	for (	node = mtkit_utree_get_node ( unode, CED_FILE_PREFS_CELL,
			MTKIT_UTREE_NODE_TYPE_ELEMENT );
		node;
		node = mtkit_utree_get_node_next ( node, CED_FILE_PREFS_CELL,
			MTKIT_UTREE_NODE_TYPE_ELEMENT )
		)
	{
		if ( mtkit_utree_get_attribute_str ( node,
			CED_FILE_PREFS_CELL_REF, &txt ) )
		{
			continue;
		}

		if ( ced_strtocellref ( txt, &cref, NULL, 1 ) )
		{
			continue;
		}

		if ( cref.row_d < 0 || cref.col_d < 0 )
		{
			continue;
		}

		cell = ced_cell_set_find ( sheet, cref.row_d, cref.col_d );
		if ( ! cell )
		{
			return 1;
		}

		if ( ! cell->prefs )
		{
			cell->prefs = ced_cell_prefs_new ();
			if ( ! cell->prefs )
			{
				return 1;
			}
		}

		cell_prefs_utree ( node, cref.row_d, cref.col_d, cell, 0,
			dprefs, NULL );

		if ( ! memcmp ( cell->prefs, dprefs, sizeof ( CedCellPrefs ) ) )
		{
			if ( ! cell->text || ! cell->text[0] )
			{
				// Remove zombie cell (no prefs or data)

				ced_sheet_delete_cell ( sheet, cref.row_d,
					cref.col_d );
			}
			else
			{
				// Text exists in this cell so keep it
				// Remove zombie prefs

				ced_cell_prefs_destroy ( cell->prefs );
				cell->prefs = NULL;
			}
		}
	}

	return 0;
}

static int cell_prefs_save (
	mtUtreeNode	* const	unode,
	CedSheet	* const	sheet
	)
{
	cprefSTATE	state = { unode, sheet, NULL, {0} };


	state.dpref = ced_cell_prefs_default ();

	return ced_sheet_scan_area ( sheet, 0, 0, 0, 0, cell_prefs_save_cb,
		&state );
}

static int sheet_prefs_utree (
	mtUtreeNode		* const	unode,
	char		const	* const	name,
	CedSheet		* const	sheet,
	int			const	write
	)
{
	mtBulkInt	book_list_int[] = {
	{ CED_FILE_PREFS_SHEET_CURSOR_R1,	&sheet->prefs.cursor_r1 },
	{ CED_FILE_PREFS_SHEET_CURSOR_C1,	&sheet->prefs.cursor_c1 },
	{ CED_FILE_PREFS_SHEET_CURSOR_R2,	&sheet->prefs.cursor_r2 },
	{ CED_FILE_PREFS_SHEET_CURSOR_C2,	&sheet->prefs.cursor_c2 },
	{ CED_FILE_PREFS_SHEET_SPLIT_R1,	&sheet->prefs.split_r1 },
	{ CED_FILE_PREFS_SHEET_SPLIT_C1,	&sheet->prefs.split_c1 },
	{ CED_FILE_PREFS_SHEET_SPLIT_R2,	&sheet->prefs.split_r2 },
	{ CED_FILE_PREFS_SHEET_SPLIT_C2,	&sheet->prefs.split_c2 },
	{ CED_FILE_PREFS_SHEET_START_ROW,	&sheet->prefs.start_row },
	{ CED_FILE_PREFS_SHEET_START_COL,	&sheet->prefs.start_col },
	{ CED_FILE_PREFS_SHEET_LOCKED,		&sheet->prefs.locked },
	{ NULL, NULL }
	};


	if ( write )
	{
		mtUtreeNode	* child;
		mtBulkInt	* bi;


		child = mtkit_utree_new_element ( unode, CED_FILE_PREFS_SHEET );
		if ( ! child )
		{
			return 1;
		}

		if ( mtkit_utree_set_attribute_str ( child,
			CED_FILE_PREFS_SHEET_NAME, name ) )
		{
			return 1;
		}

		// Clear zero int's to NULL so that nothing is generated
		for ( bi = book_list_int; bi->name; bi++ )
		{
			if ( ! bi->var[0] )
			{
				bi->var = NULL;
			}
		}

		if ( mtkit_utree_bulk_set ( child, book_list_int, NULL, NULL ) )
		{
			return 1;
		}

		if ( cell_prefs_save ( child, sheet ) )
		{
			return 1;
		}
	}
	else
	{
		if ( mtkit_utree_bulk_get ( unode, book_list_int, NULL, NULL ) )
		{
			return 1;
		}

		if ( cell_prefs_load ( unode, sheet ) )
		{
			return 1;
		}
	}

	return 0;
}

static int save_mtft_to_zip (
	mtFile			* const	mtfp,
	bookSTATE		* const	state,
	char		const	* const	archive_name
	)
{
	void		* buf;
	int64_t		buf_len;
	int		res = 1;
	char		* filename = NULL;


	if (	mtkit_file_get_mem ( mtfp, &buf, &buf_len ) ||
		buf_len > INT32_MAX
		)
	{
		return 1;
	}

	filename = mtkit_string_join ( state->tmp_fname, state->txt_dir,
		archive_name, NULL );
	if ( ! filename )
	{
		return 1;
	}

	res = mtkit_zip_save_file ( state->zip, filename, buf, (int32_t)buf_len,
		1, state->mod_year, state->mod_month, state->mod_day,
		state->mod_hour, state->mod_minute, state->mod_second );

	free ( filename );

	return res;
}


static int save_book_prefs_init (
	bookSTATE	* const	state
	)
{
	state->book_prefs = mtkit_utree_new_root ();
	if ( ! state->book_prefs )
	{
		goto error;
	}

	state->book_prefs_node = mtkit_utree_new_element ( state->book_prefs,
		CED_FILE_PREFS_BOOK );
	if ( ! state->book_prefs_node )
	{
		goto error;
	}

	if ( book_prefs_utree ( state->book_prefs_node, state->book, 1 ) )
	{
		goto error;
	}

	return 0;	// Success

error:
	return 1;
}


static int save_book_prefs_close (
	bookSTATE	* const	state
	)
{
	int		res = 1;
	mtFile		* mtft;


	mtft = mtkit_utree_save_file_mem ( state->book_prefs,
		MTKIT_UTREE_OUTPUT_DEFAULT );
	if ( mtft )
	{
		state->txt_dir = "/prefs/";
		res = save_mtft_to_zip ( mtft, state, "book.txt" );

		mtkit_file_close ( mtft );

		if ( res )
		{
			res = 1;	// Failure
		}
		else
		{
			res = 0;	// Success
		}
	}

	mtkit_utree_destroy_node ( state->book_prefs );
	state->book_prefs = NULL;

	return res;
}

static int book_save_sheet_scan (
	mtTreeNode	* const	node,
	void		* const	user_data
	)
{
	int			res = 1;
	bookSTATE	* const state = (bookSTATE *)user_data;
	mtFile			* mtft;


	state->txt_dir = "/sheet/";

	mtft = ced_sheet_save_mem ( (CedSheet *)node->data, state->filetype );
	if ( ! mtft )
	{
		goto error;
	}

	res = save_mtft_to_zip ( mtft, state, (char *)node->key );
	mtkit_file_close ( mtft );
	if ( res )
	{
		goto error;
	}

	if ( state->save_values )
	{
		res = 1;
		state->txt_dir = "/values/";

		mtft = ced_sheet_save_mem ( (CedSheet *)node->data,
			state->save_values );
		if ( ! mtft )
		{
			goto error;
		}

		res = save_mtft_to_zip ( mtft, state, (char *)node->key );
		mtkit_file_close ( mtft );
		if ( res )
		{
			goto error;
		}
	}

	res = sheet_prefs_utree ( state->book_prefs_node, (char *)node->key,
		(CedSheet *)node->data, 1 );

error:
	return res;
}

static int book_save_file_scan (
	mtTreeNode	* const	node,
	void		* const	user_data
	)
{
	int			res = 1;
	bookSTATE	* const state = (bookSTATE *)user_data;
	CedBookFile		* bookfile;
	char			* filename;


	if ( ! node || ! node->key || ! ( (char *)node->key )[0] )
	{
		return 0;
	}

	bookfile = (CedBookFile *)node->data;

	filename = mtkit_string_join ( state->tmp_fname, "/",
		(char const *)node->key, NULL );
	if ( ! filename )
	{
		return 1;
	}

	res = mtkit_zip_save_file ( state->zip, filename, bookfile->mem,
		bookfile->size, 1, bookfile->timestamp[0],
		(int8_t)bookfile->timestamp[1], (int8_t)bookfile->timestamp[2],
		(int8_t)bookfile->timestamp[3], (int8_t)bookfile->timestamp[4],
		(int8_t)bookfile->timestamp[5] );

	free ( filename );

	return res;
}

int ced_book_save (
	CedBook			* const	book,
	char		const	* const	filename,
	int			const	type
	)
{
	int		res = 1,
			ierr = 0
			;
	char	const	* scon;
	char		* s;
	bookSTATE	state = { book, NULL, NULL, NULL, NULL, 0, 0, NULL,
				0, 0, NULL, NULL, NULL, NULL, NULL,
				0, 0, 0, 0, 0, 0
				};
	time_t		now;
	struct tm	* now_tm;


	now = time ( NULL );
	now_tm = localtime ( &now );

	if ( ! book || ! filename )
	{
		return 1;
	}

	switch ( type )
	{
	case CED_FILE_TYPE_TSV_VAL_BOOK:
		state.save_values = CED_FILE_TYPE_TSV_VALUE;
		/* FALLTHROUGH */

	case CED_FILE_TYPE_TSV_BOOK:
		state.filetype = CED_FILE_TYPE_TSV_CONTENT;
		break;

	case CED_FILE_TYPE_LEDGER_VAL_BOOK:
		state.save_values = CED_FILE_TYPE_LEDGER_VAL;
		/* FALLTHROUGH */

	case CED_FILE_TYPE_LEDGER_BOOK:
		state.filetype = CED_FILE_TYPE_LEDGER;
		break;

	default:
		return 1;
	}

	scon = strrchr ( filename, MTKIT_DIR_SEP );
	if ( ! scon )
	{
		scon = filename;
	}
	else
	{
		scon++;
	}

	state.tmp_fname = strdup ( scon );
	if ( ! state.tmp_fname )
	{
		goto error;
	}

	s = strrchr ( state.tmp_fname, '.' );
	if ( s && s != state.tmp_fname )
	{
		// Remove the extension for files x.y
		s[0] = 0;
	}

	state.zip = mtkit_zip_save_open ( filename );
	if ( ! state.zip )
	{
		goto error;
	}

	state.mod_year = now_tm->tm_year + 1900;
	state.mod_month = (int8_t)(now_tm->tm_mon + 1);
	state.mod_day = (int8_t)now_tm->tm_mday;
	state.mod_hour = (int8_t)now_tm->tm_hour;
	state.mod_minute = (int8_t)now_tm->tm_min;
	state.mod_second = (int8_t)now_tm->tm_sec;


	if ( save_book_prefs_init ( &state ) )
	{
		ierr = 1;
	}
	else if ( mtkit_tree_scan ( book->sheets, book_save_sheet_scan, &state,
		0 ) )
	{
		ierr = 2;
	}
	else if ( save_book_prefs_close ( &state ) )
	{
		ierr = 3;
	}
	else if ( mtkit_tree_scan ( book->files, book_save_file_scan, &state,
		0 ) )
	{
		ierr = 4;
	}

error:
	res = mtkit_zip_save_close ( state.zip );

	if ( res == 0 && ierr )
	{
		res = 1;
	}

	// No point in clearing dangling pointers before exiting
	free ( state.tmp_fname );

	mtkit_utree_destroy_node ( state.book_prefs );

	return res;
}

static int load_cb (
	char	const	* const	name,
	void		* const	buf,
	int32_t		const	buf_len,
	int32_t		const	year,
	int8_t		const	month,
	int8_t		const	day,
	int8_t		const	hour,
	int8_t		const	minute,
	int8_t		const	second,
	void		* const	user_data
	)
{
	if ( name )
	{
		int			i = 0,
					res;
		char		const	* s = NULL;
		char	const	* const	fname[] = {
					"/sheet/",
					"/prefs/book.txt",
					"/values/",
					NULL
					};
		char		const	* sheet_name;
		char			* free_sheet_name = NULL;
		bookSTATE	* const state = (bookSTATE *)user_data;
		CedSheet		* sheet;
		CedBookFile		* bookfile;


		for ( i = 0; fname[i]; i++ )
		{
			s = strstr ( name, fname[i] );
			if ( s && s == strchr ( name, '/' ) )
			{
				break;
			}
		}

		sheet_name = s + 7;

		switch ( i )
		{
		case 2:	// LOAD SHEET (values)
			if ( ! s[8] )
			{
				// Catches nameless sheets
				return MTKIT_ZIP_OK;
			}

			sheet_name = s + 8;
			// FALLTHROUGH

		case 0:	// LOAD SHEET
			if ( ! s[7] )
			{
				// Catches nameless sheets
				return MTKIT_ZIP_OK;
			}

			if (	state->encoding &&
				! mtkit_utf8_string_legal (
					(unsigned char const *)sheet_name, 0 ) )
			{
				if ( mtkit_string_encoding_conversion (
					sheet_name, state->encoding,
					&free_sheet_name, "UTF-8" ) )
				{
					return MTKIT_ZIP_ERROR_USER;
				}

				sheet_name = free_sheet_name;
			}

			// This file has "/sheet/" in it so its a sheet
			sheet = ced_sheet_load_mem ( (char *)buf,
				(size_t)buf_len, state->encoding,
				&state->filetype );

			if ( ! sheet )
			{
				free ( free_sheet_name );
				return MTKIT_ZIP_ERROR_USER;
			}

			if ( i == 0 )
			{
				res = ced_book_add_sheet ( state->book, sheet,
					sheet_name );
			}
			else
			{
				if ( ! state->book_values )
				{
					state->book_values =
						ced_book_new_real ();
				}

				if ( ! state->book_values )
				{
					res = 1;
				}
				else
				{
					res = ced_book_add_sheet (
						state->book_values, sheet,
						sheet_name );
				}
			}

			free ( free_sheet_name );

			if ( res )
			{
				ced_sheet_destroy ( sheet );
				return MTKIT_ZIP_ERROR_USER;
			}
			break;

		case 1:	// LOAD PREFS
			if ( s[15] )
			{
				// Catches bad filename
				return MTKIT_ZIP_OK;
			}

			if ( ! state->book_prefs )
			{
				if (	state->encoding	&&
					buf		&&
					buf_len > 0	&&
					! mtkit_utf8_string_legal (
						(unsigned char *)buf,
						(size_t)buf_len )
					)
				{
					char	* newmem,
						* ob = (char *)buf;


					ob[ buf_len - 1 ] = 0;

					if ( mtkit_string_encoding_conversion (
						(char *)buf, state->encoding,
						&newmem, "UTF-8" ) )
					{
						return MTKIT_ZIP_ERROR_USER;
					}

					state->book_prefs =
						mtkit_utree_load_mem ( NULL,
						newmem, strlen ( newmem ) + 1,
						NULL );

					free ( newmem );
				}
				else
				{
					state->book_prefs =
						mtkit_utree_load_mem ( NULL,
						(char *)buf, (size_t)buf_len,
						NULL );
				}
			}
			break;

		default: // LOAD FILE
			s = strchr ( name, '/' );
			if ( ! s )
			{
				s = name;
			}
			else
			{
				s++;
			}

			bookfile = ced_book_add_file ( state->book, (char *)buf,
				buf_len, s );

			if ( bookfile == NULL )
			{
				return MTKIT_ZIP_ERROR_USER;
			}

			bookfile->timestamp[0] = year;
			bookfile->timestamp[1] = month;
			bookfile->timestamp[2] = day;
			bookfile->timestamp[3] = hour;
			bookfile->timestamp[4] = minute;
			bookfile->timestamp[5] = second;

			return MTKIT_ZIP_OK_DONT_FREE;
		}
	}

	return MTKIT_ZIP_OK;	// Request next file
}

static void parse_book_prefs (
	bookSTATE	* const	state
	)
{
	char	const	* txt;
	mtUtreeNode	* node,
			* chn;
	CedSheet	* sheet;


	node = mtkit_utree_get_node ( state->book_prefs, CED_FILE_PREFS_BOOK,
			MTKIT_UTREE_NODE_TYPE_ELEMENT );
	if ( node )
	{
		book_prefs_utree ( node, state->book, 0 );

		// Populate sheet preferences from the Utree
		for (	chn = mtkit_utree_get_node ( node, CED_FILE_PREFS_SHEET,
				MTKIT_UTREE_NODE_TYPE_ELEMENT );
			chn ;
			chn = mtkit_utree_get_node_next ( chn,
				CED_FILE_PREFS_SHEET,
				MTKIT_UTREE_NODE_TYPE_ELEMENT )
			)
		{
			if ( mtkit_utree_get_attribute_str ( chn,
				CED_FILE_PREFS_SHEET_NAME, &txt ) )
			{
				continue;
			}

			sheet = ced_book_get_sheet ( state->book, txt );
			if ( ! sheet )
			{
				continue;
			}

			sheet_prefs_utree ( chn, NULL, sheet, 0 );
		}
	}
}

static int sheet_vals_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	if ( cell->type == CED_CELL_TYPE_VALUE )
	{
		bookSTATE	* state = (bookSTATE *)user_data;


		state->book_cell = ced_sheet_get_cell ( state->book_sheet, row,
			col );

		if ( state->book_cell )
		{
			state->book_cell->value = cell->value;

			// Flush out errors
			if ( state->book_cell->type == CED_CELL_TYPE_ERROR )
			{
/*
	NOTE

An error here implies some sort of recalc error when the load function
populated the cell, so it must be a formula that relies on evaluation.
A valid number was saved to the values sheet so when
this file was saved there was no error.

MT 14-5-2010

*/
				state->book_cell->type =
					CED_CELL_TYPE_FORMULA_EVAL;
			}
		}
	}

	return 0;	// Continue
}

static int book_vals_cb (
	CedSheet		* const	sheet,
	char		const	* const	name,
	void			* const	user_data
	)
{
	bookSTATE	* const state = (bookSTATE *)user_data;


	// Get same named sheet in book loaded
	state->book_sheet = ced_book_get_sheet ( state->book, name );

	if ( state->book_sheet )
	{
		// Scan all cells in value sheet and copy values across to main
		// book/sheet

		ced_sheet_scan_area ( sheet, 1, 1, 0, 0, sheet_vals_cb,
			user_data );
	}

	return 0;	// Continue
}

CedBook * ced_book_load (
	char	const	* const	filename,
	int		* const	type,
	char	const	* const	encoding
	)
{
	bookSTATE	state;


	memset ( &state, 0, sizeof(state) );

	if ( ! filename )
	{
		return NULL;
	}

	state.book = ced_book_new_real ();
	if ( ! state.book )
	{
		return NULL;
	}

	state.encoding = encoding;

	if ( mtkit_zip_load ( filename, load_cb, &state ) )
	{
		goto error;
	}

	if ( state.book_prefs )
	{
		parse_book_prefs ( &state );
		mtkit_utree_destroy_node ( state.book_prefs );
		state.book_prefs = NULL;
	}
	else
	{
		if ( ! state.book->sheets || ! state.book->sheets->root )
		{
			// No prefs and no sheets so this is NOT a valid book
			goto error;
		}
	}

	if ( state.book && type )
	{
		switch ( state.filetype )
		{
		default: // If no sheets are found, but prefs exist

		case CED_FILE_TYPE_TSV_CONTENT:
			if ( state.book_values )
			{
				type[0] = CED_FILE_TYPE_TSV_VAL_BOOK;
			}
			else
			{
				type[0] = CED_FILE_TYPE_TSV_BOOK;
			}
			break;

		case CED_FILE_TYPE_LEDGER:
			if ( state.book_values )
			{
				type[0] = CED_FILE_TYPE_LEDGER_VAL_BOOK;
			}
			else
			{
				type[0] = CED_FILE_TYPE_LEDGER_BOOK;
			}
			break;
		}
	}

	if ( state.book_values )
	{
		ced_book_scan ( state.book_values, book_vals_cb, &state );
		ced_book_destroy ( state.book_values );
	}

	return state.book;

error:
	ced_book_destroy ( state.book_values );
	ced_book_destroy ( state.book );

	return NULL;
}



typedef struct
{
	CedFuncBookScan	callback;
	void		* user_data;
	int		res;
} scanSTATE;



static int tree_scan_cb (
	mtTreeNode	* const	node,
	void		* const	user_data
	)
{
	scanSTATE	* state = (scanSTATE *)user_data;


	state->res = state->callback ( (CedSheet *)node->data,
		(char *)node->key, state->user_data );

	if ( state->res )
	{
		return 1;	// User stop
	}

	return 0;		// Continue
}

int ced_book_scan (
	CedBook		* const	book,
	CedFuncBookScan	const	callback,
	void		* const	user_data
	)
{
	scanSTATE	state = { callback, user_data, 0 };


	if ( ! book || ! callback )
	{
		return 1;
	}

	if ( ! book->sheets || ! book->sheets->root )
	{
		return 0;
	}

	mtkit_tree_scan ( book->sheets, tree_scan_cb, &state, 0 );

	return state.res;
}


static int recalc_book_callback (
	CedSheet		* const	sheet,
	char		const	* const	ARG_UNUSED ( name ),
	void			* const	user_data
	)
{
	ced_sheet_recalculate ( sheet, NULL, (int)(intptr_t)user_data );

	return 0;	// Continue
}

int ced_book_recalculate (
	CedBook		* const	book,
	int		const	mode
	)
{
	return ced_book_scan ( book, recalc_book_callback,
		(void *) (intptr_t)mode );
}



typedef struct
{
	CedBook		* book_dest,
			* book_insert;
	CedFuncBookMerge callback;
	void		* user_data;

	CedSheet	** sheet_a;
	mtTreeNode	** file_a;

	int		exists,
			res,
			sheet_i,
			file_i,
			sheet_tot,
			file_tot
			;
} mergeSTATE;



static int merge_sheet_count (
	CedSheet		* const	ARG_UNUSED ( sheet ),
	char		const	* const	ARG_UNUSED ( name ),
	void			* const	user_data
	)
{
	mergeSTATE	* const	state = (mergeSTATE *)user_data;


	state->sheet_tot ++;

	return 0;	// Continue
}

static int merge_sheet_cb (
	CedSheet		* const	sheet,
	char		const	* const	name,
	void			* const	user_data
	)
{
	mergeSTATE	* const	state = (mergeSTATE *)user_data;


	if ( ced_book_get_sheet ( state->book_dest, name ) )
	{
		state->exists = 1;
	}
	else
	{
		state->exists = 0;
	}

	state->res = state->callback ( state->book_dest, state->book_insert,
		(void *)sheet, 0, name, state->exists, state->user_data );

	switch ( state->res )
	{
	case 0:			// Move this sheet to dest
		state->sheet_a[ state->sheet_i ] = sheet;
		state->sheet_i ++;
		return 0;

	case 1:			// Don't move this sheet
		return 0;

	case 2:			// Stop this operation
		return 3;
	}

	return 1;		// Unexpected return value from callback
}

static int merge_files_count (
	mtTreeNode	* const	ARG_UNUSED ( node ),
	void		* const	user_data
	)
{
	mergeSTATE	* state = (mergeSTATE *)user_data;


	state->file_tot ++;

	return 0;	// Continue
}

static int merge_files_cb (
	mtTreeNode	* const	node,
	void		* const	user_data
	)
{
	mergeSTATE	* const	state = (mergeSTATE *)user_data;


	if ( ced_book_get_file ( state->book_dest, (char const *)node->key ) )
	{
		state->exists = 1;
	}
	else
	{
		state->exists = 0;
	}

	state->res = state->callback ( state->book_dest, state->book_insert,
		(void *)node->data, 1, (char const *)node->key, state->exists,
		state->user_data );

	switch ( state->res )
	{
	case 0:			// Move this file to dest
		state->file_a[ state->file_i ] = node;
		state->file_i ++;
		return 0;

	case 1:			// Don't move this file
		return 0;

	case 2:			// Stop this operation
		return 3;
	}

	return 1;		// Unexpected return value from callback
}

int ced_book_merge (
	CedBook		* const	book_dest,
	CedBook		* const	book_insert,
	CedFuncBookMerge const	callback,
	void		* const	user_data
	)
{
	mergeSTATE	state = { book_dest, book_insert, callback, user_data,
				NULL, NULL, 0, 0, 0, 0, 0, 0 };


	if ( ! book_dest || ! book_insert || ! callback )
	{
		return 1;
	}

	// COUNT & MERGE SHEETS
	state.res = ced_book_scan ( book_insert, merge_sheet_count, &state );
	if ( state.res == 0 && state.sheet_tot > 0 )
	{
		state.sheet_a = (CedSheet **)calloc ( (size_t)state.sheet_tot,
			sizeof ( state.sheet_a[0] ) );

		if ( ! state.sheet_a )
		{
			goto finish;
		}

		state.res = ced_book_scan ( book_insert, merge_sheet_cb, &state
			);
	}

	if ( state.res )
	{
		goto finish;
	}

	if ( state.sheet_a )
	{
		int		sp;
		CedSheet	* sheet;
		char		* name;

		for ( sp = 0; sp < state.sheet_i; sp++ )
		{
			sheet = state.sheet_a[ sp ];
			if ( ! sheet )
			{
				break;
			}

			name = strdup ( (char const *)sheet->book_tnode->key );

			if ( ! name || ced_book_detach_sheet ( sheet ) )
			{
				free ( name );
				state.res = 2;
				break;
			}

			if ( ced_book_add_sheet ( book_dest, sheet, name ) )
			{
				// NOTE: This should never really happen, but
				// just in case !

				// Destroy orphaned sheet
				ced_sheet_destroy ( sheet );
				free ( name );
				state.res = 2;
				break;
			}

			free ( name );
		}
	}


	// COUNT & MERGE FILES
	state.res = mtkit_tree_scan ( book_insert->files, merge_files_count,
		&state, 0 );

	if ( state.res == 0 && state.file_tot > 0 )
	{
		state.file_a = (mtTreeNode **)calloc ( (size_t)state.file_tot,
			sizeof ( state.file_a[0] ) );

		if ( ! state.file_a )
		{
			goto finish;
		}

		state.res = mtkit_tree_scan ( book_insert->files,
			merge_files_cb, &state, 0 );
	}

	if ( state.res )
	{
		goto finish;
	}

	if ( state.file_a )
	{
		int		sp;
		CedBookFile	* bookfile;
		char		* filename;


		for ( sp = 0; sp < state.file_i; sp++ )
		{
			if ( ! state.file_a[ sp ] )
			{
				break;
			}

			bookfile = (CedBookFile *) state.file_a[ sp ]->data;
			filename = (char *) state.file_a[ sp ]->key;

			if ( NULL == ced_book_add_file ( book_dest,
				bookfile->mem, bookfile->size, filename ) )
			{
				state.res = 2;
				break;
			}

			bookfile->mem = NULL;
			ced_book_destroy_file ( book_insert, filename );
			// Silently ignore errors - they don't matter as the
			// memory has passed across
		}
	}

finish:
	free ( state.sheet_a );
	free ( state.file_a );
	// Dangling pointers are safe as we exit immediately

	return state.res;
}

