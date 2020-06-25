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

#include "qt5_gl_vertex.h"



VertexQtGL::VertexQtGL ( GLenum const mode )
	:
	m_mode		( mode ),
	m_projMatrixLoc	( 0 ),
	m_camMatrixLoc	( 0 ),
	m_lightMatrixLoc ( 0 ),
	m_program	( NULL ),
	m_vertex_tot	( 0 )
{
	if ( mode != GL_TRIANGLES && mode != GL_LINES )
	{
		throw "VertexQtGL - bad mode";
	}
}

VertexQtGL::~VertexQtGL ()
{
	destroy ();
}

void VertexQtGL::init ( std::string const & version )
{
	destroy ();

	m_program = new QOpenGLShaderProgram;

	if ( m_mode == GL_TRIANGLES )
	{
		static char const * const vert_shader_triangles =
			"in vec4 vertex;\n"
			"in vec3 vertexRGB;\n"
			"in vec3 vertexNormal;\n"
			"out vec3 vertRGB;\n"
			"out vec3 vertXYZ;\n"
			"out vec3 vertNormal;\n"
			"uniform mat4 projMatrix;\n"
			"uniform mat4 camMatrix;\n"
			"void main() {\n"
			"   vertRGB = vertexRGB;\n"
			"   vertXYZ = vertex.xyz;\n"
			"   vertNormal = vertexNormal;\n"
			"   gl_Position = projMatrix * camMatrix * vertex;\n"
			"}\n";

		static char const * const frag_shader_triangles =
			"in highp vec3 vertRGB;\n"
			"in highp vec3 vertXYZ;\n"
			"in highp vec3 vertNormal;\n"
			"out highp vec4 fragColor;\n"
			"uniform highp vec3 lightMatrix;\n"
			"void main() {\n"
			"   highp vec3 L = normalize(lightMatrix - vertXYZ);\n"
			"   highp float NL = max(dot(normalize(vertNormal), L)"
				", 0.0);\n"
			"   highp vec3 color = vertRGB;\n"
			"   highp vec3 col = clamp(color * 0.4 + color * 0.6 *"
				" NL, 0.0, 1.0);\n"
			"   fragColor = vec4(col, 1.0);\n"
			"}\n";

		m_program->addShaderFromSourceCode ( QOpenGLShader::Vertex,
			(version + vert_shader_triangles).c_str () );

		m_program->addShaderFromSourceCode ( QOpenGLShader::Fragment,
			(version + frag_shader_triangles).c_str () );
	}
	else	// GL_LINES
	{
		static char const * const vert_shader_lines =
			"in vec4 vertex;\n"
			"in vec3 vertexRGB;\n"
			"out vec3 vertRGB;\n"
			"uniform mat4 projMatrix;\n"
			"uniform mat4 camMatrix;\n"
			"void main() {\n"
			"   vertRGB = vertexRGB;\n"
			"   gl_Position = projMatrix * camMatrix * vertex;\n"
			"}\n";

		static char const * const frag_shader_lines =
			"in highp vec3 vertRGB;\n"
			"out highp vec4 fragColor;\n"
			"void main() {\n"
			"   fragColor = vec4(vertRGB, 1.0);\n"
			"}\n";

		m_program->addShaderFromSourceCode ( QOpenGLShader::Vertex,
			(version + vert_shader_lines).c_str () );

		m_program->addShaderFromSourceCode ( QOpenGLShader::Fragment,
			(version + frag_shader_lines).c_str () );
	}

	m_program->bindAttributeLocation ( "vertex", 0 );
	m_program->bindAttributeLocation ( "vertexRGB", 1 );

	if ( m_mode == GL_TRIANGLES )
	{
		m_program->bindAttributeLocation ( "vertexNormal", 2 );
	}

	m_program->link ();

	m_projMatrixLoc	= m_program->uniformLocation ( "projMatrix" );
	m_camMatrixLoc	= m_program->uniformLocation ( "camMatrix" );

	if ( m_mode == GL_TRIANGLES )
	{
		m_lightMatrixLoc = m_program->uniformLocation ( "lightMatrix" );
	}
}

void VertexQtGL::destroy ()
{
	m_vbo.destroy ();
	m_vertex_tot = 0;

	delete m_program;
	m_program = NULL;
}

void VertexQtGL::populate ( std::vector<Crul::VertexGL> const & vertices )
{
	m_vertex_tot = vertices.size ();
	if ( m_vertex_tot > 0 )
	{
		Crul::VertexGL const * const src = vertices.data ();

		m_vbo.create ();
		m_vbo.bind ();
		m_vbo.allocate ( src, (int)(m_vertex_tot * sizeof(src[0])) );
		m_vbo.release ();
	}
}

void VertexQtGL::render (
	QMatrix4x4 const & camera,
	QMatrix4x4 const & proj,
	QVector3D const & light
	)
{
	if ( ! m_program || m_vertex_tot < 1 )
	{
		return;
	}

	m_program->bind ();
	m_program->setUniformValue ( m_projMatrixLoc, proj );
	m_program->setUniformValue ( m_camMatrixLoc, camera );

	if ( m_mode == GL_TRIANGLES )
	{
		m_program->setUniformValue ( m_lightMatrixLoc, light );
	}

	m_vbo.bind ();
	QOpenGLFunctions * const f = QOpenGLContext::currentContext ()->
		functions ();

	f->glEnableVertexAttribArray ( 0 );
	f->glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Crul::VertexGL), 0 );

	f->glEnableVertexAttribArray ( 1 );
	f->glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE,
		sizeof(Crul::VertexGL),
		reinterpret_cast<void *>(3 * sizeof(GLfloat)) );

	if ( m_mode == GL_TRIANGLES )
	{
		f->glEnableVertexAttribArray ( 2 );
		f->glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE,
			sizeof(Crul::VertexGL),
			reinterpret_cast<void *>(6 * sizeof(GLfloat)) );
	}

	m_vbo.release ();

	glDrawArrays ( m_mode, 0, (GLsizei)m_vertex_tot );

	m_program->release ();
}

