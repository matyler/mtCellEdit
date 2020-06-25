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

#include "qt5_gl_cloud.h"



CloudQtGL::CloudQtGL ()
	:
	m_projMatrixLoc		( 0 ),
	m_camMatrixLoc		( 0 ),
	m_ptSizeLoc		( 0 ),
	m_program		( NULL ),
	m_point_size		( 3.0 ),
	m_vertex_tot		( 0 )
{
}

CloudQtGL::~CloudQtGL ()
{
	destroy ();
}

void CloudQtGL::destroy ()
{
	m_vbo.destroy ();
	m_vertex_tot = 0;

	delete m_program;
	m_program = NULL;
}

void CloudQtGL::init ( std::string const & version )
{
	destroy ();

	m_program = new QOpenGLShaderProgram;

	static char const * const vert_shader_src =
		"in vec4 vertex;\n"
		"in vec4 vertexRGB;\n"
		"out vec4 vertRGB;\n"
		"uniform mat4 projMatrix;\n"
		"uniform mat4 camMatrix;\n"
		"uniform float ptSize;\n"
		"void main() {\n"
		"   vertRGB = vertexRGB;\n"
		"   gl_Position = projMatrix * camMatrix * vertex;\n"
		"   gl_PointSize = ptSize;\n"
		"}\n";

	static char const * const frag_shader_src =
		"in highp vec4 vertRGB;\n"
		"out highp vec4 fragColor;\n"
		"void main() {\n"
		"   fragColor = vertRGB;\n"
		"}\n";

	m_program->addShaderFromSourceCode ( QOpenGLShader::Vertex,
		(version + vert_shader_src).c_str () );

	m_program->addShaderFromSourceCode ( QOpenGLShader::Fragment,
		(version + frag_shader_src).c_str () );

	m_program->bindAttributeLocation ( "vertex", 0 );
	m_program->bindAttributeLocation ( "vertexRGB", 1 );
	m_program->link ();

	m_projMatrixLoc	= m_program->uniformLocation ( "projMatrix" );
	m_camMatrixLoc	= m_program->uniformLocation ( "camMatrix" );
	m_ptSizeLoc	= m_program->uniformLocation ( "ptSize" );
}

void CloudQtGL::populate ( Crul::Cloud const * const cloud )
{
	// Setup our vertex buffer object.
	m_vertex_tot = cloud->size ();

	if ( m_vertex_tot < 1 )
	{
		return;
	}

	Crul::PointGL const * const src = cloud->data ();

	m_vbo.create ();
	m_vbo.bind ();
	m_vbo.allocate ( src, (int)(m_vertex_tot * sizeof(src[0])) );
	m_vbo.release ();
}

void CloudQtGL::render (
	QMatrix4x4	const &	camera,
	QMatrix4x4	const &	proj
	)
{
	if ( ! m_program || m_vertex_tot < 1 )
	{
		return;
	}

	m_program->bind ();
	m_program->setUniformValue ( m_projMatrixLoc, proj );
	m_program->setUniformValue ( m_camMatrixLoc, camera );
	m_program->setUniformValue ( m_ptSizeLoc, GLfloat(m_point_size) );

	m_vbo.bind ();
	QOpenGLFunctions * const f = QOpenGLContext::currentContext ()->
		functions ();

	f->glEnableVertexAttribArray ( 0 );
	f->glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Crul::PointGL), 0 );

	f->glEnableVertexAttribArray ( 1 );
	f->glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE,
		sizeof(Crul::PointGL),
		(void const *)(3 * sizeof(GLfloat)) );

	m_vbo.release ();

	glDrawArrays ( GL_POINTS, 0, (GLsizei)m_vertex_tot );

	m_program->release ();
}

