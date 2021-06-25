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



int Global::pixy_cmp ()
{
	mtPixmap * pixmap;

	switch ( m_image_pair.open_file ( s_arg, i_ftype_in, &pixmap ) )
	{
	case 0:
		// Both images are loaded, so do some work.
		break;

	case -1:
		// Only one image loaded.
		return 0;

	case 1:
		i_error = 1;	// Error, so bail out
		return 0;
	}

	set_pixmap ( pixmap );

	mtPixmap const * const pixmap_a = m_image_pair.get_pixmap_a ();
	mtPixmap const * const pixmap_b = m_image_pair.get_pixmap_b ();

	unsigned char const * const mem_a = pixy_pixmap_get_canvas ( pixmap_a );
	unsigned char const * const mem_b = pixy_pixmap_get_canvas ( pixmap_b );

	int64_t const pixels = pixy_pixmap_get_bytes_per_pixel ( pixmap_a ) *
		pixy_pixmap_get_width ( pixmap_a ) *
		pixy_pixmap_get_height( pixmap_a );

	for ( int64_t p = 0; p < pixels; p++ )
	{
		if ( mem_a[p] != mem_b[p] )
		{
			std::cerr << "Images not identical\n";
			i_error = 1;	// Bail out

			return 0;
		}
	}

	return 0;
}

int Global::pixy_delta ()
{
	mtPixmap * pixmap;

	switch ( m_image_pair.open_file ( s_arg, i_ftype_in, &pixmap ) )
	{
	case 0:
		// Both images are loaded, so do some work.
		break;

	case -1:
		// Only one image loaded.
		return 0;

	case 1:
		i_error = 1;	// Error, so bail out
		return 0;
	}

	set_pixmap ( pixmap );

	mtPixmap const * const pixmap_a = m_image_pair.get_pixmap_a ();
	mtPixmap const * const pixmap_b = m_image_pair.get_pixmap_b ();

	unsigned char const * const mem_a = pixy_pixmap_get_canvas ( pixmap_a );
	unsigned char const * const mem_b = pixy_pixmap_get_canvas ( pixmap_b );
	unsigned char * const dest = pixy_pixmap_get_canvas ( m_pixmap.get() );

	int64_t const pixels = pixy_pixmap_get_bytes_per_pixel ( pixmap_a ) *
		pixy_pixmap_get_width ( pixmap_a ) *
		pixy_pixmap_get_height( pixmap_a );

	for ( int64_t p = 0; p < pixels; p++ )
	{
		dest[p] = (unsigned char)(128 + mem_a[p] - mem_b[p]);
	}

	return 0;
}

int Global::pixy_ls ()
{
	if ( ut_load_file () )
	{
		// Unable to load file - non-fatal error for 'ls'

		printf ( "????? %s\n", s_arg );

		return 0;
	}

	if ( i_verbose )
	{
		mtPixmap const * const pixmap = m_pixmap.get();

		printf ( "w=%-5i h=%-5i cols=%-3i bpp=%i%-3s",
			pixy_pixmap_get_width ( pixmap ),
			pixy_pixmap_get_height ( pixmap ),
			pixy_pixmap_get_palette_size ( pixmap ),
			pixy_pixmap_get_bytes_per_pixel( pixmap ),
			pixy_pixmap_get_alpha ( pixmap ) ? "+A" : "" );
	}

	printf ( "%-5s %s\n", pixy_file_type_text ( i_ftype_in ), s_arg );

	return 0;
}

int Global::pixy_new ()
{
	if (	i_palette < 0 ||
		i_palette > PIXY_PALETTE_UNIFORM_MAX
		)
	{
		return ERROR_BAD_PALETTE; // Fail: caller tells user of failure
	}

	mtPixmap * pixmap = pixy_pixmap_new ( i_image_type, i_width, i_height );

	if ( ! pixmap )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	set_pixmap ( pixmap );

	mtPalette * const pal = pixy_pixmap_get_palette ( pixmap );

	switch ( i_palette )
	{
	case 0:
		break;

	case 1:
		pixy_palette_set_grey ( pal );
		break;

	default:
		pixy_palette_set_uniform ( pal, i_palette );
		break;
	}

	return 0;
}

int Global::pixy_resize ()
{
	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail: caller tells user of failure
	}

	mtPixmap * pixmap = pixy_pixmap_resize ( m_pixmap.get(), i_x, i_y,
		i_width, i_height );

	if ( ! pixmap )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	set_pixmap ( pixmap );

	return 0;
}

int Global::pixy_scale ()
{
	if ( ut_load_file () )
	{
		return ERROR_LOAD_FILE;	// Fail: caller tells user of failure
	}

	int st = PIXY_SCALE_BLOCKY;

	if (	pixy_pixmap_get_bytes_per_pixel ( m_pixmap.get() ) ==
			PIXY_PIXMAP_BPP_RGB
		&& i_scale == 0
		)
	{
		st = PIXY_SCALE_SMOOTH;
	}

	mtPixmap * const pixmap = pixy_pixmap_scale ( m_pixmap.get(), i_width,
		i_height, st );

	if ( ! pixmap )
	{
		return ERROR_LIBMTPIXY; // Fail: caller tells user of failure
	}

	set_pixmap ( pixmap );

	return 0;
}

