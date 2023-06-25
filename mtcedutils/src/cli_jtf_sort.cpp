/*
	Copyright (C) 2012-2022 Mark Tyler

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

#include "cli.h"



#define EXP_MAX 10



static int parse_expression (
	char	const *	input,
	int	* const	rc,
	int	* const	rc_mode
	)
{
	int		i;


	for ( i = 0; i < EXP_MAX; i++ )
	{
		int	num;
		char	* nxt;

		// NUMBER
		if ( mtkit_strtoi ( input, &num, &nxt, 0 ) )
		{
			fprintf ( stderr, "Bad number: '%s'\n\n", input );

			return 1;
		}

		input = nxt;
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
	CuiBook			* const cubook,
	CedSheet		* const	sheet,
	int			const	rows,
	char	const * const * const	args
	)
{
	int		rc_mode[EXP_MAX + 1]	= {0};
	int		rc[EXP_MAX + 1]		= {0};

	if ( ! sheet || parse_expression ( args[0], rc, rc_mode ) )
	{
		return 2;
	}

	int		res, r1, c1, r2, c2;

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	if ( rows )
	{
		res = cui_sheet_sort_rows ( cubook, sheet,
			r1, r2 - r1 + 1, rc, 0, rc_mode );
	}
	else
	{
		res = cui_sheet_sort_columns ( cubook, sheet,
			c1, c2 - c1 + 1, rc, 0, rc_mode );
	}

	if ( res )
	{
		fprintf ( stderr, "Error sorting.\n\n" );

		return 2;
	}

	return 0;
}

int Backend::jtf_sort_column (
	char	const * const * const	args
	)
{
	return sort_rowcol ( file()->cubook, sheet (), 0, args );
}

int Backend::jtf_sort_row (
	char	const * const * const	args
	)
{
	return sort_rowcol ( file()->cubook, sheet (), 1, args );
}

