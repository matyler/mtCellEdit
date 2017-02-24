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



static char const * get_temp_clip_filename ( void )
{
	static char	temp_clip_filename [ 2048 ];


	if ( ! temp_clip_filename[0] )
	{
		// First time usage

		snprintf ( temp_clip_filename, sizeof ( temp_clip_filename ),
			"%s/.cache", mtkit_file_home () );

		mkdir ( temp_clip_filename, S_IRWXU | S_IRWXG | S_IRWXO );

		mtkit_strnncat ( temp_clip_filename, "/" APP_NAME,
			sizeof ( temp_clip_filename ) );

		mkdir ( temp_clip_filename, S_IRWXU | S_IRWXG | S_IRWXO );

		mtkit_strnncat ( temp_clip_filename, "/clipboard.ledger.zip",
			sizeof ( temp_clip_filename ) );
	}

	return temp_clip_filename;
}


int cui_clip_set_timestamp (
	CuiClip		* const	clipboard,
	char	const	* const	txt
	)
{
	if ( ! clipboard )
	{
		return 1;
	}


	memset ( clipboard->timestamp, 0, CUI_CLIPBOARD_TIMESTAMP_SIZE );

	if ( txt )
	{
		mtkit_strnncat ( clipboard->timestamp, txt,
			CUI_CLIPBOARD_TIMESTAMP_SIZE );
	}
	else
	{
		snprintf ( clipboard->timestamp, CUI_CLIPBOARD_TIMESTAMP_SIZE,
			"%u", (unsigned int)time ( NULL ) );
	}

	clipboard->timestamp[CUI_CLIPBOARD_TIMESTAMP_SIZE - 1] = 0;

	return 0;
}

CuiClip * cui_clip_new ( void )
{
	CuiClip		* clipboard;


	clipboard = (CuiClip *)calloc ( sizeof ( CuiClip ), 1 );
	if ( ! clipboard )
	{
		return NULL;
	}

	return clipboard;
}

int cui_clip_free (
	CuiClip		* const	clipboard
	)
{
	cui_clip_flush ( clipboard );
	free ( clipboard );

	return 0;
}

int cui_clip_flush (
	CuiClip		* const	clipboard
	)
{
	if ( ! clipboard )
	{
		return 1;
	}

	if ( clipboard->temp )
	{
		// Remove the temp file

		remove ( get_temp_clip_filename () );
		clipboard->temp = 0;
	}

	if ( clipboard->sheet )
	{
		ced_sheet_destroy ( clipboard->sheet );
		clipboard->sheet = NULL;
	}

	free ( clipboard->tsv );
	clipboard->tsv = NULL;

	free ( clipboard->ced );
	clipboard->ced = NULL;

	return 0;
}

int cui_clip_import_text (
	CuiClip		* const	clipboard,
	char		* const	text
	)
{
	int		rtot,
			ctot;
	size_t		txt_len;
	CedSheet	* newsheet;


	if ( ! clipboard || ! text )
	{
		return 1;
	}

	txt_len = strlen ( text );
	newsheet = ced_sheet_load_mem ( text, txt_len + 1, "ISO-8859-1", NULL );

	if ( newsheet )
	{
		rtot = ctot = 0;
		if ( ced_sheet_tsvmem_geometry ( text, txt_len + 1, &rtot,
			&ctot ) )
		{
			ced_sheet_get_geometry ( newsheet, &rtot, &ctot );
		}

		if ( rtot < 1 )
		{
			rtot = 1;
		}

		if ( ctot < 1 )
		{
			ctot = 1;
		}

		if ( cui_clip_flush ( clipboard ) || clipboard->sheet )
		{
			// Should never happen

			return 1;
		}

		clipboard->sheet = newsheet;
		clipboard->rows = rtot;
		clipboard->cols = ctot;
	}

	return 0;
}

int cui_clip_export_text (
	CuiClip		* const	clipboard
	)
{
	if ( ! clipboard || ! clipboard->sheet )
	{
		return 1;
	}

	if ( ! clipboard->tsv )
	{
		mtFile		* mtfp;
		void		* buf;
		size_t		sl;


		mtfp = ced_sheet_save_mem ( clipboard->sheet,
			CED_FILE_TYPE_TSV_CONTENT );
		if ( ! mtfp )
		{
			return 1;
		}

		if (	mtkit_file_write ( mtfp, "", 1 )	||
			mtkit_file_get_mem ( mtfp, &buf, NULL )
			)
		{
			mtkit_file_close ( mtfp );

			return 1;
		}

		clipboard->tsv = strdup ( (char const *)buf );
		mtkit_file_close ( mtfp );

		// Remove trailing newline (convenient for single cell pastes)
		sl = strlen ( clipboard->tsv );
		if ( sl > 0 && clipboard->tsv[ sl - 1 ] == '\n' )
		{
			clipboard->tsv[ sl - 1 ] = 0;
		}
	}

	return 0;
}

