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

#include "mandy_main.h"



Mainwindow::Mainwindow ( CommandLine & cline )
	:
	m_window_width	( cline.get_output_width () ),
	m_window_height	( cline.get_output_height () ),
	m_cline		( cline ),
	m_gin		( SDL_INIT_VIDEO ),
	m_window	( GIN_WINDOW_CANVAS_2D )
{
	m_window.init ( APP_NAME " - Mandelbrot set explorer",
		cline.mprefs.window_x, cline.mprefs.window_y,
		m_window_width, m_window_height, 0 );

	m_gin.window_add ( m_window );

	m_window.set_size_min ( m_window_width, m_window_height );

	m_window.add_signal ( GIN_SIGNAL_SDL_EVENT, [this]
	(mtGin::CB_Data const * const data)
	{
		auto const d = dynamic_cast<mtGin::CB_SDLEvent const *>(data);

		if ( ! d || ! d->sdl_event )
		{
			return 1;	// Not actioned
		}

		return window_event ( d->sdl_event );
	} );

	m_window.add_signal( GIN_SIGNAL_DESTROY_WINDOW, [this] (void *)
	{
		window_close ();
		return 0;
	} );

	m_window.m_render.set ( [this]()
	{
		window_render ();
	} );
	m_window.m_render.start ();

	m_gin.frame_callback.set ( [this]()
	{
		frame_cycle ();
	} );
	m_gin.frame_callback.start ();

	m_gin.set_sdl_event_callback ( []( SDL_Event const & event )
	{
		switch ( event.type )
		{
		case SDL_QUIT:
			// Ignore this here. window_event deals with closures.
			break;
		}
	} );

	if ( m_mandy.set_pixmap_size ( m_window_width, m_window_height ) )
	{
		return;
	}

	m_mandy.set_depth_max ( cline.get_depth_max() );
	m_mandy.set_verbose ( cline.get_verbose() );
	m_mandy.set_threads ( cline.get_thread_total() );
	m_mandy.set_deep_zoom_type ( cline.get_deep_zoom_type() );
	m_mandy.set_deep_zoom_on ( cline.get_deep_zoom_on() );

	centre_frame ();
	set_mode ( MODE_INTERACTIVE_FRAMING );

	try
	{
		m_mandy.zoom_cxyrange (
			cline.get_axis_cx_st(),
			cline.get_axis_cy_st(),
			cline.get_axis_range_st()
			);
	}
	catch (...)
	{
		std::cerr << "Bad arguments:\n"
			" axis_cx_st = " << cline.get_axis_cx_st() << "\n"
			" axis_cy_st = " << cline.get_axis_cy_st() << "\n"
			" axis_range_st = " << cline.get_axis_range_st() << "\n";
	}

	start_rendering ( ZOOM_NONE );

	m_gin.main_loop ();
}

Mainwindow::~Mainwindow ()
{
	SDL_DestroyTexture ( m_texture );
	m_texture = nullptr;
}

void Mainwindow::window_render ()
{
	if ( MODE_INTERACTIVE_RENDERING == m_mode )
	{
		prepare_surface ();
	}

	auto const renderer = m_window.renderer();

	if ( m_texture )
	{
		SDL_RenderCopy ( renderer, m_texture, nullptr, nullptr );
	}
	else
	{
		Uint8 r=0, g=0, b=0;

		SDL_SetRenderDrawColor ( renderer, r, g, b, SDL_ALPHA_OPAQUE );
		SDL_RenderClear ( renderer );
	}

	if ( MODE_INTERACTIVE_FRAMING == m_mode )
	{
		SDL_SetRenderDrawColor ( renderer, 255, 255, 255,
			SDL_ALPHA_OPAQUE );

		SDL_RenderDrawRect ( renderer, &m_frame );

		SDL_RenderDrawLine ( renderer, m_frame.x, m_frame.y,
			m_frame.x + m_frame.w - 1, m_frame.y + m_frame.h - 1 );

		SDL_RenderDrawLine ( renderer,
			m_frame.x + m_frame.w - 1, m_frame.y,
			m_frame.x, m_frame.y + m_frame.h - 1 );
	}

	SDL_RenderPresent ( renderer );
}

