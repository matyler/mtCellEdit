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

#ifndef MTMANDY_H_
#define MTMANDY_H_

#undef BIN_NAME
#define BIN_NAME "mtmandy"



#include <mtcelledit.h>
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



#define AXIS_RANGE_MIN		1.0e-15



enum
{
	ANIM_FPS_MIN		= 10,
	ANIM_FPS_MAX		= 120,
	ANIM_FPS_DEFAULT	= 60,

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
class Manimate;
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

	inline int get_thread_total () const	{ return m_thread_total; }
	inline double get_axis_cx () const	{ return m_axis_cx; }
	inline double get_axis_cy () const	{ return m_axis_cy; }
	inline double get_axis_range () const	{ return m_axis_range; }
	inline int get_depth_max () const	{ return m_depth_max; }
	inline int get_verbose () const		{ return m_verbose; }
	inline int get_output_width () const	{ return m_output_width; }
	inline int get_output_height () const	{ return m_output_height; }
	inline char const * get_anim_dir () const { return m_anim_dir; }
	inline char const * get_anim_tsv () const { return m_anim_tsv; }
	inline int get_anim_fps () const	{ return m_anim_fps; }

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();

/// ----------------------------------------------------------------------------

	char	const *	m_prefs_filename	= nullptr;
	int		m_thread_total		= THREAD_TOTAL_DEFAULT;

	double		m_axis_cx		= AXIS_CXY_DEFAULT;
	double		m_axis_cy		= AXIS_CXY_DEFAULT;
	double		m_axis_range		= AXIS_RANGE_DEFAULT;
	int		m_depth_max		= DEPTH_MAX_DEFAULT;
	int		m_output_width		= OUTPUT_WIDTH_DEFAULT;
	int		m_output_height		= OUTPUT_HEIGHT_DEFAULT;

	int		m_verbose		= 0;

	char	const *	m_anim_dir		= nullptr;
	char	const *	m_anim_tsv		= nullptr;
	int		m_anim_fps		= ANIM_FPS_DEFAULT;

	MTKIT_RULE_OF_FIVE( Core )
};



class Manimate
{
public:
	Manimate ( Mainwindow & mw, Core & core );
	~Manimate ();

	int init ( Mandelbrot * mandy );
	int prepare_frame ( int frame ) const;
	int save_frame ( int frame ) const;

	inline int get_frame_total () const { return m_frame_total; }

private:
	void clear ();

/// ----------------------------------------------------------------------------

	Mainwindow	& m_mainwindow;
	Core		& m_core;
	Mandelbrot	* m_mandy	= nullptr;
	std::string	m_output_dir;
	int		m_fps		= ANIM_FPS_DEFAULT;
	int		m_frame_total	= 0;
	int		m_verbose	= 0;

	mtGin::BezierPath	m_path;
	std::vector<double>	m_time;
	std::vector<double>	m_range;

	MTKIT_RULE_OF_FIVE( Manimate )
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



class Mandelbrot
{
public:
	Mandelbrot ();
	~Mandelbrot ();

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

	void zoom_cxyrange ( double cx, double cy, double range_w );

	int zoom_in ( int x, int y );
	int zoom_out ( int x, int y );

	void zoom_left ();
	void zoom_right ();
	void zoom_up ();
	void zoom_down ();

	int zoom_reset ();

	inline mtPixmap const * get_pixmap () const { return m_pixmap.get(); }

	inline double get_cx () const { return m_cx; }
	inline double get_cy () const { return m_cy; }
	inline double get_range_w () const { return m_range_w; }

private:
	int zoom ( int x, int y, double scale );
	void clear_pixmap () const;
	void calculate_range_h ();
	void calculate_xyo ();

	// Thread funcs
	void build_single_worker ();
	void build_multi_leader ();
	void build_multi_workers ();
	void render_line ( int y ) const;
	size_t calculate ( double x, double y ) const;

	void start_timer ();
	void finish_timer () const;

/// ----------------------------------------------------------------------------

	MandelPalette		m_palette;
	mtPixy::Pixmap		m_pixmap;

	double			m_cx		= AXIS_CXY_DEFAULT;
	double			m_cy		= AXIS_CXY_DEFAULT;
	double			m_range_w	= AXIS_RANGE_DEFAULT;
	double			m_range_h	= AXIS_RANGE_DEFAULT;
	size_t			m_depth_max	= DEPTH_MAX_DEFAULT;
	int			m_verbose	= 0;

	double			m_time_start	= 0.0;

/// Temp variables for the threads ---------------------------------------------

	int			m_thread_tot	= 0;
	unsigned char		* m_dest	= nullptr;
	double			m_xo		= 0.0;
	double			m_yo		= 0.0;
	int			m_w		= 0;
	int			m_h		= 0;
	int			m_stride	= 0;
	double			m_w_dec		= 0.0;
	double			m_h_dec		= 0.0;
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
		MODE_ANIMATION_RENDERING_FPS	= 30,

		MODE_NULL			= -1,
		MODE_INTERACTIVE_RENDERING	= 0,
		MODE_INTERACTIVE_FRAMING	= 1,
		MODE_ANIMATION_RENDERING	= 2
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
	Manimate	m_animate;
	mtGin::App	m_gin;
	mtGin::Window	m_window;

	MTKIT_RULE_OF_FIVE( Mainwindow )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTMANDY_H_

