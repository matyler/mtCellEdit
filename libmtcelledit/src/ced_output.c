/*
	Copyright (C) 2008-2024 Mark Tyler

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

#include "ced.h"



enum
{
	CELL_TYPE_NONE,

	CELL_TYPE_TEXT,
	CELL_TYPE_FIXED_DECIMAL,
	CELL_TYPE_HEX,
	CELL_TYPE_BINARY,
	CELL_TYPE_SCIENTIFIC,
	CELL_TYPE_PERCENTAGE,
	CELL_TYPE_DATETIME,

	CELL_TYPE_GENERAL_NUMBER,
	CELL_TYPE_ERROR,

	CELL_TYPE_TOTAL
};



#define GUI_PRINTF_NUM		"%.8g"
#define CELL_TXT_SIZE		2048



static int get_cell_type (
	CedCell		const * const	cell
	)
{
	// Errors and text cannot be manually overidden
	if ( cell->type == CED_CELL_TYPE_ERROR )
	{
		return CELL_TYPE_ERROR;
	}

	if (	cell->type == CED_CELL_TYPE_TEXT ||
		cell->type == CED_CELL_TYPE_TEXT_EXPLICIT
		)
	{
		return CELL_TYPE_TEXT;
	}

	if ( cell->prefs && cell->prefs->format )
	{
		return cell->prefs->format;
	}

	// General cell type (based on internal cell types)
	switch ( cell->type )
	{
	case CED_CELL_TYPE_DATE:
		return CELL_TYPE_DATETIME;

	default:
		break;
	}

/*
	CED_CELL_TYPE_VALUE:
	CED_CELL_TYPE_FORMULA:
	CED_CELL_TYPE_FORMULA_EVAL:
	unknown
*/
	return CELL_TYPE_GENERAL_NUMBER;
}