int Mainwindow::window_event ( SDL_Event const * const event )
{
	switch ( event->type )
	{
	case SDL_MOUSEBUTTONDOWN:
		mouse_click ();
		break;

	case SDL_MOUSEMOTION:
		mouse_click ();
		break;

	case SDL_KEYDOWN:
		key_click ( event );
		break;

	case SDL_WINDOWEVENT:
		switch ( event->window.event )
		{
		case SDL_WINDOWEVENT_CLOSE:
			quit ();
			break;
		}

		break;
	}

	return 0;		// Actioned
}

void Mainwindow::window_close ()
{
	int x, y, w, h;
	Uint32 flags;
	m_window.get_geometry ( x, y, w, h, flags );

	m_cline.uprefs.set ( PREFS_WINDOW_X, x );
	m_cline.uprefs.set ( PREFS_WINDOW_Y, y );
}

void Mainwindow::frame_cycle ()
{
	if ( MODE_INTERACTIVE_RENDERING == m_mode )
	{
		if ( STATUS_IDLE == m_mandy.get_status() )
		{
			set_mode ( MODE_INTERACTIVE_FRAMING );
		}
	}
	else if ( MODE_INTERACTIVE_FRAMING == m_mode )
	{
		if ( m_quit )
		{
			m_gin.stop_main_loop ();
		}
	}
}

void Mainwindow::mouse_click ()
{
	if ( MODE_INTERACTIVE_FRAMING != m_mode )
	{
		return;
	}

	int x, y;
	Uint32 const button = SDL_GetMouseState ( &x, &y );

	if ( button & SDL_BUTTON_LMASK )
	{
		set_frame ( x, y );
	}
	else if ( button & SDL_BUTTON_RMASK )
	{
		start_rendering ( ZOOM_IN );
	}
	else
	{
		return;		// Nothing to do
	}
}

void Mainwindow::key_click ( SDL_Event const * const event )
{
	if ( MODE_INTERACTIVE_RENDERING == m_mode )
	{
		switch ( event->key.keysym.sym )
		{
		case SDLK_ESCAPE:
		case SDLK_DELETE:
			std::cerr << "Mainwindow::key_click -"
				" build_terminate ()\n";
			m_mandy.build_terminate ();
			break;
		}
	}
	else if ( MODE_INTERACTIVE_FRAMING == m_mode )
	{
		int dx=0, dy=0;
		int const shift =
			(event->key.keysym.mod & KMOD_LSHIFT) ? 16 :
			(event->key.keysym.mod & KMOD_RSHIFT) ? 16 : 1;
		int const ctrl =
			(event->key.keysym.mod & KMOD_LCTRL) ? 1 :
			(event->key.keysym.mod & KMOD_RCTRL) ? 1 : 0;

		switch ( event->key.keysym.sym )
		{
		case SDLK_q:
			quit ();
			break;

		case SDLK_MINUS:
		case SDLK_KP_MINUS:
		case SDLK_PAGEDOWN:
			start_rendering ( ZOOM_OUT );
			break;

		case SDLK_PLUS:
		case SDLK_KP_PLUS:
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
		case SDLK_PAGEUP:
			start_rendering ( ZOOM_IN );
			break;

		case SDLK_HOME:
			start_rendering ( ZOOM_HOME );
			break;

		case SDLK_END:
			start_rendering ( ZOOM_NONE );
			break;

		case SDLK_s:
			output_image_file ();
			break;

		case SDLK_F1:
			output_position ();
			break;

		case SDLK_LEFT:
			if ( ctrl )
			{
				start_rendering ( ZOOM_LEFT );
			}
			else
			{
				dx = -shift;
			}
			break;

		case SDLK_RIGHT:
			if ( ctrl )
			{
				start_rendering ( ZOOM_RIGHT );
			}
			else
			{
				dx = shift;
			}
			break;

		case SDLK_UP:
			if ( ctrl )
			{
				start_rendering ( ZOOM_UP );
			}
			else
			{
				dy = -shift;
			}
			break;

		case SDLK_DOWN:
			if ( ctrl )
			{
				start_rendering ( ZOOM_DOWN );
			}
			else
			{
				dy = shift;
			}
			break;
		}

		if ( dx || dy )
		{
			int const min_x = -(m_window_width/4);
			int const max_x = min_x + m_window_width;

			m_frame.x = mtkit_int_bound ( m_frame.x + dx, min_x,
				max_x );

			int const min_y = -(m_window_height/4);
			int const max_y = min_y + m_window_height;

			m_frame.y = mtkit_int_bound ( m_frame.y + dy, min_y,
				max_y );
		}
	}
}

