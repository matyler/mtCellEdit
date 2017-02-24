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

#include "private.h"



int cedut_append ( void )
{
	int		res = 0,
			r = 0,
			c = 0;
	CedSheet	* sheet;


	sheet = global.sheet;
	global.sheet = NULL;

	if ( ut_load_file () || global.sheet == NULL )
	{
		res = ERROR_LOAD_FILE;	// Fail, and tell user load failed

		goto finish;
	}

	if ( ced_sheet_get_geometry ( sheet, &r, &c ) )
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

	if ( global.i_rowcol == 1 )
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

	if ( ced_sheet_paste_area ( sheet, global.sheet, r, c, 0, 0, 0, 0,
		CED_PASTE_ACTIVE_CELLS ) )
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

finish:
	ced_sheet_destroy ( global.sheet );
	global.sheet = sheet;

	return res;
}

int cedut_clear ( void )
{
	int		r,
			c,
			rtot,
			ctot;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	r = global.i_range[0];
	c = global.i_range[2];
	rtot = global.i_range[1] - r + 1;
	ctot = global.i_range[3] - c + 1;

	if ( ced_sheet_clear_area ( global.sheet, r, c, rtot, ctot, 0 ) )
	{
		return ERROR_LIBMTCELLEDIT;	// Fail, and tell caller
	}

	return 0;
}

int cedut_cut ( void )
{
	int		res = 0;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	if ( global.i_rowcol == 0 )
	{
		res = ced_sheet_delete_row ( global.sheet, global.i_start,
			global.i_total );
	}
	else
	{
		res = ced_sheet_delete_column ( global.sheet, global.i_start,
			global.i_total );
	}

	if ( res )
	{
		return ERROR_LIBMTCELLEDIT;	// Fail, and tell caller
	}

	return 0;
}

