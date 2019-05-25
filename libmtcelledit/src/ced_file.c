/*
	Copyright (C) 2008-2018 Mark Tyler

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



#define TEXT_LEN		1024
#define LEDGER_HEADER_SIZE	20



static char const * const ledger_header = "ledger\t==!\"$CED\tTSV\n";



char const * ced_file_type_text (
	int		const	filetype
	)
{
	char const * const txt[CED_FILE_TYPE_TOTAL] = {
		"?",
		"TSV content",
		"TSV.gz content",
		"TSV content (no quotes)",
		"TSV value",
		"TSV.gz value",
		"TSV value (no quotes)",
		"CSV content",
		"CSV content (no quotes)",
		"CSV value",
		"CSV value (no quotes)",
		"Ledger",
		"Ledger.gz",
		"Ledger value",
		"Ledger.gz value",
		"TSV Book",
		"TSV Book (+value)",
		"Ledger Book",
		"Ledger Book (+value)",
		"Output TSV",
		"Output TSV (quoted)",
		"Output HTML"
		};


	if (	filetype <= CED_FILE_TYPE_NONE ||
		filetype >= CED_FILE_TYPE_TOTAL
		)
	{
		return txt[0];
	}

	return txt [ filetype ];
}

int ced_file_type_class (
	int		const	type
	)
{
	if (	type <= CED_FILE_TYPE_NONE ||
		type >= CED_FILE_TYPE_TOTAL
		)
	{
		return 0;
	}

	if ( type <= CED_FILE_TYPE_TEXT )
	{
		return 1;
	}

	if ( type <= CED_FILE_TYPE_BOOK )
	{
		return 2;
	}

	return 3;
}

static CedSheet * stat_sheet_load_tsv_mem (
	char		* const	mem,
	size_t		const	bytes
	)
{
	CedSheet * const sheet = ced_sheet_new ();
	if ( ! sheet )
	{
		return NULL;
	}

	char const * const end = mem + bytes;
	int row = 1;
	int col = 1;
	char * s1 = mem;

	while ( s1 < end )
	{
		char * s2 = s1;

		while (	s2[0] != '\t' &&
			s2[0] != '\n' &&
			s2[0] != '\r' &&
			s2[0] != '\0'
			)
		{
			s2++;
		}

		char const c = s2[0];
		if ( s1 != s2 )
		{
			// We have data to add
			s2[0] = 0;

			if ( ! ced_sheet_set_cell ( sheet, row, col, s1 ) )
			{
				ced_sheet_destroy ( sheet );

				return NULL;
			}

			s2[0] = c;
		}

		col++;

		if ( c == '\r' && s2[1] == '\n' )
		{
			// DOS newline
			s2++;
		}

		if ( c == '\n' || c == '\r' )
		{
			// Row delimiter
			col = 1;
			row++;
		}

		s1 = s2 + 1;
	}

	return sheet;
}


#define OBTAIN_NEXT_INT( VAR ) \
		s2 = strchr ( s1, '\t' ); \
		if ( ! s2 ) \
		{ \
			break; \
		} \
		s2[0] = 0; \
		if ( mtkit_strtoi ( s1, &VAR, NULL, 1 ) || VAR < 1 ) \
		{ \
			break; \
		} \
		s1 = s2 + 1; \
		if ( s1 >= end ) \
		{ \
			break; \
		}


static CedSheet * stat_sheet_load_ledger_mem (
	char		* const	mem,
	size_t		const	bytes
	)
{
	int		row, col;
	char		* const end = mem + bytes;
	char		* s1, * s2;

	CedSheet	* sheet = ced_sheet_new ();
	if ( ! sheet )
	{
		return NULL;
	}

	s1 = mem + LEDGER_HEADER_SIZE;

	while ( s1 < end )
	{
		// Get Row
		OBTAIN_NEXT_INT ( row )

		// Get Column
		OBTAIN_NEXT_INT ( col )

		s2 = strchr ( s1, '\n' );
		if ( s2 )
		{
			s2[0] = 0;

			if ( s2[-1] == '\r' )
			{
				// DOS newline
				s2[-1] = 0;
			}
		}
		else
		{
			s2 = end;
		}

		if ( ! ced_sheet_set_cell ( sheet, row, col, s1 ) )
		{
			ced_sheet_destroy ( sheet );

			return NULL;
		}

		s1 = s2 + 1;
	}

	return sheet;
}

CedSheet * ced_sheet_load_mem (
	char		*	mem,
	size_t			bytes,
	char	const	* const	encoding,
	int		* const	filetype
	)
{
	char		* freestart = NULL;
	CedSheet	* sheet = NULL;


	if ( bytes > 0 && encoding )
	{
		char	* start;


		if ( ! mtkit_utf8_string_legal ( (unsigned char *)mem, bytes ) )
		{
			if ( mtkit_string_encoding_conversion ( mem, encoding,
				&start, "UTF-8" ) )
			{
				return NULL;
			}

			bytes = strlen ( start ) + 1;

			freestart = start;
			mem = start;
		}
	}

	if (	bytes >= LEDGER_HEADER_SIZE &&
		! memcmp ( mem, ledger_header, LEDGER_HEADER_SIZE )
		)
	{
		sheet = stat_sheet_load_ledger_mem ( mem, bytes );
		if ( sheet && filetype )
		{
			filetype[0] = CED_FILE_TYPE_LEDGER;
		}
	}

	if ( ! sheet )
	{
		// If ledger load fails, fallback to basic TSV load here
		sheet = stat_sheet_load_tsv_mem ( mem, bytes );
		if ( sheet && filetype )
		{
			filetype[0] = CED_FILE_TYPE_TSV_CONTENT;
		}
	}

	free ( freestart );

	return sheet;
}

CedSheet * ced_sheet_load (
	char	const	* const	filename,
	char	const	* const	encoding,
	int		* const	filetype
	)
{
	int		size,
			load_flag = 0;
	char		* mem;
	CedSheet	* sheet = NULL;
	int		type;


	if ( ! filename || filename[0] == 0 )
	{
		return NULL;
	}

	mem = mtkit_file_load ( filename, &size,
		MTKIT_FILE_ZERO | MTKIT_FILE_GUNZIP, &load_flag );

	if ( ! mem )
	{
		return NULL;
	}

	sheet = ced_sheet_load_mem ( mem, (size_t)size, encoding, &type );
	free ( mem );

	if ( sheet && filetype )
	{
		// Convert type to gz compressed as required
		if ( load_flag & MTKIT_FILE_GUNZIP )
		{
			if ( type == CED_FILE_TYPE_TSV_CONTENT )
			{
				type = CED_FILE_TYPE_TSV_CONTENT_GZ;
			}

			if ( type == CED_FILE_TYPE_LEDGER )
			{
				type = CED_FILE_TYPE_LEDGER_GZ;
			}
		}

		filetype[0] = type;
	}

	return sheet;
}

static CedSheet * stat_sheet_load_csv_mem (
	char		* const	mem,
	size_t		const	bytes
	)
{
	CedSheet	* const sheet = ced_sheet_new ();
	if ( ! sheet )
	{
		return NULL;
	}

	char const * const end = mem + bytes;
	int row = 1;
	int col = 1;
	char * s1 = mem;

	while ( s1 < end )
	{
		char * s2 = s1;

		if ( s2[0] == '"' )
		{
			s1++;
			s2++;
			int quotes = 0;

			// Find closing "
			while (	s2[0] != '\n' &&
				s2[0] != '\r' &&
				s2[0] != '\0' )
			{
				if ( s2[0] == '"' )
				{
					if ( s2[1] == '"' )
					{
					// Double "" is not the end of field
						s2++;
						quotes = 1;
					}
					else
					{
						// Single " is end of field
						s2[0] = 0;
						s2++;

						break;
					}
				}

				s2++;
			}

			if ( quotes )
			{
				char * src, * dest;

				// Drop out the double quotes
				for (	src = s1, dest = s1;
					src < s2;
					src++, dest++ )
				{
					if ( src[0] == '"' && src[1] == '"' )
					{
						src++;
					}

					dest[0] = src[0];
				}

			// We have removed chars so we must terminate here
				dest[0] = 0;
			}

			// Find next , EOR or EOF & put into s2
			// Use the same while loop as when not using
			// a " around the field
		}

		while (	s2[0] != ',' &&
			s2[0] != '\n' &&
			s2[0] != '\r' &&
			s2[0] != '\0'
			)
		{
			s2++;
		}

		char const c = s2[0];
		if ( s1 != s2 )		// We have data to add
		{
			s2[0] = 0;
			if ( ! ced_sheet_set_cell ( sheet, row, col, s1 ) )
			{
				ced_sheet_destroy ( sheet );

				return NULL;
			}

			s2[0] = c;
		}

		col++;

		if ( c == '\r' && s2[1] == '\n' )
		{
			// DOS newline
			s2++;
		}

		if ( c == '\n' || c == '\r' )
		{
			// Row delimiter
			col = 1;
			row++;
		}

		s1 = s2 + 1;
	}

	return sheet;
}

CedSheet * ced_sheet_load_csv_mem (
	char		*	mem,
	size_t			bytes,
	char	const	* const	encoding
	)
{
	char		* freestart = NULL;
	CedSheet	* sheet = NULL;


	if ( encoding && ! mtkit_utf8_string_legal ( (unsigned char *)mem, 0 ) )
	{
		char		* start;


		if ( mtkit_string_encoding_conversion ( mem, encoding, &start,
			"UTF-8" ) )
		{
			free ( freestart );

			return NULL;
		}

		bytes = strlen ( start ) + 1;

		freestart = start;
		mem = start;
	}

	sheet = stat_sheet_load_csv_mem ( mem, bytes );

	free ( freestart );

	return sheet;
}

CedSheet * ced_sheet_load_csv (
	char	const * const	filename,
	char	const * const	encoding
	)
{
	int		size;
	char		* mem;
	CedSheet	* sheet = NULL;


	if ( ! filename || filename[0] == 0 )
	{
		return NULL;
	}

	mem = mtkit_file_load ( filename, &size,
		MTKIT_FILE_ZERO | MTKIT_FILE_GUNZIP, NULL );

	if ( ! mem )
	{
		return NULL;
	}

	sheet = ced_sheet_load_csv_mem ( mem, (size_t)size, encoding );
	free ( mem );

	return sheet;
}



typedef struct
{
	mtFile		* mtfp;

	int		content,
			filetype;

	char		gapchar,
			qchar;

	int		last_row,
			last_col;

	char		txt_buf[TEXT_LEN];
} tsv_state;



static size_t csv_quote_count (		// Count " characters in the string
	char	const *	txt
	)
{
	size_t		i = 0;


	while ( (txt = strchr ( txt, '"' ) ) )
	{
		i ++;
		txt ++;
	}

	return i;
}


/*
Replace " character with "" and create new string if substitutions needed.
Return NULL = none done
*/

