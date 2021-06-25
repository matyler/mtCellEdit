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

#ifndef MTLANTER_H_
#define MTLANTER_H_

#undef BIN_NAME
#define BIN_NAME "mtlanter"

#include <mtgin.h>
#include <mtgin_sdl.h>
#include <mtgin_gl.h>
#include <mtcelledit.h>



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"



enum
{
	LANDSCAPE_VIEW_RANGE		= 1000
};


#ifdef __cplusplus
extern "C" {
#endif

// C API

enum
{
	ANIM_FPS_MIN		= 10,
	ANIM_FPS_MAX		= 120,
	ANIM_FPS_DEFAULT	= 60,

	MAP_SIZE_MIN		= 4,		// rows/cols = 2^4 + 1	= 17
	MAP_SIZE_MAX		= 10,		// rows/cols = 2^10 + 1	= 1025
	DEFAULT_MAP_SIZE	= 6,		// (1 << 6)+1 = 65 rows
	DEFAULT_MAP_SEED	= 0,

	OUTPUT_WH_MIN		= 128,
	OUTPUT_WH_MAX		= 15360,
	OUTPUT_WIDTH_DEFAULT	= 500,
	OUTPUT_HEIGHT_DEFAULT	= 500
};



#ifdef __cplusplus
}

// C++ API

class Core;
class Lanimate;
class Mainwindow;
class MemPrefs;
class TerraCamera;
class TerraGL;
class TerraMap;



class MemPrefs
{
public:
	MemPrefs () {}

	int	window_x		= 0;
	int	window_y		= 0;
	int	window_w		= 0;
	int	window_h		= 0;
	int	window_maximized	= 0;

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

	inline char const * get_anim_dir () const { return m_anim_dir; }
	inline char const * get_anim_tsv () const { return m_anim_tsv; }
	inline int get_anim_fps () const	{ return m_anim_fps; }
	inline int get_map_size () const	{ return m_map_size; }
	inline int get_map_seed () const	{ return m_map_seed; }
	inline int get_output_width () const	{ return m_output_width; }
	inline int get_output_height () const	{ return m_output_height; }
	inline char const * get_save_map () const { return m_save_map; }
	inline int get_verbose () const		{ return m_verbose; }

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();

/// ----------------------------------------------------------------------------

	char	const *	m_anim_dir		= nullptr;
	int		m_anim_fps		= ANIM_FPS_DEFAULT;
	char	const *	m_anim_tsv		= nullptr;
	int		m_map_size		= DEFAULT_MAP_SIZE;
	int		m_map_seed		= DEFAULT_MAP_SEED;
	int		m_output_width		= 0;
	int		m_output_height		= 0;
	char	const *	m_prefs_filename	= nullptr;
	char	const *	m_save_map		= nullptr;
	int		m_verbose		= 0;

	MTKIT_RULE_OF_FIVE( Core )
};



class Lanimate
{
public:
	Lanimate ( Mainwindow & mw, Core & core );
	~Lanimate ();

	int init ();
	int prepare_frame ( int frame );
	int save_frame ( int frame ) const;

	inline int get_frame_total () const { return m_frame_total; }
	inline mtGin::Vertex const & get_position () const
		{ return m_view_position; }
	inline mtGin::Vertex const & get_focus () const
		{ return m_view_focus; }

private:
	void clear ();

/// ----------------------------------------------------------------------------

	Mainwindow	& m_mainwindow;
	Core		& m_core;
	std::string	m_output_dir;
	int		m_fps		= ANIM_FPS_DEFAULT;
	int		m_frame_total	= 0;

	mtGin::BezierPath	m_camera_path;
	mtGin::BezierPath	m_focus_path;
	std::vector<double>	m_time_path;

	mtGin::Vertex		m_view_position;
	mtGin::Vertex		m_view_focus;

	MTKIT_RULE_OF_FIVE( Lanimate )
};



class TerraMap
{
public:
	TerraMap ();
	~TerraMap ();

	inline void set_seed ( uint64_t const seed )
	{
		m_random.set_seed ( seed );
	}

	inline void set_value( int const x, int const y, int const value ) const
	{
		m_mem[ x + m_rows * y ] = value;
	}

