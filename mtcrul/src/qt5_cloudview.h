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

#ifndef QT5_CLOUDVIEW_H
#define QT5_CLOUDVIEW_H



#include "qt5.h"



class CloudView;
class RulerQtGL;



class CloudView : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	CloudView (
		mtKit::UserPrefs & prefs,
		char	const	* prefs_prefix,
		mtGin::GL::Points & cloud_gl,
		RulerQtGL	& ruler_gl,
		mtGin::GL::Triangles & model_gl,
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

	void get_camera_matrix ( mtGin::GL::Matrix4x4 & camera ) const;

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

	mtKit::UserPrefs	& m_uprefs;
	char	const * const	m_prefs_prefix;

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
	mtGin::GL::Matrix4x4 m_proj;

	QColor	const	m_color_white;
	QColor	const	m_color_crosshair;
	QBrush	const	m_brush_black_semi;
	QBrush	const	m_brush_red_semi;

	mtGin::GL::Points	& m_cloud_gl;
	RulerQtGL		& m_ruler_gl;
	mtGin::GL::Triangles	& m_model_gl;
};



class RulerQtGL
{
public:
	RulerQtGL ();
	~RulerQtGL ();

	void init ( std::string const & version );
	void destroy ();

	void populate (
		std::map<int, Crul::Ruler> const & map,
		double plane_range
		);

	void render (
		mtGin::GL::Matrix4x4 const & camera,
		mtGin::GL::Matrix4x4 const & proj,
		mtGin::Vertex const & light
		) const;

	inline void set_active_ruler_id ( int const i) { m_active_ruler_id = i;}
	inline void set_show_lines ( bool const s ) { m_show_lines = s; }
	inline void set_show_plane ( bool const s ) { m_show_plane = s; }
	inline void set_line_butt_size ( double const s ) { m_line_butt_size=s;}
	inline void set_line_thickness ( double const s ) { m_line_thickness=s;}

private:
	inline bool is_visible() const { return (m_show_lines || m_show_plane);}

/// ----------------------------------------------------------------------------

	int		m_active_ruler_id;

	bool		m_show_lines;
	bool		m_show_plane;

	double		m_line_butt_size;
	double		m_line_thickness;

	mtGin::GL::Triangles	m_triangles;
	mtGin::GL::Lines	m_lines;
};



#endif		// QT5_CLOUDVIEW_H