static int csv_quote_sub (
	char	const	*	txt,
	char	* *	const	txt_ns
	)
{
	if ( ! txt )
	{
		return 0;
	}

	size_t const quots = csv_quote_count ( txt );

	if ( quots == 0 )
	{
		return 0;
	}

	size_t len = strlen ( txt );

	// Overflow check
	if ( len > (SIZE_MAX - quots - 1) )
	{
		return 1;
	}

	char * const ns = (char *)calloc ( len + quots + 1, 1 );
	if ( ! ns )
	{
		return 1;
	}

	char * c;

	for ( c = ns; txt[0]; c++, txt++ )
	{
		c[0] = txt[0];
		if ( c[0] == '"' )
		{
			c++;
			c[0] = '"';
		}
	}

	txt_ns[0] = ns;

	return 0;
}

static int send_n_chars (
	mtFile		* const	mtfp,
	char		* const	buf,
	char		const	ch,
	int			n
	)
{
	if ( n < 1 )
	{
		return 0;
	}

	memset ( buf, ch, (size_t)( MIN ( n, TEXT_LEN ) ) );

	while ( n > 0 )
	{
		if ( mtkit_file_write ( mtfp, buf, MIN ( n, TEXT_LEN ) ) )
		{
			return 1;
		}

		n = n - TEXT_LEN;
	}

	return 0;
}

