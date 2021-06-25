/*
	Copyright (C) 2020-2021 Mark Tyler

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

#include "qt5_cloudview.h"



CloudView::CloudView (
	mtKit::UserPrefs	& prefs,
	char	const * const	prefs_prefix,
	mtGin::GL::Points	& cloud_gl,
	RulerQtGL		& ruler_gl,
	mtGin::GL::Triangles	& model_gl,
	QWidget		* const	parent
	)
	:
	QOpenGLWidget	( parent ),
	m_uprefs	( prefs ),
	m_prefs_prefix	( prefs_prefix ),
	m_frames	( 0 ),
	m_fps		( 0.0 ),
	m_point_range	( 1000.0 ),
	m_view_nudge	( Crul::VIEW_NUDGE_DEFAULT ),
	m_gl_width	( 1 ),
	m_gl_height	( 1 ),
	m_font_height	( fontMetrics ().height () ),
	m_font_descent	( fontMetrics ().descent () ),
	m_mode		( MODE_CAMERA ),
	m_light_camera	( false ),
	m_light_x	( 0.0 ),
	m_light_y	( 0.0 ),
	m_light_z	( 0.0 ),
	m_antialiasing	( true ),
	m_show_crosshair ( true ),
	m_show_statusbar ( true ),
	m_show_cloud	( true ),
	m_show_model	( true ),
	m_color_white	( QColor ( 255, 255, 255, 255 ) ),
//	m_color_crosshair ( QColor ( 200, 200, 255, 255 ) ),
	m_color_crosshair ( QColor ( 255, 255, 255, 255 ) ),
	m_brush_black_semi ( QColor ( 0, 0, 0, 128 ) ),
	m_brush_red_semi ( QColor ( 128, 0, 0, 128 ) ),
	m_cloud_gl	( cloud_gl ),
	m_ruler_gl	( ruler_gl ),
	m_model_gl	( model_gl )
{
	m_camera.set_angle ( Crul::CAM_B_XROT_DEFAULT, 0,
		Crul::CAM_B_ZROT_DEFAULT );

	m_camera.set_position ( Crul::CAM_B_X_DEFAULT,
		Crul::CAM_B_Y_DEFAULT, Crul::CAM_B_Z_DEFAULT );

	setMinimumSize ( 200, 200 );

	restore_prefs ();

	// Enable antialiasing samples
	QSurfaceFormat fmt = format ();
	fmt.setSamples ( 7 );
	fmt.setSwapBehavior ( QSurfaceFormat::TripleBuffer );
	setFormat ( fmt );

	set_antialiasing ( m_antialiasing );
}

CloudView::~CloudView ()
{
	store_prefs ();
}

void CloudView::set_antialiasing ( bool const on )
{
	m_antialiasing = on;

	if ( isValid () )
	{
		// If initialized, repaint
		paintGL ();
	}
}

void CloudView::set_mode ( int const mode )
{
	switch ( mode )
	{
	case MODE_CAMERA:	break;
	case MODE_RULER:	break;

	default:
		return;
	}

	m_mode = mode;

	update ();
}

void CloudView::set_xrotation ( int const degrees )
{
	int const angle = (int)mtkit_angle_normalize ( degrees );

	if ( angle != m_camera.get_rot_x() )
	{
		m_camera.set_angle ( angle, m_camera.get_rot_y(),
			m_camera.get_rot_z() );

		emit xrotation_changed ( angle );
		emit camera_changed ();
	}
}

void CloudView::set_zrotation ( int const degrees )
{
	int const angle = (int)mtkit_angle_normalize ( degrees );

	if ( angle != m_camera.get_rot_z() )
	{
		m_camera.set_angle ( m_camera.get_rot_x(), m_camera.get_rot_y(),
			angle );

		emit zrotation_changed ( angle );
		emit camera_changed ();
	}
}

void CloudView::set_xrotation ( double const degrees )
{
	double const angle = mtkit_angle_normalize ( degrees );

	if ( angle != m_camera.get_rot_x() )
	{
		m_camera.set_angle ( angle, m_camera.get_rot_y(),
			m_camera.get_rot_z() );

		emit xrotation_changed ( angle );
		emit camera_changed ();
	}
}

void CloudView::set_zrotation ( double const degrees )
{
	double const angle = mtkit_angle_normalize ( degrees );

	if ( angle != m_camera.get_rot_z() )
	{
		m_camera.set_angle ( m_camera.get_rot_x(), m_camera.get_rot_y(),
			angle );

		emit zrotation_changed ( angle );
		emit camera_changed ();
	}
}

void CloudView::initializeGL ()
{
	initializeOpenGLFunctions ();

	glClearColor ( 0.2f, 0.2f, 0.3f, 1.0f );
	glEnable ( GL_VERTEX_PROGRAM_POINT_SIZE );
	glEnable ( GL_LINE_WIDTH );
	glDisable ( GL_CULL_FACE );

	std::string const vbuf ( mtGin::GL::get_shader_language_version() );

	m_cloud_gl.init ( vbuf );
	m_ruler_gl.init ( vbuf );
	m_model_gl.init ( vbuf );
}

void CloudView::get_camera_matrix ( mtGin::GL::Matrix4x4 & camera ) const
{
	camera = m_camera.get_matrix();
}

void CloudView::paintGL ()
{
/// Render OpenGL objects ------------------------------------------------------

	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable ( GL_DEPTH_TEST );

	glEnable ( GL_LIGHTING );
	glEnable ( GL_LIGHT0 );
	glLightModeli ( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
	glLightModelf ( GL_LIGHT_MODEL_TWO_SIDE, GLfloat(1.0) );

//	glDisable ( GL_MULTISAMPLE );	// Antialiasing

//	glEnable ( GL_BLEND );		// Transparency
//	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	mtGin::GL::Matrix4x4 const & camera = m_camera.get_matrix();
	mtGin::Vertex const light (
		m_light_camera ? m_camera.get_x() : m_light_x,
		m_light_camera ? m_camera.get_y() : m_light_y,
		m_light_camera ? m_camera.get_z() : m_light_z
		);

	try
	{
		if ( m_show_cloud )
		{
			m_cloud_gl.render ( camera, m_proj );
		}

		if ( m_show_model )
		{
			m_model_gl.render ( camera, m_proj, light );
		}

		m_ruler_gl.render ( camera, m_proj, light );
	}
	catch (...)
	{
	}

//	glDisable ( GL_BLEND );		// Transparency

	glDisable ( GL_DEPTH_TEST );

/// Timing & Qt rendering ------------------------------------------------------

	QPainter painter;
	painter.begin ( this );

	m_frames++;

	if ( m_frames >= 10 )
	{
		double const elapsed = (double)m_time.elapsed ();

		if ( 0 == elapsed )
		{
		}
		else
		{
			m_fps = 10.0 * (1000.0 / elapsed);
		}

		m_time.start ();
		m_frames = 0;
	}

	if ( m_show_statusbar )
	{
		char txt[128];
		snprintf ( txt, sizeof(txt),
			"(%.2f, %.2f, %.2f) [%.2f°, , %.2f°] %i fps",
			m_camera.get_x(), m_camera.get_y(), m_camera.get_z(),
			m_camera.get_rot_x(), m_camera.get_rot_z(),
			(int)round (m_fps) );

		QBrush const & brush = (m_mode == MODE_RULER) ?
			m_brush_red_semi : m_brush_black_semi;

		painter.fillRect ( 0, 0, m_gl_width, m_font_height, brush );
		painter.setPen ( m_color_white );
		painter.drawText ( 5, m_font_height - m_font_descent, txt );
	}

	if ( m_show_crosshair )
	{
		painter.setPen ( m_color_crosshair );
		painter.drawLine ( m_gl_width / 2, 0, m_gl_width / 2,
			m_gl_height );
		painter.drawLine ( 0, m_gl_height / 2, m_gl_width,
			m_gl_height / 2 );
	}

	if ( m_antialiasing )
	{
		// Needed to smooth edges of OpenGL items but not Qt lines above
		painter.setRenderHint ( QPainter::Antialiasing );
	}

	painter.end ();
}

void CloudView::resizeGL ( int const width, int const height )
{
	m_gl_width = width;
	m_gl_height = height;

	init_gl_perspective ();
}

void CloudView::restore_prefs_update ()
{
	restore_prefs ();

	emit camera_changed ();
}

void CloudView::restore_prefs ()
{
	std::string const prefix ( m_prefs_prefix );

	m_point_range = m_uprefs.get_double ( PREFS_GL_POINT_RANGE );

	m_light_camera = m_uprefs.get_int ( PREFS_GL_LIGHT_CAMERA );
	m_light_x = m_uprefs.get_double ( PREFS_GL_LIGHT_X );
	m_light_y = m_uprefs.get_double ( PREFS_GL_LIGHT_Y );
	m_light_z = m_uprefs.get_double ( PREFS_GL_LIGHT_Z );

	m_camera.set_position (
		m_uprefs.get_double ( (prefix + PREFS_CAM_X).c_str()),
		m_uprefs.get_double ( (prefix + PREFS_CAM_Y).c_str()),
		m_uprefs.get_double ( (prefix + PREFS_CAM_Z).c_str()) );

	m_camera.set_angle (
		m_uprefs.get_double ( (prefix + PREFS_CAM_XROT). c_str() ),
		0,
		m_uprefs.get_double ( (prefix + PREFS_CAM_ZROT). c_str() ) );

	init_gl_perspective ();
}

void CloudView::store_prefs () const
{
	std::string const prefix ( m_prefs_prefix );

	m_uprefs.set ( (prefix + PREFS_CAM_X).c_str(), m_camera.get_x() );
	m_uprefs.set ( (prefix + PREFS_CAM_Y).c_str(), m_camera.get_y() );
	m_uprefs.set ( (prefix + PREFS_CAM_Z).c_str(), m_camera.get_z() );

	m_uprefs.set ( (prefix + PREFS_CAM_XROT).c_str(), m_camera.get_rot_x());
	m_uprefs.set ( (prefix + PREFS_CAM_ZROT).c_str(), m_camera.get_rot_z());
}

void CloudView::init_gl_perspective ()
{
	m_proj.set_identity ();
	m_proj.perspective ( 45.0f, float(m_gl_width) / float(m_gl_height),
		0.01f, float(m_point_range) );
}

void CloudView::spin_180 ()
{
	m_camera.turn_around ();

	emit xrotation_changed ( m_camera.get_rot_x() );
	emit zrotation_changed ( m_camera.get_rot_z() );
	emit camera_changed ();
}

void CloudView::reset_camera ()
{
	m_camera.set_angle ( Crul::CAM_B_XROT_DEFAULT, 0,
		Crul::CAM_B_ZROT_DEFAULT );

	m_camera.set_position ( Crul::CAM_B_X_DEFAULT, Crul::CAM_B_Y_DEFAULT,
		Crul::CAM_B_Z_DEFAULT );

	emit xrotation_changed ( m_camera.get_rot_x() );
	emit zrotation_changed ( m_camera.get_rot_z() );
	emit camera_changed ();
}

void CloudView::clone_camera ( CloudView const * view )
{
	m_camera = view->m_camera;

	emit xrotation_changed ( m_camera.get_rot_x() );
	emit zrotation_changed ( m_camera.get_rot_z() );
	emit camera_changed ();
}

void CloudView::set_xyz_snap_nudge ()
{
	double const x = m_camera.get_x();
	double const y = m_camera.get_y();
	double const z = m_camera.get_z();

	m_camera.set_position (
		x - fmod ( x, m_view_nudge ),
		y - fmod ( y, m_view_nudge ),
		z - fmod ( z, m_view_nudge ) );

	emit camera_changed ();
}

void CloudView::set_camera ( Crul::Camera const * const camera )
{
	if ( ! camera )
	{
		return;
	}

	m_camera = *camera;

	emit xrotation_changed ( m_camera.get_rot_x() );
	emit zrotation_changed ( m_camera.get_rot_z() );
	emit camera_changed ();
}

void CloudView::store_camera ( Crul::Camera * const camera ) const
{
	if ( ! camera || camera->get_read_only () )
	{
		return;
	}

	camera->set_angle ( m_camera.get_rot_x (), m_camera.get_rot_y (),
		m_camera.get_rot_z () );

	camera->set_position ( m_camera.get_x(), m_camera.get_y(),
		m_camera.get_z() );
}

void CloudView::show_crosshair ( bool const visible )
{
	m_show_crosshair = visible;
	update ();
}

void CloudView::show_statusbar ( bool const visible )
{
	m_show_statusbar = visible;
	update ();
}

void CloudView::show_cloud ( bool const visible )
{
	m_show_cloud = visible;
	update ();
}

void CloudView::show_model ( bool const visible )
{
	m_show_model = visible;
	update ();
}

