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



mtPixyUI::PaletteMask::PaletteMask ()
{
	clear ();
}

void mtPixyUI::PaletteMask::clear ()
{
	memset ( color, 0, sizeof( color ) );
}

bool mtPixyUI::PaletteMask::is_masked (
	mtPixy::Image	* const	img,
	int	const	x,
	int	const	y
	) const
{
	int		const	bpp = img->get_canvas_bpp ();
	int		const	w = img->get_width ();
	int		const	h = img->get_height ();
	unsigned char	* const	s = img->get_canvas ();


	if ( x >= w || y >= h || ! s )
	{
		return false;
	}

	if ( bpp == 1 )
	{
		unsigned char	const	pix = s [ x + y*w ];


		if ( 0 == color [ pix ] )
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else if ( bpp == 3 )
	{
		unsigned char	const	r = s [ 3*(x + y*w) + 0 ];
		unsigned char	const	g = s [ 3*(x + y*w) + 1 ];
		unsigned char	const	b = s [ 3*(x + y*w) + 2 ];
		mtPixy::Palette		* const	pal = img->get_palette ();
		mtPixy::Color	const * const	col = pal->get_color ();
		int			const	tot = pal->get_color_total ();


		for ( int i = 0; i < tot; i++ )
		{
			if ( 0 == color[i] )
			{
				continue;
			}

			if (	col[i].red == r &&
				col[i].green == g &&
				col[i].blue == b
				)
			{
				return true;
			}
		}
	}

	return false;
}

static int pal_tree_cmp (
	void	const	* const k1,
	void	const	* const k2
	)
{
	if ( (intptr_t)k1 < (intptr_t)k2 )
	{
		return -1;
	}

	if ( (intptr_t)k1 > (intptr_t)k2 )
	{
		return 1;
	}

	return 0;
}

void mtPixyUI::PaletteMask::protect (
	mtPixy::Image	* const	src,
	mtPixy::Image	* const	dest,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h
	) const
{
	int	const	coltot = src->get_palette ()->get_color_total ();
	int		i;


	for ( i = 0; i < coltot; i++ )
	{
		if ( 0 != color[i] )
		{
			break;
		}
	}

	if ( i == coltot )
	{
		// No colours are masked, so nothing to do
		return;
	}


	unsigned char	* const	s = src->get_canvas ();
	unsigned char	* const	d = dest->get_canvas ();
	unsigned char		* sc, * dc;
	int		const	bpp = src->get_canvas_bpp ();
	int		const	iw = src->get_width ();
	int		const	ix2 = x + w;
	int		const	iy2 = y + h;
	int			ix, iy, rgb;


	if ( bpp == 1 )
	{
		for ( iy = y; iy < iy2; iy++ )
		{
			sc = s + iy * iw;
			dc = d + iy * iw;

			for ( ix = x; ix < ix2; ix++ )
			{
				if ( color[ sc[ ix ] ] > 0 )
				{
					dc[ ix ] = sc[ ix ];
				}
			}
		}
	}
	else if ( bpp == 3 )
	{
		mtTree		* pal_tree;	// Key=RGB Data=Palette Index
		mtTreeNode	* tnode;
		mtPixy::Color	* const	pal = src->get_palette ()->get_color ();


		pal_tree = mtkit_tree_new ( pal_tree_cmp, NULL );
		if ( ! pal_tree )
		{
			return;
		}

		for ( i = 0; i < coltot; i++ )
		{
			if ( 0 != color[i] )
			{
				rgb = mtPixy::rgb_2_int ( pal[i].red,
					pal[i].green, pal[i].blue );

				if ( 0 == mtkit_tree_node_add ( pal_tree,
					(void *)(intptr_t)rgb,
					(void *)(intptr_t)i )
					)
				{
					mtkit_tree_destroy ( pal_tree );
					return;
				}
			}
		}

		for ( iy = y; iy < iy2; iy++ )
		{
			sc = s + (iy * iw + x) * 3;
			dc = d + (iy * iw + x) * 3;

			for ( ix = x; ix < ix2; ix++ )
			{
				rgb = mtPixy::rgb_2_int ( sc[0], sc[1], sc[2] );

				tnode = mtkit_tree_node_find ( pal_tree,
					(void *)(intptr_t)rgb );
				if ( tnode )
				{
					*dc++ = *sc++;
					*dc++ = *sc++;
					*dc++ = *sc++;
				}
				else
				{
					dc += 3;
					sc += 3;
				}
			}
		}

		mtkit_tree_destroy ( pal_tree );
	}
}

