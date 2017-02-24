/*
	Copyright (C) 2012-2016 Mark Tyler

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

#include <sys/stat.h>



static int fio_checkset_ro (
	CuiFile		* const file
	)
{
	if ( 0 == mtkit_file_writable ( file->name ) )
	{
		// File cannot be locked as it is already read only

		file->lock_state = CUI_FILE_LOCK_RO;

		return 1;		// RO
	}

	return 0;			// RW
}

static void fio_try_lock (
	CuiFile		* const file
	)
{
	if (	CUI_FILE_LOCK_RW != file->lock_state	||
		! file->name
		)
	{
		return;
	}

	if ( 1 == fio_checkset_ro ( file ) )
	{
		// File cannot be locked as it is already read only
		return;
	}


	struct stat	finfo;


	if ( 0 == stat ( file->name, &finfo ) )
	{
		mode_t		m;


		file->perm_mode = finfo.st_mode;

		m = finfo.st_mode & (mode_t)( ~(S_IWUSR | S_IWGRP | S_IWOTH) );

		if ( 0 == chmod ( file->name, m ) )
		{
			// File was successfully locked
			file->lock_state = CUI_FILE_LOCK_RWL;

			return;
		}
	}

	// Lock not set, but file is writable
	file->lock_state = CUI_FILE_LOCK_RW;
}

static void fio_unset_unlock (
	CuiFile		* const file
	)
{
	if ( ! file->name )
	{
		// Ensure integrity for no filename
		file->lock_state = CUI_FILE_LOCK_RW;

		return;
	}

	if (	CUI_FILE_LOCK_RWL == file->lock_state		&&
		0 == chmod ( file->name, file->perm_mode )
		)
	{
		// File permissions now same as when original opened
		file->lock_state = CUI_FILE_LOCK_RW;

		return;
	}

	// Either file was opened RO or chmod had an error so just unset
	file->lock_state = CUI_FILE_LOCK_RW;
}

CuiFile * cui_file_new ( void )
{
	CuiFile		* file;


	file = (CuiFile *)calloc ( sizeof ( CuiFile ), 1 );
	if ( ! file )
	{
		return NULL;
	}

	file->cubook = cui_book_new ();
	if ( ! file->cubook )
	{
		cui_file_free ( file );

		return NULL;
	}

	file->type = CED_FILE_TYPE_TSV_VAL_BOOK;

	return file;
}

int cui_file_free (
	CuiFile		* const	file
	)
{
	if ( ! file )
	{
		return 1;
	}

	fio_unset_unlock ( file );

	cui_book_destroy ( file->cubook );
	free ( file->name );
	free ( file );

	return 0;
}

static CedBook * load_sheet_to_book (
	int		const	cft,
	char	const *	const	filename,
	int	*	const	file_type
	)
{
	char	const	* sheet_name;
	char		* free_sheet_name = NULL;
	CedSheet	* newsheet = NULL;
	CedBook		* newbook = NULL;


	// TSV or CSV
	if ( cft == CED_FILE_DETECT_TSV )
	{
		newsheet = ced_sheet_load ( filename, "ISO-8859-1", file_type );
	}
	else if ( cft == CED_FILE_DETECT_CSV )
	{
		newsheet = ced_sheet_load_csv ( filename, "ISO-8859-1");
		file_type[0] = CED_FILE_TYPE_CSV_CONTENT;
	}

	if ( ! newsheet )
	{
		return NULL;
	}

	newbook = ced_book_new ();
	if ( ! newbook )
	{
		ced_sheet_destroy ( newsheet );

		return NULL;
	}

	sheet_name = strrchr ( filename, MTKIT_DIR_SEP );
	if ( sheet_name )
	{
		sheet_name ++;
	}
	else
	{
		sheet_name = filename;
	}

	free_sheet_name = mtkit_utf8_from_cstring ( sheet_name );
	if ( ! free_sheet_name )
	{
		ced_sheet_destroy ( newsheet );
		ced_book_destroy ( newbook );

		return NULL;
	}

	if ( ced_book_add_sheet ( newbook, newsheet, free_sheet_name ) )
	{
		free ( free_sheet_name );
		ced_sheet_destroy ( newsheet );
		ced_book_destroy ( newbook );

		return NULL;
	}

	free ( free_sheet_name );

	return newbook;
}

int cui_file_load (
	CuiFile		* const	file,
	char	const	* const	filename_CONST,
	int		const	force
	)
{
	char		* filename;
	int		cft,
			ftype;
	CuiBook		* new_cubook = NULL;
	CedBook		* newbook = NULL;


	if ( ! file || ! filename_CONST || ! filename_CONST[0] )
	{
		return 1;
	}

	/*
	File must not be a symlink.  We do this to avoid problems when saving
	as we rename onto this input filename which destroys the symlink and
	zero's the linked file.
	*/

	filename = realpath ( filename_CONST, NULL );
	if ( ! filename )
	{
		return 1;
	}

	cft = ced_file_type_detect ( filename, force );
	if ( cft == CED_FILE_DETECT_ERROR )
	{
		free ( filename );

		return 1;
	}

	if ( cft == CED_FILE_DETECT_BOOK )
	{
		newbook = ced_book_load ( filename, &ftype, "ISO-8859-1" );
	}
	else
	{
		newbook = load_sheet_to_book ( cft, filename, &ftype );
	}

	if ( ! newbook )
	{
		free ( filename );
		return 1;
	}

	new_cubook = cui_book_new ();
	if ( ! new_cubook )
	{
		ced_book_destroy ( newbook );

		free ( filename );
		return 1;
	}

	// Everything has worked so lets commit to the file structure
	new_cubook->book = newbook;

	fio_unset_unlock ( file );	// Unlock if currently locked
	cui_book_destroy ( file->cubook );

	file->cubook = new_cubook;
	file->type = ftype;

	free ( file->name );
	file->name = filename;

	// If sheet was loaded populate the active sheet name
	if ( cft != CED_FILE_DETECT_BOOK )
	{
		cui_file_get_sheet ( file );
	}

	fio_checkset_ro ( file );	// Set RO if needed

	return 0;
}

