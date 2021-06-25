/*
	Copyright (C) 2021 Mark Tyler

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

#ifndef MTGIN_GL_H_
#define MTGIN_GL_H_

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include "mtgin.h"



//	mtGin = Mark Tyler's Graphical Interface Nexus



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API

namespace mtGin
{

/* NOTES:
Whenever using OpenGL always set the context before the calls, and restrict to
the GUI thread.
*/



namespace GL		// OpenGL
{

class Camera;
class Lines;
class Matrix3x3;
class Matrix4x4;
class Points;
class ShaderProgram;
class Shapes;
class VertexBuffer;
class VertexRGB;
class VertexRGBnormal;

typedef GLfloat Array3x3_float[3][3];
typedef GLfloat Array4x4_float[4][4];
typedef GLdouble Array4x4_double[4][4];



std::string get_shader_language_version ();



class VertexRGB
{
public:
	static size_t constexpr FLOAT_TOT = 6;

	inline void set_xyz ( GLfloat xx, GLfloat yy, GLfloat zz )
	{ x=xx; y=yy; z=zz; }

	inline void set_rgb ( GLfloat rr, GLfloat gg, GLfloat bb )
	{ r=rr; g=gg; b=bb; }

	inline void set_rgb ( VertexRGB const & v )
	{ r=v.r; g=v.g; b=v.b; }

	GLfloat	x, y, z;
	GLfloat	r, g, b;	// 0.0 -> 1.0
};



class VertexRGBnormal
{
public:
	static size_t constexpr FLOAT_TOT = 9;

	inline void set_xyz ( GLfloat xx, GLfloat yy, GLfloat zz )
	{ x=xx; y=yy; z=zz; }

	inline void set_rgb ( GLfloat rr, GLfloat gg, GLfloat bb )
	{ r=rr; g=gg; b=bb; }

	inline void set_rgb ( VertexRGBnormal const & v )
	{ r=v.r; g=v.g; b=v.b; }

	inline void set_normal ( VertexRGBnormal const & v )
	{ nx=v.nx; ny=v.ny; nz=v.nz; }

	void calc_normal ( VertexRGBnormal & p2, VertexRGBnormal & p3 );

	GLfloat	x, y, z;
	GLfloat	r, g, b;	// 0.0 -> 1.0
	GLfloat	nx, ny, nz;	// Normal
};



class Matrix3x3
{
public:
	Matrix3x3 () { set_identity(); }
	~Matrix3x3 () {}

	inline Array3x3_float & data () { return m_data; }
	inline Array3x3_float const & data () const { return m_data; }

	void set_identity ();

private:
	Array3x3_float	m_data;		// Column major as OpenGL
};



class Matrix4x4
{
public:
	Matrix4x4 () { set_identity(); }
	~Matrix4x4 () {}

	inline Array4x4_float & data () { return m_data; }
	inline Array4x4_float const & data () const { return m_data; }
	void copy_to_doubles ( Array4x4_double & dest ) const;

	Matrix3x3 normal () const;

	void set_identity ();
	void rotate ( GLfloat angle, GLfloat x, GLfloat y, GLfloat z );
	void translate ( GLfloat x, GLfloat y, GLfloat z );
	void perspective (
		GLfloat vert_angle,
		GLfloat aspect_ratio,
		GLfloat near,
		GLfloat far );

private:
	Array4x4_float	m_data;		// Column major as OpenGL
};



class Camera
{
public:
	inline Matrix4x4 const & get_matrix () const { return m_matrix; }

	inline double get_x () const		{ return m_x; }
	inline double get_y () const		{ return m_y; }
	inline double get_z () const		{ return m_z; }
	inline double get_rot_x () const	{ return m_rot_x; }
	inline double get_rot_y () const	{ return m_rot_y; }
	inline double get_rot_z () const	{ return m_rot_z; }

	void set_position ( double x, double y, double z );
	void set_position ( mtGin::Vertex const & position );
	void set_angle ( double rot_x, double rot_y, double rot_z );
	void look_at ( mtGin::Vertex const & focus );

