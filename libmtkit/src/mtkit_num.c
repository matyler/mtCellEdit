/*
	Copyright (C) 2016-2020 Mark Tyler

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
		return min;
	}

	if ( isinf ( num ) )
	{
		return (num < 0) ? min : max;
	}

	return MAX( MIN( num, max ), min );
}

double mtkit_angle_normalize ( double degrees )
{
	degrees = fmod ( degrees, 360 );

	if ( degrees < 0 )
	{
		degrees += 360;
	}

	return degrees;
}