static int tsv_save_cb (
	CedSheet	* ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	tsv_state		* const	state = (tsv_state *)user_data;
	char		const	*	celltext = cell->text;
	char			*	newtext = NULL;


	if ( ! celltext )
	{
		// Nothing to save here so continue
		return 0;
	}

	// Is this a new row?
	if ( state->last_row < row )
	{
		state->last_col = 1;

		// Add required number of newlines
		if ( send_n_chars ( state->mtfp, state->txt_buf, '\n',
			row - state->last_row ) )
		{
			return 1;
		}

		state->last_row = row;
	}

	// Add required tabs/commas for new col
	if ( send_n_chars ( state->mtfp, state->txt_buf, state->gapchar,
		col - state->last_col ) )
	{
		return 1;
	}

	state->last_col = col;

	if ( state->gapchar == ',' )
	{
		if ( mtkit_file_write ( state->mtfp, "\"", 1 ) )
		{
			return 1;
		}

		if ( csv_quote_sub ( celltext, &newtext ) )
		{
			return 1;
		}

// NOTE: here onwards newtext is allocated so all returns must free it

		if ( newtext )
		{
			celltext = newtext;
		}
	}

	switch ( cell->type )
	{
	case CED_CELL_TYPE_TEXT_EXPLICIT:
		if ( state->qchar != 0 )
		{
			if ( mtkit_file_write ( state->mtfp, &state->qchar, 1 )
				)
			{
				goto error;
			}
		}
		/* FALLTHROUGH */

	case CED_CELL_TYPE_TEXT:
	case CED_CELL_TYPE_VALUE:
	case CED_CELL_TYPE_DATE:
	case CED_CELL_TYPE_ERROR:
		if ( mtkit_file_write_string ( state->mtfp, celltext ) )
		{
			goto error;
		}
		break;

	case CED_CELL_TYPE_FORMULA:
	case CED_CELL_TYPE_FORMULA_EVAL:
		if ( state->content )
		{
			if ( mtkit_file_write_string ( state->mtfp, celltext ) )
			{
				goto error;
			}
		}
		else
		{
			snprintf ( state->txt_buf, sizeof(state->txt_buf),
				CED_PRINTF_NUM, cell->value );

			if ( mtkit_file_write_string ( state->mtfp,
				state->txt_buf ) )
			{
				goto error;
			}
		}
		break;

	default:		// Skip unknown cells
		break;
	}

	if ( state->gapchar == ',' )
	{
		if ( mtkit_file_write ( state->mtfp, "\"", 1 ) )
		{
			goto error;
		}
	}

	free ( newtext );

	return 0;		// Continue

error:
	free ( newtext );

	return 1;
}

