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



int pixyut_ls ( void )
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

int pixyut_new ( void )
{
	mtPixy::Image	* image;
	mtPixy::Palette	* pal;


	if (	global.i_palette < 0 ||
		global.i_palette > mtPixy::Palette::UNIFORM_MAX
		)
	{
		return ERROR_BAD_PALETTE; // Fail: caller tells user of failure
	}

	image = mtPixy::image_create ((mtPixy::Image::Type)global.i_image_type,
		global.i_width, global.i_height );

	if ( ! image )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	global.set_image ( image );

	pal = image->get_palette ();

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

int pixyut_resize ( void )
{
	mtPixy::Image	* image;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail: caller tells user of failure
	}

	image = global.image->resize ( global.i_x, global.i_y, global.i_width,
		global.i_height );

	if ( ! image )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	global.set_image ( image );

	return 0;
}

int pixyut_scale ( void )
{
	mtPixy::Image	* image;


	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail: caller tells user of failure
	}

	mtPixy::Image::ScaleType st = mtPixy::Image::BLOCKY;

	if (	global.image->get_type () == mtPixy::Image::RGB &&
		global.i_scale == 0
		)
	{
		st = mtPixy::Image::SMOOTH;
	}

	image = global.image->scale ( global.i_width, global.i_height, st );

	if ( ! image )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	global.set_image ( image );

	return 0;
}