int ced_cell_create_output (
	CedCell	const	* const	cell,
	int		* const	hjustify,
	char		* const out,
	size_t		const	outlen
	)
{
	out[0] = 0;

	if ( ! cell || ! cell->text )
	{
		return 1;
	}

	char		txt [ CELL_TXT_SIZE ];
	char		buf [ 128 ];
	char		* fmt_ddt = NULL;
	char		* thou = NULL;
	int		justify = 0;
	int		dp = 0;
	int		nz = 0;
	int	const	ctype = get_cell_type ( cell );

	if ( ctype == CELL_TYPE_ERROR )
	{
		int const e = (int)cell->value;

		justify = CED_CELL_JUSTIFY_LEFT;
		snprintf ( out, outlen, "Err:%i,%i", e%1000, e/1000 );

		goto finish;
	}

	// PREFIX
	if (	cell->prefs &&
		cell->prefs->text_prefix &&
		cell->prefs->text_prefix[0]
		)
	{
		if ( mtkit_strnncat ( out, cell->prefs->text_prefix, outlen ) )
		{
			goto finish_long;
		}
	}

	justify = CED_CELL_JUSTIFY_RIGHT;		// Default

	if ( cell->prefs )
	{
		dp = cell->prefs->num_decimal_places;
		nz = MAX ( 0, cell->prefs->num_zeros );
		fmt_ddt = cell->prefs->format_datetime;
		thou = cell->prefs->num_thousands;
	}

	switch ( ctype )
	{
	case CELL_TYPE_TEXT:
		justify = CED_CELL_JUSTIFY_LEFT;

		if ( mtkit_strnncat ( out, cell->text, outlen ) )
		{
			goto finish_long;
		}

		break;

	case CELL_TYPE_FIXED_DECIMAL:
	case CELL_TYPE_PERCENTAGE:

		if ( fabs ( cell->value ) > 1000000000000000.0 )
		{
			// Enforce sanity for very large numbers
			snprintf ( txt, sizeof ( txt ), GUI_PRINTF_NUM,
				ctype == CELL_TYPE_PERCENTAGE ?
				cell->value * 100 : cell->value );
		}
		else
		{
			snprintf ( buf, sizeof ( buf ), "%%.%if", dp );
			snprintf ( txt, sizeof ( txt ), buf,
				ctype == CELL_TYPE_PERCENTAGE
				? cell->value * 100 : cell->value );
		}

		if ( thou )
		{
			mtkit_strtothou ( txt, txt, sizeof ( txt ), thou[0],
				'-', '.', 3, 0 );
		}

		if ( mtkit_strnncat ( out, txt, outlen ) )
		{
			goto finish_long;
		}

		if ( ctype == CELL_TYPE_PERCENTAGE )
		{
			if ( mtkit_strnncat ( out, "%", outlen ) )
			{
				goto finish_long;
			}
		}

		break;

	case CELL_TYPE_HEX:
		snprintf ( buf, sizeof ( buf ), "%%.%i" PRIx64, nz );
		snprintf ( txt, sizeof ( txt ), buf, (uint64_t)cell->value );

		if ( mtkit_strnncat ( out, txt, outlen ) )
		{
			goto finish_long;
		}

		break;

	case CELL_TYPE_BINARY:
		{
			int		ii;
			uint64_t	v = (uint64_t)cell->value;
			char		* cp = txt;


			if ( v == 0 )
			{
				cp[0] = '0';
				cp++;
			}
			else for ( ii = 63; ii >= 0; ii-- )
			{
				uint64_t const b = ((uint64_t)1) << ii;

				if ( b & v )
				{
					cp[0] = '1';
					cp++;
				}
				else
				{
					if ( cp != txt )
					{
						cp[0] = '0';
						cp++;
					}
				}
			}

			cp[0] = 0;

			size_t const len = strlen ( txt );

			if ( (size_t)nz > len && len <= 64 && len > 0 )
			{
				int		lzr;


				lzr = nz - (int)len; // Leading zeros required
				txt [ nz ] = 0;

				// Shift old string to the right
				for ( ii = nz - 1; ii >= lzr; ii-- )
				{
					txt [ ii ] = txt [ ii - lzr ];
				}

				// Put in leading zeros
				for ( ; ii >= 0; ii-- )
				{
					txt [ ii ] = '0';
				}
			}
		}

		if ( mtkit_strnncat ( out, txt, outlen ) )
		{
			goto finish_long;
		}

		break;

	case CELL_TYPE_SCIENTIFIC:
		snprintf ( buf, sizeof ( buf ), "%%.%ie", dp );
		snprintf ( txt, sizeof ( txt ), buf, cell->value );

		if ( mtkit_strnncat ( out, txt, outlen ) )
		{
			goto finish_long;
		}

		break;

	case CELL_TYPE_DATETIME:
		justify = CED_CELL_JUSTIFY_LEFT;

		{
			int		day = 0,
					month = 0,
					year = 0,
					hour = 0,
					min = 0,
					sec = 0;
			struct tm	tm;


			memset ( &tm, 0, sizeof(tm) );

			mtkit_ddttoi ( cell->value, &day, &month, &year, &hour,
				&min, &sec );

			if ( fmt_ddt && fmt_ddt[0] )
			{
				char * dest = txt;
				char * src = fmt_ddt;
				int backslash = 0;

				dest[0] = 0;

				for ( ; src[0] && (dest - txt) <
					(CELL_TXT_SIZE - 35); src++ )
				{
					int val = -1, chsq;

					for (	chsq = 0;
						src[0] == src[chsq + 1];
						chsq++
						)
					{
					}

					// Skip the leading backslash, record
					// its existence.
					if ( src[0] == '\\' && ! backslash )
					{
						backslash = 1;
						continue;
					}

					if ( ! backslash ) switch ( src[0] )
					{
					case 'd':
						if ( chsq >= 2 )
						{
						tm.tm_wday = mtkit_ddt_weekday (
							cell->value );

						if ( chsq == 2 )
						{
							strftime ( dest, 32,
								"%a", &tm );
						}
						else
						{
							strftime ( dest, 32,
								"%A", &tm );
						}

						dest[32] = 0;

						if ( chsq > 3 )
						{
							chsq = 3;
						}

						}
						val = day;
						break;

					case 'm':
						if ( chsq >= 2 )
						{
						 tm.tm_mon = month - 1;
						 if ( chsq == 2 )
						 {
							strftime ( dest, 32,
								"%b", &tm );
						 }
						 else
						 {
							strftime ( dest, 32,
								"%B", &tm );
						 }

						 dest[32] = 0;

						 if ( chsq > 3 )
						 {
							chsq = 3;
						 }

						}
						val = month;
						break;

					case 'y':
						val = year;
						if ( chsq > 1 )
						{
							chsq = 1;
						}
						break;

					case 'H':
						val = hour;
						if ( chsq > 1 )
						{
							chsq = 1;
						}
						break;

					case 'M':
						val = min;
						if ( chsq > 1 )
						{
							chsq = 1;
						}
						break;

					case 'S':
						val = sec;
						if ( chsq > 1 )
						{
							chsq = 1;
						}
						break;
					}

					backslash = 0;

					if ( val < 0 )
					{
						dest[0] = src[0];
						dest++;

						dest[0] = 0;
					}
					else	// Numerical substitution
					{
					 if ( chsq == 1 )
					 {
						snprintf ( dest, 32,
							"%02i", val % 100 );
					 }
					 else if ( chsq == 0 )
					 {
						snprintf ( dest, 32,
							"%i", val );
					 }

					 src += chsq;
					}

					dest += strlen ( dest );
				}
			}
			else
			{
				snprintf ( txt, sizeof ( txt ), "%i-%i-%i",
					year, month, day );
			}
		}

		if ( mtkit_strnncat ( out, txt, outlen ) )
		{
			goto finish_long;
		}

		break;

	case CELL_TYPE_GENERAL_NUMBER:
	default:
		snprintf ( txt, sizeof ( txt ), GUI_PRINTF_NUM, cell->value );

		if ( thou )
		{
			mtkit_strtothou ( txt, txt, sizeof ( txt ),
				thou[0], '-', '.', 3, 0 );
		}

		if ( mtkit_strnncat ( out, txt, outlen ) )
		{
			goto finish_long;
		}

		break;
	}

	if (	cell->prefs &&
		cell->prefs->text_suffix &&
		cell->prefs->text_suffix[0]
		)
	{
		if ( mtkit_strnncat ( out, cell->prefs->text_suffix, outlen ) )
		{
			goto finish_long;
		}
	}

	goto finish;

finish_long:
	// On entry: mtkit_strnncat has truncated the text, so fix to UTF8 char
	{
		int const lim = (int)((outlen/4) - 1);
		int const off = mtkit_utf8_offset ( (unsigned char *)out, lim );

		if ( off < 0 )
		{
			snprintf ( out, outlen, "Err:11,0" );
			return -1;		// Unable to truncate properly
		}

		out[ off ] = 0;
	}

finish:

	if ( hjustify )
	{
		if ( cell->prefs && cell->prefs->align_horizontal )
		{
			hjustify[0] = cell->prefs->align_horizontal;
		}
		else
		{
			hjustify[0] = justify;
		}
	}

	return 0;		// Success
}