static int stat_sheet_save_tsv (
	mtFile		* const	mtfp,
	CedSheet	* const	sheet,
	int		const	content,
	int		const	filetype
	)
{
	tsv_state	state = { mtfp, content, filetype, '\t', '\'', 1, 1,
				{0} };


	if ( ! sheet )
	{
		return 1;
	}

	if (	filetype == CED_FILE_TYPE_CSV_CONTENT ||
		filetype == CED_FILE_TYPE_CSV_VALUE ||
		filetype == CED_FILE_TYPE_CSV_CONTENT_NOQ ||
		filetype == CED_FILE_TYPE_CSV_VALUE_NOQ
		)
	{
		state.gapchar = ',';
	}

	if (	filetype == CED_FILE_TYPE_TSV_CONTENT_NOQ ||
		filetype == CED_FILE_TYPE_TSV_VALUE_NOQ ||
		filetype == CED_FILE_TYPE_CSV_CONTENT_NOQ ||
		filetype == CED_FILE_TYPE_CSV_VALUE_NOQ
		)
	{
		state.qchar = 0;
	}

	int res = ced_sheet_scan_area( sheet, 1, 1, 0, 0, tsv_save_cb, &state );
	if ( ! res )
	{
		// Add final newline
		if ( mtkit_file_write ( mtfp, "\n", 1 ) )
		{
			res = 1;
		}
	}

	return res;
}

static int ledger_save_cb (
	CedSheet	* ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	tsv_state	* const	state = (tsv_state *)user_data;


	if ( ! cell->text )
	{
		// Nothing to save here so continue
		return 0;
	}

	snprintf( state->txt_buf, sizeof(state->txt_buf), "%i\t%i\t", row, col);

	if ( mtkit_file_write_string ( state->mtfp, state->txt_buf ) )
	{
		return 1;
	}

	if (	cell->type == CED_CELL_TYPE_TEXT_EXPLICIT &&
		mtkit_file_write ( state->mtfp, "'", 1 )
		)
	{
		return 1;
	}

	if (	state->content ||
		! (cell->type == CED_CELL_TYPE_FORMULA ||
		cell->type == CED_CELL_TYPE_FORMULA_EVAL)
		)
	{
		if (	mtkit_file_write_string ( state->mtfp, cell->text ) ||
			mtkit_file_write ( state->mtfp, "\n", 1 )
			)
		{
			return 1;
		}
	}
	else
	{
		snprintf ( state->txt_buf, sizeof(state->txt_buf),
			CED_PRINTF_NUM"\n", cell->value );

		if ( mtkit_file_write_string ( state->mtfp, state->txt_buf ) )
		{
			return 1;
		}
	}

	return 0;		// Continue
}

