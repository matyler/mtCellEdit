/*
	Copyright (C) 2016-2021 Mark Tyler

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



static void blit_pixels (
	unsigned char	* const dst,
	int		const	dw,	// dst image width
	int		const	dx1,	// dst start X
	int		const	dy1,	// dst start Y
	int		const	dx2,	// dst finish X
	int		const	dy2,	// dst finish Y
	unsigned char const * const src,
	int		const	sw,	// src image width
	int		const	sx1,	// src start X
	int		const	sy1,	// src start Y
	int		const	canvas_bpp	// Bytes per pixel
	)
{
	if ( ! dst || ! src )
	{
		return;
	}


	size_t		const	llen = (size_t)((dx2 - dx1) * canvas_bpp);
	int			y;
	unsigned char	const	* sm = src + (sx1 + sw * sy1) * canvas_bpp;
	unsigned char		* dm = dst + (dx1 + dw * dy1) * canvas_bpp;


	for ( y = dy1; y < dy2; y++ )
	{
		memcpy ( dm, sm, llen );

		sm += sw * canvas_bpp;
		dm += dw * canvas_bpp;
	}
}

int pixy_pixmap_paste (
	mtPixmap	* const	dest,
	mtPixmap const * const	src,
	int		const	x,
	int		const	y
	)
{
	if ( ! src || ! dest || dest->bpp != src->bpp )
	{
		return 1;
	}

	PASTE_SETUP

	switch ( dest->bpp )
	{
	case PIXY_PIXMAP_BPP_INDEXED:
		blit_pixels ( dest->canvas, dest->width, dx1, dy1, dx2,
			dy2, src->canvas, src->width, sx1,
			sy1, src->bpp );
		break;

	case PIXY_PIXMAP_BPP_RGB:
		blit_pixels ( dest->canvas, dest->width, dx1, dy1, dx2,
			dy2, src->canvas, src->width, sx1,
			sy1, src->bpp );
		break;

	default:
		// Ignore other image types
		break;
	}

	blit_pixels ( dest->alpha, dest->width, dx1, dy1, dx2, dy2,
		src->alpha, src->width, sx1, sy1, 1 );

	return 0;
}

int pixy_pixmap_paste_alpha_blend (
	mtPixmap	* const	dest,
	mtPixmap const * const	src,
	int		const	x,
	int		const	y
	)
{
	if ( ! src || ! dest || dest->bpp != src->bpp )
	{
		return 1;
	}

	switch ( dest->bpp )
	{
	case PIXY_PIXMAP_BPP_INDEXED:
		pixy_pixmap_blit_idx_alpha_blend ( src, dest->canvas, x, y,
			dest->width, dest->height );
		break;

	case PIXY_PIXMAP_BPP_RGB:
		pixy_pixmap_blit_rgb_alpha_blend ( src, &dest->palette,
			dest->canvas, x, y, dest->width,
			dest->height, 1 );
		break;

	default:
		// Ignore other image types
		break;
	}

	return 0;
}

static void blit_pixels_alpha_or (
	unsigned char	* const dst,
	int		const	dw,	// dst image width
	int		const	dx1,	// dst start X
	int		const	dy1,	// dst start Y
	int		const	dx2,	// dst finish X
	int		const	dy2,	// dst finish Y
	unsigned char const * const src_alpha,
	int		const	sw,	// src image width
	int		const	sx1,	// src start X
	int		const	sy1	// src start Y
	)
{
	if ( ! dst || ! src_alpha )
	{
		return;
	}

	for ( int y = dy1; y < dy2; y++ )
	{
		unsigned char const * am = src_alpha + (sx1 + sw *
			(sy1 + y - dy1));
		unsigned char * dm = dst + (dx1 + dw * y);

		for ( int x = dx1; x < dx2; x++ )
		{
			dm[0] = (dm[0] | *am++);
			dm++;
		}
	}
}

int pixy_pixmap_paste_alpha_or (
	mtPixmap	* const	dest,
	mtPixmap const * const	src,
	int		const	x,
	int		const	y
	)
{
	if ( ! src || ! dest )
	{
		return 1;
	}

	PASTE_SETUP

	blit_pixels_alpha_or ( dest->alpha, dest->width, dx1, dy1, dx2,
		dy2, src->alpha, src->width, sx1, sy1 );

	return 0;
}

