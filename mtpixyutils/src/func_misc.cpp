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



int pixyut_cmp ()
{
	if ( global.image_pair.open_file ( global.s_arg ) )
	{
		return 0;
	}

	mtPixy::Image * const image_a = global.image_pair.get_image_a ();
	mtPixy::Image * const image_b = global.image_pair.get_image_b ();

	unsigned char const * const mem_a = image_a->get_canvas ();
	unsigned char const * const mem_b = image_b->get_canvas ();

	int64_t const pixels = image_a->get_canvas_bpp () *
		(image_a->get_width () * image_a->get_height ());

	for ( int64_t p = 0; p < pixels; p++ )
	{
		if ( mem_a[p] != mem_b[p] )
		{
			std::cerr << "Images not identical\n";
			global.i_error = 1;	// Bail out

			return 0;
		}
	}

	return 0;
}

int pixyut_delta ()
{
	if ( global.image_pair.open_file ( global.s_arg ) )
	{
		return 0;
	}

	mtPixy::Image * const image_a = global.image_pair.get_image_a ();
	mtPixy::Image * const image_b = global.image_pair.get_image_b ();

	unsigned char const * const mem_a = image_a->get_canvas ();
	unsigned char const * const mem_b = image_b->get_canvas ();
	unsigned char * const dest = global.image->get_canvas ();

	int64_t const pixels = image_a->get_canvas_bpp () *
		(image_a->get_width () * image_a->get_height ());

	for ( int64_t p = 0; p < pixels; p++ )
	{
		dest[p] = (unsigned char)(128 + mem_a[p] - mem_b[p]);
	}

	return 0;
}

int pixyut_ls ()
{
	if ( ut_load_file () )
	{
		// Unable to load file - non-fatal error for 'ls'

		printf ( "????? %s\n", global.s_arg );

		return 0;
	}

	if ( global.i_verbose )
	{
		printf ( "w=%-5i h=%-5i cols=%-3i bpp=%i%-3s",
			global.image->get_width (),
			global.image->get_height (),
			global.image->get_palette ()->get_color_total (),
			global.image->get_canvas_bpp (),
			global.image->get_alpha () ? "+A" : "" );
	}

	printf ( "%-5s %s\n", mtPixy::File::type_text (
		(mtPixy::File::Type) global.i_ftype_in ), global.s_arg );

	return 0;
}

int pixyut_new ()
{
	if (	global.i_palette < 0 ||
		global.i_palette > mtPixy::Palette::UNIFORM_MAX
		)
	{
		return ERROR_BAD_PALETTE; // Fail: caller tells user of failure
	}

	mtPixy::Image * image = mtPixy::Image::create (
		(mtPixy::Image::Type)global.i_image_type,
		global.i_width, global.i_height );

	if ( ! image )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	global.set_image ( image );

	mtPixy::Palette	* const pal = image->get_palette ();

	switch ( global.i_palette )
	{
	case 0:
		break;

	case 1:
		pal->set_grey ();
		break;

	default:
		pal->set_uniform ( global.i_palette );
		break;
	}

	return 0;
}

int pixyut_resize ()
{
	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail: caller tells user of failure
	}

	mtPixy::Image * image = global.image->resize ( global.i_x, global.i_y,
		global.i_width, global.i_height );

	if ( ! image )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	global.set_image ( image );

	return 0;
}

int pixyut_scale ()
{
	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail: caller tells user of failure
	}

	mtPixy::Image::ScaleType st = mtPixy::Image::SCALE_BLOCKY;

	if (	global.image->get_type () == mtPixy::Image::TYPE_RGB &&
		global.i_scale == 0
		)
	{
		st = mtPixy::Image::SCALE_SMOOTH;
	}

	mtPixy::Image * image = global.image->scale ( global.i_width,
		global.i_height, st );

	if ( ! image )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	global.set_image ( image );

	return 0;
}

