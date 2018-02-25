/*
	Copyright (C) 2016-2017 Mark Tyler

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



static int prepare_canvas ()
{
	// Rescale if too small

	int	const	w = global.image->get_width ();
	int	const	h = global.image->get_height ();
	int		factor = 1;

	while ( factor < 100 && MIN ( w * factor, h * factor ) < 1000 )
	{
		factor *= 2;
	}

	if ( factor > 1 )
	{
		mtPixy::Image * image = global.image->scale ( w * factor,
			h * factor, mtPixy::Image::SCALE_BLOCKY );

		if ( ! image )
		{
			return 1;
		}

		global.set_image ( image );
	}

	// Create RGB canvas, if not already RGB

	if ( 3 != global.image->get_canvas_bpp () )
	{
		mtPixy::Image * image = global.image->convert_to_rgb ();

		if ( ! image )
		{
			// Fail: caller tells user of failure
			return ERROR_LIBMTPIXY;
		}

		global.set_image ( image );
	}

	return 0;
}

static int prepare_alpha ()
{
	if ( global.image->create_alpha () )
	{
		return 1;
	}

	int		const	w = global.image->get_width ();
	int		const	h = global.image->get_height ();
	unsigned char * const	mem = global.image->get_alpha ();
	size_t		const	pixtot = (size_t)(w * h);

	mem[0] = 254;

	if ( pixtot > 1 )
	{
		memset ( mem + 1, 255, pixtot - 1 );
	}

	return 0;
}

int pixyut_twit ()
{
	if ( ut_load_file () )
	{
		// Fail: caller tells user of failure
		return ERROR_LOAD_FILE;
	}

	if ( prepare_canvas () || prepare_alpha () )
	{
		// Fail: caller tells user of failure
		return ERROR_LIBMTPIXY;
	}

	global.i_ftype_out = mtPixy::File::TYPE_PNG;

	return 0;
}

