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

#include "lanter.h"



Mainwindow::Mainwindow ( Core & core )
	:
	m_core		( core ),
	m_animate	( *this, core ),
	m_gin		( SDL_INIT_VIDEO ),
	m_window	( GIN_WINDOW_CANVAS_OPENGL )
{
	int const w = core.get_output_width();
	int const h = core.get_output_height();

	if (	w >= OUTPUT_WH_MIN	&& w <= OUTPUT_WH_MAX
		&& h >= OUTPUT_WH_MIN	&& h <= OUTPUT_WH_MAX
		)
	{
		m_gl_width = w;
		m_gl_height = h;
		m_store_wh = 0;
	}
	else
	{
		m_gl_width = core.mprefs.window_w;
		m_gl_height = core.mprefs.window_h;
		m_store_wh = 1;
	}

	// Antialiase edges if glEnable(GL_MULTISAMPLE) is used during render
	SDL_GL_SetAttribute ( SDL_GL_MULTISAMPLESAMPLES, 8 );

//	SDL_CaptureMouse ( SDL_TRUE );

	Uint32 const flags = m_store_wh ?
		(core.mprefs.window_maximized ? SDL_WINDOW_MAXIMIZED : 0)
			| SDL_WINDOW_RESIZABLE : 0;

	m_window.init ( "mtLanter - Landscape Terraformer",
		core.mprefs.window_x, core.mprefs.window_y,
		m_gl_width, m_gl_height, flags );

	m_terragl.set_window_size( m_gl_width, m_gl_height, m_gl_range );

	m_gin.window_add ( m_window );

	m_window.set_size_min ( 100, 100 );

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

	m_window.add_signal( GIN_SIGNAL_DESTROY_WINDOW, [this]
	(void *)
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

	m_terramap.set_seed ( (uint64_t)core.get_map_seed () );

	if ( init_map () )
	{
		return;
	}

//	m_gin.set_fps_max ( 30 );

	init_gl ();
	move_camera_home (0);

	if ( 0 == m_animate.init () )
	{
		set_mode ( MODE_ANIMATION );
	}
	else
	{
		set_mode ( MODE_INTERACTIVE );
	}

	m_gin.main_loop ();
}

Mainwindow::~Mainwindow ()
{
}

void Mainwindow::move_camera_home ( int const top )
{
	if ( top )
	{
		m_camera.set_position ( 0, 0, -15 );
		m_camera.set_angle ( 0, 0, 0 );
	}
	else
	{
		m_camera.set_position ( 1180, -1600, 910 );
//		m_camera.set_position ( -327, -207, 160 );

//		m_camera.set_position ( -1100, 1900, -700 );
//		m_camera.set_position ( 1100, 1900, -700 );
//		m_camera.set_position ( 1100, -1900, -700 );
//		m_camera.set_position ( -1100, -1900, -700 );

//		m_camera.set_angle ( 245, 0, 325 );
//		m_camera.set_angle ( 252, 0, 325 );

		m_camera.look_at ( {0,0,0} );
//		m_camera.look_at ( {0, -100, 1000} );

/*
		m_camera.look_at ( {LANDSCAPE_VIEW_RANGE * 0.5,
			LANDSCAPE_VIEW_RANGE * 0.5,
			LANDSCAPE_VIEW_RANGE * 0.7} );
//*/
	}
}

int Mainwindow::init_map ()
{
	if ( m_terramap.create_landscape ( m_core.get_map_size () ) )
	{
		return 1;
	}

	char	const * const	filename = m_core.get_save_map ();

	if ( filename )
	{
		if ( m_terramap.save_tsv ( filename ) )
		{
			return 1;
		}
	}

	return 0;
}

void Mainwindow::init_gl ()
{
	m_window.set_opengl_current ();

	glEnable ( GL_VERTEX_PROGRAM_POINT_SIZE );
	glEnable ( GL_LINE_WIDTH );
	glDisable ( GL_CULL_FACE );

	m_terragl.init_buffers ( m_terramap );
	m_terragl.set_light ( LANDSCAPE_VIEW_RANGE * 0.5f,
		LANDSCAPE_VIEW_RANGE * 0.5f,
		LANDSCAPE_VIEW_RANGE * 0.7f );

	init_gl_perspective ();
}

void Mainwindow::init_gl_perspective ()
{
	m_window.set_opengl_current ();
	glViewport ( 0, 0, m_gl_width, m_gl_height );

	double const fovY = 45.0;
	double const aspect = (double)m_gl_width / (double)m_gl_height;

	m_zNear = 0.1;
//	m_fH = tan( (fovY / 2) / 180 * M_PI ) * m_zNear;
	m_fH = tan( fovY / 360 * M_PI ) * m_zNear;
	m_fW = m_fH * aspect;

	glMatrixMode ( GL_MODELVIEW );
}

void Mainwindow::set_gl_perspective_flat ()
{
	glMatrixMode ( GL_PROJECTION );
	glLoadIdentity ();

	glMatrixMode ( GL_MODELVIEW );
}

void Mainwindow::set_gl_perspective_3d () const
{
	glMatrixMode ( GL_PROJECTION );
	glLoadIdentity ();
	glFrustum ( -m_fW, m_fW, -m_fH, m_fH, m_zNear, m_gl_range );

	glMatrixMode ( GL_MODELVIEW );
}

void Mainwindow::window_render () const
{
	set_gl_perspective_3d ();

	glEnable ( GL_MULTISAMPLE );	// Antialiasing

	mtGin::GL::Matrix4x4 const & camera = m_camera.get_matrix();
	mtGin::GL::Array4x4_float const & camera_matrix = camera.data();

	glLoadIdentity ();
	glLoadMatrixf ( &camera_matrix[0][0] );

	glClearColor ( 0.2f, 0.2f, 0.3f, 1.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable ( GL_DEPTH_TEST );

	// Disable lighting for these lines & shapes
	glDisable ( GL_LIGHTING );
	glDisable ( GL_LIGHT0 );

	m_terragl.render_light ();
	m_terragl.render_axis ();

	// Model ---------------------------------------------------------------

//	glDisable ( GL_MULTISAMPLE );	// Antialiasing

	// Allow lighting for the landscape
	glEnable ( GL_LIGHTING );
	glEnable ( GL_LIGHT0 );
	glLightModeli ( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
	glLightModelf ( GL_LIGHT_MODEL_TWO_SIDE, GLfloat(1.0) );

	m_terragl.render ( camera );

	// Top level stuff -----------------------------------------------------
	set_gl_perspective_flat();
	glLoadIdentity ();

	glDisable ( GL_DEPTH_TEST );
	glDisable ( GL_MULTISAMPLE );	// Antialiasing
	glDisable ( GL_LIGHTING );
	glDisable ( GL_LIGHT0 );

	glColor3f ( 1.0f, 1.0f, 1.0f );
	glBegin ( GL_LINES );
	glVertex3f (-1.0f, 0.0f, 0.0f );
	glVertex3f ( 1.0f, 0.0f, 0.0f );
	glEnd ();
	glBegin ( GL_LINES );
	glVertex3f ( 0.0f,-1.0f, 0.0f );
	glVertex3f ( 0.0f, 1.0f, 0.0f );
	glEnd ();

	SDL_GL_SwapWindow ( m_window.sdl() );

	m_animate.save_frame ( m_anim_frame );
}

int Mainwindow::window_event ( SDL_Event const * const event )
{
	switch ( event->type )
	{
	case SDL_MOUSEBUTTONDOWN:
		mouse_click ( 0, 0 );
		break;

	case SDL_MOUSEMOTION:
		mouse_click ( event->motion.xrel, event->motion.yrel );
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

		case SDL_WINDOWEVENT_RESIZED:
			m_gl_width = event->window.data1;
			m_gl_height = event->window.data2;
			m_terragl.set_window_size ( m_gl_width, m_gl_height,
				m_gl_range );
			init_gl_perspective ();
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

	int const maximized = (flags & SDL_WINDOW_MAXIMIZED) ? 1 : 0;

	m_core.uprefs.set ( PREFS_WINDOW_MAXIMIZED, maximized );

	if ( 0 == maximized )
	{
		m_core.uprefs.set ( PREFS_WINDOW_X, x );
		m_core.uprefs.set ( PREFS_WINDOW_Y, y );

		if ( m_store_wh )
		{
			m_core.uprefs.set ( PREFS_WINDOW_W, w );
			m_core.uprefs.set ( PREFS_WINDOW_H, h );
		}
	}
}

void Mainwindow::frame_cycle ()
{
	if ( MODE_ANIMATION == m_mode )
	{
		if ( ! m_quit )
		{
			m_anim_frame++;

			if ( m_animate.prepare_frame ( m_anim_frame ) )
			{
				quit();
			}
			else
			{
				m_camera.set_position(m_animate.get_position());
				m_camera.look_at ( m_animate.get_focus() );
			}
		}

		if ( m_quit )
		{
			m_gin.stop_main_loop ();
		}
	}
	else if ( MODE_INTERACTIVE == m_mode )
	{
		if ( m_quit )
		{
			m_gin.stop_main_loop ();
		}
	}
}

void Mainwindow::mouse_click (
	int	const	dx,
	int	const	dy
	)
{
	if ( MODE_ANIMATION == m_mode )
	{
		return;
	}

	SDL_Keymod const mod = SDL_GetModState ();
	int const scale =
		(mod & KMOD_LSHIFT) ? 10 :
		(mod & KMOD_RSHIFT) ? 10 : 1;

	int ax, ay;
	Uint32 const button = SDL_GetMouseState ( &ax, &ay );

	if ( button & SDL_BUTTON_LMASK )
	{
		double const nudge = m_camera.get_nudge() * scale;

		m_camera.move ( -dx * nudge, dy * nudge, 0 );
	}
	else if ( button & SDL_BUTTON_RMASK )
	{
		double const nudge = scale / 8.0 / 4.0;

		m_camera.turn ( dy * nudge, 0, -dx * nudge );
	}
	else
	{
		return;		// Nothing to do
	}
}

void Mainwindow::key_click ( SDL_Event const * const event )
{
	if ( MODE_ANIMATION == m_mode )
	{
		switch ( event->key.keysym.sym )
		{
		case SDLK_ESCAPE:
		case SDLK_DELETE:
			quit ();
			break;
		}
	}
	else if ( MODE_INTERACTIVE == m_mode )
	{
		int const scale =
			(event->key.keysym.mod & KMOD_LSHIFT) ? 10 :
			(event->key.keysym.mod & KMOD_RSHIFT) ? 10 : 1;

		int const ctrl =
			(event->key.keysym.mod & KMOD_LCTRL) ? 1 :
			(event->key.keysym.mod & KMOD_RCTRL) ? 1 : 0;

		if ( ctrl )
		{
			double const nudge = (double)scale / 8.0;

			switch ( event->key.keysym.sym )
			{
			case SDLK_UP:
				m_camera.turn ( nudge, 0, 0 );
				break;

			case SDLK_DOWN:
				m_camera.turn ( -nudge, 0, 0 );
				break;

			case SDLK_LEFT:
				m_camera.turn ( 0, 0, -nudge );
				break;

			case SDLK_RIGHT:
				m_camera.turn ( 0, 0, nudge );
				break;

			case SDLK_HOME:
				move_camera_home ( ctrl );
				return;
			}

			return;
		}

		double const nudge = m_camera.get_nudge() * scale;

		switch ( event->key.keysym.sym )
		{
		case SDLK_PAGEUP:
			m_camera.move ( 0, 0, nudge );
			break;

		case SDLK_PAGEDOWN:
			m_camera.move ( 0, 0, -nudge );
			break;

		case SDLK_UP:
			m_camera.move ( 0, nudge, 0 );
			break;

		case SDLK_DOWN:
			m_camera.move ( 0, -nudge, 0 );
			break;

		case SDLK_LEFT:
			m_camera.move ( -nudge, 0, 0 );
			break;

		case SDLK_RIGHT:
			m_camera.move ( nudge, 0, 0 );
			break;

		case SDLK_HOME:
			move_camera_home ( ctrl );
			return;

		case SDLK_END:
			m_camera.turn_around ();
			return;

		case SDLK_ESCAPE:
			quit ();
			break;

		case SDLK_F1:
			printf ( "x=%.15g y=%.15g z=%.15g "
				"xrot=%.15g yrot=%.15g zrot=%.15g\n",
				m_camera.get_x(), m_camera.get_y(),
				m_camera.get_z(),
				m_camera.get_rot_x(), m_camera.get_rot_y(),
				m_camera.get_rot_z() );
			return;

		case SDLK_s:
			output_image_file ();
			break;

		}
	}
}

void Mainwindow::set_mode ( int const mode )
{
	m_mode = mode;
}

void Mainwindow::quit ()
{
	m_quit = 1;
}

void Mainwindow::output_image_file ()
{
	char buf[256];
	snprintf ( buf, sizeof(buf), BIN_NAME "_%.15g_%.15g_%.15g.png",
		m_camera.get_x(),
		m_camera.get_y(),
		m_camera.get_z() );

	std::string filename ( mtkit_file_home () );

	filename += MTKIT_DIR_SEP;
	filename += buf;

	export_image ( filename );
}

int Mainwindow::export_image ( std::string const & filename )
{
	if ( m_core.get_verbose() )
	{
		std::cout << filename << "\n";
	}

	mtPixy::Pixmap pm ( m_window.dump_to_pixmap () );

	if ( pixy_pixmap_save_png ( pm.get(), filename.c_str(), 5 ) )
	{
		std::cerr << "export_image: Unable to save pixmap\n";
		return 1;
	}

	return 0;
}

