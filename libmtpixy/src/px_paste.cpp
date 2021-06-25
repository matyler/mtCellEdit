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



static void blit_pattern_rgb (
	unsigned char	* const dst,
	int		const	dw,	// dst image width
	int		const	dx1,	// dst start X
	int		const	dy1,	// dst start Y
	int		const	dx2,	// dst finish X
	int		const	dy2,	// dst finish Y
	unsigned char const * const src_alpha,
	int		const	sw,	// src image width
	int		const	sx1,	// src start X
	int		const	sy1,	// src start Y
	mtPixmap	* const	pat
	)
{
	if ( ! dst || ! pat )
	{
		return;
	}


	int		const	ps = mtPixy::Brush::PATTERN_SIZE;
	unsigned char const * const pat_canvas = pixy_pixmap_get_canvas ( pat );


	for ( int y = dy1; y < dy2; y++ )
	{
		unsigned char const * am = src_alpha + (sx1 + sw *
			(sy1 + y - dy1));
		unsigned char const * const sm = pat_canvas + (y % ps) * ps * 3;
		unsigned char * dm = dst + (dx1 + dw * y) * 3;

		for ( int x = dx1; x < dx2; x++ )
		{
			unsigned char const * const smx = sm + 3 * (x % ps);
			unsigned char const a = *am++;
			unsigned char const b = (unsigned char)(255 - a);

			dm[0] = (unsigned char)((a * smx[0] + dm[0] * b) / 255);
			dm[1] = (unsigned char)((a * smx[1] + dm[1] * b) / 255);
			dm[2] = (unsigned char)((a * smx[2] + dm[2] * b) / 255);

			dm += 3;
		}
	}
}

static void blit_pattern_idx (
	unsigned char	* const dst,
	int		const	dw,	// dst image width
	int		const	dx1,	// dst start X
	int		const	dy1,	// dst start Y
	int		const	dx2,	// dst finish X
	int		const	dy2,	// dst finish Y
	unsigned char const * const src_alpha,
	int		const	sw,	// src image width
	int		const	sx1,	// src start X
	int		const	sy1,	// src start Y
	mtPixmap	* const	pat
	)
{
	if ( ! dst || ! pat )
	{
		return;
	}


	int		const	ps = mtPixy::Brush::PATTERN_SIZE;
	unsigned char const * const pat_canvas = pixy_pixmap_get_canvas ( pat );


	for ( int y = dy1; y < dy2; y++ )
	{
		unsigned char const * am = src_alpha + (sx1 + sw *
			(sy1 + y - dy1));
		unsigned char const * const sm = pat_canvas + (y % ps) * ps;
		unsigned char * dm = dst + (dx1 + dw * y);

		for ( int x = dx1; x < dx2; x++ )
		{
			if ( *am++ > 0 )
			{
				// Use pixel from src
				*dm++ = sm [ x % ps ];
			}
			else
			{
				// Use pixel currently on dest
				dm++;
			}
		}
	}
}

int mtPixy::paste_alpha_pattern (
	mtPixmap	* const	dest,
	mtPixmap const * const	src,
	Brush			&bru,
	int		const	x,
	int		const	y
	)
{
	if ( ! dest || ! src || ! src->alpha )
	{
		return 1;
	}

	PASTE_SETUP

	switch ( dest->bpp )
	{
	case PIXY_PIXMAP_BPP_INDEXED:
		blit_pattern_idx ( dest->canvas, dest->width, dx1, dy1,
			dx2, dy2, src->alpha, src->width,
			sx1, sy1, bru.get_pattern_idx () );
		break;

	case PIXY_PIXMAP_BPP_RGB:
		blit_pattern_rgb ( dest->canvas, dest->width, dx1, dy1,
			dx2, dy2, src->alpha, src->width,
			sx1, sy1, bru.get_pattern_rgb () );
		break;

	default:
		// Ignore other image types
		break;
	}

	return 0;
}

