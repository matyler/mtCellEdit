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



mtGin::App::App ( Uint32 const flags )
{
	if ( SDL_Init ( flags ) )
	{
		std::cout << "Unable to initialize SDL: " << SDL_GetError()
			<< "\n";
	}

	set_fps_max ( GIN_FPS_DEFAULT );
}

mtGin::App::~App ()
{
	SDL_Quit();
}

void mtGin::App::main_loop ()
{
	main_loop ( 1 );
}

void mtGin::App::main_loop_iterate ()
{
	main_loop ( 0 );
}

void mtGin::App::set_fps_max ( int const fps )
{
	m_frame_delay = (Uint32)(1000 / mtkit_int_bound ( fps,
		GIN_FPS_MIN, GIN_FPS_MAX ) );
}

void mtGin::App::main_loop ( int const loop )
{
	if ( m_main_loop_nest > GIN_MAIN_LOOP_NEST_MAX )
	{
		return;
	}

	m_main_loop_nest++;

	while ( 0 == m_quit )
	{
		frame_callback.run ();

		poll_events ();

		if ( m_quit )
		{
			break;
		}

/// OpenGL or SDL rendering ----------------------------------------------------
		for (	auto it = m_window_map.begin();
			it != m_window_map.end();
			++it
			)
		{
			// Get the calling app to render the window surface
			it->second->m_render.run ();
		}

		frame_time_delay ();

		if ( 0 == loop )
		{
			break;
		}
	}

	m_main_loop_nest--;
}

void mtGin::App::window_add ( Window & window )
{
	SDL_Window * const sdl = window.sdl();

	if ( ! sdl )
	{
		std::cerr << "App::window_add ERROR - SDL window hasn't been "
			"initialized yet.\n";
		return;
	}

	m_window_map[ sdl ] = & window;
}

Uint32 mtGin::App::get_window_event_id ( Uint32 const id )
{
	if ( id > 0 )
	{
		m_win_id_old = id;
		return id;
	}

	return m_win_id_old;
}

void mtGin::App::poll_events ()
{
	SDL_Event	event;
	CB_SDLEvent	data;

	data.sdl_event = & event;

	while ( SDL_PollEvent ( &event ) )
	{
		Uint32 win_id = 0;

		switch ( event.type )
		{
		case SDL_QUIT:
			if ( ! m_sdl_event_cb )
			{
				// Ctrl+C at terminal, etc. not handled by user
				// app so terminate.
				stop_main_loop();
			}
			break;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
		case SDL_TEXTEDITING:
		case SDL_TEXTINPUT:
		case SDL_KEYMAPCHANGED:
			win_id = get_window_event_id ( event.key.windowID );
			break;

		case SDL_MOUSEMOTION:
			win_id = get_window_event_id ( event.motion.windowID );
			break;

		case SDL_MOUSEBUTTONDOWN:
			win_id = get_window_event_id ( event.button.windowID );
			break;

		case SDL_MOUSEBUTTONUP:
			win_id = get_window_event_id ( event.button.windowID );
			break;

		case SDL_MOUSEWHEEL:
			win_id = get_window_event_id ( event.wheel.windowID );
			break;

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			// NOTE: CentOS 7 SDL 2.0.10 doesn't work
//			win_id = get_window_event_id ( event.tfinger.windowID );
			break;

/* NOTE: Salix 14.2 SDL 2.0.4 doesn't work
		case SDL_DROPFILE:
		case SDL_DROPTEXT:
		case SDL_DROPBEGIN:
		case SDL_DROPCOMPLETE:
			win_id = get_window_event_id ( event.drop.windowID );
			break;
*/

		case SDL_WINDOWEVENT:
			win_id = get_window_event_id ( event.window.windowID );
			break;

		default:
			// Unknown event so ignore it and leave to the app.
			break;
		}

		Window * window = get_window_from_id ( win_id );
		if ( ! window )
		{
			// No window to handle this event so send to App
			if ( m_sdl_event_cb )
			{
				m_sdl_event_cb ( event );
			}

			continue;
		}

		window->emit_signal ( GIN_SIGNAL_SDL_EVENT, &data );
	}
}

mtGin::Window * mtGin::App::get_window_from_sdl ( SDL_Window * const sdl )
{
	if ( ! sdl )
	{
		return nullptr;
	}

	auto const it = m_window_map.find ( sdl );
	if ( it == m_window_map.end() )
	{
		return nullptr;
	}

	return it->second;
}

mtGin::Window * mtGin::App::get_window_from_id ( Uint32 id )
{
	SDL_Window * const sdl =  SDL_GetWindowFromID ( id );

	return get_window_from_sdl ( sdl );
}

void mtGin::App::frame_time_delay ()
{
	m_frames++;

	Uint32 const tick = SDL_GetTicks();

	if ( tick < m_old_tick )
	{
		m_old_tick = tick;
		m_old_tick_sec = tick;
		return;		// Should only happen every 49 days!
	}

	Uint32 const fsec_gap = tick - m_old_tick_sec;

	if ( fsec_gap > 1000 )
	{
		m_frames_per_sec = m_frames - m_frames_old;

		m_old_tick_sec = tick;
		m_frames_old = 0;
		m_frames = 0;
	}

	Uint32 const gap = tick - m_old_tick;

	if ( gap < m_frame_delay )
	{
		Uint32 const delay = m_frame_delay - gap;
		SDL_Delay ( delay );
	}
	else
	{
//		SDL_Delay ( 1 );
	}

	m_old_tick = SDL_GetTicks();
}

