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



mtPixy::LineOverlay::LineOverlay ()
	:
	m_x1		( 0 ),
	m_y1		( 0 ),
	m_x2		( 0 ),
	m_y2		( 0 )
{
}

void mtPixy::LineOverlay::set_start (
	int	const	x,
	int	const	y
	)
{
	m_x1 = x;
	m_y1 = y;
	m_x2 = x;
	m_y2 = y;
}

void mtPixy::LineOverlay::set_end (
	int	const	x,
	int	const	y,
	int		&dx,
	int		&dy,
	int		&dw,
	int		&dh
	)
{
	dx = MIN ( x, MIN ( m_x1, m_x2 ) );
	dy = MIN ( y, MIN ( m_y1, m_y2 ) );
	dw = MAX ( x, MAX ( m_x1, m_x2 ) ) - dx + 1;
	dh = MAX ( y, MAX ( m_y1, m_y2 ) ) - dy + 1;

	m_x2 = x;
	m_y2 = y;
}

static inline void blit_square (
	unsigned char	const * const	&pat,
	int			const	&x,
	int			const	&y,
	int			const	&w,
	int			const	&i,
	int			const	&j,
	int			const	&zs,
	int			const	&vx2,
	int			const	&vy2,
	unsigned char		* const	&rgb
	)
{
	unsigned char const * const src = pat + 3 * ( (i%8) + 8 * (j%8) );
	int	const	ax = MAX ( x, i * zs );
	int	const	ay = MAX ( y, j * zs );
	int	const	bx = MIN ( vx2, (i + 1) * zs - 1 );
	int	const	by = MIN ( vy2, (j + 1) * zs - 1 );

	for ( int ky = ay; ky <= by; ky++ )
	{
		unsigned char * dest = rgb + 3 * (ax - x + (ky - y) * w);

		for ( int kx = ax; kx <= bx; kx++ )
		{
			*dest++ = src[0];
			*dest++ = src[1];
			*dest++ = src[2];
		}
	}
}

