/*
	Copyright (C) 2021-2024 Mark Tyler

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

#include "mandy_brot.h"



// Switch to arbitrary precision in this range:
static constexpr char const * const AXIS_RANGE_MIN_BIGNUM_TXT = "1.0e-120";
static constexpr char const * const AXIS_RANGE_MAX_BIGNUM_TXT = "1.0e-12";



Mandelbrot::Mandelbrot ()
	:
	k_axis_range_min_bignum	( AXIS_RANGE_MIN_BIGNUM_TXT ),
	k_axis_range_max_bignum	( AXIS_RANGE_MAX_BIGNUM_TXT ),
	k_axis_range_default	( AXIS_RANGE_DEFAULT ),
	k_axis_range_max	( AXIS_RANGE_MAX ),
	k_cxy_min		( AXIS_CXY_MIN ),
	k_cxy_max		( AXIS_CXY_MAX ),
	k_cxy_default		( AXIS_CXY_DEFAULT ),
	m_double		( m_settings ),
	m_float			( m_settings ),
	m_float_fast		( m_settings ),
	m_rational		( m_settings )
{
}

Mandelbrot::~Mandelbrot ()
{
}

int Mandelbrot::get_status () const
{
	switch ( m_settings.thread[0].get_status () )
	{
	case mtGin::Thread::THREAD_NOT_STARTED:
	case mtGin::Thread::THREAD_FINISHED:
		return STATUS_IDLE;
	}

	return STATUS_BUSY;
}

int Mandelbrot::set_pixmap_size (
	int	const	width,
	int	const	height
	)
{
	if (	width < OUTPUT_WH_MIN || width > OUTPUT_WH_MAX ||
		height < OUTPUT_WH_MIN || height > OUTPUT_WH_MAX
		)
	{
		std::cerr << "set_pixmap_size: bad size w=" << width
			<< " height=" << height << "\n";
		return 1;
	}

	if ( get_status () != STATUS_IDLE )
	{
		std::cerr << "set_pixmap_size: busy worker thread.\n";
		return 1;
	}

	mtPixmap * const pixmap = pixy_pixmap_new_rgb ( width, height );
	if ( ! pixmap )
	{
		std::cerr << "set_pixmap_size: unable to create pixmap.\n";
		return 1;
	}

	m_settings.pixmap.reset ( pixmap );

	clear_pixmap ();

	return 0;
}

void Mandelbrot::calculate_range_h ()
{
	int const w = pixy_pixmap_get_width ( get_pixmap() );
	int const h = pixy_pixmap_get_height ( get_pixmap() );

/*
	double const aspect = (w == 0) ? 0.0 : (double)w / (double)h;
	m_range_h = m_range_w / aspect;
*/

	if ( w == 0 )
	{
		m_settings.range_h.set_number ( m_settings.range_w );
		return;
	}

	m_settings.range_h = m_settings.range_w;
	m_settings.range_h *= h;
	m_settings.range_h /= w;
}

void Mandelbrot::zoom_cxyrange (
	char	const * const	cx,
	char	const * const	cy,
	char	const * const	range_w
	)
{
	if ( cx )
	{
		m_settings.cx.set_number ( cx );
		m_settings.cx.set_bound ( k_cxy_min, k_cxy_max );
	}
	else
	{
		m_settings.cx.set_number ( k_cxy_default );
	}

	if ( cy )
	{
		m_settings.cy.set_number ( cy );
		m_settings.cy.set_bound ( k_cxy_min, k_cxy_max );
	}
	else
	{
		m_settings.cy.set_number ( k_cxy_default );
	}

	if ( range_w )
	{
		m_settings.range_w.set_number ( range_w );
		m_settings.range_w.set_bound ( k_axis_range_min_bignum,
			k_axis_range_max );
	}
	else
	{
		m_settings.range_w.set_number ( k_axis_range_default );
	}

	calculate_range_h ();
}

void Mandelbrot::set_depth_max ( int const d )
{
	m_settings.depth_max = (size_t)mtkit_int_bound( d, DEPTH_MAX_MIN,
		DEPTH_MAX_MAX);
}

void Mandelbrot::set_verbose ( int const v )
{
	m_settings.verbose = v;
}

void Mandelbrot::set_threads ( int const v )
{
	m_settings.threads = v;
}

void Mandelbrot::set_deep_zoom_type ( int const v )
{
	m_settings.deep_zoom_type = v;
}

void Mandelbrot::set_deep_zoom_on ( int const v )
{
	m_settings.deep_zoom_on = v;
}

int Mandelbrot::build_mandelbrot_set ()
{
	if ( m_settings.threads < THREAD_TOTAL_MIN
		|| m_settings.threads > THREAD_TOTAL_MAX )
	{
		std::cerr << "build_mandelbrot_set: bad thread number="
			<< m_settings.threads << "\n";
		return 1;
	}

	if ( get_status () != STATUS_IDLE )
	{
		std::cerr << "build_mandelbrot_set: busy worker thread.\n";
		return 1;
	}

	if ( ! get_pixmap() )
	{
		std::cerr << "build_mandelbrot_set: no pixmap.\n";
		return 1;
	}

	clear_pixmap ();

	int res;

	if ( m_settings.verbose )
	{
		printf ( "Mandelbrot::build_mandelbrot_set " );
		info_to_stdout ();
	}

	if ( m_settings.deep_zoom_on
		&& m_settings.range_w > k_axis_range_max_bignum )
	{
		if ( m_settings.verbose )
		{
			std::cout << "m_double build\n";
		}

		res = m_double.build ();
	}
	else
	{
		switch ( m_settings.deep_zoom_type )
		{
		case DEEP_ZOOM_RATIONAL:
			res = m_rational.build ();

			if ( m_settings.verbose )
			{
				std::cout << "m_rational build\n";
			}
			break;

		case DEEP_ZOOM_FLOAT:
			res = m_float.build ();

			if ( m_settings.verbose )
			{
				std::cout << "m_float build\n";
			}
			break;

		case DEEP_ZOOM_FLOAT_FAST:
			res = m_float_fast.build ();

			if ( m_settings.verbose )
			{
				std::cout << "m_float_fast build\n";
			}
			break;

		case DEEP_ZOOM_DOUBLE:
			res = m_double.build ();

			if ( m_settings.verbose )
			{
				std::cout << "m_double build\n";
			}
			break;

		default:
			res = 0;

			if ( m_settings.verbose )
			{
				std::cout << "No deep render build\n";
			}
			break;
		}
	}

	return res;
}

