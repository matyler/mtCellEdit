/*
	Copyright (C) 2017-2018 Mark Tyler

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



int * create_cube_analysis (
	int			const	i,
	unsigned char	const * const	rgb,
	int			const	w,
	int			const	h
	)
{
	if ( i < 1 || i > 8 || ! rgb || w < 1 || h < 1 )
	{
		return NULL;
	}

	size_t			const	tot_pixels = (size_t)(w * h);
	unsigned char	const * const	lim = rgb + 3 * tot_pixels;
	int			const	span = 1 << i;
	int			const	j = 8 - i;
	int			const	tot_mem = (span * span * span);

	int * mem = (int *)calloc ( (size_t)tot_mem, sizeof(int) );

	if ( ! mem )
	{
		printf ( "Unable to allocate memory.\n" );
		return NULL;
	}

	unsigned char	const *	src = rgb;

	for ( ; src < lim; src += 3 )
	{
		int const r = src[0] >> j;
		int const g = src[1] >> j;
		int const b = src[2] >> j;

		mem[ r + (g * span) + (b * span * span) ]++;
	}

	return mem;
}

static int prepare_output (
	mtPixy::Image	* const	im
	)
{
	if ( ! im )
	{
		printf ( "Unable to create output image.\n" );
		return 1;
	}

	mtPixy::Palette * const pal = im->get_palette ();

	pal->set_grey ();

	mtPixy::Color * const col = pal->get_color ();

	col[1].red = 32;
	col[1].green = 32;
	col[1].blue = 32;

	// Set up blue chequers on the top line
	unsigned char * const mem = im->get_canvas ();

	for ( int i = 0; i < 256; i++ )
	{
		unsigned char * dest = mem + 256;

		for ( int j = 0; j < 8; j++ )
		{
			dest[ i + 2*j*256 ] = 1;
		}
	}

	// Create next 255 lines below top
	for ( int i = 1; i < 256; i++ )
	{
		memcpy ( mem + 4096*i, mem, 4096 );
	}

	// Copy top chequers to 2nd row (slight overlap so use memove)
	memmove ( mem + 256*4096, mem + 256, 256*4096 );

	// Copy next 14 rows of chequers
	for ( int i = 1; i < 8; i++ )
	{
		memcpy ( mem + 4096*512*i, mem, 4096*512 );
	}

	return 0;
}

static void analyse_rgb_image (
	unsigned char	const * const	rgb,
	int			const	w,
	int			const	h
	)
{
	int	* mem = create_cube_analysis ( 8, rgb, w, h );

	if ( ! mem )
	{
		return;
	}

	mtPixy::Image * im = mtPixy::Image::create( mtPixy::Image::TYPE_INDEXED,
		4096, 4096 );

	if ( prepare_output ( im ) )
	{
		free ( mem );
		return;
	}

	// Map RGB cube totals to the image
	int		const	memtot = 256*256*256;
	unsigned char * const	dest = im->get_canvas ();

	for ( int i = 0; i < memtot; i++ )
	{
		if ( 0 == mem[i] )
		{
			continue;
		}

		int const r = (i % 256);
		int const g = ((i >> 8) % 256);
		int const b = ((i >> 16) % 256);

		int const bx = (b % 16);
		int const by = ((b / 16) % 16);

		int const x = r + 256 * bx;
		int const y = g + 256 * by;

		int const pix = MAX ( 96, MIN ( 255, 95 + mem[i] ) );

		dest[ x + y * 4096 ] = (unsigned char)pix;
	}

	global.set_image ( im );

	free ( mem );
}

static int prepare_output64 (
	mtPixy::Image	* const	im
	)
{
	if ( ! im )
	{
		printf ( "Unable to create output image.\n" );
		return 1;
	}

	mtPixy::Palette * const pal = im->get_palette ();

	pal->set_grey ();

	mtPixy::Color * const col = pal->get_color ();

	col[1].red = 32;
	col[1].green = 32;
	col[1].blue = 32;

	// Set up blue chequers on the top line
	unsigned char * const mem = im->get_canvas ();

	for ( int i = 0; i < 64; i++ )
	{
		unsigned char * dest = mem + 64;

		for ( int j = 0; j < 4; j++ )
		{
			dest[ i + 2*j*64 ] = 1;
		}
	}

	// Create next 255 lines below top
	for ( int i = 1; i < 64; i++ )
	{
		memcpy ( mem + 512*i, mem, 512 );
	}

	// Copy top chequers to 2nd row (slight overlap so use memove)
	memmove ( mem + 64*512, mem + 64, 64*512 );

	// Copy next 7 rows of chequers
	for ( int i = 1; i < 4; i++ )
	{
		memcpy ( mem + 512*128*i, mem, 512*128 );
	}

	return 0;
}

static void analyse_rgb_image64 (
	unsigned char	const * const	rgb,
	int			const	w,
	int			const	h
	)
{
	int	* mem = create_cube_analysis ( 6, rgb, w, h );

	if ( ! mem )
	{
		return;
	}

	mtPixy::Image * im = mtPixy::Image::create( mtPixy::Image::TYPE_INDEXED,
		512, 512 );

	if ( prepare_output64 ( im ) )
	{
		free ( mem );
		return;
	}

	// Map RGB cube totals to the image
	int		const	memtot = 64*64*64;
	unsigned char * const	dest = im->get_canvas ();

	for ( int i = 0; i < memtot; i++ )
	{
		if ( 0 == mem[i] )
		{
			continue;
		}

		int const r = ((i >> 0) & 63);
		int const g = ((i >> 6) & 63);
		int const b = ((i >> 12) & 63);

		int const bx = (b & 7);
		int const by = ((b / 8) & 7);

		int const x = r + 64 * bx;
		int const y = g + 64 * by;

		int const pix = MAX ( 96, MIN ( 255, 95 + mem[i] ) );

		dest[ x + y * 512 ] = (unsigned char)pix;
	}

	global.set_image ( im );

	free ( mem );
}

int pixyut_risa ()
{
	if ( ut_load_file () )
	{
		// Unable to load file - non-fatal error for 'risa'

		printf ( "????? %s\n", global.s_arg );

		return 0;
	}

	int			const	bpp = global.image->get_canvas_bpp ();
	int			const	w = global.image->get_width ();
	int			const	h = global.image->get_height ();
	unsigned char	const * const	rgb = global.image->get_canvas ();

	if ( global.i_verbose )
	{
		printf ( "w=%-5i h=%-5i cols=%-3i bpp=%i%-3s",
			w, h,
			global.image->get_palette ()->get_color_total (),
			bpp,
			global.image->get_alpha () ? "+A" : "" );
	}

	printf ( "%-5s %s\n", mtPixy::File::type_text (
		(mtPixy::File::Type) global.i_ftype_in ), global.s_arg );

	if ( bpp != 3 || ! rgb )
	{
		printf ( "Not an RGB image canvas.\n\n" );

		return 0;
	}

	if ( global.i_verbose )
	{
		analyse_rgb_image ( rgb, w, h );
	}
	else
	{
		analyse_rgb_image64 ( rgb, w, h );
	}

	return 0;
}