	inline int get_rows () const { return m_rows; }

	inline int get_value ( int x, int y ) const
	{
		return m_mem[ x + m_rows * y ];
	}

	inline float get_value_real ( int x, int y ) const
	{
		return float( get_value(x,y) ) / float(m_rows) * m_scale / 3;
	}

	inline float get_xy_real ( int xy ) const
	{
		return m_scale * ( float(xy) / float(m_rows-1) ) - m_scale2;
	}

	int create_landscape ( int size );
	int save_tsv ( char const * filename ) const;

private:
	int set_size ( int size );
	void set_mem ( int * mem, int rows );
	inline int get_random_value ( int const gap)
	{
		return (m_random.get_int ( gap + 1 ) - gap/2);
	}

/// ----------------------------------------------------------------------------

	mtKit::Random	m_random;

	int		* m_mem		= nullptr;
	int		m_rows		= 0;

	static float constexpr m_scale	= float(LANDSCAPE_VIEW_RANGE);
	static float constexpr m_scale2	= float(LANDSCAPE_VIEW_RANGE) / 2;

	MTKIT_RULE_OF_FIVE( TerraMap )
};



class TerraCamera : public mtGin::GL::Camera
{
public:
	static double constexpr NUDGE_MIN = 0.1;
	static double constexpr NUDGE_MAX = 10.0;
	static double constexpr NUDGE_DEFAULT = 1.0;

	inline double get_nudge() const { return m_nudge; }
	inline void set_nudge ( double nudge )
	{
		m_nudge = mtkit_double_bound ( nudge, NUDGE_MIN, NUDGE_MAX );
	}

private:
	double		m_nudge = NUDGE_DEFAULT;
};



class TerraGL
{
public:
	TerraGL ();
	~TerraGL ();

	void set_window_size ( int width, int height, float range );

	void init_buffers ( TerraMap const & map );
	void set_light ( double x, double y, double z );

	void render ( mtGin::GL::Matrix4x4 const & camera ) const;

	static void render_pyramid ( GLfloat x, GLfloat y, GLfloat z,
		GLfloat size );
	void render_light () const;
	static void render_axis ();

private:
	mtGin::GL::Matrix4x4	m_proj;
	mtGin::Vertex		m_light;

	mtGin::GL::Triangles	m_gl_triangles;
	mtGin::GL::Lines	m_gl_lines;
	mtGin::GL::Lines	m_gl_lines_bez;
};



class Mainwindow
{
public:
	explicit Mainwindow ( Core & core );
	~Mainwindow ();

	int export_image ( std::string const & filename );

private:
	enum
	{
		MODE_NULL		= -1,
		MODE_INTERACTIVE	= 0,
		MODE_ANIMATION		= 2
	};

	int init_map ();
	void init_gl ();
	void init_gl_perspective ();

	static void set_gl_perspective_flat ();
	void set_gl_perspective_3d () const;

	void move_camera_home ( int top );

	void window_render () const;
	int window_event ( SDL_Event const * event );
	void window_close ();

	void output_image_file ();

	void frame_cycle ();
	void mouse_click ( int dx, int dy );
	void key_click ( SDL_Event const * event );

	void set_mode ( int mode );

	void quit ();

/// ----------------------------------------------------------------------------

	int		m_quit		= 0;
	int		m_store_wh	= 0;
	int		m_mode		= MODE_NULL;
	int		m_anim_frame	= -1;

	Core		& m_core;
	TerraMap	m_terramap;
	TerraGL		m_terragl;
	TerraCamera	m_camera;
	Lanimate	m_animate;
	mtGin::App	m_gin;
	mtGin::Window	m_window;

	// OpenGL stuff
	int		m_gl_width	= OUTPUT_WIDTH_DEFAULT;
	int		m_gl_height	= OUTPUT_HEIGHT_DEFAULT;
	static float constexpr m_gl_range = LANDSCAPE_VIEW_RANGE * 10;
	double		m_fW		= 0;
	double		m_fH		= 0;
	double		m_zNear		= 0;

	MTKIT_RULE_OF_FIVE( Mainwindow )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTLANTER_H_

