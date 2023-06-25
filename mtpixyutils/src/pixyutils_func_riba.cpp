/*
	Copyright (C) 2017-2022 Mark Tyler

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



static void analyse_rgb_image (
	int			const	verbose,
	unsigned char	const * const	rgb,
	int			const	w,
	int			const	h
	)
{
	size_t			const	tot_pixels = (size_t)(w * h);
	unsigned char	const * const	lim = rgb + 3 * tot_pixels;


	for ( int i = 1; i <= 8; i++ )
	{
		int	const	span = 1 << i;
		int	const	tot_mem = (span * span * span);
		int		* mem = create_cube_analysis ( i, rgb, w, h );

		if ( ! mem )
		{
			return;
		}

		int	const * const	smlim = mem + tot_mem;
		int			buckets = 0;

		for ( int const * sm = mem; sm < smlim; sm++ )
		{
			if ( sm[0] )
			{
				buckets++;
			}
		}

		double	const	perc = 100 * (double)buckets / (double)tot_mem;

		printf( "%i. %8i / %8i = %4.2f%%\n", i, buckets, tot_mem, perc);


		if ( verbose && 8 == i )
		{
			printf ( "--------------------------------\n" );
			printf ( "R+G+B	Pixels	Cols	Tot\n" );
			printf ( "--------------------------------\n" );

			int const tot = 766;
			int tot_pix[tot], tot_cols[tot], tot_tot[tot];

			memset ( tot_pix, 0, sizeof(tot_pix) );
			memset ( tot_cols, 0, sizeof(tot_cols) );
			memset ( tot_tot, 0, sizeof(tot_cols) );

			unsigned char	const *	src = rgb;

			for ( ; src < lim; src += 3 )
			{
				int const v = src[0] + src[1] + src[2];

				tot_pix[ v ]++;
			}

			for ( int const * sm = mem; sm < smlim; sm++ )
			{
				int const o = (int)(sm - mem);
				int const r = o % 256;
				int const g = (o >> 8) % 256;
				int const b = (o >> 16) % 256;

				tot_tot[ r + g + b ]++;

				if ( sm[0] )
				{
					tot_cols[ r + g + b ]++;
				}
			}

			for ( int k = 0; k < tot; k++ )
			{
				printf ( "%i	%i	%i	%i\n", k,
					tot_pix[k], tot_cols[k], tot_tot[k] );
			}
		}

		free ( mem );
	}

	puts ( "" );
}

int Global::pixy_riba ()
{
	if ( ut_load_file () )
	{
		// Unable to load file - non-fatal error for 'riba'

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

	analyse_rgb_image ( i_verbose, rgb, w, h );

	return 0;
}

