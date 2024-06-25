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

#ifndef MTMANDY_BROT_H_
#define MTMANDY_BROT_H_



#include <mtdatawell_math.h>
#include <mtgin_sdl.h>

#include "mandy_palette.h"



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API



	enum
	{
		AXIS_RANGE_MAX		= 20,
		AXIS_RANGE_DEFAULT	= 4,

		AXIS_CXY_MIN		= -2,
		AXIS_CXY_MAX		= 2,
		AXIS_CXY_DEFAULT	= 0,

		DEPTH_MAX_MIN		= 8,
		DEPTH_MAX_MAX		= 1 << 24,
		DEPTH_MAX_DEFAULT	= 1024,

		OUTPUT_WH_MIN		= 16,
		OUTPUT_WH_MAX		= 15360,
		OUTPUT_WIDTH_DEFAULT	= 500,
		OUTPUT_HEIGHT_DEFAULT	= 500,

		THREAD_TOTAL_MIN	= 1,
		THREAD_TOTAL_MAX	= 16,
		THREAD_TOTAL_DEFAULT	= 0,		// Detect from CPU

		STATUS_IDLE		= 0,
		STATUS_BUSY		= 1,

		DEEP_ZOOM_NONE		= 0,
		DEEP_ZOOM_FLOAT		= 1,
		DEEP_ZOOM_FLOAT_FAST	= 2,
		DEEP_ZOOM_DOUBLE	= 3,
		DEEP_ZOOM_RATIONAL	= 4,

		WORK_QUEUE_SIZE		= THREAD_TOTAL_MAX * 2
	};




#ifdef __cplusplus
}

// C++ API

class Mandelbrot;
class MandelbrotDouble;
class MandelbrotFloat;
class MandelbrotFloatFast;
class MandelbrotRational;
class MandelbrotSettings;

template < typename Tnum > class MandelbrotBase;



class MandelbrotSettings
{
public:
	MandelbrotSettings ()
		:
		work		( WORK_QUEUE_SIZE )
	{
	}

/// ----------------------------------------------------------------------------

	MandelPalette		palette;
	mtPixy::Pixmap		pixmap;

	mtDW::Rational		cx;
	mtDW::Rational		cy;
	mtDW::Rational		range_w;
	mtDW::Rational		range_h;

	size_t			depth_max	= DEPTH_MAX_DEFAULT;
	int			verbose		= 1;
	int			threads		= THREAD_TOTAL_DEFAULT;
	int			deep_zoom_type	= DEEP_ZOOM_FLOAT_FAST;
	int			deep_zoom_on	= 1;

	mtGin::ThreadWork<int>	work;
	mtGin::Thread		thread[ THREAD_TOTAL_MAX + 1 ];

/// ----------------------------------------------------------------------------

	MTKIT_RULE_OF_FIVE( MandelbrotSettings )
};



template < typename Tnum >
class MandelbrotBase
{
public:
	explicit MandelbrotBase ( MandelbrotSettings & settings )
		:
		m_settings	( settings ),
		m_pal		( settings.palette.get_palette () ),
		m_knum_4	( 4 )
	{
	}

	virtual ~MandelbrotBase ()
	{
	}

	virtual int build () = 0;

protected:
/*
	virtual void build_single_worker ()
	virtual void build_multi_workers ()
	virtual void render_line ( int const y ) const

	int build_init ()
	void build_multi_leader ()
	void start_timer ()
	void finish_timer () const

	size_t calculate (
		Tnum	const	x,
		Tnum	const	y
		) const
*/

	int build_init ()
{
	m_dest		= pixy_pixmap_get_canvas ( m_settings.pixmap.get () );
	m_w		= pixy_pixmap_get_width ( m_settings.pixmap.get () );
	m_h		= pixy_pixmap_get_height ( m_settings.pixmap.get () );
	m_stride	= m_w * 3;

	m_w_dec = m_w - 1;
	m_h_dec = m_h - 1;

	if ( m_settings.threads > 1 )
	{
		if ( m_settings.thread[0].init ( [this]() { build_multi_leader (); } ) )
		{
			std::cerr <<
				"build_mandelbrot_set: thread init failure.\n";
			return 1;
		}

		if ( m_settings.thread[0].start () )
		{
			std::cerr <<
				"build_mandelbrot_set: thread start failure.\n";
			return 1;
		}
	}
	else
	{
		if ( m_settings.thread[0].init ( [this]() { build_single_worker (); } ) )
		{
			std::cerr <<
				"build_mandelbrot_set: thread init failure.\n";
			return 1;
		}

		if ( m_settings.thread[0].start () )
		{
			std::cerr <<
				"build_mandelbrot_set: thread start failure.\n";
			return 1;
		}
	}

	return 0;
}

