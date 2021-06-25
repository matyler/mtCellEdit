/*
	Copyright (C) 2021 Mark Tyler

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

#include "mandy.h"



Mandelbrot::Mandelbrot ()
	:
	m_pal		( m_palette.get_palette () ),
	m_work		( WORK_QUEUE_SIZE )
{
}

Mandelbrot::~Mandelbrot ()
{
}

int Mandelbrot::get_status () const
{
	switch ( m_thread[0].get_status () )
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

	m_pixmap.reset ( pixmap );

	clear_pixmap ();

	return 0;
}

void Mandelbrot::calculate_range_h ()
{
	int const w = pixy_pixmap_get_width ( m_pixmap.get() );
	int const h = pixy_pixmap_get_height ( m_pixmap.get() );
	double const aspect = (w == 0) ? 0.0 : (double)w / (double)h;

	m_range_h = m_range_w / aspect;
}

void Mandelbrot::calculate_xyo ()
{
	m_xo = m_cx - m_range_w / 2.0;
	m_yo = m_cy - m_range_h / 2.0;
}

void Mandelbrot::zoom_cxyrange (
	double	const	cx,
	double	const	cy,
	double	const	range_w
	)
{
	m_cx = mtkit_double_bound ( cx, AXIS_CXY_MIN, AXIS_CXY_MAX );
	m_cy = mtkit_double_bound ( cy, AXIS_CXY_MIN, AXIS_CXY_MAX );
	m_range_w = mtkit_double_bound(range_w, AXIS_RANGE_MIN, AXIS_RANGE_MAX);

	calculate_range_h ();
}

void Mandelbrot::set_depth_max ( int const d )
{
	m_depth_max = (size_t)mtkit_int_bound( d, DEPTH_MAX_MIN, DEPTH_MAX_MAX);
}

void Mandelbrot::set_verbose ( int const v )
{
	m_verbose = v;
}

int Mandelbrot::build_mandelbrot_set ( int const threads )
{
	if ( threads < THREAD_TOTAL_MIN || threads > THREAD_TOTAL_MAX )
	{
		std::cerr << "build_mandelbrot_set: bad thread number="
			<< threads << "\n";
		return 1;
	}

	if ( get_status () != STATUS_IDLE )
	{
		std::cerr << "build_mandelbrot_set: busy worker thread.\n";
		return 1;
	}

	mtPixmap * const pixmap = m_pixmap.get ();
	if ( ! pixmap )
	{
		std::cerr << "build_mandelbrot_set: no pixmap.\n";
		return 1;
	}

	clear_pixmap ();

	m_thread_tot	= threads;
	m_dest		= pixy_pixmap_get_canvas ( pixmap );
	calculate_xyo ();
	m_w		= pixy_pixmap_get_width ( pixmap );
	m_h		= pixy_pixmap_get_height ( pixmap );
	m_stride	= m_w * 3;
	m_w_dec		= m_w - 1;
	m_h_dec		= m_h - 1;
	m_pal_size	= m_pal.size();

	if ( threads > 1 )
	{
		if ( m_thread[0].init ( [this]() { build_multi_leader (); } ) )
		{
			std::cerr <<
				"build_mandelbrot_set: thread init failure.\n";
			return 1;
		}

		if ( m_thread[0].start () )
		{
			std::cerr <<
				"build_mandelbrot_set: thread start failure.\n";
			return 1;
		}
	}
	else
	{
		if ( m_thread[0].init ( [this]() { build_single_worker (); } ) )
		{
			std::cerr <<
				"build_mandelbrot_set: thread init failure.\n";
			return 1;
		}

		if ( m_thread[0].start () )
		{
			std::cerr <<
				"build_mandelbrot_set: thread start failure.\n";
			return 1;
		}
	}

	return 0;
}

void Mandelbrot::start_timer ()
{
	if ( ! m_verbose )
	{
		return;
	}

	m_time_start = (double)SDL_GetTicks();
	printf ( "Mandelbrot::build_thread -cx %.15g -cy %.15g -range %.15g"
		" -depth %i\n",
		get_cx(), get_cy(), m_range_w, (int)m_depth_max );
}

void Mandelbrot::finish_timer () const
{
	if ( ! m_verbose )
	{
		return;
	}

	double const time_finish = (double)SDL_GetTicks();
	printf ( "Mandelbrot::build_thread time=%.15g secs\n",
		(time_finish - m_time_start) / 1000.0 );
}

void Mandelbrot::build_single_worker ()
{
	start_timer ();

	for ( int y = 0; y < m_h; y++ )
	{
		switch ( m_thread[0].get_status() )
		{
		case mtGin::Thread::THREAD_TERMINATED:
			return;
		}

		render_line ( y );
	}

	finish_timer ();
}

void Mandelbrot::build_multi_leader ()
{
	start_timer ();

	m_work.clear ();

	for ( int t = 1; t <= m_thread_tot; t++ )
	{
		if ( m_thread[ t ].init ( [this]()
			{
				build_multi_workers ();
			} )
			)
		{
			std::cerr <<
				"build_mandelbrot_set: thread init failure.\n";
			return;
		}

		if ( m_thread[ t ].start () )
		{
			std::cerr <<
				"build_mandelbrot_set: thread start failure.\n";
			return;
		}
	}

	for ( int y = 0; y < m_h; )
	{
		switch ( m_thread[0].get_status() )
		{
		case mtGin::Thread::THREAD_TERMINATED:
			goto finish;
		}

		int const status = m_work.push ( y );

		if ( status == mtGin::THREADWORK_PUSH_ADDED )
		{
			y++;
			continue;
		}
		else if ( status == mtGin::THREADWORK_PUSH_NOT_ADDED )
		{
			SDL_Delay ( 1 );

			continue;
		}

		std::cerr << "build_multi_leader: unexpected error!\n";
	}

finish:
	m_work.finished ();

	for ( int i = 1; i <= m_thread_tot; i++ )
	{
		m_thread[i].join();
	}

	finish_timer ();
}

void Mandelbrot::build_multi_workers ()
{
	int y = 0;

	while ( 1 )
	{
		switch ( m_work.pop ( y ) )
		{
		case mtGin::THREADWORK_POP_EMPTY_WORK_TO_COME:
			SDL_Delay ( 1 );
			continue;

		case mtGin::THREADWORK_POP_HAS_WORK:
			render_line ( y );
			continue;

		case mtGin::THREADWORK_POP_EMPTY_WORK_FINISHED:
		case mtGin::THREADWORK_POP_ERROR:
		default:
			return;
		}
	}
}

size_t Mandelbrot::calculate (
	double	const	x,
	double	const	y
	) const
{
	double x2 = 0.0;
	double y2 = 0.0;
	double w = 0.0;
	size_t i = 0;

	while ( (x2 + y2) <= 4.0 && i < m_depth_max )
	{
		double const x1 = x2 - y2 + x;
		double const y1 = w - x2 - y2 + y;

		x2 = x1 * x1;
		y2 = y1 * y1;

		w = (x1 + y1) * (x1 + y1);

		i++;
	}

	return i;
}

void Mandelbrot::render_line ( int const y ) const
{
	unsigned char	* d = m_dest + m_stride * y;
	double	const	mby = m_yo + ((double)y / m_h_dec) * m_range_h;

	for ( int x = 0; x < m_w; x++ )
	{
		unsigned char	r, g, b;
		double	const	mbx = m_xo + ((double)x / m_w_dec) * m_range_w;
		size_t	const	i = this->calculate ( mbx, mby );

		if ( i < m_depth_max )
		{
			size_t const col = i % m_pal_size;

			r = m_pal[ col ].red;
			g = m_pal[ col ].green;
			b = m_pal[ col ].blue;
		}
		else
		{
			r = g = b = 0;
		}

		*d++ = r;
		*d++ = g;
		*d++ = b;
	}
}

void Mandelbrot::build_terminate ()
{
	m_thread[0].terminate ();
}

int Mandelbrot::zoom_in (
	int	const	x,
	int	const	y
	)
{
	return zoom ( x, y, 0.5 );
}

int Mandelbrot::zoom_out (
	int	const	x,
	int	const	y
	)
{
	return zoom ( x, y, 2.0 );
}

void Mandelbrot::zoom_left ()
{
	zoom_cxyrange ( m_cx - m_range_w/2, m_cy, m_range_w );
}

void Mandelbrot::zoom_right ()
{
	zoom_cxyrange ( m_cx + m_range_w/2, m_cy, m_range_w );
}

void Mandelbrot::zoom_up ()
{
	zoom_cxyrange ( m_cx, m_cy - m_range_h/2, m_range_w );
}

void Mandelbrot::zoom_down ()
{
	zoom_cxyrange ( m_cx, m_cy + m_range_h/2, m_range_w );
}

int Mandelbrot::zoom_reset ()
{
	if ( get_status () != STATUS_IDLE )
	{
		std::cerr << "zoom_reset: busy worker thread.\n";
		return 1;
	}

	m_cx = AXIS_CXY_DEFAULT;
	m_cy = AXIS_CXY_DEFAULT;
	m_range_w = AXIS_RANGE_DEFAULT;

	calculate_range_h ();

	return 0;
}

int Mandelbrot::zoom (
	int	const	x,
	int	const	y,
	double	const	scale
	)
{
	if ( get_status () != STATUS_IDLE )
	{
		std::cerr << "zoom: busy worker thread.\n";
		return 1;
	}

	double const range_w = m_range_w * scale;

	if ( range_w < AXIS_RANGE_MIN || range_w > AXIS_RANGE_MAX )
	{
		std::cerr << "zoom: zoom limit reached. range_w=" << range_w
			<< "\n";
		return 1;
	}

	mtPixmap const * const	pixmap = m_pixmap.get ();
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
	calculate_xyo ();
	m_cx = m_xo + m_range_w * ((double)x / ((double)(w - 1)));
	m_cy = m_yo + m_range_h * ((double)y / ((double)(h - 1)));

	m_range_w = range_w;
	calculate_range_h ();

	return 0;
}

void Mandelbrot::clear_pixmap () const
{
	mtPixmap	* const	pixmap = m_pixmap.get();
	unsigned char	* const	dest = pixy_pixmap_get_canvas ( pixmap );
	int		const	w = pixy_pixmap_get_width ( pixmap );
	int		const	h = pixy_pixmap_get_height ( pixmap );

	if ( dest )
	{
		memset ( dest, 64, (size_t)(w*h*3) );
	}
}
