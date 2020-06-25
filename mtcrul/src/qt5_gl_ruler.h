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

#ifndef QT5_GL_RULER_H
#define QT5_GL_RULER_H



#include "qt5_gl_vertex.h"



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
		QMatrix4x4 const & camera,
		QMatrix4x4 const & proj,
		QVector3D const & light
		);

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

	VertexQtGL	m_triangles;
	VertexQtGL	m_lines;
};



#endif		// QT5_GL_RULER_H

