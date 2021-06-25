/*
	Copyright (C) 2017-2020 Mark Tyler

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



static int prepare_output ( mtPixmap * const pixmap )
{
	if ( ! pixmap )
	{
		printf ( "Unable to create output image.\n" );
		return 1;
	}

	mtPalette * const pal = pixy_pixmap_get_palette ( pixmap );

	pixy_palette_set_grey ( pal );

	mtColor * const col = &pal->color[0];

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
	unsigned char	* const mem = pixy_pixmap_get_canvas ( pixmap );
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

static mtPixmap * analyse_rgb_image (
	unsigned char	const * const	rgb,
	int			const	w,
	int			const	h
	)
{
	int		* mem_r = (int *)calloc ( 256*256, sizeof(int) );
	int		* mem_g = (int *)calloc ( 256*256, sizeof(int) );
	int		* mem_b = (int *)calloc ( 256*256, sizeof(int) );
	mtPixmap	* pixmap = nullptr;

	if ( ! mem_r || ! mem_g || ! mem_b )
	{
		fprintf ( stderr, "Unable to allocate memory\n" );
		goto finish;
	}

	pixmap = pixy_pixmap_new_indexed ( 768, 256 );

	if ( prepare_output ( pixmap ) )
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
		unsigned char * const	dest = pixy_pixmap_get_canvas( pixmap );
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

finish:
	free ( mem_r );
	mem_r = NULL;

	free ( mem_g );
	mem_g = NULL;

	free ( mem_b );
	mem_b = NULL;

	return pixmap;
}

int Global::pixy_rida ()
{
	if ( ut_load_file () )
	{
		// Unable to load file - non-fatal error for 'rida'

		printf ( "????? %s\n", s_arg );

		return 0;
	}

	mtPixmap const * const pixmap = m_pixmap.get();
	int	const	bpp = pixy_pixmap_get_bytes_per_pixel ( pixmap );
	int	const	w = pixy_pixmap_get_width ( pixmap );
	int	const	h = pixy_pixmap_get_height ( pixmap );
	unsigned char	const * const	rgb = pixy_pixmap_get_canvas ( pixmap );

	if ( i_verbose )
	{
		printf ( "w=%-5i h=%-5i cols=%-3i bpp=%i%-3s",
			w, h,
			pixy_pixmap_get_palette_size ( pixmap ),
			bpp,
			pixy_pixmap_get_alpha ( pixmap ) ? "+A" : "" );
	}

	printf ( "%-5s %s\n", pixy_file_type_text ( i_ftype_in ), s_arg );

	if ( bpp != 3 || ! rgb )
	{
		printf ( "Not an RGB image canvas.\n\n" );

		return 0;
	}

	mtPixmap * const im = analyse_rgb_image ( rgb, w, h );
	if ( im )
	{
		set_pixmap ( im );
	}

	return 0;
}

