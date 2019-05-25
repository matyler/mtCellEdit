/*
	Copyright (C) 2019 Mark Tyler

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



int ImagePair::open_file ( char const * const filename )
{
	mtPixy::File::Type	ft;
	mtPixy::Image		* image = mtPixy::Image::load ( filename, &ft );

	if ( ! image )
	{
		fprintf ( stderr, "Unable to load '%s'\n", filename );
		global.i_error = 1;	// Bail out

		return 1;
	}

	global.i_ftype_in = (int)ft;

	m_image_a.reset ( m_image_b.release () );
	m_image_b.reset ( image );

	if ( false == global.image_pair.both_loaded () )
	{
		return 1;
	}

	if (	false == global.image_pair.both_match () ||
		global.image_pair.prepare_output ()
		)
	{
		global.i_error = 1;	// Bail out, error already reported
		return 1;
	}

	return 0;		// Success - loaded & prepared OK
}

bool ImagePair::both_loaded ()
{
	if ( get_image_a () && get_image_b () )
	{
		return true;
	}

	return false;
}

bool ImagePair::both_match ()
{
	mtPixy::Image * const a = get_image_a ();
	mtPixy::Image * const b = get_image_b ();

	if (	a && b
		&& a->get_width ()	== b->get_width ()
		&& a->get_height ()	== b->get_height ()
		&& a->get_type ()	== b->get_type ()
		&& a->get_canvas ()
		&& b->get_canvas ()
		)
	{
		return true;
	}

	std::cerr << "Images do not share the same geometry.\n";

	return false;
}

int ImagePair::prepare_output ()
{
	mtPixy::Image * image = get_image_a ();

	if ( ! image )
	{
		std::cerr << "No image to duplicate.\n";
		global.i_error = 1;	// Bail out

		return 1;
	}

	image = image->duplicate ();
	if ( ! image )
	{
		std::cerr << "Unable to duplicate image.\n";
		global.i_error = 1;	// Bail out

		return 1;
	}

	global.set_image ( image );

	return 0;
}

