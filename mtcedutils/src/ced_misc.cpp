/*
	Copyright (C) 2011-2020 Mark Tyler

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



int Global::ced_append ()
{
	int		res = 0, r = 0, c = 0;
	CedSheet	* sh = m_sheet;
	m_sheet = NULL;

	if ( load_file () || m_sheet == NULL )
	{
		res = ERROR_LOAD_FILE;	// Fail, and tell user load failed

		goto finish;
	}

	if ( ced_sheet_get_geometry ( sh, &r, &c ) )
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

	if ( i_rowcol == 1 )
	{
		// Column Append

		r = 1;
		c = c + 1;
	}
	else
	{
		// Row Append

		r = r + 1;
		c = 1;
	}

	if ( ced_sheet_paste_area ( sh, m_sheet, r, c, 0, 0, 0, 0,
		CED_PASTE_ACTIVE_CELLS ) )
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

finish:
	set_sheet ( sh );

	return res;
}

int Global::ced_clear ()
{
	if ( load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	int const r = i_range[0];
	int const c = i_range[2];
	int const rtot = i_range[1] - r + 1;
	int const ctot = i_range[3] - c + 1;

	if ( ced_sheet_clear_area ( m_sheet, r, c, rtot, ctot, 0 ) )
	{
		return ERROR_LIBMTCELLEDIT;	// Fail, and tell caller
	}

	return 0;
}

int Global::ced_cut ()
{
	if ( load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	int		res = 0;

	if ( i_rowcol == 0 )
	{
		res = ced_sheet_delete_row ( m_sheet, i_start, i_total );
	}
	else
	{
		res = ced_sheet_delete_column ( m_sheet, i_start, i_total );
	}

	if ( res )
	{
		return ERROR_LIBMTCELLEDIT;	// Fail, and tell caller
	}

	return 0;
}

int Global::ced_eval ()
{
	CedParser state = ced_sheet_parse_text ( m_sheet, 1, 1, s_arg, NULL );

	if (	state.ced_errno ||
		(state.flag & CED_PARSER_FLAG_ERROR)
		)
	{
		printf ( "\nerrno = %i flag = %i sp = %i\n%s\t=\t"
			CED_PRINTF_NUM"\n",
			state.ced_errno, state.flag, state.sp, s_arg,
			state.data );

		for ( int i = 0; i < (state.sp - 1); i++ )
		{
			printf (" ");
		}

		printf ( "^\n" );
	}
	else
	{
		if ( i_verbose )
		{
			printf ( "%s\t=\t" CED_PRINTF_NUM "\n", s_arg,
				state.data );
		}
		else
		{
			printf ( CED_PRINTF_NUM"\n", state.data );
		}
	}

	return 0;			// Success
}

static int find_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	auto const i_verbose = (int const *)user_data;

	printf ( "%i	%i", row, col );

	if ( *i_verbose && cell->text )
	{
		printf ( "	%s", cell->text );
	}

	printf ( "\n" );

	return 0;			// Continue searching
}

int Global::ced_find ()
{
	int	const	r = i_range[0];
	int	const	c = i_range[2];
	int	const	rtot = i_range[1] - r + 1;
	int	const	ctot = i_range[3] - c + 1;
	int		res = 0;

	if ( i_num == 0 )
	{
		int		mode = 0;


		if ( i_case )
		{
			mode |= CED_FIND_MODE_CASE;
		}

		if ( i_wildcard )
		{
			mode |= CED_FIND_MODE_WILDCARD;
		}

		res = ced_sheet_find_text ( m_sheet, s_arg, mode, r, c, rtot,
			ctot, find_cb, &i_verbose );
	}
	else
	{
		double		value;


		if ( mtkit_strtod ( s_arg, &value, NULL, 1 ) )
		{

			fprintf ( stderr,
				"Find Error!  '%s' is not a number.\n",
				s_arg );

			i_error = 1;

			// We return 0 from this function but stop NOW!
			return 0;
		}

		res = ced_sheet_find_value ( m_sheet, value, 0, r, c, rtot,
			ctot, find_cb, &i_verbose );
	}

	if ( res )
	{
		return ERROR_LIBMTCELLEDIT;
	}

	return 0;
}

int Global::ced_flip ()
{
	if ( load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	CedSheet	* sh = NULL;

	if ( i_rowcol == 1 )
	{
		sh = ced_sheet_flip_horizontal ( m_sheet );
	}
	else
	{
		sh = ced_sheet_flip_vertical ( m_sheet );
	}

	if ( ! sh )
	{
		return ERROR_LIBMTCELLEDIT;	// Get caller to report error
	}

	set_sheet ( sh );

	return 0;
}

int Global::ced_insert ()
{
	if ( load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	int		res;

	if ( i_rowcol == 1 )
	{
		res = ced_sheet_insert_column ( m_sheet, i_start, i_total );
	}
	else
	{
		res = ced_sheet_insert_row ( m_sheet, i_start, i_total );
	}

	if ( res )
	{
		return ERROR_LIBMTCELLEDIT;	// Get caller to report error
	}

	return 0;
}

static int cb_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	double		* tot = (double *)user_data;

	tot[0] ++;

	return 0;			// Continue
}

int Global::ced_ls ()
{
	if ( load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	if ( i_verbose )
	{
		int		r, c;
		double		tot = 0, perc = 0;


		if ( ced_sheet_get_geometry ( m_sheet, &r, &c ) )
		{
			return ERROR_LIBMTCELLEDIT;
		}

		if ( r && c )
		{
			if ( ced_sheet_scan_area ( m_sheet, 1, 1, 0, 0, cb_scan,
				&tot ) )
			{
				return ERROR_LIBMTCELLEDIT;
			}

			perc = 100 * tot / ( ( (double)r ) * c );
		}

		printf ( "%5i %5i %5.0f %6.1f%% ", r, c, tot, perc );
	}

	printf ( "%s [%s]\n", s_arg, ced_file_type_text ( i_ftype_in ) );

	return 0;
}

int Global::ced_paste ()
{
	int		res = 0, r, c, rtot, ctot;
	CedSheet	* paste = NULL;

	CedSheet	* sh = m_sheet;
	m_sheet = NULL;

	if ( load_file () || m_sheet == NULL )
	{
		res = ERROR_LOAD_FILE;	// Fail, and tell user load failed

		goto finish;
	}

	r = i_range[0];
	c = i_range[2];
	rtot = i_range[1] - r + 1;
	ctot = i_range[3] - c + 1;

	paste = ced_sheet_copy_area ( m_sheet, r, c, rtot, ctot );

	r = i_dest[0];
	c = i_dest[2];
	rtot = i_dest[1] - r + 1;
	ctot = i_dest[3] - c + 1;

	if ( rtot == 1 && ctot == 1 )
	{
		rtot = 0;
		ctot = 0;
	}

	if (	! paste ||
		ced_sheet_paste_area ( sh, paste, r, c, rtot, ctot, 0, 0, 0 )
		)
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

finish:
	ced_sheet_destroy ( paste );

	set_sheet ( sh );

	return res;
}

int Global::ced_rotate ()
{
	if ( load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	CedSheet	* sh = NULL;

	if ( i_clock == 1 )
	{
		sh = ced_sheet_rotate_anticlockwise ( m_sheet );
	}
	else
	{
		sh = ced_sheet_rotate_clockwise ( m_sheet );
	}

	if ( ! sh )
	{
		return ERROR_LIBMTCELLEDIT;	// Get caller to report error
	}

	set_sheet ( sh );

	return 0;
}

int Global::ced_set ()
{
	char	const	* next;
	CedCellRef	cr;


	if (	ced_strtocellref ( s_arg, &cr, &next, 0 ) ||
		cr.row_m != 0 ||
		cr.col_m != 0
		)
	{
		fprintf ( stderr, "Set Error!  Bad cell reference - '%s'\n",
			s_arg );

		i_error = 1;	// Terminate ASAP

		return 0;
	}

	while ( next[0] == ' ' )
	{
		next ++;
	}

	if ( next[0] == 0 )
	{
		return 0;	// Nothing to input to sheet
	}

	if ( ! ced_sheet_set_cell ( m_sheet, cr.row_d, cr.col_d, next ) )
	{
		return ERROR_LIBMTCELLEDIT;
	}

	return 0;
}

int Global::ced_transpose ()
{
	if ( load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	CedSheet * const sh = ced_sheet_transpose ( m_sheet );

	if ( ! sh )
	{
		return ERROR_LIBMTCELLEDIT;	// Get caller to report error
	}

	set_sheet ( sh );

	return 0;		// Success
}