static int stat_sheet_save_ledger (
	mtFile		* const	mtfp,
	CedSheet	* const	sheet,
	int		const	filetype
	)
{
	tsv_state	state = { mtfp, 0, 0, 0, 0, 0, 0, {0} };


	if ( ! sheet )
	{
		return 1;
	}

	state.filetype = filetype;
	if (	filetype == CED_FILE_TYPE_LEDGER ||
		filetype == CED_FILE_TYPE_LEDGER_GZ
		)
	{
		state.content = 1;
	}

	int res = mtkit_file_write_string ( state.mtfp, ledger_header );
	if ( res == 0 )
	{
		res = ced_sheet_scan_area ( sheet, 1, 1, 0, 0, ledger_save_cb,
			&state );
	}

	return res;
}



typedef struct
{
	int		filetype,
			r,
			c;

	mtFile		* mtfp;

	CedSheet	* sheet;

	char		txt_buf[TEXT_LEN];
} outSTATE;



static int export_tsv_output_cb (
	CedSheet	* ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	outSTATE	* const	state = (outSTATE *)user_data;
	char			* txt = NULL;
	int			res = 1;


	if ( ! cell->text )
	{
		return 0;
	}

	txt = ced_cell_create_output ( cell, NULL );
	if ( ! txt )
	{
		goto error;
	}

	// Is this a new row?
	if ( state->r < row )
	{
		state->c = 1;

		// Add required number of newlines
		if ( send_n_chars ( state->mtfp, state->txt_buf, '\n',
			row - state->r )
			)
		{
			goto error;
		}

		state->r = row;
	}

	// Add required tabs for new col
	if ( send_n_chars ( state->mtfp, state->txt_buf, '\t',
		col - state->c ) )
	{
		goto error;
	}

	state->c = col;

	if ( state->filetype == CED_FILE_TYPE_OUTPUT_TSV_QUOTED )
	{
		// Output ' if requested
		if ( mtkit_file_write ( state->mtfp, "'", 1 ) )
		{
			goto error;
		}
	}

	if ( mtkit_file_write_string ( state->mtfp, txt ) )
	{
		goto error;
	}

	res = 0;		// Continue

error:
	free ( txt );

	return res;
}

static int export_tsv (
	outSTATE	* const	state
	)
{
	int		res;


	res = ced_sheet_scan_area ( state->sheet, 1, 1, 0, 0,
		export_tsv_output_cb, state );
	if ( ! res )
	{
		// Add final newline
		if ( mtkit_file_write ( state->mtfp, "\n", 1 ) )
		{
			res = 1;
		}
	}

	if ( res )
	{
		res = 1;
	}

	return res;
}


// On entry cell->prefs must exist
static int html_borders (
	outSTATE	* const	state,
	CedCell		* const	cell
	)
{
	char	const	* type = "";
	char	const	* pos[4] = { "top", "bottom", "left", "right" };
	int		shft[4] = {
				CED_CELL_BORDER_TOP_SHIFT,
				CED_CELL_BORDER_BOTTOM_SHIFT,
				CED_CELL_BORDER_LEFT_SHIFT,
				CED_CELL_BORDER_RIGHT_SHIFT
				};


	int i;

	for ( i = 0; i < 4; i ++ )
	{
		int const bord = ( cell->prefs->border_type >> shft[i] ) &
			CED_CELL_BORDER_MASK;

		switch ( bord )
		{
		case CED_CELL_BORDER_THIN:
			type = "thin solid";
			break;

		case CED_CELL_BORDER_THICK:
			type = "thick solid";
			break;

		case CED_CELL_BORDER_DOUBLE:
			type = "thick double";
			break;

		default:
			continue;
		};

		snprintf ( state->txt_buf, sizeof(state->txt_buf),
			"border-%s:%s #%06x;", pos[i], type,
			(unsigned int)cell->prefs->border_color );

		if ( mtkit_file_write_string ( state->mtfp, state->txt_buf ) )
		{
			return 1;
		}
	}

	return 0;
}

