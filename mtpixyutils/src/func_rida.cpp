/*
	Copyright (C) 2017 Mark Tyler

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

	col[1].red = 64;
	col[1].green = 0;
	col[1].blue = 0;
	col[2].red = 0;
	col[2].green = 48;
	col[2].blue = 0;
	col[3].red = 0;
	col[3].green = 0;
	col[3].blue = 64;

	// Set up blue chequers on the top line
	unsigned char	* const mem = im->get_canvas ();
	unsigned char		c = 0;

	for ( int i = 0; i < 768; i+=256 )
	{
		unsigned char * dest = mem + i;
		c++;

		for ( int j = 0; j < 256; j++ )
		{
			dest[ j ] = c;
		}
	}

	// Create next 255 lines below top
	for ( int i = 1; i < 256; i++ )
	{
		memcpy ( mem + 768*i, mem, 768 );
	}

	return 0;
}

static void analyse_rgb_image (
	unsigned char	const * const	rgb,
	int			const	w,
	int			const	h
	)
{
	int		* mem_r = (int *)calloc ( 256*256, sizeof(int) );
	int		* mem_g = (int *)calloc ( 256*256, sizeof(int) );
	int		* mem_b = (int *)calloc ( 256*256, sizeof(int) );
	mtPixy::Image	* im = NULL;

	if ( ! mem_r || ! mem_g || ! mem_b )
	{
		fprintf ( stderr, "Unable to allocate memory\n" );
		goto finish;
	}

	im = image_create ( mtPixy::Image::TYPE_INDEXED, 768, 256 );

	if ( prepare_output ( im ) )
	{
		goto finish;
	}
	else
	{
		int		const	pixtot = w*h;
		unsigned char	const	* src = rgb + 3;

		for ( int i = 1; i < pixtot; i++, src += 3 )
		{
			mem_r [ src[-3] + 256 * src[0] ]++;
			mem_g [ src[-2] + 256 * src[1] ]++;
			mem_b [ src[-1] + 256 * src[2] ]++;
		}

		// Map RGB totals to the image

		int		const	memtot = 256*256;
		unsigned char * const	dest = im->get_canvas ();
		int		const	* memarr[3] = { mem_r, mem_g, mem_b };

		for ( int k = 0; k < 3; k++ )
		{
			int		const	bx = 256 * k;
			int	const * const	mem = memarr[k];

			for ( int i = 0; i < memtot; i++ )
			{
				if ( 0 == mem[i] )
				{
					continue;
				}

				int const r1 = (i % 256);
				int const r2 = ((i >> 8) % 256);

				int const x = r1 + bx;
				int const y = r2;

				int const pix = MAX(96, MIN( 255, 95 + mem[i]));

				dest[ x + y * 768 ] = (unsigned char)pix;
			}
		}
	}

	global.set_image ( im );

finish:
	free ( mem_r );
	mem_r = NULL;

	free ( mem_g );
	mem_g = NULL;

	free ( mem_b );
	mem_b = NULL;
}

int pixyut_rida ()
{
	if ( ut_load_file () )
	{
		// Unable to load file - non-fatal error for 'rida'

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

	analyse_rgb_image ( rgb, w, h );

	return 0;
}