void mtPixy::LineOverlay::render (
	Brush			&bru,
	unsigned char	* const	rgb,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	int		const	zs
	) const
{
	if ( ! rgb || zs == 0 || x < 0 || y < 0 )
	{
		return;
	}

	// Canvas coords
	int		x1 = MIN ( m_x1, m_x2 );
	int		x2 = MAX ( m_x1, m_x2 );
	int		y1 = MIN ( m_y1, m_y2 );
	int		y2 = MAX ( m_y1, m_y2 );
	int	const	cw = x2 - x1 + 1;
	int	const	ch = y2 - y1 + 1;
	int		zx1, zy1, zx2, zy2;

	// View area coords
	int	const	vx2 = x + w - 1;
	int	const	vy2 = y + h - 1;

	if ( zs < 0 )
	{
		x1 = MAX ( x1, x * -zs );
		x2 = MIN ( x2, (x + w - 1) * -zs );

		y1 = MAX ( y1, y * -zs );
		y2 = MIN ( y2, (y + h - 1) * -zs );

		zx1 = m_x1 / -zs;
		zy1 = m_y1 / -zs;
		zx2 = m_x2 / -zs;
		zy2 = m_y2 / -zs;
	}
	else
	{
		x1 = MAX ( x1, x / zs );
		x2 = MIN ( x2, (x + w - 1) / zs );

		y1 = MAX ( y1, y * -zs );
		y2 = MIN ( y2, (y + h - 1) / zs );

		zx1 = m_x1;
		zy1 = m_y1;
		zx2 = m_x2;
		zy2 = m_y2;
	}

	if ( x2 < x1 || y2 < y1 )
	{
		// No overlap so nothing to do
		return;
	}

	mtPixy::Image * const	ipat = bru.get_pattern_rgb ();
	if ( ! ipat )
	{
		return;
	}

	unsigned char	const * const	pat = ipat->get_canvas ();
	if ( ! pat )
	{
		return;
	}

	if ( zs < 2 )
	{
		if ( cw >= ch )
		{
			double	const	pd = zx1 == zx2 ? 1 : (zx2 - zx1);
			int	const	a = MAX ( x, MIN ( zx1, zx2 ) );
			int	const	b = MIN ( vx2, MAX ( zx1, zx2 ) );

			for ( int i = a; i <= b; i++ )
			{
				double	const p = ((double)(i - zx1)) / pd;
				int	const j = (int)( 0.5 + (1.0 - p) * zy1 +
							p * zy2 );

				if ( j >= y && j <= vy2 )
				{
					unsigned char const * src =
						pat + 3 * ( (i%8) + 8 * (j%8) );
					unsigned char * dest =
						rgb + 3 * (i-x + (j-y) * w);

					dest[0] = src[0];
					dest[1] = src[1];
					dest[2] = src[2];
				}
			}
		}
		else
		{
			double	const	pd = zy1 == zy2 ? 1 : (zy2 - zy1);
			int	const	a = MAX ( y, MIN ( zy1, zy2 ) );
			int	const	b = MIN ( vy2, MAX ( zy1, zy2 ) );

			for ( int j = a; j <= b; j++ )
			{
				double	const p = ((double)(j - zy1)) / pd;
				int	const i = (int)( 0.5 + (1.0 - p) * zx1 +
							p * zx2 );

				if ( i >= x && i <= vx2 )
				{
					unsigned char const * src =
						pat + 3 * ( (i%8) + 8 * (j%8) );
					unsigned char * dest =
						rgb + 3 * (i-x + (j-y) * w);

					dest[0] = src[0];
					dest[1] = src[1];
					dest[2] = src[2];
				}
			}
		}

		return;
	}

	// zs >= 2 : CAUTION! LARGE PIXELS AHEAD!

	if ( cw >= ch )
	{
		double	const	pd = zx1 == zx2 ? 1 : (zx2 - zx1);

		for ( int i = x1; i <= x2; i++ )
		{
			double	const p = ((double)(i - zx1)) / pd;
			int	const j = (int)( 0.5 + (1.0-p) * zy1 + p * zy2);

			blit_square ( pat, x, y, w, i, j, zs, vx2, vy2, rgb );
		}
	}
	else
	{
		double	const	pd = zy1 == zy2 ? 1 : (zy2 - zy1);

		for ( int j = y1; j <= y2; j++ )
		{
			double	const p = ((double)(j - zy1)) / pd;
			int	const i = (int)( 0.5 + (1.0-p) * zx1 + p * zx2);

			blit_square ( pat, x, y, w, i, j, zs, vx2, vy2, rgb );
		}
	}
}

int mtPixy::LineOverlay::get_x1 () const
{
	return m_x1;
}

int mtPixy::LineOverlay::get_y1 () const
{
	return m_y1;
}

void mtPixy::LineOverlay::get_xy (
	int		&x1,
	int		&y1,
	int		&x2,
	int		&y2
	) const
{
	x1 = m_x1;
	y1 = m_y1;
	x2 = m_x2;
	y2 = m_y2;
}

int mtPixy::RecSelOverlay::set (
	int			const	x,
	int			const	y,
	int			const	w,
	int			const	h,
	mtPixy::Image	const * const	im
	)
{
	if ( ! im )
	{
		return 1;
	}

	int	const	xmax = im->get_width () - 1;
	int	const	ymax = im->get_height () - 1;

	m_x1 = MAX ( 0, MIN ( x, xmax ) );
	m_y1 = MAX ( 0, MIN ( y, ymax ) );
	m_x2 = MAX ( 0, MIN ( x + w - 1, xmax ) );
	m_y2 = MAX ( 0, MIN ( y + h - 1, ymax ) );

	return 0;
}

void mtPixy::RecSelOverlay::set_start (
	int	const	x,
	int	const	y
	)
{
	LineOverlay::set_start ( x, y );
}

void mtPixy::RecSelOverlay::set_end (
	int	const	x,
	int	const	y,
	int		&dx,
	int		&dy,
	int		&dw,
	int		&dh
	)
{
	LineOverlay::set_end ( x, y, dx, dy, dw, dh );
}

