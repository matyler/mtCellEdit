/*
	Copyright (C) 2012-2014 Mark Tyler

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



#define EXP_MAX 10



static int parse_expression (
	char		*	input,
	int		* const	rc,
	int		* const	rc_mode
	)
{
	int		i,
			num;


	for ( i = 0; i < EXP_MAX; i++ )
	{
		// NUMBER
		if ( mtkit_strtoi ( input, &num, &input, 0 ) )
		{
			fprintf ( stderr, "Bad number: '%s'\n\n", input );

			return 1;
		}

		rc[i] = num;

		// COMMA
		if ( input[0] == ',' )
		{
			input ++;
		}
		else
		{
			fprintf ( stderr, "Bad char: '%s'\n\n", input );

			return 1;
		}

		// a|d
		if ( input[0] == 'a' )
		{
			rc_mode[i] |= CED_SORT_MODE_ASCENDING;
		}
		else if ( input[0] == 'd' )
		{
			rc_mode[i] |= CED_SORT_MODE_DESCENDING;
		}
		else
		{
			fprintf ( stderr, "Bad char: '%s'\n\n", input );

			return 1;
		}

		input ++;

		// [c]
		if ( input[0] == 'c' )
		{
			rc_mode[i] |= CED_SORT_MODE_CASE;
			input ++;
		}

		// ,|EOL
		if ( input[0] == ',' )
		{
			input ++;
		}
		else if ( input[0] == 0 )
		{
			break;
		}
	}

	if ( i == EXP_MAX )
	{
		fprintf ( stderr, "Too many items.\n\n" );

		return 1;
	}

	return 0;	// Success
}

static int sort_rowcol (
	int		const	rows,
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedSheet	* sheet;
	int		res,
			r1,
			c1,
			r2,
			c2,
			rc_mode[EXP_MAX + 1] = {0}
			;
	int		rc[EXP_MAX + 1] = {0};


	sheet = cedcli_get_sheet ( state );

	if ( ! sheet || parse_expression ( args[0], rc, rc_mode ) )
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	if ( rows )
	{
		res = cui_sheet_sort_rows ( CEDCLI_FILE->cubook, sheet,
			r1, r2 - r1 + 1, rc, 0, rc_mode );
	}
	else
	{
		res = cui_sheet_sort_columns ( CEDCLI_FILE->cubook, sheet,
			c1, c2 - c1 + 1, rc, 0, rc_mode );
	}

	if ( res )
	{
		fprintf ( stderr, "Error sorting.\n\n" );

		return 1;
	}

	return 0;
}

int jtf_sort_column (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	return sort_rowcol ( 0, state, args );
}

int jtf_sort_row (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	return sort_rowcol ( 1, state, args );
}

