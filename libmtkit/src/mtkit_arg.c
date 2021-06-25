/*
	Copyright (C) 2009-2017 Mark Tyler

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



int mtkit_arg_int_boundary_check (
	char	const * const	arg_name,
	int		const	arg_val,
	int		const	min,
	int		const	max
	)
{
	if ( ! arg_name || min > max )
	{
		return -1;		// Invalid argument
	}

	if (	arg_val < min ||
		arg_val > max
		)
	{
		fprintf ( stderr, "Argument '%s' (%i) out of bounds [%i,%i]\n",
			arg_name, arg_val, min, max );

		return 1;		// Invalid
	}

	return 0;			// Valid
}

/*
int mtkit_arg_double_boundary_check (
	char	const * const	arg_name,
	double		const	arg_val,
	double		const	min,
	double		const	max
	)
{
	if ( ! arg_name || min > max )
	{
		return -1;		// Invalid argument
	}

	if (	arg_val < min ||
		arg_val > max
		)
	{
		fprintf ( stderr, "Argument '%s' (%f) out of bounds [%f,%f]\n",
			arg_name, arg_val, min, max );

		return 1;		// Invalid
	}

	return 0;			// Valid
}
*/

int mtkit_arg_string_boundary_check (
	char	const * const	arg_name,
	char	const * const	arg_val,
	int		const	min,
	int		const	max
	)
{
	if ( ! arg_name || ! arg_val )
	{
		return -1;		// Invalid argument
	}

	if ( min >= 0 || max >= 0 )
	{
		size_t		len = strlen ( arg_val );


		if (	(min >= 0 && len < (size_t)min)	||
			(max >= 0 && len > (size_t)max)
			)
		{
			fprintf ( stderr,
				"Argument '%s' length out of bounds [%i,%i]\n",
				arg_name, min, max );

			return 1;	// Invalid
		}
	}

	return 0;			// Valid
}