void mtPixy::RecSelOverlay::move_selection (
	int	const	x,
	int	const	y,
	int	const	max_x,
	int	const	max_y,
	int		&dx,
	int		&dy,
	int		&dw,
	int		&dh
	)
{
	int	const	x1 = MAX ( 0, MIN ( max_x, x + m_x1 ) );
	int	const	x2 = MAX ( 0, MIN ( max_x, x + m_x2 ) );
	int	const	y1 = MAX ( 0, MIN ( max_y, y + m_y1 ) );
	int	const	y2 = MAX ( 0, MIN ( max_y, y + m_y2 ) );

	dx = MIN ( MIN ( x1, x2 ), MIN ( m_x1, m_x2 ) );
	dy = MIN ( MIN ( y1, y2 ), MIN ( m_y1, m_y2 ) );
	dw = MAX ( MAX ( x1, x2 ), MAX ( m_x1, m_x2 ) ) - dx + 1;
	dh = MAX ( MAX ( y1, y2 ), MAX ( m_y1, m_y2 ) ) - dy + 1;

	m_x1 = x1;
	m_y1 = y1;
	m_x2 = x2;
	m_y2 = y2;
}

void mtPixy::RecSelOverlay::move_selection_end (
	int	const	x,
	int	const	y,
	int	const	max_x,
	int	const	max_y,
	int		&dx,
	int		&dy,
	int		&dw,
	int		&dh
	)
{
	int	const	x2 = MAX ( 0, MIN ( max_x, x + m_x2 ) );
	int	const	y2 = MAX ( 0, MIN ( max_y, y + m_y2 ) );

	dx = MIN ( x2, MIN ( m_x1, m_x2 ) );
	dy = MIN ( y2, MIN ( m_y1, m_y2 ) );
	dw = MAX ( x2, MAX ( m_x1, m_x2 ) ) - dx + 1;
	dh = MAX ( y2, MAX ( m_y1, m_y2 ) ) - dy + 1;

	m_x2 = x2;
	m_y2 = y2;
}

int mtPixy::RecSelOverlay::set_paste (
	Image	* const	im,
	Image	* const	pa,
	int	const	px,
	int	const	py
	)
{
	if ( ! im || ! pa )
	{
		return 1;
	}

	m_x1 = px;
	m_y1 = py;

	int	const	iw = im->get_width ();
	int	const	ih = im->get_height ();
	int	const	pw = pa->get_width ();
	int	const	ph = pa->get_height ();

	m_x1 = MAX ( m_x1, 1 - pw );
	m_y1 = MAX ( m_y1, 1 - ph );
	m_x1 = MIN ( m_x1, iw - 1 );
	m_y1 = MIN ( m_y1, ih - 1 );

	m_x2 = m_x1 + pw - 1;
	m_y2 = m_y1 + ph - 1;

	return 0;
}

int mtPixy::RecSelOverlay::set_paste (
	Image	* const	im,
	Image	* const	pa
	)
{
	return set_paste ( im, pa, m_x1, m_y1 );
}

int mtPixy::RecSelOverlay::move_paste (
	int		const	x,
	int		const	y,
	Image	const * const	im,
	Image	const * const	pa,
	int			&dx,
	int			&dy,
	int			&dw,
	int			&dh
	)
{
	if ( ! im || ! pa )
	{
		return 0;
	}

	int		x1 = x, y1 = y, x2, y2;
	int	const	iw = im->get_width ();
	int	const	ih = im->get_height ();
	int	const	pw = pa->get_width ();
	int	const	ph = pa->get_height ();

	x1 = MAX ( x1, 1 - pw );
	y1 = MAX ( y1, 1 - ph );
	x1 = MIN ( x1, iw - 1 );
	y1 = MIN ( y1, ih - 1 );

	if ( x1 == m_x1 && y1 == m_y1 )
	{
		return 0;
	}

	x2 = x1 + pw - 1;
	y2 = y1 + ph - 1;

	dx = MIN ( x1, m_x1 );
	dy = MIN ( y1, m_y1 );
	dw = MAX ( x2, m_x2 ) - dx + 1;
	dh = MAX ( y2, m_y2 ) - dy + 1;

	m_x1 = x1;
	m_y1 = y1;
	m_x2 = x2;
	m_y2 = y2;

	return 1;
}

static inline void blit_hline (
	int	const	&x1,
	int	const	&x2,
	int	const	&y,
	int	const	&w,
	int	const	&h,
	unsigned char	coltab[3][2],
	unsigned char	* rgb,
	int	const	&xo
	)
{
	if ( y >=0 && y < h && x2 >= 0 && x1 < w )
	{
		int		cx1 = MAX ( 0, x1 );
		int		cx2 = MIN ( w - 1, x2 );
		int		p;
		unsigned char	* dest = rgb + 3 * (cx1 + y * w);

		for ( int cx = cx1; cx <= cx2; cx++ )
		{
			p = ((cx + xo) / 8) % 2;

			*dest++ = coltab[0][p];
			*dest++ = coltab[1][p];
			*dest++ = coltab[2][p];
		}
	}
}