int cedut_eval ( void )
{
	CedParser	state;


	state = ced_sheet_parse_text ( global.sheet, 1, 1, global.s_arg, NULL );

	if (	state.ced_errno ||
		(state.flag & CED_PARSER_FLAG_ERROR)
		)
	{
		int		i;


		printf ( "\nerrno = %i flag = %i sp = %i\n%s\t=\t"
			CED_PRINTF_NUM"\n",
			state.ced_errno, state.flag, state.sp, global.s_arg,
			state.data );

		for ( i = 0; i < (state.sp - 1); i++ )
		{
			printf (" ");
		}

		printf ( "^\n" );
	}
	else
	{
		if ( global.i_verbose )
		{
			printf ( "%s\t=\t" CED_PRINTF_NUM "\n", global.s_arg,
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
	void		* const	ARG_UNUSED ( user_data )
	)
{
	printf ( "%i	%i", row, col );

	if ( global.i_verbose && cell->text )
	{
		printf ( "	%s", cell->text );
	}

	printf ( "\n" );

	return 0;			// Continue searching
}

int cedut_find ( void )
{
	int		res = 0, r, c, rtot, ctot;


	r = global.i_range[0];
	c = global.i_range[2];

	rtot = global.i_range[1] - r + 1;
	ctot = global.i_range[3] - c + 1;

	if ( global.i_num == 0 )
	{
		int		mode = 0;


		if ( global.i_case )
		{
			mode |= CED_FIND_MODE_CASE;
		}

		if ( global.i_wildcard )
		{
			mode |= CED_FIND_MODE_WILDCARD;
		}

		res = ced_sheet_find_text ( global.sheet, global.s_arg, mode,
			r, c, rtot, ctot, find_cb, NULL );
	}
	else
	{
		double		value;


		if ( mtkit_strtod ( global.s_arg, &value, NULL, 1 ) )
		{

			fprintf ( stderr,
				"Find Error!  '%s' is not a number.\n",
				global.s_arg );

			global.i_error = 1;

			// We return 0 from this function but stop NOW!
			return 0;
		}

		res = ced_sheet_find_value ( global.sheet, value, 0,
			r, c, rtot, ctot, find_cb, NULL );
	}

	if ( res )
	{
		return ERROR_LIBMTCELLEDIT;
	}

	return 0;
}

int cedut_flip ( void )
{
	CedSheet	* sheet = NULL;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	if ( global.i_rowcol == 1 )
	{
		sheet = ced_sheet_flip_horizontal ( global.sheet );
	}
	else
	{
		sheet = ced_sheet_flip_vertical ( global.sheet );
	}

	if ( ! sheet )
	{
		return ERROR_LIBMTCELLEDIT;	// Get caller to report error
	}

	ced_sheet_destroy ( global.sheet );
	global.sheet = sheet;

	return 0;
}

int cedut_insert ( void )
{
	int		res;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	if ( global.i_rowcol == 1 )
	{
		res = ced_sheet_insert_column ( global.sheet, global.i_start,
			global.i_total );
	}
	else
	{
		res = ced_sheet_insert_row ( global.sheet, global.i_start,
			global.i_total );
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

int cedut_ls ( void )
{
	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	if ( global.i_verbose )
	{
		int		r,
				c;
		double		tot = 0,
				perc = 0;


		if ( ced_sheet_get_geometry ( global.sheet, &r, &c ) )
		{
			return ERROR_LIBMTCELLEDIT;
		}

		if ( r && c )
		{
			if ( ced_sheet_scan_area ( global.sheet, 1, 1, 0, 0,
				cb_scan, &tot ) )
			{
				return ERROR_LIBMTCELLEDIT;
			}

			perc = 100 * tot / ( ( (double)r ) * c );
		}

		printf ( "%5i %5i %5.0f %6.1f%% ", r, c, tot, perc );
	}

	printf ( "%s [%s]\n", global.s_arg,
		ced_file_type_text ( global.i_ftype_in) );

	return 0;
}

int cedut_paste ( void )
{
	int		res = 0,
			r,
			c,
			rtot,
			ctot;
	CedSheet	* sheet,
			* paste = NULL;


	sheet = global.sheet;
	global.sheet = NULL;

	if ( ut_load_file () || global.sheet == NULL )
	{
		res = ERROR_LOAD_FILE;	// Fail, and tell user load failed

		goto finish;
	}

	r = global.i_range[0];
	c = global.i_range[2];
	rtot = global.i_range[1] - r + 1;
	ctot = global.i_range[3] - c + 1;

	paste = ced_sheet_copy_area ( global.sheet, r, c, rtot, ctot );

	r = global.i_dest[0];
	c = global.i_dest[2];
	rtot = global.i_dest[1] - r + 1;
	ctot = global.i_dest[3] - c + 1;

	if ( rtot == 1 && ctot == 1 )
	{
		rtot = 0;
		ctot = 0;
	}

	if (	! paste ||
		ced_sheet_paste_area ( sheet, paste, r, c, rtot, ctot, 0, 0, 0 )
		)
	{
		res = ERROR_LIBMTCELLEDIT;	// Fail, and tell caller

		goto finish;
	}

finish:
	ced_sheet_destroy ( paste );
	ced_sheet_destroy ( global.sheet );
	global.sheet = sheet;

	return res;
}

int cedut_rotate ( void )
{
	CedSheet	* sheet = NULL;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	if ( global.i_clock == 1 )
	{
		sheet = ced_sheet_rotate_anticlockwise ( global.sheet );
	}
	else
	{
		sheet = ced_sheet_rotate_clockwise ( global.sheet );
	}

	if ( ! sheet )
	{
		return ERROR_LIBMTCELLEDIT;	// Get caller to report error
	}

	ced_sheet_destroy ( global.sheet );
	global.sheet = sheet;

	return 0;
}

int cedut_set ( void )
{
	char	const	* next;
	CedCellRef	cr;


	if (	ced_strtocellref ( global.s_arg, &cr, &next, 0 ) ||
		cr.row_m != 0 ||
		cr.col_m != 0
		)
	{
		fprintf ( stderr, "Set Error!  Bad cell reference - '%s'\n",
			global.s_arg );

		global.i_error = 1;	// Terminate ASAP

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

	if ( ! ced_sheet_set_cell ( global.sheet, cr.row_d, cr.col_d, next ) )
	{
		return ERROR_LIBMTCELLEDIT;
	}

	return 0;
}

int cedut_transpose ( void )
{
	CedSheet	* sheet;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail, and tell user load failed
	}

	sheet = ced_sheet_transpose ( global.sheet );

	if ( ! sheet )
	{
		return ERROR_LIBMTCELLEDIT;	// Get caller to report error
	}

	ced_sheet_destroy ( global.sheet );
	global.sheet = sheet;

	return 0;		// Success
}