static int export_html_output_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	outSTATE	* const	state = (outSTATE *)user_data;
	char			* txt = NULL,
				* txt_html = NULL;
	int			res = 1,
				align = 0;


	// We do this early to get the alignment position
	if ( cell->text )
	{
		txt = ced_cell_create_output ( cell, &align );
	}

	// Is this a new row?
	if ( state->r < row )
	{
		state->c = 1;

		// Add required number of new rows
		for ( ; state->r < row; state->r ++ )
		{
			if ( mtkit_file_write_string ( state->mtfp,
				"\t</TR>\n\t<TR>\n" ) )
			{
				goto error;
			}

			if ( ( state->r + 1 ) < row )
			{
				// This ensures that an empty row has a cell
				// (adds spacing)

				if ( mtkit_file_write_string ( state->mtfp,
					"\t\t<TD></TD>\n" ) )
				{
					goto error;
				}
			}
		}
	}

	// Add required number of empty cells skipped in scan
	for ( ; state->c < col; state->c ++ )
	{
		if ( mtkit_file_write_string ( state->mtfp, "\t\t<TD></TD>\n") )
		{
			goto error;
		}
	}

	// Create new opening cell tag
	if ( mtkit_file_write_string ( state->mtfp, "\t\t<TD" ) )
	{
		goto error;
	}

	if ( align == 2 )
	{
		if ( mtkit_file_write_string ( state->mtfp, " ALIGN=CENTER" ) )
		{
			goto error;
		}
	}
	else if ( align == 3 )
	{
		if ( mtkit_file_write_string ( state->mtfp, " ALIGN=RIGHT" ) )
		{
			goto error;
		}
	}

	if ( cell->prefs )
	{
		// Style attribute start
		if (	cell->prefs->color_background != 16777215 ||
			cell->prefs->border_type
			)
		{
			if ( mtkit_file_write_string ( state->mtfp,
				" STYLE=\"" ) )
			{
				goto error;
			}
		}

		if ( cell->prefs->color_background != 16777215 )
		{
			snprintf ( state->txt_buf, sizeof(state->txt_buf),
				"background: #%06x;",
				(unsigned int)cell->prefs->color_background
				);

			if ( mtkit_file_write_string ( state->mtfp,
				state->txt_buf ) )
			{
				goto error;
			}
		}

		if ( html_borders ( state, cell ) )
		{
			goto error;
		}

		// Style attribute finish
		if (	cell->prefs->color_background != 16777215 ||
			cell->prefs->border_type
			)
		{
			if ( mtkit_file_write ( state->mtfp, "\"", 1 ) )
			{
				goto error;
			}
		}
	}

	// Complete opening cell tag
	if ( mtkit_file_write ( state->mtfp, ">", 1 ) )
	{
		goto error;
	}

	if ( cell->prefs )
	{
		if ( cell->prefs->color_foreground != 0 )
		{
			snprintf ( state->txt_buf, sizeof(state->txt_buf),
				"<FONT COLOR=\"#%06x\">",
				(unsigned int)cell->prefs->color_foreground
				);

			if ( mtkit_file_write_string ( state->mtfp,
				state->txt_buf ) )
			{
				goto error;
			}
		}

		if ( CED_TEXT_STYLE_IS_BOLD ( cell->prefs->text_style ) )
		{
			if ( mtkit_file_write_string ( state->mtfp, "<B>" ) )
			{
				goto error;
			}
		}

		if ( CED_TEXT_STYLE_IS_ITALIC ( cell->prefs->text_style ) )
		{
			if ( mtkit_file_write_string ( state->mtfp, "<I>" ) )
			{
				goto error;
			}
		}

		if ( CED_TEXT_STYLE_IS_UNDERLINE ( cell->prefs->text_style ) )
		{
			if ( mtkit_file_write_string ( state->mtfp, "<U>" ) )
			{
				goto error;
			}
		}

		if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( cell->prefs->text_style))
		{
			if ( mtkit_file_write_string ( state->mtfp, "<DEL>" ) )
			{
				goto error;
			}
		}
	}

	// OUTPUT THE TEXT

	if ( ! txt )
	{
		// Ensure empty cells have borders
		txt_html = strdup ( "&nbsp;" );
	}
	else
	{
		txt_html = mtkit_strtohtml ( txt );
	}

	if ( ! txt_html )
	{
		goto error;
	}

	if ( mtkit_file_write_string ( state->mtfp, txt_html ) )
	{
		goto error;
	}

	if ( cell->prefs )
	{
		// FINISH THE CELL
		if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( cell->prefs->text_style))
		{
			if ( mtkit_file_write_string ( state->mtfp, "</DEL>" ) )
			{
				goto error;
			}
		}

		if ( CED_TEXT_STYLE_IS_UNDERLINE ( cell->prefs->text_style ) )
		{
			if ( mtkit_file_write_string ( state->mtfp, "</U>" ) )
			{
				goto error;
			}
		}

		if ( CED_TEXT_STYLE_IS_ITALIC ( cell->prefs->text_style ) )
		{
			if ( mtkit_file_write_string ( state->mtfp, "</I>" ) )
			{
				goto error;
			}
		}

		if ( CED_TEXT_STYLE_IS_BOLD ( cell->prefs->text_style ) )
		{
			if ( mtkit_file_write_string ( state->mtfp, "</B>" ) )
			{
				goto error;
			}
		}

		if ( cell->prefs->color_foreground != 0 )
		{
			if ( mtkit_file_write_string ( state->mtfp, "</FONT>" )
				)
			{
				goto error;
			}
		}
	}

	// Create new closing cell tag
	if ( mtkit_file_write_string ( state->mtfp, "</TD>\n" ) )
	{
		goto error;
	}

	// Prepare for next column
	state->c ++;

	res = 0;		// Continue

