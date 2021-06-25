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



#define EXP_MAX 10



static char const * parse_expression (
	char	const	*	input,
	int		* const	rc,
	int		* const	rc_mode,
	int		const	verbose
	)
{
	// Quietly ignore excess items > EXP_MAX

	for ( int i = 0; i < EXP_MAX; i++ )
	{
		int		num;
		char		* unsafe;


		// NUMBER
		if ( mtkit_strtoi ( input, &num, &unsafe, 0 ) )
		{
			return input;
		}

		input = unsafe;

		rc[i] = num;

		// COMMA
		if ( input[0] == ',' )
		{
			input ++;
		}
		else
		{
			return input;
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
			return input;
		}

		input ++;

		// [c]
		if ( input[0] == 'c' )
		{
			rc_mode[i] |= CED_SORT_MODE_CASE;
			input ++;
		}

		if ( verbose )
		{
			printf ( "sort[%i] = %i,%i\n", i, rc[i], rc_mode[i] );
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

	return NULL;			// Success
}

int Global::ced_sort ()
{
	int		res = 0,
			rc_mode[EXP_MAX + 1] = {0};
	int		rc[EXP_MAX + 1] = {0};
	char	const * sp = parse_expression ( s_arg, rc, rc_mode, i_verbose );

	if ( sp )
	{
		printf ( "Sort Error!  Bad expression.\n%s\n", s_arg );

		for ( ; sp > s_arg; sp -- )
		{
			putchar ( ' ' );
		}

		printf ( "^\n" );

		i_error = 1;		// Terminate ASAP

		return 0;		// Error printed here
	}

	int tot = i_total;

	if ( tot <= 1 )
	{
		tot = 0;
	}

	if ( i_rowcol == 1 )
	{
		res = ced_sheet_sort_columns ( m_sheet, i_start, tot, rc, 0,
			rc_mode );
	}
	else
	{
		res = ced_sheet_sort_rows ( m_sheet, i_start, tot, rc, 0,
			rc_mode );
	}

	if ( res )
	{
		return ERROR_LIBMTCELLEDIT;	// Error printed by caller
	}

	return 0;			// Success
}

