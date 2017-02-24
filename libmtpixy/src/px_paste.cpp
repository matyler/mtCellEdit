/*
	Copyright (C) 2016 Mark Tyler

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



#define PASTE_SETUP						\
	if ( x <= -src->m_width || y <= -src->m_height )	\
	{							\
		return 0;					\
	}							\
	int dx1 = x;						\
	int dy1 = y;						\
	int dx2 = x + src->m_width;				\
	int dy2 = y + src->m_height;				\
	int sx1 = 0;						\
	int sy1 = 0;						\
	/* Clip copy rectangle to destination */		\
	if ( dx1 < 0 )						\
	{							\
		sx1 = -dx1;					\
		dx1 = 0;					\
	}							\
	if ( dy1 < 0 )						\
	{							\
		sy1 = -dy1;					\
		dy1 = 0;					\
	}							\
	dx2 = MIN ( dx2, m_width );				\
	dy2 = MIN ( dy2, m_height );				\
	if (	sx1 >= src->m_width	||			\
		sy1 >= src->m_height	||			\
		dx2 < dx1		||			\
		dy2 < dy1					\
		)						\
	{							\
		/* No rectangle overlap so nothing to paste */	\
		return 0;					\
	}							\


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

int mtPixy::Image::paste (
	Image	* const	src,
	int	const	x,
	int	const	y
	)
{
	if ( ! src || m_type != src->m_type )
	{
		return 1;
	}

	PASTE_SETUP

	switch ( m_type )
	{
	case INDEXED:
		blit_pixels ( m_canvas, m_width, dx1, dy1, dx2, dy2,
			src->m_canvas, src->m_width, sx1, sy1, m_canvas_bpp );
		break;

	case RGB:
		blit_pixels ( m_canvas, m_width, dx1, dy1, dx2, dy2,
			src->m_canvas, src->m_width, sx1, sy1, m_canvas_bpp );
		break;

	default:
		// Ignore other image types
		break;
	}

	blit_pixels ( m_alpha, m_width, dx1, dy1, dx2, dy2, src->m_alpha,
		src->m_width, sx1, sy1, 1 );

	return 0;
}

int mtPixy::Image::paste_alpha_blend (
	Image	* const	src,
	int	const	x,
	int	const	y
	)
{
	if ( ! src || m_type != src->m_type )
	{
		return 1;
	}

	switch ( m_type )
	{
	case INDEXED:
		src->blit_idx_alpha_blend ( m_canvas, x, y, m_width, m_height );
		break;

	case RGB:
		src->blit_rgb_alpha_blend ( m_palette.get_color (), m_canvas, x,
			y, m_width, m_height );
		break;

	default:
		// Ignore other image types
		break;
	}

	return 0;
}

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
	mtPixy::Image	* const	pat
	)
{
	if ( ! dst || ! pat )
	{
		return;
	}


	int			x, y;
	int		const	ps = mtPixy::Brush::PATTERN_SIZE;
	unsigned char const * const pat_canvas = pat->get_canvas ();
	unsigned char	const	* am;
	unsigned char	const	* sm;
	unsigned char	const	* smx;
	unsigned char		* dm, a, b;


	for ( y = dy1; y < dy2; y++ )
	{
		am = src_alpha	+ (sx1 + sw * (sy1 + y - dy1));
		sm = pat_canvas	+ (y % ps) * ps * 3;
		dm = dst + (dx1 + dw * y) * 3;

		for ( x = dx1; x < dx2; x++ )
		{
			smx = sm + 3 * (x % ps);
			a = *am++;
			b = (unsigned char)(255 - a);

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
	mtPixy::Image	* const	pat
	)
{
	if ( ! dst || ! pat )
	{
		return;
	}


	int			x, y;
	int		const	ps = mtPixy::Brush::PATTERN_SIZE;
	unsigned char const * const pat_canvas = pat->get_canvas ();
	unsigned char	const	* am;
	unsigned char	const	* sm;
	unsigned char		* dm;


	for ( y = dy1; y < dy2; y++ )
	{
		am = src_alpha	+ (sx1 + sw * (sy1 + y - dy1));
		sm = pat_canvas	+ (y % ps) * ps;
		dm = dst + (dx1 + dw * y);

		for ( x = dx1; x < dx2; x++ )
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

int mtPixy::Image::paste_alpha_pattern (
	Image	* const	src,
	Brush		&bru,
	int	const	x,
	int	const	y
	)
{
	if ( ! src || ! src->m_alpha )
	{
		return 1;
	}

	PASTE_SETUP

	switch ( m_type )
	{
	case INDEXED:
		blit_pattern_idx ( m_canvas, m_width, dx1, dy1,
			dx2, dy2, src->m_alpha, src->m_width, sx1, sy1,
			bru.get_pattern_idx () );
		break;

	case RGB:
		blit_pattern_rgb ( m_canvas, m_width, dx1, dy1,
			dx2, dy2, src->m_alpha, src->m_width, sx1, sy1,
			bru.get_pattern_rgb () );
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


	int			x, y;
	unsigned char const	* am;
	unsigned char		* dm;


	for ( y = dy1; y < dy2; y++ )
	{
		am = src_alpha	+ (sx1 + sw * (sy1 + y - dy1));
		dm = dst + (dx1 + dw * y);

		for ( x = dx1; x < dx2; x++ )
		{
			dm[0] = (dm[0] | *am++);
			dm++;
		}
	}
}

int mtPixy::Image::paste_alpha_or (
	Image	* const	src,
	int	const	x,
	int	const	y
	)
{
	if ( ! src )
	{
		return 1;
	}

	PASTE_SETUP

	blit_pixels_alpha_or ( m_alpha, m_width, dx1, dy1, dx2, dy2,
		src->m_alpha, src->m_width, sx1, sy1 );

	return 0;
}

