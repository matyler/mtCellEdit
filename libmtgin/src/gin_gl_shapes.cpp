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

#include "private.h"



mtGin::GL::Shapes::Shapes ()
{
}

mtGin::GL::Shapes::~Shapes ()
{
	destroy();
}

void mtGin::GL::Shapes::destroy ()
{
	m_vbuf.clear ();

	m_vertex_tot = 0;

	m_shader_prg.clear ();
}



///	Points	----------------------------------------------------------------



int mtGin::GL::Points::init ( std::string const & version )
{
	destroy ();

	if ( 0 == m_shader_prg.init () )
	{
		return 1;
	}

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

	std::string const vert (version + vert_shader_src);
	std::string const frag (version + frag_shader_src);

	if ( 0 == m_shader_prg.add_shader ( vert.c_str(), GL_VERTEX_SHADER ) ||
		0 == m_shader_prg.add_shader ( frag.c_str(), GL_FRAGMENT_SHADER)
		)
	{
		destroy ();
		return 1;
	}

	m_shader_prg.bind_attribute ( "vertex", 0 );
	m_shader_prg.bind_attribute ( "vertexRGB", 1 );

	if ( m_shader_prg.link () )
	{
		destroy ();
		return 1;
	}

	m_proj_loc = m_shader_prg.get_uniform_location ( "projMatrix" );
	m_cam_loc = m_shader_prg.get_uniform_location ( "camMatrix" );
	m_ptSize_loc = m_shader_prg.get_uniform_location ( "ptSize" );

	return 0;
}

void mtGin::GL::Points::populate (
	mtGin::GL::VertexRGB	const * const	vertices,
	size_t	const	tot
	)
{
	m_vbuf.clear ();
	m_vertex_tot = tot;

	if ( m_vertex_tot < 1 )
	{
		return;
	}

	m_vbuf.create ();
	m_vbuf.bind ();
	m_vbuf.allocate ( vertices, m_vertex_tot * sizeof(vertices[0]) );
	m_vbuf.release ();
}

void mtGin::GL::Points::render (
	mtGin::GL::Matrix4x4 const & camera,
	mtGin::GL::Matrix4x4 const & proj
	) const
{
	if ( m_vertex_tot < 1 || ! get_visible() )
	{
		return;
	}

	m_shader_prg.bind ();
	m_shader_prg.set_uniform_4x4f ( m_proj_loc, proj );
	m_shader_prg.set_uniform_4x4f ( m_cam_loc, camera );
	m_shader_prg.set_uniform_1f ( m_ptSize_loc, GLfloat(m_point_size) );

	m_vbuf.bind ();

	glEnableVertexAttribArray ( 0 );
	glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE,
		sizeof(mtGin::GL::VertexRGB), 0 );

	glEnableVertexAttribArray ( 1 );
	glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE,
		sizeof(mtGin::GL::VertexRGB),
		(void const *)(3 * sizeof(GLfloat)) );

	glDrawArrays ( GL_POINTS, 0, (GLsizei)m_vertex_tot );

	m_vbuf.release ();
	m_shader_prg.release ();
}



///	Lines	----------------------------------------------------------------



int mtGin::GL::Lines::init ( std::string const & version )
{
	destroy ();

	if ( 0 == m_shader_prg.init () )
	{
		return 1;
	}

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

	std::string const vert (version + vert_shader_lines);
	std::string const frag (version + frag_shader_lines);

	if ( 0 == m_shader_prg.add_shader ( vert.c_str(), GL_VERTEX_SHADER ) ||
		0 == m_shader_prg.add_shader ( frag.c_str(), GL_FRAGMENT_SHADER)
		)
	{
		destroy ();
		return 1;
	}

	m_shader_prg.bind_attribute ( "vertex", 0 );
	m_shader_prg.bind_attribute ( "vertexRGB", 1 );

	if ( m_shader_prg.link () )
	{
		destroy ();
		return 1;
	}

	m_proj_loc = m_shader_prg.get_uniform_location ( "projMatrix" );
	m_cam_loc = m_shader_prg.get_uniform_location ( "camMatrix" );

	return 0;
}

