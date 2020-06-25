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

#ifndef QT5_GL_CLOUD_H
#define QT5_GL_CLOUD_H



#include "qt5.h"



class CloudQtGL
{
public:
	CloudQtGL ();
	~CloudQtGL ();

	void init ( std::string const & version );
	void destroy ();

	inline void set_point_size ( double const s ) { m_point_size = s; }
	inline double get_point_size () const { return m_point_size; }

	void populate ( Crul::Cloud const * cloud );
	void render (
		QMatrix4x4 const & camera,
		QMatrix4x4 const & proj
		);

private:
	int		m_projMatrixLoc;
	int		m_camMatrixLoc;
	int		m_ptSizeLoc;
	QOpenGLShaderProgram * m_program;

	double		m_point_size;

	QOpenGLBuffer	m_vbo;
	size_t		m_vertex_tot;
};



#endif		// QT5_GL_CLOUD_H

