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



std::string mtGin::GL::get_shader_language_version ()
{
	double dver = 1.3;		// Fallback version
	char const * const gl_str =
		(char const *)glGetString(GL_SHADING_LANGUAGE_VERSION);

	if ( gl_str )
	{
		char const * gl_num;

		for (	gl_num = gl_str;
			*gl_num != 0 && !isdigit ( *gl_num );
			gl_num++
			)
		{
		}

		if ( 0 != *gl_num )
		{
			mtkit_strtod ( gl_num, &dver, NULL, 0 );
		}
	}

	char const * const glv = (char const *)glGetString ( GL_VERSION );
	char const * const gl_ver = glv ? glv : "";

	int const gles = (mtkit_strmatch(gl_ver, "OpenGL ES*", 0) >= 0) ? 1 : 0;

	char vbuf[128];
	snprintf ( vbuf, sizeof(vbuf), "#version %.0f%s\n", dver * 100,
		gles ? " es" : "" );

#ifdef DEBUG
	std::cout << "GL_VERSION='" << gl_ver << "' gles=" << gles << "\n";

	char const * const gl_str_deref = gl_str ? gl_str : "???";

	std::cout << "OpenGL shading language version = " << vbuf
		<< "	from GL_SHADING_LANGUAGE_VERSION='" << gl_str_deref
		<< "'\n";
#endif

	return std::string ( vbuf );
}

mtGin::GL::ShaderProgram::ShaderProgram ()
{
}

mtGin::GL::ShaderProgram::~ShaderProgram ()
{
	clear ();
}

unsigned int mtGin::GL::ShaderProgram::init ()
{
	clear ();

	m_program_id = glCreateProgram ();

	if ( 0 == m_program_id )
	{
		std::cerr << "Unable to init ShaderProgram\n";
	}

	return m_program_id;
}

unsigned int mtGin::GL::ShaderProgram::add_shader (
	char	const * const	src,
	unsigned int	const	type
	) const
{
	if ( ! src )
	{
		std::cerr << "NULL shader\n";
		return 0;
	}

	unsigned int shader = glCreateShader ( GLenum(type) );
	if ( ! shader )
	{
		std::cerr << "Unable to create shader: type=" << type << "\n";
		return 0;
	}

	glShaderSource ( shader, 1, &src, nullptr );
	glCompileShader ( shader );

	int success = 0;
	glGetShaderiv ( shader, GL_COMPILE_STATUS, &success );
	if ( ! success )
	{
		char info[512];
		glGetShaderInfoLog ( shader, sizeof(info), nullptr, info );
		std::cerr << "Unable to compile shader: " << info << "\n";
		glDeleteShader ( shader );
		return 0;
	}

	glAttachShader ( m_program_id, shader );
	glDeleteShader ( shader );

	return shader;
}

int mtGin::GL::ShaderProgram::link () const
{
	glLinkProgram ( m_program_id );

	int success = 0;
	glGetProgramiv ( m_program_id, GL_LINK_STATUS, &success );
	if ( ! success )
	{
		char info[512];
		glGetProgramInfoLog( m_program_id, sizeof(info), nullptr, info);
		std::cerr << "Unable to link shaders:" << info << "\n";
		return 1;
	}

	return 0;
}

void mtGin::GL::ShaderProgram::bind_attribute (
	char	const * const	name,
	unsigned int	const	index
	) const
{
	glBindAttribLocation ( m_program_id, index, name );
}

int mtGin::GL::ShaderProgram::get_uniform_location (
	char	const * const	name
	) const
{
	return glGetUniformLocation ( m_program_id, name );
}

void mtGin::GL::ShaderProgram::set_uniform_1f (
	int		const	location,
	GLfloat		const	v0
	)
{
	glUniform1f ( location, v0 );
}

void mtGin::GL::ShaderProgram::set_uniform_3f (
	int		const	location,
	GLfloat		const	v0,
	GLfloat		const	v1,
	GLfloat		const	v2
	)
{
	glUniform3f ( location, v0, v1, v2 );
}

void mtGin::GL::ShaderProgram::set_uniform_4x4f (
	int		const	location,
	Matrix4x4	const	& matrix
	)
{
	glUniformMatrix4fv ( location, 1, GL_FALSE, &(matrix.data()[0][0]) );
}

void mtGin::GL::ShaderProgram::bind () const
{
	glUseProgram ( m_program_id );
}

void mtGin::GL::ShaderProgram::release ()
{
	glUseProgram ( 0 );
}

void mtGin::GL::ShaderProgram::clear ()
{
	delete_program ();
}

void mtGin::GL::ShaderProgram::delete_program ()
{
	if ( m_program_id > 0 )
	{
		glDeleteProgram ( m_program_id );
		m_program_id = 0;
	}
}



/// ----------------------------------------------------------------------------



mtGin::GL::VertexBuffer::VertexBuffer ()
{
}

mtGin::GL::VertexBuffer::~VertexBuffer ()
{
	clear ();
}


void mtGin::GL::VertexBuffer::create ()
{
	clear ();

	glGenBuffers ( 1, &m_buffer_id );
}

void mtGin::GL::VertexBuffer::allocate (
	void	const * const	data,
	size_t		const	size
	)
{
	glBufferData( GL_ARRAY_BUFFER, GLsizeiptr(size), data, GL_STATIC_DRAW );
}

void mtGin::GL::VertexBuffer::bind () const
{
	glBindBuffer ( GL_ARRAY_BUFFER, m_buffer_id );
}

void mtGin::GL::VertexBuffer::release ()
{
	glBindBuffer ( GL_ARRAY_BUFFER, 0 );
}

void mtGin::GL::VertexBuffer::clear ()
{
	if ( m_buffer_id > 0 )
	{
		glDeleteBuffers ( 1, &m_buffer_id );
		m_buffer_id = 0;
	}
}

void mtGin::GL::VertexRGBnormal::calc_normal (
	VertexRGBnormal	& p2,
	VertexRGBnormal	& p3
	)
{
	VertexRGBnormal const & p1 = *this;

	// u = p2 - p1
	GLfloat const ux = p2.x - p1.x;
	GLfloat const uy = p2.y - p1.y;
	GLfloat const uz = p2.z - p1.z;

	// v = p3 - p1
	GLfloat const vx = p3.x - p1.x;
	GLfloat const vy = p3.y - p1.y;
	GLfloat const vz = p3.z - p1.z;

	nx = uy * vz - uz * vy;
	ny = uz * vx - ux * vz;
	nz = ux * vy - uy * vx;

	p2.set_normal ( p1 );
	p3.set_normal ( p1 );
}

