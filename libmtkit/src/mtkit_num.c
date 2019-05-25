/*
	Copyright (C) 2016-2019 Mark Tyler

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



int mtkit_int32_unpack (
	unsigned char	const * const	mem
	)
{
	return ( mem[0] + (mem[1] << 8) + (mem[2] << 16) + (mem[3] << 24) );
}

void mtkit_int32_pack (
	unsigned char	* const	mem,
	int		const	num
	)
{
	mem[0] = (unsigned char)(num);
	mem[1] = (unsigned char)(num >> 8);
	mem[2] = (unsigned char)(num >> 16);
	mem[3] = (unsigned char)(num >> 24);
}

int mtkit_int_bound (
	int	const	num,
	int	const	min,
	int	const	max
	)
{
	return MAX( MIN( num, max ), min );
}

double mtkit_double_bound (
	double	const	num,
	double	const	min,
	double	const	max
	)
{
	if ( isnan ( num ) )
	{
		return 0.0;
	}

	if ( isinf ( num ) )
	{
		return 0.0;
	}

	return MAX( MIN( num, max ), min );
}

