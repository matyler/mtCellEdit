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

#include "soup.h"



namespace {



static void init_main_window (
	Core		&core,
	mtGin::App	&gin,
	mtGin::Window	&window
	)
{
	window.init ( "mtSoup - SOUnd Player",
		core.mprefs.window_x,
		core.mprefs.window_y,
		core.mprefs.window_w,
		core.mprefs.window_h,
		(core.mprefs.window_maximized ? SDL_WINDOW_MAXIMIZED : 0)
			| SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_BORDERLESS*/ );

	gin.window_add ( window );

	window.set_size_min ( 100, 100 );
//	window.set_icon ( core.get_svg_filename() );

	window.add_signal ( GIN_SIGNAL_SDL_EVENT, [&gin, &core]
	(mtGin::CB_Data const * const data)
	{
		auto const d = dynamic_cast<mtGin::CB_SDLEvent const *>(data);

		if ( ! d || ! d->sdl_event )
		{
			return 1;	// Not actioned
		}

		SDL_Event const * const event = d->sdl_event;

		switch ( event->type )
		{
		case SDL_KEYDOWN:
			switch ( event->key.keysym.sym )
			{
			case SDLK_SPACE:
				core.audio_play.toggle_pause_resume ();
				break;

			case SDLK_ESCAPE:
				gin.stop_main_loop ();
				break;
			}
			break;

		case SDL_WINDOWEVENT:
			switch ( event->window.event )
			{
			case SDL_WINDOWEVENT_CLOSE:
				gin.stop_main_loop ();
				break;
			}

			break;
		}

		return 0;		// Actioned
	} );

	window.add_signal( GIN_SIGNAL_DESTROY_WINDOW, [&core, &window]
	(void *)
	{
		int x, y, w, h;
		Uint32 flags;
		window.get_geometry ( x, y, w, h, flags );

		int const maximized = (flags & SDL_WINDOW_MAXIMIZED) ? 1 : 0;

		core.uprefs.set ( PREFS_WINDOW_MAXIMIZED, maximized );

		if ( 0 == maximized )
		{
			core.uprefs.set ( PREFS_WINDOW_X, x );
			core.uprefs.set ( PREFS_WINDOW_Y, y );
			core.uprefs.set ( PREFS_WINDOW_W, w );
			core.uprefs.set ( PREFS_WINDOW_H, h );
		}

		return 0;
	} );

	window.m_render.set ( [&window, &gin, &core]()
	{
		auto const renderer = window.renderer();
		int const status = core.audio_play.get_status ();

		Uint8 r, g, b;

		switch ( status )
		{
		default:
		case SDL_AUDIO_STOPPED:
			r = 128;
			g = 0;
			b = 128;
			break;

		case SDL_AUDIO_PLAYING:
			r = 0;
			g = 200;
			b = 0;
			break;

		case SDL_AUDIO_PAUSED:
			r = 200;
			g = 0;
			b = 0;
			break;
		}

		SDL_SetRenderDrawColor ( renderer, r, g, b, SDL_ALPHA_OPAQUE );
		SDL_RenderClear ( renderer );

		mtGin::AudioVU	const & vu = core.audio_play.get_vu ();
		size_t	const	chantot = vu.m_level.size();
		int		w=100, h=100;
		SDL_Rect	rect;
		auto	const	sdl = window.sdl();

		SDL_GetWindowSize ( sdl, &w, &h );

		int	const	bar_w = (w / 2) / (int)chantot;

		SDL_SetRenderDrawColor ( renderer, 0, 0, 0, 0 );
		rect.x = 0;
		rect.y = 0;
		rect.w = w / 2;
		rect.h = h;
		SDL_RenderFillRect ( renderer, &rect );

		for ( size_t c = 0; c < chantot; c++ )
		{
			double const val = mtkit_double_bound (
				(double)(vu.m_level[c]) / 32767.0, 0.0, 1.0 );
			int const vpix = (int)(0.5 + val * (h - 2));

			SDL_SetRenderDrawColor ( renderer, 0, 100, 200, 0 );
			rect.x = 1 + (int)c * bar_w;
			rect.y = h - vpix - 1;
			rect.w = bar_w - 2;
			rect.h = vpix;
			SDL_RenderFillRect ( renderer, &rect );
		}

		SDL_RenderPresent ( renderer );
	} );

	window.m_render.start ();
}



}		// namespace {



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Core	core;

	try
	{
		if ( core.command_line ( argc, argv ) )
		{
			return core.exit.value();
		}

		core.gin.set_fps_max ( 30 );

		if ( core.get_gui () )
		{
			SDL_InitSubSystem ( SDL_INIT_VIDEO );
		}

		std::unique_ptr<mtGin::Window> window;

		if ( core.get_gui () )
		{
			window.reset ( new mtGin::Window(GIN_WINDOW_CANVAS_2D));

			init_main_window ( core, core.gin, *window.get() );
		}

		core.gin.frame_callback.set ( [&core]()
		{
			core.audio_play.queue_file_data ();

			if ( SDL_AUDIO_STOPPED == core.audio_play.get_status() )
			{
				core.gin.stop_main_loop ();
			}
		} );
		core.gin.frame_callback.start ();

		// Start playing here:
		core.audio_play.resume ();

		core.gin.main_loop ();
	}
	catch (...)
	{
		std::cerr << "Exception while initializing the app UI.\n";
		return 1;
	}

	return core.exit.value();
}