void mtGin::GL::Lines::populate (
	mtGin::GL::VertexRGB	const * const	vertices,
	size_t	const	tot
	)
{
	m_vbuf.clear ();
	m_vertex_tot = tot;

	if ( tot < 1 )
	{
		return;
	}

	size_t const byte_tot = tot * sizeof(vertices[0]);

	m_vbuf.create ();
	m_vbuf.bind ();
	m_vbuf.allocate ( vertices, byte_tot );
	m_vbuf.release ();
}

void mtGin::GL::Lines::render (
	mtGin::GL::Matrix4x4 const & camera,
	mtGin::GL::Matrix4x4 const & proj
	) const
{
	if ( m_vertex_tot < 1 || ! get_visible() )
	{
		return;
	}

	m_shader_prg.bind ();
	m_shader_prg.set_uniform_4x4f ( m_proj_loc, proj );
	m_shader_prg.set_uniform_4x4f ( m_cam_loc, camera );

	m_vbuf.bind ();

	glEnableVertexAttribArray ( 0 );
	glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE,
		sizeof(mtGin::GL::VertexRGB), 0 );

	glEnableVertexAttribArray ( 1 );
	glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE,
		sizeof(mtGin::GL::VertexRGB),
		reinterpret_cast<void *>(3 * sizeof(GLfloat)) );

	glDrawArrays ( GL_LINES, 0, (GLsizei)m_vertex_tot );

	m_vbuf.release ();
	m_shader_prg.release ();
}



///	Triangles	--------------------------------------------------------



int mtGin::GL::Triangles::init ( std::string const & version )
{
	destroy ();

	if ( 0 == m_shader_prg.init () )
	{
		return 1;
	}

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

	std::string const vert (version + vert_shader_triangles);
	std::string const frag (version + frag_shader_triangles);

	if ( 0 == m_shader_prg.add_shader ( vert.c_str(), GL_VERTEX_SHADER ) ||
		0 == m_shader_prg.add_shader ( frag.c_str(), GL_FRAGMENT_SHADER)
		)
	{
		destroy ();
		return 1;
	}

	m_shader_prg.bind_attribute ( "vertex", 0 );
	m_shader_prg.bind_attribute ( "vertexRGB", 1 );
	m_shader_prg.bind_attribute ( "vertexNormal", 2 );

	if ( m_shader_prg.link () )
	{
		destroy ();
		return 1;
	}

	m_proj_loc = m_shader_prg.get_uniform_location ( "projMatrix" );
	m_cam_loc = m_shader_prg.get_uniform_location ( "camMatrix" );
	m_light_loc = m_shader_prg.get_uniform_location( "lightMatrix");

	return 0;
}

void mtGin::GL::Triangles::populate (
	mtGin::GL::VertexRGBnormal const * const vertices,
	size_t	const	tot
	)
{
	m_vbuf.clear ();
	m_vertex_tot = tot;

	if ( tot < 1 )
	{
		return;
	}

	size_t const byte_tot = tot * sizeof(vertices[0]);

	m_vbuf.create ();
	m_vbuf.bind ();
	m_vbuf.allocate ( vertices, byte_tot );
	m_vbuf.release ();
}

void mtGin::GL::Triangles::render (
	mtGin::GL::Matrix4x4 const & camera,
	mtGin::GL::Matrix4x4 const & proj,
	mtGin::Vertex	const	& light
	) const
{
	if ( m_vertex_tot < 1 || ! get_visible() )
	{
		return;
	}

	m_shader_prg.bind ();
	m_shader_prg.set_uniform_4x4f ( m_proj_loc, proj );
	m_shader_prg.set_uniform_4x4f ( m_cam_loc, camera );
	m_shader_prg.set_uniform_3f ( m_light_loc, (GLfloat)light.x,
		(GLfloat)light.y, (GLfloat)light.z );

	m_vbuf.bind ();

	glEnableVertexAttribArray ( 0 );
	glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE,
		sizeof(mtGin::GL::VertexRGBnormal), 0 );

	glEnableVertexAttribArray ( 1 );
	glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE,
		sizeof(mtGin::GL::VertexRGBnormal),
		reinterpret_cast<void *>(3 * sizeof(GLfloat)) );

	glEnableVertexAttribArray ( 2 );
	glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE,
		sizeof(mtGin::GL::VertexRGBnormal),
		reinterpret_cast<void *>(6 * sizeof(GLfloat)) );

	glDrawArrays ( GL_TRIANGLES, 0, (GLsizei)m_vertex_tot );

	m_vbuf.release ();
	m_shader_prg.release ();
}