void Mainwindow::centre_frame ()
{
	set_frame ( m_window_width / 2, m_window_height / 2 );
}

void Mainwindow::set_frame ( int const x, int const y )
{
	m_frame.x = x - m_window_width/4;
	m_frame.y = y - m_window_height/4;
	m_frame.w = m_window_width/2;
	m_frame.h = m_window_height/2;
}

void Mainwindow::set_mode ( int const mode )
{
	prepare_surface ();

	m_mode = mode;
}

void Mainwindow::quit ()
{
	m_mandy.build_terminate ();
	m_quit = 1;
}

void Mainwindow::prepare_surface ()
{
	SDL_Surface * surface = mtGin::surface_from_pixmap ( m_mandy.
		get_pixmap () );

	if ( ! surface )
	{
		std::cerr << "Unable to create SDL surface from Mandelbrot\n";
		return;
	}

	SDL_Texture * texture = SDL_CreateTextureFromSurface (
		m_window.renderer (), surface );

	SDL_FreeSurface ( surface );
	surface = nullptr;

	if ( ! texture )
	{
		std::cerr << "Unable to create SDL texture from Mandelbrot\n";
		return;
	}

	SDL_DestroyTexture ( m_texture );

	m_texture = texture;
}

void Mainwindow::start_rendering ( int const zoom )
{
	if ( MODE_INTERACTIVE_RENDERING == m_mode || m_quit )
	{
		return;
	}

	int const x = m_frame.x + m_frame.w / 2;
	int const y = m_frame.y + m_frame.h / 2;

	switch ( zoom )
	{
	case ZOOM_OUT:
		if ( m_mandy.zoom_out ( x, y ) )
		{
			return;
		}
		break;

	case ZOOM_IN:
		if ( m_mandy.zoom_in ( x, y ) )
		{
			return;
		}
		break;

	case ZOOM_HOME:
		if ( m_mandy.zoom_reset () )
		{
			return;
		}
		break;

	case ZOOM_LEFT:
		m_mandy.zoom_left ();
		break;

	case ZOOM_RIGHT:
		m_mandy.zoom_right ();
		break;

	case ZOOM_UP:
		m_mandy.zoom_up ();
		break;

	case ZOOM_DOWN:
		m_mandy.zoom_down ();
		break;
	}

	set_mode ( MODE_INTERACTIVE_RENDERING );

	m_mandy.build_mandelbrot_set ();

	centre_frame ();
}

void Mainwindow::output_position () const
{
	m_mandy.info_to_stdout ();
}


namespace {
static void replace_text (
	std::string		& src,
	std::string	const	& from,
	std::string	const	& to
	)
{
	size_t const pos = src.find ( from );
	if ( pos != std::string::npos )
	{
		src.replace ( pos, from.length(), to );
	}
}
} // namespace


void Mainwindow::output_image_file () const
{
	std::string x ( m_mandy.get_cx().to_string() );
	std::string y ( m_mandy.get_cy().to_string() );
	std::string r ( m_mandy.get_range_w().to_string() );

	replace_text ( x, " / ", "~" );
	replace_text ( y, " / ", "~" );
	replace_text ( r, " / ", "~" );

	std::string filename ( mtkit_file_home () );

	filename += MTKIT_DIR_SEP;
	filename += BIN_NAME;
	filename += '_';
	filename += x;
	filename += '_';
	filename += y;
	filename += '_';
	filename += r;
	filename += ".png";

	export_image ( filename );
}

int Mainwindow::export_image ( std::string const & filename ) const
{
	if ( m_cline.get_verbose() )
	{
		std::cout << filename << "\n";
	}

#if 1==0
	// Include all window pixels including zoom area
	mtPixy::Pixmap pixmap ( m_window.dump_to_pixmap() );
	mtPixmap const * pm = pixmap.get();
#else
	mtPixmap const * pm = m_mandy.get_pixmap();
#endif

	if ( ! pm )
	{
		std::cerr << "export_image: No pixmap\n";
		return 1;
	}

	if ( pixy_pixmap_save_png ( pm, filename.c_str(), 5 ) )
	{
		std::cerr << "export_image: Unable to save pixmap\n";
		return 1;
	}

	return 0;
}

