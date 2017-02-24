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



mtPixy::PolySelOverlay::PolySelOverlay ()
	:
	m_point_total	( 0 )
{
	memset ( m_x, 0, sizeof( m_x ) );
	memset ( m_y, 0, sizeof( m_y ) );
}

void mtPixy::PolySelOverlay::set_start (
	int	const	x,
	int	const	y
	)
{
	LineOverlay::set_start ( x, y );
}

void mtPixy::PolySelOverlay::set_end (
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

int mtPixy::PolySelOverlay::get_x1 () const
{
	return m_x1;
}

int mtPixy::PolySelOverlay::get_y1 () const
{
	return m_y1;
}

void mtPixy::PolySelOverlay::get_xywh (
	int		&x,
	int		&y,
	int		&w,
	int		&h
	) const
{
	if ( m_point_total < 1 )
	{
		x = 0;
		y = 0;
		w = 1;
		h = 1;
	}

	int		minx = m_x[0];
	int		miny = m_y[0];
	int		maxx = minx;
	int		maxy = miny;

	for ( int i = 1; i < m_point_total; i++ )
	{
		minx = MIN ( minx, m_x[i] );
		maxx = MAX ( maxx, m_x[i] );
		miny = MIN ( miny, m_y[i] );
		maxy = MAX ( maxy, m_y[i] );
	}

	x = minx;
	y = miny;
	w = maxx - minx + 1;
	h = maxy - miny + 1;
}

void mtPixy::PolySelOverlay::render (
	mtPixy::Brush		&bru,
	unsigned char	* const	rgb,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	int		const	zs
	) const
{
	LineOverlay::render ( bru, rgb, x, y, w, h, zs );
}

void mtPixy::PolySelOverlay::clear ()
{
	m_point_total = 0;
}

int mtPixy::PolySelOverlay::add ()
{
	if ( m_point_total >= POINT_MAX )
	{
		return 1;
	}

	m_x [ m_point_total ] = m_x1;
	m_y [ m_point_total ] = m_y1;

	m_point_total++;

	return 0;
}

mtPixy::Image * mtPixy::PolySelOverlay::copy (
	Image	* const	src,
	int		&x,
	int		&y,
	int		&w,
	int		&h
	) const
{
	if ( ! src )
	{
		return NULL;
	}

	Image * mask = create_mask ( x, y, w, h );
	if ( ! mask )
	{
		return NULL;
	}

	Image * im = src->resize ( x, y, w, h );
	if ( ! im )
	{
		delete mask;
		return NULL;
	}

	if ( im->move_alpha_destroy ( mask ) )
	{
		delete im;
		delete mask;
		return NULL;
	}

	return im;
}



class PolyState
{
public:
	PolyState (
		mtPixy::PolySelOverlay const &ovl,
		unsigned char * mem,
		int const &x,
		int const &y,
		int const &w,
		int const &h
		);

private:
	void line ( int x1, int y1, int x2, int y2 ) const;

///	------------------------------------------------------------------------

	unsigned char		* const	m_mem;
	int			const	&m_w;
	int			const	&m_h;
};



static int cmp_double (
	void	const * const	a,
	void	const * const	b
	)
{
	double	const	* fa = (double const *)a;
	double	const	* fb = (double const *)b;


	if ( fa[0] < fb[0] )
	{
		return -1;
	}

	if ( fa[0] > fb[0] )
	{
		return 1;
	}

	return 0;
}

PolyState::PolyState (
	mtPixy::PolySelOverlay	const	&ovl,
	unsigned char		* const	mem,
	int			const	&x,
	int			const	&y,
	int			const	&w,
	int			const	&h
	)
	:
	m_mem		( mem ),
	m_w		( w ),
	m_h		( h )
{
	int		poly_lines[ mtPixy::PolySelOverlay::POINT_MAX ][2][2];


	// Populate poly_lines - smallest Y is first point
	for ( int i = 0; i < ovl.m_point_total; i++ )
	{
		int		i2 = i + 1;
		int		j;

		if ( i2 >= ovl.m_point_total )
		{
			i2 = 0;		// Remember last point back to 1st point
		}

		if ( ovl.m_y[i] < ovl.m_y[i2] )
		{
			j = 0;
		}
		else
		{
			j = 1;
		}

		poly_lines[i][j][0] = ovl.m_x[i] - x;
		poly_lines[i][j][1] = ovl.m_y[i] - y;
		poly_lines[i][1-j][0] = ovl.m_x[i2] - x;
		poly_lines[i][1-j][1] = ovl.m_y[i2] - y;

		line (	poly_lines[i][j][0], poly_lines[i][j][1],
			poly_lines[i][1-j][0], poly_lines[i][1-j][1] );
	}

	double		poly_cuts[ mtPixy::PolySelOverlay::POINT_MAX ];

	for ( int j = 0; j < h; j++ )	// Scanline
	{
		int		cuts = 0;

		// Count up line intersections - X value cuts
		for ( int i = 0; i < ovl.m_point_total; i++ )
		{
			if (	j >= poly_lines[i][0][1] &&
				j <= poly_lines[i][1][1]
				)
			{
				if ( poly_lines[i][0][1] == poly_lines[i][1][1]
					|| j == poly_lines[i][0][1]
					)
				{
					// Line is horizontal so skip
					// or line is start point.
					continue;
				}

				// Calculate cut X value for y = j

				double r = (double)( j - poly_lines[i][0][1] ) /
					(double)( poly_lines[i][1][1]
					- poly_lines[i][0][1] );

				poly_cuts[ cuts ++ ] = 0.5 + (double)(
					(double)poly_lines[i][0][0] +
					r * (double)( poly_lines[i][1][0] -
						poly_lines[i][0][0] )
					);
			}
		}

		if ( cuts > 1 )
		{
			qsort ( poly_cuts, (size_t)cuts, sizeof( poly_cuts[0] ),
				cmp_double );
		}

		// Paint from first X to 2nd, gap from 2-3, paint 3-4,
		// gap 4-5 ...
		for ( int i = 0; i < (cuts - 1); i += 2 )
		{
			if ( poly_cuts[i] < w && poly_cuts[i + 1] >= 0 )
			{
				int const xa = (int)( poly_cuts[i] );
				int const xb = (int)( poly_cuts[i+1] );

				for ( int xx = xa; xx <= xb; xx++ )
				{
					mem[ xx + j * w ] = 255;
				}
			}
		}
	}
}

void PolyState::line (
	int	const	x1,
	int	const	y1,
	int	const	x2,
	int	const	y2
	) const
{
	int	const	xx	= x2 - x1;
	int	const	yy	= y2 - y1;
	int	const	todo	= 1 + MAX ( abs ( xx ), abs ( yy ) );
	double	const	dxx	= (double)xx;
	double	const	dyy	= (double)yy;
	double	const	dtodo	= (double)todo;
	double	const	dx1	= (double)x1;
	double	const	dy1	= (double)y1;

	for ( int i = 0; i <= todo; i++ )
	{
		double	const	rat = ( (double) i ) / dtodo;
		int	const	px = (int)( 0.5 + dx1 + dxx * rat );
		int	const	py = (int)( 0.5 + dy1 + dyy * rat );

		if ( px >= 0 && py >= 0 && px < m_w && py < m_h )
		{
			m_mem[ px + m_w * py ] = 255;
		}
	}
}

mtPixy::Image * mtPixy::PolySelOverlay::create_mask (
	int		&x,
	int		&y,
	int		&w,
	int		&h
	) const
{
	if ( m_point_total < 3 )
	{
		return NULL;
	}

	get_xywh ( x, y, w, h );

	Image * const imask = mtPixy::image_create( mtPixy::Image::ALPHA, w, h);
	if ( ! imask )
	{
		return NULL;
	}

	unsigned char * const dst = imask->get_alpha ();
	if ( ! dst )
	{
		delete imask;
		return NULL;
	}

	PolyState	poly ( *this, dst, x, y, w, h );

	return imask;
}

