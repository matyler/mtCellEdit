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

#include "private.h"



mtGin::Window::Window ( int const canvas )
	:
	m_canvas	( canvas )
{
}

mtGin::Window::~Window ()
{
	emit_signal ( GIN_SIGNAL_DESTROY_WINDOW, nullptr );

	if ( m_sdl_window )
	{
		if ( m_canvas == GIN_WINDOW_CANVAS_OPENGL )
		{
			SDL_GL_DeleteContext ( m_glcontext );
		}

		if ( m_sdl_renderer )
		{
			SDL_DestroyRenderer ( m_sdl_renderer );
			m_sdl_renderer = nullptr;
		}

		SDL_DestroyWindow ( m_sdl_window );
		m_sdl_window = nullptr;
	}
}

char const * mtGin::Window::name() const
{
	return "mtGin::Window";
}

void mtGin::Window::get_geometry (
	int	& x,
	int	& y,
	int	& w,
	int	& h,
	Uint32	& flags
	)
{
	border_store_hack ();

	SDL_GetWindowPosition ( m_sdl_window, &x, &y );
	SDL_GetWindowSize ( m_sdl_window, &w, &h );

	flags = border_restore_hack ();
}

void mtGin::Window::set_size_min (
	int	const	w,
	int	const	h
	)
{
	border_store_hack ();

	SDL_SetWindowMinimumSize ( m_sdl_window, MAX( GIN_WINDOW_WIDTH_MIN, w ),
		MAX( GIN_WINDOW_HEIGHT_MIN, h ) );

	border_restore_hack ();
}

void mtGin::Window::init (
	char	const * const	title,
	int 		const	x,
	int 		const	y,
	int		const	w,
	int		const	h,
	Uint32			flags
	)
{
	if ( m_sdl_window )
	{
		std::cerr << "mtGin::Window::init: "
			"Only one main window is allowed\n";
		return;
	}

	if ( m_canvas == GIN_WINDOW_CANVAS_OPENGL )
	{
		flags |= SDL_WINDOW_OPENGL;
	}

	m_sdl_window = SDL_CreateWindow ( title, x, y, w, h, flags );
	if ( ! m_sdl_window )
	{
		std::cerr << "Window::init - Unable to create the SDL window: '"
			<< (title ? title : "")
			<< "' flags=" << flags
			<< " x=" << x
			<< " y=" << y
			<< " w=" << w
			<< " h=" << h
			<< " SDL(" << SDL_GetError() << ")"
			<< "\n";
		throw 123;
	}

	border_store_hack ();

	if ( m_canvas == GIN_WINDOW_CANVAS_OPENGL )
	{
		SDL_GL_SetSwapInterval ( 1 );
		m_glcontext = SDL_GL_CreateContext ( m_sdl_window );
		SDL_GL_MakeCurrent ( m_sdl_window, m_glcontext );
	}

	SDL_SetWindowMinimumSize ( m_sdl_window, GIN_WINDOW_WIDTH_MIN,
		GIN_WINDOW_HEIGHT_MIN );

	border_restore_hack ();

	m_sdl_renderer = SDL_CreateRenderer ( m_sdl_window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );

	if ( ! m_sdl_renderer )
	{
		SDL_CreateRenderer ( m_sdl_window, -1, 0 );
	}

	if ( ! m_sdl_renderer )
	{
		std::cerr << "Unable to set up any window renderer!\n";
	}
}

int mtGin::Window::set_icon ( mtPixmap const * const pixmap )
{
	SDL_Surface * surface = surface_from_pixmap ( pixmap );

	if ( surface )
	{
		SDL_SetWindowIcon ( m_sdl_window, surface );
		SDL_FreeSurface ( surface );
	}
	else
	{
		return 1;
	}

	return 0;
}

int mtGin::Window::set_icon ( char const * const filename )
{
	mtPixy::SVG	svg;

	if ( svg.load ( filename ) )
	{
		return 1;
	}

	int const w = 64;
	int const h = 64;
	void * const argb = svg.render ( w, h );

	if ( ! argb )
	{
		return 1;
	}

	SDL_Surface * const surface = SDL_CreateRGBSurfaceFrom (
		argb, w, h, 32, 4*w,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xFF000000
		);

	if ( ! surface )
	{
		return 1;
	}

	SDL_SetWindowIcon ( m_sdl_window, surface );
	SDL_FreeSurface ( surface );

	return 0;
}

mtPixmap * mtGin::Window::dump_to_pixmap () const
{
	int w = 0, h = 0;

	SDL_GetWindowSize ( m_sdl_window, &w, &h );

	mtPixy::Pixmap pm ( pixy_pixmap_new_rgb ( w, h ) );
	unsigned char * const dest = pixy_pixmap_get_canvas ( pm.get() );

	if ( ! dest )
	{
		std::cerr << "export_image: Unable to allocate pixmap\n";
		return nullptr;
	}

	if ( GIN_WINDOW_CANVAS_OPENGL == m_canvas )
	{
		unsigned char * pixels = (unsigned char *)malloc (
			size_t(4 * w * h) );

		if ( ! pixels )
		{
			std::cerr <<"export_image: Unable to allocate pixels\n";
			goto finish;
		}

		glReadPixels ( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels );

		for ( int y = 0; y < h; y++ )
		{
			unsigned char * d = dest + y * w * 3;
			unsigned char const * s = pixels + (h - y - 1) * w * 4;
			// OpenGL dumps image upside down, so reverse this

			for ( int x = 0; x < w; x++ )
			{
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
				s++;
			}
		}

		free ( pixels );
	}
	else if ( GIN_WINDOW_CANVAS_2D == m_canvas )
	{
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = w;
		rect.h = h;

		SDL_RenderReadPixels ( m_sdl_renderer, &rect,
			SDL_PIXELFORMAT_RGB24, dest, 3*w );
	}
	else
	{
		std::cerr << "mtGin::Window::dump_to_pixmap Unknown canvas"
			" type\n";
	}

finish:
	return pm.release();
}

Uint32 mtGin::Window::border_store_hack ()
{
	m_border_hack = SDL_GetWindowFlags ( m_sdl_window );

	SDL_SetWindowBordered ( m_sdl_window, SDL_FALSE );

	return m_border_hack;
}

Uint32 mtGin::Window::border_restore_hack ()
{
	Uint32 const state = (m_border_hack & SDL_WINDOW_BORDERLESS);

	SDL_SetWindowBordered ( m_sdl_window, state ? SDL_FALSE : SDL_TRUE );

	return m_border_hack;
}