	virtual void build_single_worker ()
{
	start_timer ();

	for ( int y = 0; y < m_h; y++ )
	{
		switch ( m_settings.thread[0].get_status() )
		{
		case mtGin::Thread::THREAD_TERMINATED:
			return;
		}

		render_line ( y );
	}

	finish_timer ();
}

	void build_multi_leader ()
{
	start_timer ();

	m_settings.work.clear ();

	for ( int t = 1; t <= m_settings.threads; t++ )
	{
		if ( m_settings.thread[ t ].init ( [this]()
			{
				build_multi_workers ();
			} )
			)
		{
			std::cerr <<
				"build_mandelbrot_set: thread init failure.\n";
			return;
		}

		if ( m_settings.thread[ t ].start () )
		{
			std::cerr <<
				"build_mandelbrot_set: thread start failure.\n";
			return;
		}
	}

	for ( int y = 0; y < m_h; )
	{
		switch ( m_settings.thread[0].get_status() )
		{
		case mtGin::Thread::THREAD_TERMINATED:
			goto finish;
		}

		int const status = m_settings.work.push ( y );

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
	m_settings.work.finished ();

	for ( int i = 1; i <= m_settings.threads; i++ )
	{
		m_settings.thread[i].join();
	}

	finish_timer ();
}

	virtual void build_multi_workers ()
{
	int y = 0;

	while ( 1 )
	{
		switch ( m_settings.work.pop ( y ) )
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

	void start_timer ()
{
	m_clock.restart ();
}

	void finish_timer () const
{
	if ( m_settings.verbose )
	{
		printf ( "MandelbrotBase::finish_timer time=%.15g secs\n",
			m_clock.seconds());
	}
}

	size_t calculate (
		Tnum	const	x,
		Tnum	const	y
		) const
{
	Tnum x2 ( 0 );
	Tnum y2 ( 0 );
	Tnum w ( 0 );
	size_t i = 0;

	while ( (x2 + y2) <= m_knum_4 && i < m_settings.depth_max )
	{
		Tnum const x1 = x2 - y2 + x;
		Tnum const y1 = w - x2 - y2 + y;

		x2 = x1 * x1;
		y2 = y1 * y1;

		w = (x1 + y1) * (x1 + y1);

		i++;
	}

	return i;
}

	virtual void render_line ( int const y ) const
{
	unsigned char	* d = m_dest + m_stride * y;
	Tnum	const	mby = m_yo + (y / m_h_dec) * m_range_h;
	size_t	const	pal_size = m_pal.size();

	for ( int x = 0; x < m_w; x++ )
	{
		unsigned char	r, g, b;
		Tnum	const	mbx = m_xo + (x / m_w_dec) * m_range_w;
		size_t	const	i = this->calculate ( mbx, mby );

		if ( i < m_settings.depth_max )
		{
			size_t const col = i % pal_size;

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

		switch ( m_settings.thread[0].get_status() )
		{
		case mtGin::Thread::THREAD_TERMINATED:
			return;
		}
	}
}


/// ----------------------------------------------------------------------------

	MandelbrotSettings	& m_settings;

	mtKit::Clock		m_clock;

	unsigned char		* m_dest	= nullptr;
	int			m_w		= 0;
	int			m_h		= 0;
	int			m_stride	= 0;

	std::vector<mtColor> const & m_pal;

/// ----------------------------------------------------------------------------

	Tnum			m_cx;
	Tnum			m_cy;
	Tnum			m_range_w;
	Tnum			m_range_h;

/// Temp variables for the threads ---------------------------------------------

	Tnum			m_xo;
	Tnum			m_yo;
	Tnum			m_w_dec;
	Tnum			m_h_dec;

	Tnum		const	m_knum_4;

/// ----------------------------------------------------------------------------

	MTKIT_RULE_OF_FIVE( MandelbrotBase )
};



class MandelbrotDouble : public MandelbrotBase< double >
{
public:
	explicit MandelbrotDouble ( MandelbrotSettings & settings )
		:
		MandelbrotBase	( settings )
	{
	}

	int build ()						override;

private:

/// ----------------------------------------------------------------------------

	MTKIT_RULE_OF_FIVE( MandelbrotDouble )
};



class MandelbrotFloat : public MandelbrotBase< mtDW::Float >
{
public:
	explicit MandelbrotFloat ( MandelbrotSettings & settings )
		:
		MandelbrotBase	( settings )
	{
	}

	static constexpr mpfr_prec_t NUMBER_PRECISION_BITS = 256;

	int build ()						override;

private:

	void build_single_worker ()				override;
	void build_multi_workers ()				override;

/// ----------------------------------------------------------------------------

	MTKIT_RULE_OF_FIVE( MandelbrotFloat )
};



class MandelbrotFloatFast : public MandelbrotBase< mtDW::Float >
{
public:
	explicit MandelbrotFloatFast ( MandelbrotSettings & settings )
		:
		MandelbrotBase	( settings ),
		m_knum_0	( "0" ),
		m_knum_1	( "1" )
	{
	}

	static constexpr mpfr_prec_t NUMBER_PRECISION_BITS = 128;

	int build ()						override;

private:

	class CalcTmp
	{
	public:
		mtDW::Float	x1, x2, y1, y2, x2y2;
		mtDW::Float	w, w0, mbx, mby;
		mtDW::Float	x_r, y_r;
	};

	void build_single_worker ()				override;
	void build_multi_workers ()				override;

	void render_line ( int y ) const			override;
	size_t calculate ( CalcTmp & tmp ) const;

/// ----------------------------------------------------------------------------

	mtDW::Float	const	m_knum_0;
	mtDW::Float	const	m_knum_1;
	mtDW::Float		m_x1, m_x2;
	mtDW::Float		m_y1, m_y2;

/// ----------------------------------------------------------------------------

	MTKIT_RULE_OF_FIVE( MandelbrotFloatFast )
};



class MandelbrotRational : public MandelbrotBase< mtDW::Rational >
{
public:
	explicit MandelbrotRational ( MandelbrotSettings & settings )
		:
		MandelbrotBase	( settings )
	{
	}

	int build ()						override;

private:

/// ----------------------------------------------------------------------------

	MTKIT_RULE_OF_FIVE( MandelbrotRational )
};



class Mandelbrot
{
public:
	Mandelbrot ();
	~Mandelbrot ();

	mtPixmap const * get_pixmap () const { return m_settings.pixmap.get(); }

	int get_status () const;
	mtDW::Rational const & get_cx () const { return m_settings.cx; }
	mtDW::Rational const & get_cy () const { return m_settings.cy; }
	mtDW::Rational const & get_range_w () const { return m_settings.range_w; }

	int set_pixmap_size ( int width, int height ); // Pixmap is always RGB

	void set_depth_max ( int d );
	void set_verbose ( int v );
	void set_threads ( int v );
	void set_deep_zoom_type ( int v );
	void set_deep_zoom_on ( int v );

	int build_mandelbrot_set ();
	void build_terminate ();

	void zoom_cxyrange (
		char const * cx,
		char const * cy,
		char const * range_w
		);

	int zoom_in ( int x, int y );
	int zoom_out ( int x, int y );

	void zoom_left ();
	void zoom_right ();
	void zoom_up ();
	void zoom_down ();

	int zoom_reset ();

	void info_to_stdout () const;

private:
	int zoom ( int x, int y, int multiply, int divide );

	void clear_pixmap () const;
	void calculate_range_h ();

/// ----------------------------------------------------------------------------

	// Destroy mtMath last of all
	mtDW::MathState		m_mtmath;

	MandelbrotSettings	m_settings;

	// Constants

	mtDW::Rational	const	k_axis_range_min_bignum;
	mtDW::Rational	const	k_axis_range_max_bignum;
	mtDW::Rational	const	k_axis_range_default;
	mtDW::Rational	const	k_axis_range_max;

	mtDW::Rational	const	k_cxy_min;
	mtDW::Rational	const	k_cxy_max;
	mtDW::Rational	const	k_cxy_default;

	// Text versions are better for Rational as they are evaluated correctly
	// unlike Float's, which potentially lose precision.

	MandelbrotDouble	m_double;
	MandelbrotFloat		m_float;
	MandelbrotFloatFast	m_float_fast;
	MandelbrotRational	m_rational;

/// ----------------------------------------------------------------------------

	MTKIT_RULE_OF_FIVE( Mandelbrot )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTMANDY_BROT_H_

