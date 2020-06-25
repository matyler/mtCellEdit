/*
	Copyright (C) 2020 Mark Tyler

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

#ifndef QT5_CLOUDVIEW_H
#define QT5_CLOUDVIEW_H



#include "qt5.h"
#include "qt5_gl_cloud.h"
#include "qt5_gl_ruler.h"
#include "qt5_gl_model.h"



class CloudView;



class CloudView : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	CloudView (
		mtKit::Prefs	& prefs,
		char	const	* prefs_prefix,
		CloudQtGL	& cloud_gl,
		RulerQtGL	& ruler_gl,
		ModelQtGL	& model_gl,
		QWidget		* parent
		);
	~CloudView ();

	void set_mode ( int mode );	// MODE_*
	void set_antialiasing ( bool on );

	enum
	{
		MODE_ERROR	= -1,

		MODE_MIN	= 0,

		MODE_CAMERA	= 0,
		MODE_RULER	= 1,

		MODE_MAX	= 1
	};

	void store_prefs () const;
	void restore_prefs_update ();

	void spin_180 ();
	void reset_camera ();
	void clone_camera ( CloudView const * view );

	inline void set_nudge ( double const val ) { m_view_nudge = val; }
	inline double get_nudge () const { return m_view_nudge; }
	void set_xyz_snap_nudge ();

	void set_camera ( Crul::Camera const * camera );
	void store_camera ( Crul::Camera * camera ) const;
	inline Crul::Camera const & get_camera () const { return m_camera; }

	void show_crosshair ( bool visible );
	void show_statusbar ( bool visible );
	void show_cloud ( bool visible );
	void show_model ( bool visible );

	void get_camera_matrix ( QMatrix4x4 & camera ) const;

/// ----------------------------------------------------------------------------

public slots:
	// From sliders
	void set_xrotation ( int degrees );
	void set_zrotation ( int degrees );

	// Used for accurate sub-integer changes (updates slider by blocking)
	void set_xrotation ( double degrees );
	void set_zrotation ( double degrees );

signals:
	// Emitted by slot above set_?rotation ( double )
	void xrotation_changed ( double angle ) const;
	void zrotation_changed ( double angle ) const;
	void camera_changed () const;
	void ruler_changed () const;
	void keypress_ruler ( CloudView * view, QKeyEvent * event ) const;
	void mouse_ruler (
		CloudView * view,
		QMouseEvent * event,
		int mx,
		int my
		) const;

protected:
	void initializeGL ()					override;
	void paintGL ()						override;
	void resizeGL ( int width, int height )			override;
	void mousePressEvent ( QMouseEvent * event )		override;
	void mouseMoveEvent ( QMouseEvent * event )		override;
	void keyPressEvent ( QKeyEvent * event )		override;

private:
	void restore_prefs ();

	void init_gl_perspective ();

	void keypress_camera ( QKeyEvent * event );
	void mouse_camera ( QMouseEvent * event, int mx, int my );

/// ----------------------------------------------------------------------------

	mtKit::Prefs	& m_prefs;
	char const * const m_prefs_prefix;

	QElapsedTimer	m_time;
	int		m_frames;
	double		m_fps;

	Crul::Camera	m_camera;

	double		m_point_range;
	double		m_view_nudge;

	int		m_gl_width;
	int		m_gl_height;
	int	const	m_font_height;
	int	const	m_font_descent;
	int		m_mode;

	bool		m_light_camera;
	double		m_light_x;
	double		m_light_y;
	double		m_light_z;

	bool		m_antialiasing;
	bool		m_show_crosshair;
	bool		m_show_statusbar;
	bool		m_show_cloud;
	bool		m_show_model;

	QPoint		m_last_pos;
	QMatrix4x4	m_proj;

	QColor	const	m_color_white;
	QColor	const	m_color_crosshair;
	QBrush	const	m_brush_black_semi;
	QBrush	const	m_brush_red_semi;

	CloudQtGL	& m_cloud_gl;
	RulerQtGL	& m_ruler_gl;
	ModelQtGL	& m_model_gl;
};



#endif		// QT5_CLOUDVIEW_H

