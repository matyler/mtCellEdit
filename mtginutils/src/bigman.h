/*
	Copyright (C) 2021-2023 Mark Tyler

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

#ifndef MTBIGMAN_H_
#define MTBIGMAN_H_

#undef BIN_NAME
#define BIN_NAME "mtbigman"

#undef APP_NAME
#define APP_NAME "mtBigMan"



#include <mtcelledit.h>
#include <mtdatawell_math.h>
#include <mtgin.h>
#include <mtgin_sdl.h>


// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"



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

	DEPTH_MAX_MIN		= 256,
	DEPTH_MAX_MAX		= 16384,
	DEPTH_MAX_DEFAULT	= 1024,

	OUTPUT_WH_MIN		= 128,
	OUTPUT_WH_MAX		= 15360,
	OUTPUT_WIDTH_DEFAULT	= 500,
	OUTPUT_HEIGHT_DEFAULT	= 500,

	THREAD_TOTAL_MIN	= 1,
	THREAD_TOTAL_MAX	= 16,
	THREAD_TOTAL_DEFAULT	= 0		// Detect from CPU
};



#ifdef __cplusplus
}

// C++ API

class Core;
class Mainwindow;
class Mandelbrot;
class MandelPalette;
class MemPrefs;



class MemPrefs
{
public:
	MemPrefs () {}

	int	window_x		= 0;
	int	window_y		= 0;

private:
	MTKIT_RULE_OF_FIVE( MemPrefs )
};



class Core
{
public:
	Core();
	~Core();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	int get_thread_total () const		{ return m_thread_total; }

	mtDW::Float const & get_axis_cx_r () const { return m_axis_cx_r; }
	mtDW::Float const & get_axis_cy_r () const { return m_axis_cy_r; }
	mtDW::Float const & get_axis_range_r () const { return m_axis_range_r;}

	int get_depth_max () const		{ return m_depth_max; }
	int get_verbose () const		{ return m_verbose; }
	int get_output_width () const		{ return m_output_width; }
	int get_output_height () const		{ return m_output_height; }

/// ----------------------------------------------------------------------------

	// Destroy mtMath last of all
	mtDW::MathState		m_mtmath;
	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();

/// ----------------------------------------------------------------------------

	char	const *	m_prefs_filename	= nullptr;
	int		m_thread_total		= THREAD_TOTAL_DEFAULT;

	mtDW::Float	m_axis_cx_r;
	mtDW::Float	m_axis_cy_r;
	mtDW::Float	m_axis_range_r;
	char	const	* m_axis_cx_st		= nullptr;
	char	const	* m_axis_cy_st		= nullptr;
	char	const	* m_axis_range_st	= nullptr;

	int		m_depth_max		= DEPTH_MAX_DEFAULT;
	int		m_output_width		= OUTPUT_WIDTH_DEFAULT;
	int		m_output_height		= OUTPUT_HEIGHT_DEFAULT;

	int		m_verbose		= 0;

	MTKIT_RULE_OF_FIVE( Core )
};



class MandelPalette
{
public:
	MandelPalette ();	// Default palette created here
	~MandelPalette ();

	enum
	{
		GRADIENT_SIZE_MIN	= 0,
		GRADIENT_SIZE_MAX	= 15,

		PRIMARY_SIZE_MIN	= 2,
		PRIMARY_SIZE_MAX	= 256
	};

	void set_gradients ( size_t grads );
	void set_primary_colors ( std::vector<mtColor> const & cols );

	void mix_gradient_palette ();

	inline std::vector<mtColor> const & get_palette () const
	{
		return m_palette;
	}

private:
	size_t			m_grads = 31;	// Colours between primaries
	std::vector<mtColor>	m_primary {
		{ 160,	32,	32 },	// Red
		{ 224,	224,	64 },	// Yellow
		{ 0,	128,	0 },	// Green
		{ 64,	192,	192 },	// Cyan
		{ 64,	64,	224 },	// Blue
		{ 224,	96,	224 }	// Magenta
		};
	std::vector<mtColor>	m_palette;

	MTKIT_RULE_OF_FIVE( MandelPalette )
};



class CalcTmp
{
public:
	mtDW::Float	x1, x2, y1, y2, x2y2;
	mtDW::Float	w, w0, mbx, mby;
	mtDW::Float	x_r, y_r;
};



class Mandelbrot
{
public:
	Mandelbrot ();
	~Mandelbrot ();

	static constexpr mpfr_prec_t NUMBER_PRECISION_BITS = 256;

	enum
	{
		STATUS_IDLE		= 0,
		STATUS_BUSY		= 1,

		WORK_QUEUE_SIZE		= THREAD_TOTAL_MAX * 2
	};

	int get_status () const;

	int set_pixmap_size ( int width, int height ); // Pixmap is always RGB

	void set_depth_max ( int d );
	void set_verbose ( int v );

	int build_mandelbrot_set ( int threads );
	void build_terminate ();

	void zoom_cxyrange (
		mtDW::Float const & cx,
		mtDW::Float const & cy,
		mtDW::Float const & range_w
		);

	int zoom_in ( int x, int y );
	int zoom_out ( int x, int y );

	void zoom_left ();
	void zoom_right ();
	void zoom_up ();
	void zoom_down ();

	int zoom_reset ();

	inline mtPixmap const * get_pixmap () const { return m_pixmap.get(); }

	mtDW::Float const & get_cx_r () const { return m_cx_r; }
	mtDW::Float const & get_cy_r () const { return m_cy_r; }
	mtDW::Float const & get_range_w_r () const { return m_range_w_r; }

private:
	int zoom ( int x, int y, double scale );
	void clear_pixmap () const;
	void calculate_range_h ();
	void calculate_xyo ();

	// Thread funcs
	void build_single_worker ();
	void build_multi_leader ();
	void build_multi_workers ();
	void render_line ( CalcTmp &tmp, int y ) const;
	size_t calculate ( CalcTmp &tmp ) const;

	void start_timer ();
	void finish_timer () const;

/// ----------------------------------------------------------------------------

	MandelPalette		m_palette;
	mtKit::Clock		m_clock;

	mtPixy::Pixmap		m_pixmap;

	mtDW::Float		m_cx_r;
	mtDW::Float		m_cy_r;
	mtDW::Float		m_range_w_r;
	mtDW::Float		m_range_h_r;
	mtDW::Float	const	m_axis_range_min;
	mtDW::Float	const	m_axis_range_max;

	size_t			m_depth_max	= DEPTH_MAX_DEFAULT;
	int			m_verbose	= 0;

/// Temp variables for the threads ---------------------------------------------

	int			m_thread_tot	= 0;
	unsigned char		* m_dest	= nullptr;

	mtDW::Float		m_xo_r;
	mtDW::Float		m_yo_r;

	int			m_w		= 0;
	int			m_h		= 0;
	int			m_stride	= 0;

	mtDW::Float		m_w_r;

	mtDW::Float		m_w_dec_r;
	mtDW::Float		m_h_dec_r;

	mtDW::Float	const	m_knum_0;
	mtDW::Float	const	m_knum_1;
	mtDW::Float	const	m_knum_4;
	mtDW::Float		m_x1, m_x2;
	mtDW::Float		m_y1, m_y2;

	std::vector<mtColor> const & m_pal;
	size_t			m_pal_size	= 0;

/// ----------------------------------------------------------------------------

	mtGin::ThreadWork<int>	m_work;
	mtGin::Thread		m_thread[ THREAD_TOTAL_MAX + 1 ];

	MTKIT_RULE_OF_FIVE( Mandelbrot )
};



class Mainwindow
{
public:
	explicit Mainwindow ( Core & core );
	~Mainwindow ();

	int export_image ( std::string const & filename ) const;

private:
	void window_render ();
	int window_event ( SDL_Event const * event );
	void window_close ();

	void output_position () const;
	void output_image_file () const;

	void frame_cycle ();
	void mouse_click ();
	void key_click ( SDL_Event const * event );

	void centre_frame ();
	void set_frame ( int x, int y );
	void set_mode ( int mode );
	void quit ();
	void prepare_surface ();
	void start_rendering ( int zoom );	// ZOOM_*

	enum
	{
		ZOOM_OUT	= -1,
		ZOOM_NONE	= 0,
		ZOOM_IN		= 1,
		ZOOM_HOME	= 2,
		ZOOM_LEFT	= 3,
		ZOOM_RIGHT	= 4,
		ZOOM_UP		= 5,
		ZOOM_DOWN	= 6,

		MODE_INTERACTIVE_RENDERING_FPS	= 10,
		MODE_INTERACTIVE_FRAMING_FPS	= 60,

		MODE_NULL			= -1,
		MODE_INTERACTIVE_RENDERING	= 0,
		MODE_INTERACTIVE_FRAMING	= 1
	};

/// ----------------------------------------------------------------------------

	int	const	m_window_width;
	int	const	m_window_height;
	int		m_mode		= MODE_NULL;
	int		m_anim_frame	= -1;
	int		m_quit		= 0;
	SDL_Rect	m_frame		= {0,0,0,0};
	SDL_Texture	* m_texture	= nullptr;

	Core		& m_core;
	Mandelbrot	m_mandy;
	mtGin::App	m_gin;
	mtGin::Window	m_window;

	MTKIT_RULE_OF_FIVE( Mainwindow )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTBIGMAN_H_

