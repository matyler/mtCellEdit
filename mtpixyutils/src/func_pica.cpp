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



static int analyse_image_cells (
	unsigned char	* const	mem,
	int		const	w,
	int		const	h
	)
{
	int		pal[256];		// Index frequency
	int		cell_tots[256];

	memset ( cell_tots, 0, sizeof(cell_tots) );

	for ( int yo = 0; yo < h; yo += 8 )
	{
		int const ylim = MIN ( 8, h - yo );

		for ( int xo = 0; xo < w; xo += 8 )
		{
			int const xlim = MIN ( 8, w - xo );
			memset ( pal, 0, sizeof(pal) );

			for ( int y = 0; y < ylim; y++ )
			{
				int const yoff = w * (y + yo);

				for ( int x = 0; x < xlim; x++ )
				{
					int const pix = mem[ x + xo + yoff ];

					pal[ pix ]++;
				}
			}

			int tot_cols = 0;

			for ( int i = 0; i < 256; i++ )
			{
				if ( pal[i] )
				{
					tot_cols++;
				}
			}

			cell_tots[ tot_cols ]++;
		}
	}

	printf ( "---------------------------\n" );
	printf ( "Cols\t8x8 Cells with cols\n" );
	printf ( "---------------------------\n" );

	for ( int i = 0; i < 256; i++ )
	{
		if ( 0 == cell_tots[i] )
		{
			continue;
		}

		printf ( "%i	%i\n", i, cell_tots[i] );
	}

	puts ("");

	return 0;
}

int pixyut_pica ()
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
	int const coltot = global.image->get_palette ()->get_color_total ();
	unsigned char		* const	mem = global.image->get_canvas ();

	if ( global.i_verbose )
	{
		printf ( "w=%-5i h=%-5i cols=%-3i bpp=%i%-3s",
			w, h,
			coltot,
			bpp,
			global.image->get_alpha () ? "+A" : "" );
	}

	printf ( "%-5s %s\n", mtPixy::File::type_text (
		(mtPixy::File::Type) global.i_ftype_in ), global.s_arg );

	if ( bpp != 1 || ! mem )
	{
		printf ( "Not an indexed image canvas.\n\n" );

		return 0;
	}

	analyse_image_cells ( mem, w, h );

	return 0;
}

