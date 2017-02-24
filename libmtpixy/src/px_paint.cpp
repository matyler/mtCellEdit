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



int mtPixy::Image::paint_canvas_rectangle (
	Brush		&bru,
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h
	)
{
	if ( ! m_canvas )
	{
		return 0;
	}

	int dx1 = x;
	int dy1 = y;
	int dx2 = x + w;
	int dy2 = y + h;

	dx1 = MAX ( dx1, 0 );
	dy1 = MAX ( dy1, 0 );
	dx2 = MIN ( dx2, m_width );
	dy2 = MIN ( dy2, m_height );

	if ( dx1 >= m_width || dy1 >= m_height )
	{
		// Nothing to paint
		return 0;
	}

	int			i, j;
	int		const	ps = mtPixy::Brush::PATTERN_SIZE;
	unsigned char	const	* sm;
	unsigned char		* dm;

	if ( m_type == INDEXED )
	{
		Image * pi = bru.get_pattern_idx ();
		if ( ! pi )
		{
			return 1;
		}

		unsigned char * pat = pi->get_canvas ();
		if ( ! pat )
		{
			return 1;
		}

		for ( j = dy1; j < dy2; j++ )
		{
			dm = m_canvas + dx1 + m_width * j;
			sm = pat + (j % ps) * ps;

			for ( i = dx1; i < dx2; i++ )
			{
				*dm++ = sm[ i % ps ];
			}
		}
	}
	else if ( m_type == RGB )
	{
		unsigned char	const	* smx;

		Image * pi = bru.get_pattern_rgb ();
		if ( ! pi )
		{
			return 1;
		}

		unsigned char * pat = pi->get_canvas ();
		if ( ! pat )
		{
			return 1;
		}

		for ( j = dy1; j < dy2; j++ )
		{
			dm = m_canvas + (dx1 + m_width * j) * 3;
			sm = pat + (j % ps) * ps * 3;

			for ( i = dx1; i < dx2; i++ )
			{
				smx = sm + 3 * (i % ps);
				*dm++ = smx[0];
				*dm++ = smx[1];
				*dm++ = smx[2];
			}
		}
	}

	return 0;
}

int mtPixy::Image::paint_rectangle (
	Brush		&bru,
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h
	)
{
	if ( w < 1 || h < 1 )
	{
		return 1;
	}

	if ( ! m_canvas )
	{
		return 0;
	}

	Image * const imask = mtPixy::image_create ( ALPHA, w, h );
	if ( ! imask )
	{
		return 1;
	}

	unsigned char * const dst = imask->m_alpha;
	if ( ! dst )
	{
		delete imask;
		return 1;
	}

	memset ( dst, 255, (size_t)(w * h) );

	imask->paint_flow ( bru );

	paste_alpha_pattern ( imask, bru, x, y );

	delete imask;

	return 0;
}

int mtPixy::Image::paint_polygon (
	Brush			&bru,
	PolySelOverlay	const	&ovl,
	int			&x,
	int			&y,
	int			&w,
	int			&h
	)
{
	if ( ! m_canvas )
	{
		return 0;
	}

	Image * imask = ovl.create_mask ( x, y, w, h );
	if ( ! imask )
	{
		return 1;
	}

	imask->paint_flow ( bru );

	paste_alpha_pattern ( imask, bru, x, y );

	delete imask;

	return 0;
}

