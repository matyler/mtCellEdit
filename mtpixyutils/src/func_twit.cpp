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



static int prepare_alpha ( mtPixmap * const pixmap )
{
	if ( pixy_pixmap_create_alpha ( pixmap ) )
	{
		return 1;
	}

	int		const	w = pixy_pixmap_get_width ( pixmap );
	int		const	h = pixy_pixmap_get_height ( pixmap );
	unsigned char * const	mem = pixy_pixmap_get_alpha ( pixmap );
	size_t		const	pixtot = (size_t)(w * h);

	mem[0] = 254;

	if ( pixtot > 1 )
	{
		memset ( mem + 1, 255, pixtot - 1 );
	}

	return 0;
}

int Global::pixy_twit ()
{
	if ( ut_load_file () )
	{
		// Fail: caller tells user of failure
		return ERROR_LOAD_FILE;
	}

	auto prepare_canvas = [this]() -> int
	{
		// Rescale if too small

		mtPixmap * pixmap = m_pixmap.get();
		int	const	w = pixy_pixmap_get_width ( pixmap );
		int	const	h = pixy_pixmap_get_height ( pixmap );
		int		factor = 1;

		while ( factor < 100 && MIN ( w * factor, h * factor ) < 1000 )
		{
			factor *= 2;
		}

		if ( factor > 1 )
		{
			mtPixmap * im = pixy_pixmap_scale ( pixmap, w * factor,
				h * factor, PIXY_SCALE_BLOCKY );

			if ( ! im )
			{
				return 1;
			}

			set_pixmap ( im );

			pixmap = m_pixmap.get();
		}

		// Create RGB canvas, if not already RGB

		if ( 3 != pixy_pixmap_get_bytes_per_pixel ( pixmap ) )
		{
			mtPixmap * im = pixy_pixmap_convert_to_rgb ( pixmap );
			if ( ! im )
			{
				return 1;
			}

			set_pixmap ( im );
		}

		return 0;
	};

	if (	prepare_canvas ()
		|| prepare_alpha ( m_pixmap.get() )
		)
	{
		// Fail: caller tells user of failure
		return ERROR_LIBMTPIXY;
	}

	i_ftype_out = PIXY_FILE_TYPE_PNG;

	return 0;
}

