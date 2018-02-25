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



int pixyut_delta ()
{
	static char const * file_a = NULL;
	static char const * file_b = NULL;

	if ( ! file_a )
	{
		file_a = global.s_arg;
		return 0;
	}

	file_b = global.s_arg;

	mtPixy::File::Type	ft;
	mtPixy::Image		* image_a = mtPixy::image_load ( file_a, &ft );

	if ( ! image_a )
	{
		printf ( "Unable to load '%s'\n", file_a );

		return 0;
	}

	global.i_ftype_in = (int)ft;

	mtPixy::Image * image_b = mtPixy::image_load ( file_b, NULL );
	if ( ! image_b )
	{
		delete ( image_a );
		image_a = NULL;

		printf ( "Unable to load '%s'\n", file_b );

		return 0;
	}

	// Ensure images are the same size & type
	if (	image_a->get_width ()	!= image_b->get_width ()	||
		image_a->get_height ()	!= image_b->get_height ()	||
		image_a->get_type ()	!= image_b->get_type ()		||
		! image_a->get_canvas ()				||
		! image_b->get_canvas ()
		)
	{
		printf ( "Images do not share the same geometry.\n" );

		delete ( image_a );
		image_a = NULL;

		delete ( image_b );
		image_b = NULL;

		return 0;
	}

	mtPixy::Image * image_out = image_a->duplicate ();
	if ( ! image_out )
	{
		printf ( "Unable to duplicate image.\n" );

		delete ( image_a );
		image_a = NULL;

		delete ( image_b );
		image_b = NULL;

		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	unsigned char * mem_a = image_a->get_canvas ();
	unsigned char * mem_b = image_b->get_canvas ();
	unsigned char * mem_out = image_out->get_canvas ();
	unsigned char * mem_end = mem_out + image_a->get_canvas_bpp () *
		(image_a->get_width () * image_a->get_height ());

	for ( ; mem_out < mem_end; mem_out++, mem_a++, mem_b++ )
	{
		mem_out[0] = (unsigned char)(128 + (mem_a[0] - mem_b[0]));
	}

	delete ( image_a );
	image_a = NULL;

	delete ( image_b );
	image_b = NULL;

	/* Successully done, so allow daisy chaining, e.g.
	pixydelta in1.png in2.png -o out1.png in3.png -o out2.png
	*/

	file_a = file_b;

	global.set_image ( image_out );

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

	mtPixy::Image * image = mtPixy::image_create (
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

