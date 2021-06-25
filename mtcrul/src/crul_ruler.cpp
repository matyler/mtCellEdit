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

#include "crul.h"



Crul::Ruler::Ruler ()
	:
	m_id		( 0 ),
	m_visible	( true ),
	m_read_only	( false ),
	m_plane		( 0 )
{
	m_line.x1 = 0;
	m_line.y1 = 0;
	m_line.z1 = 0;
	m_line.x2 = 100;
	m_line.y2 = 100;
	m_line.z2 = 100;
	m_line.r = 1.0;
	m_line.g = 0.0;
	m_line.b = 1.0;
}

void Crul::Ruler::set_line (
	double const x1,
	double const y1,
	double const z1,
	double const x2,
	double const y2,
	double const z2
	)
{
	m_line.x1 = GLfloat( x1 );
	m_line.y1 = GLfloat( y1 );
	m_line.z1 = GLfloat( z1 );

	m_line.x2 = GLfloat( x2 );
	m_line.y2 = GLfloat( y2 );
	m_line.z2 = GLfloat( z2 );
}

void Crul::Ruler::set_line_rgb ( int const rgb )
{
	m_line.r = GLfloat ( pixy_int_2_red  (rgb) / 255.0 );
	m_line.g = GLfloat ( pixy_int_2_green(rgb) / 255.0 );
	m_line.b = GLfloat ( pixy_int_2_blue (rgb) / 255.0 );
}

int Crul::Ruler::change_length (
	double	const	delta,
	double	const	min
	)
{
	double const old_len = m_line.get_length ();

	if ( delta == 0.0 || (old_len <= min && delta < 0.0) )
	{
		return 0;
	}

	if ( (old_len + delta) < min )
	{
		return 0;
	}

	double x, y, z;
	m_line.get_unit_vector ( x, y, z );

	m_line.x2 = GLfloat ( (double)m_line.x2 + delta * x );
	m_line.y2 = GLfloat ( (double)m_line.y2 + delta * y );
	m_line.z2 = GLfloat ( (double)m_line.z2 + delta * z );

	return 1;
}

void Crul::Ruler::swap_ab ()
{
	Line const old = m_line;

	m_line.x1 = old.x2;
	m_line.y1 = old.y2;
	m_line.z1 = old.z2;

	m_line.x2 = old.x1;
	m_line.y2 = old.y1;
	m_line.z2 = old.z1;
}

void Crul::Ruler::set_line_rgb (
	int	const	r,
	int	const	g,
	int	const	b
	)
{
	m_line.r = GLfloat ( mtkit_double_bound (r / 255.0, 0.0, 1.0) );
	m_line.g = GLfloat ( mtkit_double_bound (g / 255.0, 0.0, 1.0) );
	m_line.b = GLfloat ( mtkit_double_bound (b / 255.0, 0.0, 1.0) );
}

void Crul::Ruler::create_ends_gl (
	std::vector<mtGin::GL::VertexRGBnormal> & vertices,
	double		const	end_size
	) const
{
	GLfloat const delta = GLfloat ( end_size );
	Line line;

	// End A

	line.x1 = m_line.x1;
	line.y1 = m_line.y1;
	line.z1 = m_line.z1 - delta;

	line.x2 = m_line.x1;
	line.y2 = m_line.y1;
	line.z2 = m_line.z1 + delta;

	line.r = m_line.r * 0.3f + 0.7f;
	line.g = m_line.g * 0.3f + 0.7f;
	line.b = m_line.b * 0.3f + 0.7f;

	line.create_gl ( vertices, end_size * 2.0 );

	// End B

	line.x1 = m_line.x2;
	line.y1 = m_line.y2;
	line.z1 = m_line.z2 - delta;

	line.x2 = m_line.x2;
	line.y2 = m_line.y2;
	line.z2 = m_line.z2 + delta;

	line.r = m_line.r * 0.3f + 0.3f;
	line.g = m_line.g * 0.3f + 0.3f;
	line.b = m_line.b * 0.3f + 0.3f;

	line.create_gl ( vertices, end_size * 2.0 );
}

static void delta_plane_xy (
	mtGin::GL::VertexRGB	& p1,
	mtGin::GL::VertexRGB	& p2,
	mtGin::GL::VertexRGB	& p3,
	mtGin::GL::VertexRGB	& p4,
	GLfloat	const	delta
	)
{
	p1.x -= delta;	p1.y -= delta;
	p2.x += delta;	p2.y -= delta;
	p3.x += delta;	p3.y += delta;
	p4.x -= delta;	p4.y += delta;
}

static void delta_plane_xz (
	mtGin::GL::VertexRGB	& p1,
	mtGin::GL::VertexRGB	& p2,
	mtGin::GL::VertexRGB	& p3,
	mtGin::GL::VertexRGB	& p4,
	GLfloat	const	delta
	)
{
	p1.x -= delta;	p1.z -= delta;
	p2.x += delta;	p2.z -= delta;
	p3.x += delta;	p3.z += delta;
	p4.x -= delta;	p4.z += delta;
}

static void delta_plane_yz (
	mtGin::GL::VertexRGB	& p1,
	mtGin::GL::VertexRGB	& p2,
	mtGin::GL::VertexRGB	& p3,
	mtGin::GL::VertexRGB	& p4,
	GLfloat	const	delta
	)
{
	p1.y -= delta;	p1.z -= delta;
	p2.y += delta;	p2.z -= delta;
	p3.y += delta;	p3.z += delta;
	p4.y -= delta;	p4.z += delta;
}

void Crul::Ruler::create_plane_gl (
	std::vector<mtGin::GL::VertexRGB> & lines,
	double		const	plane_range
	) const
{
	mtGin::GL::VertexRGB p1, p2, p3, p4; // Top left, going clockwise

	GLfloat const delta = GLfloat ( plane_range );

	p1.x = m_line.x1;
	p1.y = m_line.y1;
	p1.z = m_line.z1;

	p1.set_rgb ( m_line.r * 0.7f, m_line.g * 0.7f, m_line.b * 0.7f );

	p2 = p1;
	p3 = p1;
	p4 = p1;

	switch ( m_plane )
	{
	case PLANE_XY:
		delta_plane_xy ( p1, p2, p3, p4, delta );
		break;

	case PLANE_XZ:
		delta_plane_xz ( p1, p2, p3, p4, delta );
		break;

	case PLANE_YZ:
		delta_plane_yz ( p1, p2, p3, p4, delta );
		break;
	}

	p2.set_rgb ( p1 );
	p3.set_rgb ( p1 );
	p4.set_rgb ( p1 );

	create_gl_grid ( lines, 10, p1, p2, p3, p4 );
}

