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

#include "be.h"



#define SNIP_LEN	5



static void trim_string (
	char		* const	buf
	)
{
	static char const	* snip = " ... ";	// length = SNIP_LEN
	char			* spa,
				* spb;
	int			res,
				lim_edge,
				lim_tot,
				slen
				;


	lim_tot = prefs_get_int ( GUI_INIFILE_RECENT_FILENAME_LEN );

	if ( lim_tot < 50 || lim_tot > 500 )
	{
		lim_tot = 80;
	}

	lim_edge = lim_tot / 2 - SNIP_LEN;
	slen = mtkit_utf8_len ( (unsigned char *)buf, 0 );

	if ( slen <= lim_tot )
	{
		// String is already short enough
		return;
	}

	res = mtkit_utf8_offset ( (unsigned char *)buf, lim_edge );

	if ( res < 0 )
	{
		return;
	}

	// Place snip text here later
	spa = buf + res;

	res = mtkit_utf8_offset ( (unsigned char *)spa, slen - 2 * lim_edge );

	if ( res < 0 )
	{
		return;
	}

	// Points to the last lim_edge chars in buf string
	spb = spa + res;

	// Move right hand side of snip: all chars and NUL terminator
	memmove ( spa + SNIP_LEN, spb, strlen ( spb ) + 1 );

	// Insert snip text
	memcpy ( spa, snip, SNIP_LEN );
}

char const * be_get_recent_filename (
	int	const	i
	)
{
	char		txt[64];


	snprintf ( txt, sizeof ( txt ), GUI_INIFILE_MAIN_FILE"%i", i );

	return prefs_get_string ( txt );
}

int be_snip_filename (
	int		const	num,
	char		* const	buf,
	size_t		const	buflen
	)
{
	char	const	* recent;
	char		* tmp;


	recent = be_get_recent_filename ( num );

	if ( ! recent || strlen ( recent ) < 2 )
	{
		return 1;		// Buffer not filled
	}

	tmp = mtkit_utf8_from_cstring ( recent );
	if ( ! tmp )
	{
		return 1;		// Buffer not filled
	}

	mtkit_strnncpy ( buf, tmp, buflen );

	free ( tmp );

	trim_string ( buf );

	return 0;			// Buffer filled
}

int be_titlebar_text (
	CuiFile		* const	file,
	char		* const	buf,
	size_t		const	buflen,
	int		const	changed
	)
{
	char	const	* fio;
	char		* tmp = NULL,
			* fn,
			* fd = NULL;


	if ( file->name )
	{
		tmp = mtkit_utf8_from_cstring ( file->name );
	}

	if ( tmp )
	{
		fn = strrchr ( tmp, MTKIT_DIR_SEP );

		if ( ! fn )
		{
			fn = tmp;
		}
		else
		{
			fn ++;
		}

		fd = strdup ( tmp );
	}
	else
	{
		fn = _("Untitled");
	}

	mtkit_strnncpy ( buf, fn, buflen );

	if ( changed )
	{
		mtkit_strnncat ( buf, _(" (Modified)"), buflen );
	}

	if ( ! file->name )
	{
		fio = " {}";
	}
	else if ( file->lock_state == CUI_FILE_LOCK_RO )
	{
		fio = " {RO}";
	}
	else if ( file->lock_state == CUI_FILE_LOCK_RWL )
	{
		fio = " {RWL}";
	}
	else	// CUI_FILE_LOCK_RW
	{
		fio = " {RW}";
	}

	mtkit_strnncat ( buf, fio, buflen );
	mtkit_strnncat ( buf, " [", buflen );
	mtkit_strnncat ( buf, ced_file_type_text ( file->type ), buflen );
	mtkit_strnncat ( buf, "] ", buflen );

	if ( fd )
	{
		fn = strrchr ( fd, MTKIT_DIR_SEP );
		if ( fn )
		{
			fn[1] = 0;
		}

		mtkit_strnncat ( buf, " - ", buflen );
		mtkit_strnncat ( buf, fd, buflen );

		free ( fd );
	}

	mtkit_strnncat ( buf, "    ", buflen );
	mtkit_strnncat ( buf, VERSION, buflen );

	free ( tmp );

	if (	file->type == CED_FILE_TYPE_TSV_BOOK		||
		file->type == CED_FILE_TYPE_TSV_VAL_BOOK	||
		file->type == CED_FILE_TYPE_LEDGER_BOOK		||
		file->type == CED_FILE_TYPE_LEDGER_VAL_BOOK
		)
	{
		return 1;
	}

	return 0;
}

