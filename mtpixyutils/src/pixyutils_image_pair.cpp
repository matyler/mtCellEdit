/*
	Copyright (C) 2019-2022 Mark Tyler

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

#include "pixyutils.h"



int ImagePair::open_file (
	char	const * const	filename,
	int			& i_ftype_in,
	mtPixmap	** const img
	)
{
	int		ft;
	mtPixmap	* pixmap = pixy_pixmap_load ( filename, &ft );

	if ( ! pixmap )
	{
		fprintf ( stderr, "Unable to load '%s'\n", filename );
		return 1;
	}

	i_ftype_in = (int)ft;

	m_pixmap_a.reset ( m_pixmap_b.release () );
	m_pixmap_b.reset ( pixmap );

	if ( false == both_loaded () )
	{
		return -1;
	}

	if ( false == both_match () || prepare_output ( img ) )
	{
		return 1;
	}

	return 0;		// Success - loaded & prepared OK
}

bool ImagePair::both_loaded () const
{
	if ( get_pixmap_a () && get_pixmap_b () )
	{
		return true;
	}

	return false;
}

bool ImagePair::both_match () const
{
	mtPixmap const * const a = get_pixmap_a ();
	mtPixmap const * const b = get_pixmap_b ();

	if (	a && b
		&& pixy_pixmap_get_width (a) == pixy_pixmap_get_width (b)
		&& pixy_pixmap_get_height (a) == pixy_pixmap_get_height (b)
		&& pixy_pixmap_get_bytes_per_pixel (a) ==
			pixy_pixmap_get_bytes_per_pixel (b)
		&& pixy_pixmap_get_canvas (a)
		&& pixy_pixmap_get_canvas (b)
		)
	{
		return true;
	}

	std::cerr << "Images do not share the same geometry.\n";

	return false;
}

int ImagePair::prepare_output ( mtPixmap ** const dup ) const
{
	mtPixmap * pixmap = get_pixmap_a ();

	if ( ! pixmap )
	{
		std::cerr << "No image to duplicate.\n";
		return 1;
	}

	pixmap = pixy_pixmap_duplicate ( pixmap );
	if ( ! pixmap )
	{
		std::cerr << "Unable to duplicate image.\n";
		return 1;
	}

	*dup = pixmap;

	return 0;
}