error:
	free ( txt );
	free ( txt_html );

	return res;
}

static int export_html (
	outSTATE	* const state
	)
{
	char	const * const html_header =
			"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
			"<HTML>\n"
			"<HEAD>\n"
			"	<META HTTP-EQUIV=\"CONTENT-TYPE\" CONTENT=\"text/html;charset=UTF-8\">\n"
			"	<TITLE>%s</TITLE>\n"
			"\n"
			"	<!-- %s -->\n"
			"	<!-- %s -->\n"
			"\n"
			"</HEAD>\n"
			"<BODY>\n"
			"<TABLE CELLPADDING=4 CELLSPACING=0>\n"
			"	<TR>\n"
			;

	char	const * const html_footer =
			"	</TR>\n"
			"</TABLE>\n"
			"<P><BR><BR>\n"
			"</P>\n"
			"</BODY>\n"
			"</HTML>\n"
			;

	char		txt_time[32] = {0},
			* txt_sheet_name
			;
	int		res;
	time_t		t;
	struct tm const * tm_time;



	// Setup current date/time
	t = time ( NULL );
	tm_time = localtime ( &t );
	if ( tm_time )
	{
		strftime ( txt_time, sizeof ( txt_time ), "%Y-%m-%d %H:%M:%S",
			tm_time );
	}

	// Setup sheet name
	if ( state->sheet->book_tnode && state->sheet->book_tnode->key )
	{
		txt_sheet_name = mtkit_strtohtml ( (char const *)
			state->sheet->book_tnode->key );
	}
	else
	{
		txt_sheet_name = strdup ( "" );
	}

	if ( ! txt_sheet_name )
	{
		return 1;
	}

	// Add header
	snprintf ( state->txt_buf, sizeof(state->txt_buf), html_header,
		txt_sheet_name, VERSION, txt_time );

	if ( mtkit_file_write_string ( state->mtfp, state->txt_buf ) )
	{
		res = 1;
	}
	else
	{
		res = ced_sheet_scan_area ( state->sheet, 1, 1, 0, 0,
			export_html_output_cb, state );
	}

	if ( ! res )
	{
		// Add footer
		if ( mtkit_file_write_string ( state->mtfp, html_footer ) )
		{
			res = 1;
		}
	}

	if ( res )
	{
		res = 1;
	}

	free ( txt_sheet_name );

	return res;
}

static int stat_sheet_save_output (
	mtFile		* const	mtfp,
	CedSheet	* const	sheet,
	int		const	filetype
	)
{
	int		res = 1;
	outSTATE	state = { filetype, 1, 1, mtfp, sheet, {0} };


	switch ( filetype )
	{
	case CED_FILE_TYPE_OUTPUT_TSV:
	case CED_FILE_TYPE_OUTPUT_TSV_QUOTED:
		res = export_tsv ( &state );
		break;

	case CED_FILE_TYPE_OUTPUT_HTML:
		res = export_html ( &state );
		break;

	default:
		res = 1;
	}

	return res;
}