	void move ( double lr, double du, double bf );
	void turn ( double left_right, double clock, double down_up );
	void turn_around ();

private:
	void update_matrix ();

/// ----------------------------------------------------------------------------

	Matrix4x4	m_matrix;

	double		m_x = 0.0;
	double		m_y = 0.0;
	double		m_z = 0.0;
	double		m_rot_x = 0.0;	// Degrees
	double		m_rot_y = 0.0;
	double		m_rot_z = 0.0;
};



class ShaderProgram
{
public:
	ShaderProgram ();
	~ShaderProgram ();

	unsigned int init ();
		// =id of new program. 0=Error

	unsigned int add_shader (
		char const * src,
		unsigned int type // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
		) const;
		// =id of new shader. 0=Error

	void bind_attribute ( char const * name, unsigned int index ) const;
	int get_uniform_location ( char const * name ) const;
		// -1=Error, else location

	int link () const;

	static void set_uniform_1f( int location, GLfloat v0 );
	static void set_uniform_3f( int location, GLfloat v0, GLfloat v1,
		GLfloat v2 );
	static void set_uniform_4x4f (
		int location, Matrix4x4 const & matrix
		);

	void bind () const;
	static void release ();

	void clear ();

private:
	void delete_program ();

/// ----------------------------------------------------------------------------

	unsigned int			m_program_id	= 0;

	MTKIT_RULE_OF_FIVE( ShaderProgram )
};



class VertexBuffer
{
public:
	VertexBuffer ();
	~VertexBuffer ();

	void create ();
	static void allocate ( void const * data, size_t size );
	void bind () const;
	static void release ();

	inline unsigned int id() const { return m_buffer_id; }

	void clear ();

private:

/// ----------------------------------------------------------------------------

	unsigned int	m_buffer_id	= 0;

	MTKIT_RULE_OF_FIVE( VertexBuffer )
};



class Shapes
{
public:
	Shapes ();
	virtual ~Shapes ();

	virtual int init ( std::string const & version ) = 0;
	void destroy ();

	inline void set_visible ( int const visible ) { m_visible = visible; }
	inline int get_visible() const { return m_visible; }

protected:
	int		m_proj_loc	= 0;
	int		m_cam_loc	= 0;

	mtGin::GL::ShaderProgram	m_shader_prg;
	mtGin::GL::VertexBuffer		m_vbuf;

	size_t		m_vertex_tot	= 0;

private:
	int		m_visible	= 1;

	MTKIT_RULE_OF_FIVE( Shapes )
};



class Points : public Shapes
{
public:
	int init ( std::string const & version )	override;
	void populate( mtGin::GL::VertexRGB const * vertices, size_t tot);
	void render (
		mtGin::GL::Matrix4x4 const & camera,
		mtGin::GL::Matrix4x4 const & proj
		) const;

	inline void set_point_size ( double const s ) { m_point_size = s; }
	inline double get_point_size () const { return m_point_size; }

private:
	int		m_ptSize_loc	= 0;
	double		m_point_size	= 3.0;
};



class Lines : public Shapes
{
public:
	int init ( std::string const & version )	override;
	void populate( mtGin::GL::VertexRGB const * vertices, size_t tot);
	void render (
		mtGin::GL::Matrix4x4 const & camera,
		mtGin::GL::Matrix4x4 const & proj
		) const;
};



class Triangles : public Shapes
{
public:
	int init ( std::string const & version )	override;
	void populate( mtGin::GL::VertexRGBnormal const * vertices, size_t tot);
	void render (
		mtGin::GL::Matrix4x4 const & camera,
		mtGin::GL::Matrix4x4 const & proj,
		mtGin::Vertex const & light
		) const;

private:
	int		m_light_loc	= 0;
};



}		// namespace GL



}		// namespace mtGin



#endif		// C++ API



#endif		// MTGIN_GL_H_

