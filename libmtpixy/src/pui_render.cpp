/*
	Copyright (C) 2016-2018 Mark Tyler

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



static void render_chequers (
	unsigned char	* const	rgb,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h
	)
{
	if ( ! rgb )
	{
		return;
	}

	unsigned char	const	grey[2] = { 100, 150 };
	int		const	y2a = MIN ( y + h, y + 16 );

	for ( int j = y; j < y2a; j++ )
	{
		unsigned char * dst = rgb + 3 * w * (j - y);

		for ( int i = x; i < (x + w); i++ )
		{
			unsigned char pix = grey[ ((i / 8) + (j / 8)) % 2 ];

			*dst++ = pix;
			*dst++ = pix;
			*dst++ = pix;
		}
	}

	size_t	const	llen = (size_t)(w * 3);

	for ( int j = 16; j < h; j++ )
	{
		unsigned char * src = rgb + 3 * w * (j % 16);
		unsigned char * dst = rgb + 3 * w * j;

		memcpy ( dst, src, llen );
	}
}

mtPixy::Image * mtPixyUI::File::render_canvas (
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h,
	int	const	zs
	)
{
	if ( ! m_image )
	{
		return NULL;
	}

	mtPixy::Image * const im = mtPixy::Image::create (
		mtPixy::Image::TYPE_RGB, w, h );
	if ( ! im )
	{
		return NULL;
	}

	if ( m_image->get_alpha () )
	{
		render_chequers ( im->get_canvas (), x, y, w, h );

		m_image->blit_rgb_alpha_blend ( m_image->get_palette ()->
			get_color (), im->get_canvas (), -x, -y, w, h, zs );
	}
	else
	{
		m_image->blit_rgb ( m_image->get_palette ()->get_color (),
			im->get_canvas (), -x, -y, w, h, zs );
	}

	return im;
}

void mtPixyUI::File::render_zoom_grid (
	unsigned char	* const	rgb,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	int		const	zs,
	unsigned char	const	gry
	)
{
	unsigned char	* dst;
	int		i, j, k;
	int	const	maxx = x + w;
	int	const	maxy = y + h;
	int	const	ui = MAX ( 1, zs / 20 );
	size_t	const	hb = (size_t)(3 * w);


	// Horizontal lines
	for ( j = y; j < maxy; j += zs - (j % zs) )
	{
		for ( k = (j % zs); k < ui && j < maxy; k++, j++ )
		{
			dst = rgb + 3 * w * (j - y);
			memset ( dst, gry, hb );
		}
	}

	// Vertical lines
	for ( i = x; i < maxx; i += zs - (i % zs) )
	{
		for ( k = (i % zs); k < ui && i < maxx; k++, i++ )
		{
			dst = rgb + 3 * (i - x);

			for ( j = 0; j < h; j++ )
			{
				dst[0] = gry;
				dst[1] = gry;
				dst[2] = gry;

				dst += hb;
			}
		}
	}
}