int cui_sheet_check_geometry (
	CedSheet	* const sheet
	)
{
	int		r,
			c;


	if ( ced_sheet_get_geometry ( sheet, &r, &c ) )
	{
		return 2;
	}

	if (	r > CUI_SHEET_MAX_ROW ||
		c > CUI_SHEET_MAX_COL ||
		(r * c) > CUI_SHEET_MAX_AREA
		)
	{
		return 1;
	}

	return 0;
}

static int tsv_sheets_cb (
	CedSheet	* const	sheet,
	char	const	* const	ARG_UNUSED ( name ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	return cui_sheet_check_geometry ( sheet );
}



#define MAX_TEMP_SUFFIX		1000
#define STR_EXTRA		16



int cui_file_save (
	CuiFile		*	const file,
	char		const *	const filename,
	int			const filetype
	)
{
	CedSheet	* sheet = NULL;
	CedBook		* book = NULL;
	char		* tempfile,
			* canon_filename;
	char	const	* suffix = "";
	char	const	* s;
	int		i;
	size_t		size;


	if ( ! file || ! filename || ! filename[0] )
	{
		return -1;
	}

	if (	filetype < CED_FILE_TYPE_NONE ||
		filetype > CED_FILE_TYPE_TOTAL
		)
	{
		// Type has been corrupted beyond defined boundaries
		return -1;
	}

	canon_filename = realpath ( filename, NULL );

	if (	canon_filename					&&
		file->name					&&
		0 == strcmp ( canon_filename, file->name )	&&
		CUI_FILE_LOCK_RO == file->lock_state
		)
	{
		// When this file was loaded it was read only so don't save it

		free ( canon_filename );

		return -1;
	}

	free ( canon_filename );
	canon_filename = NULL;


	/*
	Check the sheet geometry now as we are saving to a TSV format which may
	struggle with very large sheets.
	*/

	i = 0;
	switch ( filetype )
	{
	case CED_FILE_TYPE_TSV_CONTENT:
	case CED_FILE_TYPE_TSV_CONTENT_GZ:
	case CED_FILE_TYPE_TSV_CONTENT_NOQ:
	case CED_FILE_TYPE_TSV_VALUE:
	case CED_FILE_TYPE_TSV_VALUE_GZ:
	case CED_FILE_TYPE_TSV_VALUE_NOQ:
	case CED_FILE_TYPE_CSV_CONTENT:
	case CED_FILE_TYPE_CSV_CONTENT_NOQ:
	case CED_FILE_TYPE_CSV_VALUE:
	case CED_FILE_TYPE_CSV_VALUE_NOQ:
		sheet = cui_file_get_sheet ( file );
		if ( ! sheet )
		{
			return -1;
		}

		i = cui_sheet_check_geometry ( sheet );
		break;

	case CED_FILE_TYPE_LEDGER:
	case CED_FILE_TYPE_LEDGER_GZ:
	case CED_FILE_TYPE_LEDGER_VAL:
	case CED_FILE_TYPE_LEDGER_VAL_GZ:
		sheet = cui_file_get_sheet ( file );
		if ( ! sheet )
		{
			return -1;
		}
		break;

	case CED_FILE_TYPE_TSV_BOOK:
	case CED_FILE_TYPE_TSV_VAL_BOOK:
		book = file->cubook->book;
		if ( ! book )
		{
			return -1;
		}
		i = ced_book_scan ( book, tsv_sheets_cb, NULL );
		break;

	case CED_FILE_TYPE_LEDGER_BOOK:
	case CED_FILE_TYPE_LEDGER_VAL_BOOK:
		book = file->cubook->book;
		if ( ! book )
		{
			return -1;
		}
		break;

	default:
		// Bogus types
		return -1;
	}

	if ( i )
	{
		// Sheet too large
		return 1;
	}

	size = strlen ( filename );

	// Check overflow
	if ( size > (SIZE_MAX - STR_EXTRA) )
	{
		return -1;
	}

	tempfile = (char *)calloc ( size + STR_EXTRA, 1 );

	if ( ! tempfile )
	{
		goto error;
	}

	if ( book )
	{
		// Add a .zip suffix if no file type specified by user.
		// This trick ensures that the internal zip filename isn't
		// filename_000.

		s = strrchr ( filename, MTKIT_DIR_SEP );
		if ( ! s )
		{
			s = filename;
		}
		else
		{
			s++;
		}

		if ( ! strrchr ( s, '.' ) )
		{
			suffix = ".zip";
		}
	}

	// Create new temp filename that is available in form "filename_000"
	for ( i = 0; i < MAX_TEMP_SUFFIX; i++ )
	{
		snprintf ( tempfile, size + STR_EXTRA, "%s%s_%03i",
			filename, suffix, i );

		if ( ! mtkit_file_readable ( tempfile ) )
		{
			break;
		}
	}

	// No free filename found
	if ( i >= MAX_TEMP_SUFFIX )
	{
		goto error;
	}

	if ( book )
	{
		if ( ced_book_save ( book, tempfile, filetype ) )
		{
			goto error;
		}
	}
	else if ( sheet )
	{
		if ( ced_sheet_save ( sheet, tempfile, filetype ) )
		{
			goto error;
		}
	}

	{
		int		old_lock_state = file->lock_state;


		fio_unset_unlock ( file );

		// mtkit_file_writable is required to avoid removing a write
		// protected file.

		if (	! mtkit_file_writable ( filename ) ||
			rename ( tempfile, filename )
			)
		{
			if ( CUI_FILE_LOCK_RWL == old_lock_state )
			{
				// Try to regain the RW lock we gave away
				fio_try_lock ( file );
			}
			else
			{
				// Revert to old UNSET or RO state
				file->lock_state = old_lock_state;
			}

			goto error;
		}
	}

	free ( tempfile );
	tempfile = NULL;

	// Everything worked so change state values
	file->type = filetype;
	if ( filename != file->name )
	{
		canon_filename = realpath ( filename, NULL );

		if ( canon_filename )
		{
			mtkit_strfreedup ( &file->name, canon_filename );
			free ( canon_filename );
		}
		else
		{
			// This shouldn't really happen, but just in case ...
			mtkit_strfreedup ( &file->name, filename );
		}
	}

	return 0;			// Success

error:
	if ( tempfile )
	{
		remove ( tempfile );
		free ( tempfile );
		tempfile = NULL;
	}

	return -1;
}

int cui_file_set_lock (
	CuiFile		*	const	file,
	int			const	new_lock
	)
{
	if (	! file				||
		! file->name			||
		new_lock == file->lock_state	||
		CUI_FILE_LOCK_RO == file->lock_state
		)
	{
		return 0;		// Nothing to do, or RO
	}

	switch ( new_lock )
	{
	case CUI_FILE_LOCK_RW:		// Currently RWL
		fio_unset_unlock ( file );

		return 0;		// Successfully changed

	case CUI_FILE_LOCK_RWL:		// Currently RW
		fio_try_lock ( file );

		if ( CUI_FILE_LOCK_RWL != file->lock_state )
		{
			return 1;	// Unable to set as requested
		}

		return 0;		// Successfully changed

	case CUI_FILE_LOCK_RO:		// Currently RW or RWL
		fio_unset_unlock ( file );
		file->lock_state = CUI_FILE_LOCK_RO;

		return 0;		// Successfully changed
	}

	return 1;			// Unable to set as requested
}

int cui_file_sheet_add (
	CuiFile		* const	file
	)
{
	CedSheet	* newsheet = NULL;
	char		page_name[256];
	int		i,
			res = 1;


	if (	! file			||
		! file->cubook		||
		! file->cubook->book
		)
	{
		goto error;
	}

	for ( i = 1; i < CUI_SHEET_MAX_NAME; i++ )
	{
		snprintf ( page_name, sizeof ( page_name ), "Sheet %i", i );

		if ( ! ced_book_get_sheet ( file->cubook->book, page_name ) )
		{
			break;
		}

		// Keep looping until we find a valid new name
	}

	if (	i == CUI_SHEET_MAX_NAME ||
		! ( newsheet = ced_sheet_new () ) )
	{
		goto error;
	}

	res = cui_book_add_sheet ( file->cubook, newsheet, page_name );
	if (	res == CUI_ERROR_UNDO_OP ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
	// We only report an error if the operation was not carried out.
		goto error;
	}

	mtkit_strfreedup ( &file->cubook->book->prefs.active_sheet,
		page_name );

	return 0;

error:
	ced_sheet_destroy ( newsheet );

	return res;
}

static int cui_file_sheet_new (
	CuiFile		* const	file,
	char	const	* const	sheet_name
	)
{
	CedSheet	* sheet;


	if ( ced_book_get_sheet ( file->cubook->book, sheet_name ) )
	{
		fprintf ( stderr, "cui_file_sheet_new: Sheet already exists.\n"
			);

		goto fail;
	}

	sheet = ced_sheet_new ();
	if ( ! sheet )
	{
		goto fail;
	}

	if ( ced_book_add_sheet ( file->cubook->book, sheet, sheet_name ) )
	{
		ced_sheet_destroy ( sheet );

		goto fail;
	}

	mtkit_strfreedup ( &file->cubook->book->prefs.active_sheet,
		sheet_name );

	return 0;

fail:
	fprintf ( stderr,
		"cui_file_sheet_new: Unable to create new sheet '%s'.\n",
		sheet_name );

	return 1;
}

int cui_file_book_new (
	CuiFile		* const	file
	)
{
	if ( ! file )
	{
		return 1;
	}

	fio_unset_unlock ( file );
	cui_book_destroy ( file->cubook );

	file->cubook = cui_book_new ();
	if ( ! file->cubook )
	{
		goto fail;
	}

	file->cubook->book = ced_book_new ();
	if ( ! file->cubook->book )
	{
		goto fail;
	}

	if ( cui_file_sheet_new ( file, "Sheet 1" ) )
	{
		goto fail;
	}

	mtkit_strfreedup ( &file->name, NULL );
	file->type = CED_FILE_TYPE_TSV_VAL_BOOK;

	return 0;

fail:
	fprintf ( stderr, "cui_file_book_new: Unable to create new book.\n" );

	return 1;
}

CedSheet * cui_file_get_sheet (
	CuiFile		* const	file
	)
{
	CedBook		* book;
	CedSheet	* sheet;


	if ( ! file )
	{
		return NULL;
	}

	book = file->cubook->book;
	sheet = ced_book_get_sheet ( book, book->prefs.active_sheet );

	if ( ! sheet )
	{
		if ( book->prefs.active_sheet )
		{
			// No valid sheet found so flush current dangling
			// reference.

			mtkit_strfreedup ( &book->prefs.active_sheet, NULL );
		}

		if ( book->sheets->root )
		{
			// Current name is empty, but a sheet exists so pick
			// root sheet name.

			mtkit_strfreedup ( &book->prefs.active_sheet,
				(char const *)book->sheets->root->key );
			sheet = (CedSheet *)book->sheets->root->data;
		}
	}

	return sheet;
}

static int find_graph (
	CedBook		* const	ARG_UNUSED ( book ),
	char	const	* const	graph_name,
	CedBookFile	* const	ARG_UNUSED ( bookfile ),
	void		* const	user_data
	)
{
	char	const	** name = (char const **)user_data;


	name[0] = graph_name;

	return 1;			// Stop
}

CedBookFile * cui_file_get_graph (
	CuiFile		* const	file
	)
{
	CedBook		* book;
	CedBookFile	* graph;


	if ( ! file )
	{
		return NULL;
	}

	book = file->cubook->book;
	graph = cui_graph_get ( book, book->prefs.active_graph );

	if ( ! graph )
	{
		char	const	* name = NULL;


		if ( book->prefs.active_graph )
		{
			// No valid graph found so flush current dangling
			// reference.

			mtkit_strfreedup ( &book->prefs.active_graph, NULL );
		}

		// Use the first graph (if there is one)

		cui_graph_scan ( book, find_graph, &name );
		graph = cui_graph_get ( book, name );
		mtkit_strfreedup ( &book->prefs.active_graph, name );
	}

	return graph;
}

static int scan_2dyear (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	ced_cell_set_2dyear ( cell, ( (int *)user_data )[0] );

	return 0;			// Continue
}

int cui_sheet_2dyear (
	CuiFile		* const	file,
	int		year_start
	)
{
	int		res;
	CuiClip		* clip;


	if (	! file ||
		year_start < MTKIT_DDT_MIN_DATE_YEAR ||
		year_start > MTKIT_DDT_MAX_DATE_YEAR )
	{
		return 1;
	}

	clip = cui_clip_new ();
	if ( ! clip )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_clip_copy ( file, clip ) )
	{
		cui_clip_free ( clip );

		return CUI_ERROR_NO_CHANGES;
	}

	ced_sheet_scan_area ( clip->sheet, 1, 1, 0, 0, scan_2dyear,
		(void *)&year_start );

	res = cui_clip_paste ( file, clip, 0 );
	cui_clip_free ( clip );

	return res;
}

