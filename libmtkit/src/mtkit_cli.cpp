/*
	Copyright (C) 2016 Mark Tyler

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



int mtKit::cli_parse_int (
	char	const * const	input,
	int			&output,
	int		const	min,
	int		const	max
	)
{
	if ( mtkit_strtoi ( input, &output, NULL, 1 ) )
	{
		fprintf ( stderr, "Bad Integer (%s)\n", input );

		return 1;
	}

	if (	max >= min &&
		( output > max || output < min )
		)
	{
		fprintf ( stderr, "Integer %i out of bounds (%i to %i)\n",
			output, min, max );

		return 1;
	}

	return 0;
}

int mtKit::cli_parse_charint (
	char	const * const	input,
	CharInt	const * const	chint,
	int			&result
	)
{
	if ( ! input || ! chint )
	{
		fprintf ( stderr, "Invalid charint input 'NULL' argument\n" );

		return 1;
	}

	for ( int i = 0; chint[i].name; i++ )
	{
		if ( 0 == strcmp ( chint[i].name, input ) )
		{
			result = chint[i].num;

			return 0;	// Found
		}
	}

	fprintf ( stderr, "Invalid charint input '%s'\n", input );

	return 1;			// Not found
}

