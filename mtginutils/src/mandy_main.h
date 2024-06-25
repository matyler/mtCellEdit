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

#ifndef MTMANDY_MAIN_H_
#define MTMANDY_MAIN_H_



#include <mtkit.h>

#include "mandy_brot.h"



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API



#undef BIN_NAME
#define BIN_NAME "mtmandy"

#undef APP_NAME
#define APP_NAME "mtMandy"

#define PREFS_WINDOW_X		"main.window_x"
#define PREFS_WINDOW_Y		"main.window_y"



#ifdef __cplusplus
}

// C++ API

class CommandLine;
class Mainwindow;
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



class CommandLine
{
public:
	CommandLine();
	~CommandLine();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	int get_thread_total () const		{ return m_thread_total; }

	char const * get_axis_cx_st () const	{ return m_axis_cx_st; }
	char const * get_axis_cy_st () const	{ return m_axis_cy_st; }
	char const * get_axis_range_st () const	{ return m_axis_range_st; }

	int get_depth_max () const		{ return m_depth_max; }
	int get_verbose () const		{ return m_verbose; }
	int get_output_width () const		{ return m_output_width; }
	int get_output_height () const		{ return m_output_height; }
	int get_deep_zoom_type () const		{ return m_deep_zoom_type; }
	int get_deep_zoom_on () const		{ return m_deep_zoom_on; }

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();

/// ----------------------------------------------------------------------------

	char	const	* m_prefs_filename	= nullptr;

	char	const	* m_axis_cx_st		= nullptr;
	char	const	* m_axis_cy_st		= nullptr;
	char	const	* m_axis_range_st	= nullptr;

	int		m_output_width		= OUTPUT_WIDTH_DEFAULT;
	int		m_output_height		= OUTPUT_HEIGHT_DEFAULT;

	int		m_depth_max		= DEPTH_MAX_DEFAULT;
	int		m_verbose		= 1;
	int		m_thread_total		= THREAD_TOTAL_DEFAULT;
	int		m_deep_zoom_type	= DEEP_ZOOM_FLOAT_FAST;
	int		m_deep_zoom_on		= 1;

	MTKIT_RULE_OF_FIVE( CommandLine )
};



class Mainwindow
{
public:
	explicit Mainwindow ( CommandLine & cline );
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

		MODE_NULL			= -1,
		MODE_INTERACTIVE_RENDERING	= 0,
		MODE_INTERACTIVE_FRAMING	= 1
	};

/// ----------------------------------------------------------------------------

	int	const	m_window_width;
	int	const	m_window_height;
	int		m_mode		= MODE_NULL;
	int		m_quit		= 0;
	SDL_Rect	m_frame		= {0,0,0,0};
	SDL_Texture	* m_texture	= nullptr;

	CommandLine	& m_cline;
	Mandelbrot	m_mandy;
	mtGin::App	m_gin;
	mtGin::Window	m_window;

	MTKIT_RULE_OF_FIVE( Mainwindow )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTMANDY_MAIN_H_