static int ced_sheet_save_real (
	mtFile		* const	mtfp,
	CedSheet	* const	sheet,
	int		const	filetype
	)
{
	int		res = 0;


	switch ( filetype )
	{
	case CED_FILE_TYPE_TSV_CONTENT:
	case CED_FILE_TYPE_TSV_CONTENT_GZ:
	case CED_FILE_TYPE_TSV_CONTENT_NOQ:
	case CED_FILE_TYPE_CSV_CONTENT:
	case CED_FILE_TYPE_CSV_CONTENT_NOQ:
		res = stat_sheet_save_tsv ( mtfp, sheet, 1, filetype );
		break;

	case CED_FILE_TYPE_TSV_VALUE:
	case CED_FILE_TYPE_TSV_VALUE_GZ:
	case CED_FILE_TYPE_TSV_VALUE_NOQ:
	case CED_FILE_TYPE_CSV_VALUE:
	case CED_FILE_TYPE_CSV_VALUE_NOQ:
		res = stat_sheet_save_tsv ( mtfp, sheet, 0, filetype );
		break;

	case CED_FILE_TYPE_LEDGER:
	case CED_FILE_TYPE_LEDGER_GZ:
	case CED_FILE_TYPE_LEDGER_VAL:
	case CED_FILE_TYPE_LEDGER_VAL_GZ:
		res = stat_sheet_save_ledger ( mtfp, sheet, filetype );
		break;

	case CED_FILE_TYPE_OUTPUT_TSV:
	case CED_FILE_TYPE_OUTPUT_TSV_QUOTED:
	case CED_FILE_TYPE_OUTPUT_HTML:
		res = stat_sheet_save_output ( mtfp, sheet, filetype );
		break;

	default:
		return 1;
	}

	return res;
}

int ced_sheet_save (
	CedSheet	* const	sheet,
	char	const	* const	filename,
	int		const	filetype
	)
{
	mtFile		* mtfp;
	int		res = 0;


	mtfp = mtkit_file_open_disk ( filename );
	if ( ! mtfp )
	{
		return 1;
	}

	res = ced_sheet_save_real ( mtfp, sheet, filetype );
	mtkit_file_close ( mtfp );

	if (	! res && (
		filetype == CED_FILE_TYPE_TSV_CONTENT_GZ ||
		filetype == CED_FILE_TYPE_TSV_VALUE_GZ ||
		filetype == CED_FILE_TYPE_LEDGER_GZ ||
		filetype == CED_FILE_TYPE_LEDGER_VAL_GZ
		) )
	{
		char		* buf;
		int		size;


		// Re-save this uncompressed file to a .gz file

		buf = mtkit_file_load ( filename, &size, 0, NULL );
		if ( ! buf )
		{
			return 1;	// Error
		}

		res = mtkit_file_save ( filename, buf, size, MTKIT_FILE_GUNZIP
			);
		free ( buf );
	}

	return res;
}

mtFile * ced_sheet_save_mem (
	CedSheet	* const	sheet,
	int		const	filetype
	)
{
	mtFile		* mtfp;


	mtfp = mtkit_file_open_mem ();
	if ( ! mtfp )
	{
		return NULL;
	}

	if ( ced_sheet_save_real ( mtfp, sheet, filetype ) )
	{
		mtkit_file_close ( mtfp );
		return NULL;
	}

	return mtfp;
}

int ced_file_type_detect (
	char	const * const	filename,
	int		const	force
	)
{
	FILE		* fp;
	unsigned char	buf [ 8000 ];
	size_t		buf_size;


	if ( ! filename || ! filename[0] )
	{
		return CED_FILE_DETECT_ERROR;
	}

	fp = fopen ( filename, "rb" );
	if ( ! fp )
	{
		return CED_FILE_DETECT_ERROR;
	}

	buf_size = fread ( buf, 1, sizeof ( buf ), fp );

	if ( ferror ( fp ) )
	{
		fclose ( fp );

		return CED_FILE_DETECT_ERROR;
	}

	fclose ( fp );

	if ( mtkit_file_header_zip ( buf, (int)buf_size ) )
	{
		return CED_FILE_DETECT_BOOK;
	}
	else if ( mtkit_file_header_gz ( buf, (int)buf_size ) )
	{
		return CED_FILE_DETECT_TSV;
	}
	else if ( force == CED_FILE_FORCE_TSV )
	{
		return CED_FILE_DETECT_TSV;
	}
	else if ( force == CED_FILE_FORCE_CSV )
	{
		return CED_FILE_DETECT_CSV;
	}
	else if ( mtkit_strmatch ( filename, "*.tsv", 0 ) >= 0 )
	{
		return CED_FILE_DETECT_TSV;
	}
	else if ( mtkit_strmatch ( filename, "*.csv", 0 ) >= 0 )
	{
		return CED_FILE_DETECT_CSV;
	}
	else if ( buf_size > 0 && memchr ( buf, '\t', buf_size ) )
	{
		return CED_FILE_DETECT_TSV;
	}
	else if ( buf_size > 0 && memchr ( buf, ',', buf_size ) )
	{
		return CED_FILE_DETECT_CSV;
	}

	return CED_FILE_DETECT_TSV;
}

