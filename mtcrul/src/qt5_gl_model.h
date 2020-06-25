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

#ifndef QT5_GL_MODEL_H
#define QT5_GL_MODEL_H



#include "qt5_gl_vertex.h"



class ModelQtGL
{
public:
	ModelQtGL ();
	~ModelQtGL ();

	void init ( std::string const & version );
	void destroy ();

	void populate ( std::vector<Crul::VertexGL> const & triangles );

	void render (
		QMatrix4x4 const & camera,
		QMatrix4x4 const & proj,
		QVector3D const & light
		);

private:
	VertexQtGL	m_triangles;
};



#endif		// QT5_GL_MODEL_H

