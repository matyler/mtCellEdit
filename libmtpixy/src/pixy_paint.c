/*
	Copyright (C) 2016-2021 Mark Tyler

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



void pixy_paint_flow (
	mtPixmap	* const	alpha_mask,
	int		const	flow
	)
{
	if ( ! alpha_mask )
	{
		return;
	}

	if ( flow < PIXY_BRUSH_FLOW_MIN || flow > PIXY_BRUSH_FLOW_MAX )
	{
		return;
	}

	int	const	tot = alpha_mask->width * alpha_mask->height;
	int		r;
	unsigned char	* dest = alpha_mask->alpha;
	unsigned char	* destlim = dest + tot;


	for ( ; dest < destlim; dest++ )
	{
		if ( dest[0] > 0 )
		{
			r = rand () % (PIXY_BRUSH_FLOW_MAX + 1);
			if ( r > flow )
			{
				dest[0] = 0;
			}
		}
	}
}