static inline void blit_vline (
	int	const	&x,
	int	const	&y1,
	int	const	&y2,
	int	const	&w,
	int	const	&h,
	unsigned char	coltab[3][2],
	unsigned char	* rgb,
	int	const	&yo
	)
{
	if ( x >=0 && x < w && y2 >= 0 && y1 < h )
	{
		int		cy1 = MAX ( 0, y1 );
		int		cy2 = MIN ( h - 1, y2 );
		int		p;
		unsigned char	* dest = rgb + 3 * (x + cy1 * w);

		for ( int cy = cy1; cy <= cy2; cy++ )
		{
			p = ((cy + yo) / 8) % 2;

			dest[0] = coltab[0][p];
			dest[1] = coltab[1][p];
			dest[2] = coltab[2][p];

			dest += 3 * w;
		}
	}
}

void mtPixy::RecSelOverlay::render (
	unsigned char	* const	rgb,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	int		const	zs,
	int		const	col
	) const
{
	unsigned char	coltab[3][2];
	int		x1 = MIN ( m_x1, m_x2 );
	int		x2 = MAX ( m_x1, m_x2 );
	int		y1 = MIN ( m_y1, m_y2 );
	int		y2 = MAX ( m_y1, m_y2 );

	coltab[0][0] = 255;
	coltab[1][0] = 255;
	coltab[2][0] = 255;

	if ( col == 0 )
	{
		coltab[0][1] = 255;
		coltab[1][1] = 0;
		coltab[2][1] = 0;
	}
	else
	{
		coltab[0][1] = 0;
		coltab[1][1] = 0;
		coltab[2][1] = 255;
	}

	if ( zs < 0 )
	{
		x1 /= -zs;
		y1 /= -zs;
		x2 /= -zs;
		y2 /= -zs;
	}
	else
	{
		x1 *= zs;
		y1 *= zs;
		x2 = (x2 + 1) * zs - 1;
		y2 = (y2 + 1) * zs - 1;
	}

	x1 -= x;
	y1 -= y;
	x2 -= x;
	y2 -= y;

///	TOP & BOTTOM
	blit_hline ( x1, x2, y1, w, h, coltab, rgb, x );
	blit_hline ( x1, x2, y2, w, h, coltab, rgb, x );

///	LEFT & RIGHT
	blit_vline ( x1, y1, y2, w, h, coltab, rgb, y );
	blit_vline ( x2, y1, y2, w, h, coltab, rgb, y );
}


int mtPixy::RecSelOverlay::get_x1 () const
{
	return m_x1;
}

int mtPixy::RecSelOverlay::get_y1 () const
{
	return m_y1;
}
void mtPixy::RecSelOverlay::get_xy (
	int		&x1,
	int		&y1,
	int		&x2,
	int		&y2
	) const
{
	LineOverlay::get_xy ( x1, y1, x2, y2 );
}

void mtPixy::RecSelOverlay::get_xywh (
	int		&x,
	int		&y,
	int		&w,
	int		&h
	) const
{
	x = MIN ( m_x1, m_x2 );
	y = MIN ( m_y1, m_y2 );
	w = MAX ( m_x1, m_x2 ) - x + 1;
	h = MAX ( m_y1, m_y2 ) - y + 1;
}

void mtPixy::RecSelOverlay::set_corner (
	int	const	x,
	int	const	y,
	int		&dx,
	int		&dy,
	int		&dw,
	int		&dh
	)
{
	if ( abs ( x - m_x1 ) < abs ( x - m_x2 ) )
	{
		int t = m_x1;

		m_x1 = m_x2;
		m_x2 = t;
	}

	if ( abs ( y - m_y1 ) < abs ( y - m_y2 ) )
	{
		int t = m_y1;

		m_y1 = m_y2;
		m_y2 = t;
	}

	LineOverlay::set_end ( x, y, dx, dy, dw, dh );
}