void Mandelbrot::info_to_stdout () const
{
	std::cout << "-cx \"" << m_settings.cx.to_string()
		<< "\" -cy \"" << m_settings.cy.to_string()
		<< "\" -range \"" << m_settings.range_w.to_string()
		<< "\" -depth " << m_settings.depth_max
		<< " -ow " << pixy_pixmap_get_width ( get_pixmap() )
		<< " -oh " << pixy_pixmap_get_height ( get_pixmap() )
		<< "\n";
}

void Mandelbrot::build_terminate ()
{
	m_settings.thread[0].terminate ();
}

int Mandelbrot::zoom_in (
	int	const	x,
	int	const	y
	)
{
	return zoom ( x, y, 1, 2 );
}

int Mandelbrot::zoom_out (
	int	const	x,
	int	const	y
	)
{
	return zoom ( x, y, 2, 1 );
}

void Mandelbrot::zoom_left ()
{
	m_settings.cx -= m_settings.range_w/2;
}

void Mandelbrot::zoom_right ()
{
	m_settings.cx += m_settings.range_w/2;
}

void Mandelbrot::zoom_up ()
{
	m_settings.cy -= m_settings.range_h/2;
}

void Mandelbrot::zoom_down ()
{
	m_settings.cy += m_settings.range_h/2;
}

int Mandelbrot::zoom_reset ()
{
	if ( get_status () != STATUS_IDLE )
	{
		std::cerr << "zoom_reset: busy worker thread.\n";
		return 1;
	}

	m_settings.cx.set_number ( k_cxy_default );
	m_settings.cy.set_number ( k_cxy_default );
	m_settings.range_w.set_number ( k_axis_range_default );

	calculate_range_h ();

	return 0;
}

int Mandelbrot::zoom (
	int	const	x,
	int	const	y,
	int	const	multiply,
	int	const	divide
	)
{
	if ( get_status () != STATUS_IDLE )
	{
		std::cerr << "zoom: busy worker thread.\n";
		return 1;
	}

	mtDW::Rational range_w ( m_settings.range_w );

	if ( multiply != 1 )
	{
		range_w *= multiply;
	}

	if ( divide != 1 )
	{
		range_w /= divide;
	}

	if ( range_w < k_axis_range_min_bignum || range_w > k_axis_range_max )
	{
		std::cerr << "zoom: zoom limit reached. range_w="
			<< range_w.to_string()
			<< "\n";
		return 1;
	}

	mtPixmap const * const	pixmap = get_pixmap();
	int		const	w = pixy_pixmap_get_width ( pixmap );
	int		const	h = pixy_pixmap_get_height ( pixmap );

	if ( ! pixmap )
	{
		std::cerr << "zoom_in: no pixmap.\n";
		return 1;
	}

	if ( x < 0 || y < 0 || x >= w || y >= h )
	{
		std::cerr << "zoom_in: bad argument.\n";
		return 1;
	}

	// Current decimal values of this pixel point
/*
	calculate_xyo ();

	m_cx = m_xo + m_range_w * ((double)x / ((double)(w - 1)));
	m_cy = m_yo + m_range_h * ((double)y / ((double)(h - 1)));
=>
	m_cx = m_cx - m_range_w / 2.0 + m_range_w * ((double)x / ((double)(w - 1)));
	m_cy = m_cy - m_range_h / 2.0 + m_range_h * ((double)y / ((double)(h - 1)));
=>
	m_cx = m_cx + m_range_w * (x / (w - 1) - 1.0/2.0);
	m_cy = m_cy + m_range_h * (y / (h - 1) - 1.0/2.0);
=>
	m_cx = m_cx + m_range_w * ( 2*x / 2*(w - 1) - (w-1) / 2*(w - 1) );
	m_cy = m_cy + m_range_h * ( 2*y / 2*(h - 1) - (h-1) / 2*(h - 1) );
=>
	m_cx = m_cx + m_range_w * ( (2*x - w + 1) / 2*(w - 1) );
	m_cy = m_cy + m_range_h * ( (2*y - h + 1) / 2*(h - 1) );
*/

	mtDW::Rational dx ( 2*x - w + 1 );
	dx *= m_settings.range_w;
	dx /= 2*(w - 1);
	m_settings.cx += dx;

	mtDW::Rational dy ( 2*y - h + 1 );
	dy *= m_settings.range_h;
	dy /= 2*(h - 1);
	m_settings.cy += dy;

	m_settings.range_w.set_number ( range_w );

	calculate_range_h ();

	return 0;
}

void Mandelbrot::clear_pixmap () const
{
	mtPixmap	* const	pixmap = m_settings.pixmap.get();
	unsigned char	* const	dest = pixy_pixmap_get_canvas ( pixmap );
	int		const	w = pixy_pixmap_get_width ( pixmap );
	int		const	h = pixy_pixmap_get_height ( pixmap );

	if ( dest )
	{
		memset ( dest, 64, (size_t)(w*h*3) );
	}
}