void be_remember_last_dir (
	char	const	* const	filename
	)
{
	char		* frst = strdup ( filename );


	if ( ! frst )
	{
		prefs_set_string ( GUI_INIFILE_LAST_DIR, filename );

		return;
	}


	char		* c = strrchr ( frst, MTKIT_DIR_SEP );


	if ( c )
	{
		c[0] = 0;		// Strip off filename
		prefs_set_string ( GUI_INIFILE_LAST_DIR, frst );
	}

	free ( frst );
}

int be_register_project (
	CuiFile		* const	file
	)
{
	char		buf [ 64 ];
	char	const	* c;
	int		i,
			f;


	if ( ! file->name )
	{
		return 1;
	}

	be_remember_last_dir ( file->name );

	/*
	Is it already in used file list?  If so shift relevant filenames down
	and put at top.
	*/

	i = 1;
	f = 0;

	while ( i < RECENT_MENU_TOTAL && f == 0 )
	{
		c = be_get_recent_filename ( i );

		if ( strcmp ( file->name, c ) == 0 )
		{
			f = 1;		// Filename found in list
		}
		else
		{
			i++;
		}
	}

	// If file is already most recent, do nothing
	if ( i > 1 )
	{
		while ( i > 1 )
		{
			snprintf ( buf, sizeof ( buf ),
				GUI_INIFILE_MAIN_FILE"%i", i );

			c = be_get_recent_filename ( i - 1 );

			prefs_set_string ( buf, c );

			i--;
		}

		prefs_set_string ( GUI_INIFILE_MAIN_FILE"1", file->name );
	}

	return 0;
}

int be_cedrender_set_font_width (
	CuiRender	* const	render
	)
{
	if ( ! render->font )
	{
		return 1;
	}

	render->font_width = cui_font_get_width ( render->font );

	return 0;		// Success
}

void be_cedrender_set_header_width (
	CuiRender	* const	render,
	int			max
	)
{
	int		digits;


	for ( digits = 3; max >= 10; digits ++ , max /= 10 )
	{
	}

	if ( digits < 5 )
	{
		digits = 5;
	}

	render->row_header_width = digits * render->font_width;
}

void be_sheet_ref (
	CedSheet	* const	sheet,
	char		* const	buf,
	size_t		const	buflen
	)
{
	buf[0] = 0;

	if ( ! sheet )
	{
		return;
	}

	if (	sheet->prefs.cursor_c1 == sheet->prefs.cursor_c2 &&
		sheet->prefs.cursor_r1 == sheet->prefs.cursor_r2
		)
	{
		snprintf ( buf, buflen, "r%ic%i",
			sheet->prefs.cursor_r1, sheet->prefs.cursor_c1 );
	}
	else
	{
		int		r1,
				c1,
				r2,
				c2;


		ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

		snprintf ( buf, buflen, "r%ic%i:r%ic%i", r1, c1, r2, c2 );
	}
}



#define QUICKSUM_FUNCS	9



void be_quicksum_label (
	CedSheet	* const	sheet,
	char		* const	buf,
	size_t		const	buflen,
	int		const	operation
	)
{
	if ( ! buf || buflen < 2 )
	{
		return;
	}

	if (	sheet		&&
		operation >= 0	&&
		operation < QUICKSUM_FUNCS
		)
	{
		CedParser	parser;
		char	const	* funcs [ QUICKSUM_FUNCS ] =
				{	"none",
					"sum",
					"min",
					"max",
					"",
					"average",
					"median",
					"count",
					"counta"
				};
		int		r1,
				c1,
				r2,
				c2;


		ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

		if ( operation == 4 )
		{
			snprintf ( buf, buflen,
			"max( r%ic%i : r%ic%i ) - min( r%ic%i : r%ic%i )",
				r1, c1, r2, c2, r1, c1, r2, c2 );
		}
		else
		{
			snprintf ( buf, buflen, "%s( r%ic%i : r%ic%i )",
				funcs [ operation ], r1, c1, r2, c2 );
		}

		parser = ced_sheet_parse_text ( sheet, 1, 1, buf, NULL );

		if (	parser.ced_errno ||
			( parser.flag & CED_PARSER_FLAG_ERROR )
			)
		{
			snprintf ( buf, buflen, "err:%i", parser.ced_errno );
		}
		else
		{
			snprintf ( buf, buflen, "%.10g", parser.data );
		}

	}
	else
	{
		snprintf ( buf, buflen, "0" );
	}
}