int cui_clip_save_temp_file (
	CuiClip		* const	clipboard,
	char	const	* const	filename
	)
{
	CedBook		* book;


	if (	! clipboard ||
		! clipboard->sheet ||
		! filename
		)
	{
		return 1;
	}

	// Save the current clipboard as a CED ZIP file with the cursor position
	// denoting the geometry of the paste.
	book = ced_book_new ();
	if ( ! book )
	{
		return 1;
	}

	if ( ced_book_add_sheet ( book, clipboard->sheet,
		CUI_CLIPBOARD_SHEET_NAME ) )
	{
		ced_book_destroy ( book );

		return 1;
	}

	clipboard->sheet->prefs.cursor_r1 = clipboard->rows;
	clipboard->sheet->prefs.cursor_c1 = clipboard->cols;

	if ( ced_book_save ( book, filename, CED_FILE_TYPE_LEDGER_BOOK ) )
	{
		ced_book_detach_sheet ( clipboard->sheet );
		ced_book_destroy ( book );

		return 1;
	}

	ced_book_detach_sheet ( clipboard->sheet );
	ced_book_destroy ( book );

	return 0;
}

int cui_clip_save_temp (
	CuiClip		* const	clipboard
	)
{
	if ( ! clipboard )
	{
		return 1;
	}

	// Only save to temp if we haven't done so already
	if ( clipboard->temp == 0 )
	{
		if ( cui_clip_save_temp_file ( clipboard,
			get_temp_clip_filename () ) )
		{
			return 1;
		}

		clipboard->temp = 1;
		cui_clip_set_timestamp ( clipboard, NULL );
	}

	return 0;
}

int cui_clip_load_temp_file (
	CuiClip		* const	clipboard,
	char	const	* const	filename
	)
{
	CedBook		* book;
	CedSheet	* sheet;


	if ( ! clipboard || ! filename )
	{
		return 1;
	}

	book = ced_book_load ( filename, NULL, "ISO-8859-1" );
	if ( ! book )
	{
		return 1;		// No file found - error!
	}

	// Extract "clipboard" sheet and keep as new clipboard
	sheet = ced_book_get_sheet ( book, CUI_CLIPBOARD_SHEET_NAME );
	if ( ! sheet || ced_book_detach_sheet ( sheet ) )
	{
		ced_book_destroy ( book );

		return 1;		// No clipboard sheet found/extracted
	}

	ced_book_destroy ( book );

	ced_sheet_destroy ( clipboard->sheet );
	clipboard->sheet = sheet;
	clipboard->rows = sheet->prefs.cursor_r1;
	clipboard->cols = sheet->prefs.cursor_c1;

	return 0;
}

int cui_clip_load_temp (
	CuiClip		* const	clipboard
	)
{
	return cui_clip_load_temp_file ( clipboard, get_temp_clip_filename () );
}

int cui_clip_paste (
	CuiFile		* const	file,
	CuiClip		* const	clipboard,
	int		const	paste_mode
	)
{
	int		r1,
			c1,
			r2,
			c2,
			rtot,
			ctot;
	CedSheet	* sheet;


	sheet = cui_file_get_sheet ( file );

	if (	! sheet ||
		! clipboard ||
		! clipboard->sheet
		)
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	rtot = r2 - r1 + 1;
	ctot = c2 - c1 + 1;

	/*
	If selected area rows/cols are less than the rows/cols of the paste then
	the selection is exapnded to use the full clipboard width/height.
	*/

	if ( rtot < clipboard->rows )
	{
		rtot = clipboard->rows;
	}

	if ( ctot < clipboard->cols )
	{
		ctot = clipboard->cols;
	}

	return cui_sheet_paste_area ( file->cubook, sheet, clipboard->sheet,
		r1, c1, rtot, ctot, clipboard->rows, clipboard->cols,
		paste_mode );
}

int cui_clip_copy (
	CuiFile		* const	file,
	CuiClip		* const	clipboard
	)
{
	CedSheet	* sheet,
			* new_clipsheet;
	int		rtot,
			ctot;


	if ( ! file || ! clipboard )
	{
		return 1;
	}

	sheet = cui_file_get_sheet ( file );
	if ( ! sheet )
	{
		return 1;
	}

	new_clipsheet = ced_sheet_copy_selection ( sheet, &rtot, &ctot );
	if ( ! new_clipsheet )
	{
		return 1;
	}

	if ( cui_clip_flush ( clipboard ) )
	{
		ced_sheet_destroy ( new_clipsheet );

		return 1;
	}

	clipboard->sheet = new_clipsheet;
	clipboard->rows = rtot;
	clipboard->cols = ctot;

	return 0;		// Success
}