int mtPixy::Image::paint_brush (
	Brush		&bru,
	int	const	x1,
	int	const	y1,
	int	const	x2,
	int	const	y2,
	int		&dx,
	int		&dy,
	int		&dw,
	int		&dh,
	bool	const	skip
	)
{
	// Enforce sanity
	int		rx1 = MAX ( x1, 0 );
	int		ry1 = MAX ( y1, 0 );
	int		rx2 = MAX ( x2, 0 );
	int		ry2 = MAX ( y2, 0 );

	rx1 = MIN ( rx1, m_width - 1 );
	ry1 = MIN ( ry1, m_height - 1 );
	rx2 = MIN ( rx2, m_width - 1 );
	ry2 = MIN ( ry2, m_height - 1 );

	int	const	bs = mtPixy::Brush::SHAPE_SIZE;
	int	const	w = abs ( rx2 - rx1 ) + bs;
	int	const	h = abs ( ry2 - ry1 ) + bs;

	Image * const imask = mtPixy::image_create ( ALPHA, w, h );
	if ( ! imask )
	{
		return 1;
	}

	Image * const bshape = bru.get_shape_mask ();
	if ( ! bshape )
	{
		delete imask;
		return 1;
	}

	int	const	spac = bru.get_spacing ();
	int	const	xa = x1 - MIN ( x1, x2 );
	int	const	ya = y1 - MIN ( y1, y2 );
	int	const	xb = x2 - MIN ( x1, x2 );
	int	const	yb = y2 - MIN ( y1, y2 );
	int		xd = (x1 == x2) ? 0 : (x1 < x2) ? 1 : -1;
	int	const	yd = (y1 == y2) ? 0 : (y1 < y2) ? 1 : -1;
	int		spacmod = bru.get_space_mod ();
	int		x, y;
	double		p;

	if ( spac == 0 )
	{
		x = x2 - MIN ( x1, x2 );
		y = y2 - MIN ( y1, y2 );

		imask->paste_alpha_or ( bshape, x, y );
	}
	else if ( w >= h )
	{
		double		pdiv;


		if ( xd == 0 )
		{
			xd = 1;
			pdiv = 1.0;
		}
		else
		{
			pdiv = xb - xa;
		}

		for ( x = skip ? xa + xd : xa; x != (xb + xd); x += xd )
		{
			p = (double)(x - xa) / pdiv;
			y = (int)( 0.5 + (1.0 - p) * ya + p * yb );

			if ( spacmod == 0 )
			{
				imask->paste_alpha_or ( bshape, x, y );
			}

			spacmod = (spacmod + 1) % spac;
		}

		bru.set_space_mod ( spacmod );
	}
	else	// h > w
	{
		for ( y = skip ? ya + yd : ya; y != (yb + yd); y += yd )
		{
			p = (double)(y - ya) / (double)(yb - ya);
			x = (int)( 0.5 + (1.0 - p) * xa + p * xb );

			if ( spacmod == 0 )
			{
				imask->paste_alpha_or ( bshape, x, y );
			}

			spacmod = (spacmod + 1) % spac;
		}

		bru.set_space_mod ( spacmod );
	}

	imask->paint_flow ( bru );

	x = MIN ( rx1, rx2 ) - bs / 2;
	y = MIN ( ry1, ry2 ) - bs / 2;

	paste_alpha_pattern ( imask, bru, x, y );

	delete imask;

	dx = x;
	dy = y;
	dw = w;
	dh = h;

	if ( dx < 0 )
	{
		dw += dx;
		dx = 0;
	}

	if ( dy < 0 )
	{
		dh += dy;
		dy = 0;
	}

	if ( (dx + dw) > m_width )
	{
		dw = m_width - dx;
	}

	if ( (dy + dh) > m_height )
	{
		dh = m_height - dy;
	}

	return 0;
}

void mtPixy::Image::paint_flow (
	Brush		&bru
	) const
{
	int	const	fl = bru.get_flow ();


	if ( fl >= Brush::FLOW_MIN && fl < Brush::FLOW_MAX )
	{
		int	const	tot = m_width * m_height;
		int		r;
		unsigned char	* dest = m_alpha;
		unsigned char	* destlim = dest + tot;


		for ( ; dest < destlim; dest++ )
		{
			if ( dest[0] > 0 )
			{
				r = rand () % (Brush::FLOW_MAX + 1);
				if ( r > fl )
				{
					dest[0] = 0;
				}
			}
		}
	}
}