char const * be_get_error_update_text (
	int		const	error,
	char		* const	buf,
	size_t		const	buflen
	)
{
	if ( error == 0 )
	{
		return NULL;
	}

	static char const * mes[] = { "?",
		_("Error during operation."),
		_("Unable to begin operation due to problem with undo system."),
		_("Undo history lost."),
		_("Undo history lost.  Possible data corruption."),
		_("cell locked.  Operation aborted."),
		_("Sheet locked.  Operation aborted.")
				};
	char	const	* msg;
	int		mi;


	mi = -error;

	if ( mi < 0 || mi >= (int)( sizeof ( mes ) / sizeof ( mes[0] ) ) )
	{
		mi = 0;
	}

	msg = mes[mi];

	if ( error == CUI_ERROR_LOCKED_CELL )
	{
		snprintf ( buf, buflen, "%s %s", cui_error_str (), msg );

		msg = buf;
	}

	return msg;
}

void be_update_file_to_book (
	CuiFile		* const	file
	)
{
	int		newtype;


	switch ( file->type )
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
		newtype = CED_FILE_TYPE_TSV_VAL_BOOK;
		break;

	case CED_FILE_TYPE_LEDGER:
	case CED_FILE_TYPE_LEDGER_GZ:
	case CED_FILE_TYPE_LEDGER_VAL:
	case CED_FILE_TYPE_LEDGER_VAL_GZ:
		newtype = CED_FILE_TYPE_LEDGER_VAL_BOOK;
		break;

	default:
		newtype = CED_FILE_TYPE_NONE;
	}

	if ( newtype != CED_FILE_TYPE_NONE )
	{
		file->type = newtype;
		mtkit_strfreedup ( &file->name, NULL );
	}
}

int be_fix_years (
	CuiFile		* const	file
	)
{
	int		year;


	year = prefs_get_int ( GUI_INIFILE_2DYEAR_START );

	if ( year < 0 )
	{
		struct tm	* tm_now;
		time_t		now;


		now = time ( NULL );
		tm_now = localtime ( &now );

		year = 1900 + tm_now->tm_year - 50;
		if (	year < MTKIT_DDT_MIN_DATE_YEAR ||
			year > MTKIT_DDT_MAX_DATE_YEAR
			)
		{
			return 2;
		}
	}

	return cui_sheet_2dyear ( file, year );
}

int be_export_sheet (
	CedSheet	* const	src,
	char	const	* const	filename,
	int		const	filetype
	)
{
	if (	filetype == CED_FILE_TYPE_TSV_BOOK	||
		filetype == CED_FILE_TYPE_TSV_VAL_BOOK	||
		filetype == CED_FILE_TYPE_LEDGER_BOOK	||
		filetype == CED_FILE_TYPE_LEDGER_VAL_BOOK
		)
	{
		int		res;
		CedBook		* book;
		CedSheet	* sheet;


		book = ced_book_new ();
		if ( ! book )
		{
			return 1;
		}

		sheet = ced_sheet_duplicate ( src );
		if ( ! sheet )
		{
			ced_book_destroy ( book );

			return 1;
		}

		if ( ced_book_add_sheet ( book, sheet,
			(char *)src->book_tnode->key ) )
		{
			ced_sheet_destroy ( sheet );
			ced_book_destroy ( book );

			return 1;
		}

		res = ced_book_save ( book, filename, filetype );
		ced_book_destroy ( book );

		if ( res )
		{
			return 1;
		}
	}
	else
	{
		if ( ced_sheet_save ( src, filename, filetype ) )
		{
			return 1;
		}
	}

	return 0;			// Success
}

