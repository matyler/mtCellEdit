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

#include "crul.h"



double Crul::normalize_angle ( double angle )
{
	angle = fmod ( angle, 360 );

	if ( angle < 0 )
	{
		angle += 360;
	}

	return angle;
}

int Crul::get_float_rgb_int ( GLfloat const val )
{
	return mtkit_int_bound ( (int)round((double)val * 255.0), 0, 255);
}

void Crul::create_gl_grid (
	std::vector<Crul::VertexGL> & lines,
	int		const	count,
	VertexGL	const	& v1,
	VertexGL	const	& v2,
	VertexGL	const	& v3,
	VertexGL	const	& v4
	)
{
	GLfloat const tot = GLfloat(count);
	VertexGL a, b;

	a.set_rgb ( v1 );
	b.set_rgb ( v1 );

	for ( GLfloat i = 0; i <= tot; i++ )
	{
		a.x = GLfloat ( (v1.x * i + v4.x * (tot - i))/tot );
		a.y = GLfloat ( (v1.y * i + v4.y * (tot - i))/tot );
		a.z = GLfloat ( (v1.z * i + v4.z * (tot - i))/tot );
		lines.push_back ( a );

		b.x = GLfloat ( (v2.x * i + v3.x * (tot - i))/tot );
		b.y = GLfloat ( (v2.y * i + v3.y * (tot - i))/tot );
		b.z = GLfloat ( (v2.z * i + v3.z * (tot - i))/tot );
		lines.push_back ( b );

		a.x = GLfloat ( (v1.x * i + v2.x * (tot - i))/tot );
		a.y = GLfloat ( (v1.y * i + v2.y * (tot - i))/tot );
		a.z = GLfloat ( (v1.z * i + v2.z * (tot - i))/tot );
		lines.push_back ( a );

		b.x = GLfloat ( (v4.x * i + v3.x * (tot - i))/tot );
		b.y = GLfloat ( (v4.y * i + v3.y * (tot - i))/tot );
		b.z = GLfloat ( (v4.z * i + v3.z * (tot - i))/tot );
		lines.push_back ( b );
	}
}

void Crul::create_gl_face (
	std::vector<Crul::VertexGL> & vertices,
	VertexGL	& v1,
	VertexGL	& v2,
	VertexGL	& v3,
	VertexGL	& v4
	)
{
	create_gl_triangle ( vertices, v1, v2, v3 );
	create_gl_triangle ( vertices, v1, v3, v4 );
}

void Crul::create_gl_triangle (
	std::vector<Crul::VertexGL> & vertices,
	VertexGL	& v1,
	VertexGL	& v2,
	VertexGL	& v3
	)
{
	v1.normal ( v2, v3 );

	vertices.push_back ( v1 );
	vertices.push_back ( v2 );
	vertices.push_back ( v3 );
}

Crul::Camera::Camera ()
	:
	m_id		( 0 ),
	m_x		( CAM_A_X_DEFAULT ),
	m_y		( CAM_A_Y_DEFAULT ),
	m_z		( CAM_A_Z_DEFAULT ),
	m_rotX		( CAM_A_XROT_DEFAULT ),
	m_rotY		( CAM_A_YROT_DEFAULT ),
	m_rotZ		( CAM_A_ZROT_DEFAULT ),
	m_read_only	( false )
{
}

double Crul::Line::get_length () const
{
	return sqrt (	(x2 - x1) * (x2 - x1) +
			(y2 - y1) * (y2 - y1) +
			(z2 - z1) * (z2 - z1)
		);
}

void Crul::Line::get_unit_vector ( double &x, double &y, double &z ) const
{
	double const len = get_length ();

	if ( 0.0 == len )
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
		return;
	}

	x = (double)(x2 - x1) / len;
	y = (double)(y2 - y1) / len;
	z = (double)(z2 - z1) / len;
}

double Crul::Line::get_angle_xy () const
{
	double const len = sqrt (
		(double)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));

	if ( len == 0.0 )
	{
		return 90.0;
	}

	return (asin ( abs((double)(x2 - x1)) / len ) / M_PI * 180.0);
}

double Crul::Line::get_angle_z () const
{
	double const len = get_length ();

	if ( len == 0.0 )
	{
		return 90.0;
	}

	return (asin ( abs((double)(z2 - z1)) / len ) / M_PI * 180.0);
}

int Crul::Line::get_rgb_int () const
{
	return mtPixy::rgb_2_int ( get_r_int (), get_g_int (), get_b_int () );
}

static void set_vcolor (
	std::vector<Crul::VertexGL> &vs,
	Crul::Line	const	&line,
	GLfloat		const	m
	)
{
	for ( auto && i : vs )
	{
		i.r = line.r * m;
		i.g = line.g * m;
		i.b = line.b * m;
	}
}

static void set_xyz (
	Crul::VertexGL	& v,
	GLfloat	const	x,
	GLfloat	const	y,
	GLfloat	const	z
	)
{
	v.x = x;
	v.y = y;
	v.z = z;
}

void Crul::Line::create_gl (
	std::vector<Crul::VertexGL> & vertices,
	double		const	line_size
	) const
{
	GLfloat const len = GLfloat (line_size / 2.0);
	double dx, dy, dz;
	get_unit_vector ( dx, dy, dz );

	GLfloat const dzf = GLfloat(dz);

	std::vector<Crul::VertexGL> vs (8);

	// Set up default centre values for either end
	for ( size_t i = 0; i < 4; i++ )
	{
		set_xyz ( vs[i+0], x1, y1, z1 );
		set_xyz ( vs[i+4], x2, y2, z2 );
	}

	vs[0].x -= len * dzf;
	vs[1].x += len * dzf;
	vs[2].x += len * dzf;
	vs[3].x -= len * dzf;

	vs[0].y -= len * dzf;
	vs[1].y -= len * dzf;
	vs[2].y += len * dzf;
	vs[3].y += len * dzf;

	vs[4].x -= len * dzf;
	vs[5].x += len * dzf;
	vs[6].x += len * dzf;
	vs[7].x -= len * dzf;

	vs[4].y -= len * dzf;
	vs[5].y -= len * dzf;
	vs[6].y += len * dzf;
	vs[7].y += len * dzf;

	set_vcolor ( vs, *this, 1.0f );

/*
	* = Point 1
	& = Point 2
	0-7 = Vertices

0-x
|\
z y
	0-------1
	|\   *  |\
	| 3-----+-2
	| |     | |
	| |     | |
	| |     | |
	| |     | |
	| |     | |
	| |     | |
	| |     | |
	| |     | |
	4-+-----5 |
	 \|  &   \|
	  7-------6
*/

	// NOTE: the winding order is set to let OpenGL light the exterior sides

	// Bottom XY axis
	create_gl_face ( vertices, vs[3], vs[2], vs[1], vs[0] );
	// Top XY axis
	create_gl_face ( vertices, vs[4], vs[5], vs[6], vs[7] );

//	set_vcolor ( vs, *this, 0.8f );

	// Right XZ axis
	create_gl_face ( vertices, vs[0], vs[1], vs[5], vs[4] );
	// Left XZ axis
	create_gl_face ( vertices, vs[7], vs[6], vs[2], vs[3] );

//	set_vcolor ( vs, *this, 0.6f );

	// Nearside YZ axis
	create_gl_face ( vertices, vs[4], vs[7], vs[3], vs[0] );
	// Farside YZ axis
	create_gl_face ( vertices, vs[1], vs[2], vs[6], vs[5] );
}

void Crul::VertexGL::normal ( VertexGL & p2, VertexGL & p3 )
{
	VertexGL const & p1 = *this;

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

