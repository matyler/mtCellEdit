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



void mtGin::GL::Camera::set_position (
	double	const	x,
	double	const	y,
	double	const	z
	)
{
	m_x = x;
	m_y = y;
	m_z = z;

	update_matrix ();
}

void mtGin::GL::Camera::set_position ( mtGin::Vertex const & position )
{
	m_x = position.x;
	m_y = position.y;
	m_z = position.z;

	update_matrix ();
}

void mtGin::GL::Camera::set_angle (
	double	const	rot_x,
	double	const	rot_y,
	double	const	rot_z
	)
{
	m_rot_x = mtkit_angle_normalize ( rot_x );
	m_rot_y = mtkit_angle_normalize ( rot_y );
	m_rot_z = mtkit_angle_normalize ( rot_z );

	update_matrix ();
}

void mtGin::GL::Camera::look_at (
	mtGin::Vertex	const	& focus
	)
{
	mtGin::Vertex const vec (	// Vector: Camera -> Focus
		focus.x - m_x,
		focus.y - m_y,
		focus.z - m_z
		);
	double const h = sqrt ( vec.x * vec.x + vec.y * vec.y );

	set_angle ( 270 + atan2 ( vec.z, h ) / M_PI * 180.0,
		0.0,
		90 - atan2 ( vec.y, vec.x ) / M_PI * 180.0 );
}

void mtGin::GL::Camera::move (
	double	const	lr,
	double	const	du,
	double	const	bf
	)
{
	Matrix3x3 const normal ( this->get_matrix().normal() );
	Array3x3_float const & data = normal.data ();

	double const x = m_x +
		 lr * (double)data[0][0] +
		 du * (double)data[0][1] +
		-bf * (double)data[0][2];

	double const y = m_y +
		 lr * (double)data[1][0] +
		 du * (double)data[1][1] +
		-bf * (double)data[1][2];

	double const z = m_z +
		 lr * (double)data[2][0] +
		 du * (double)data[2][1] +
		-bf * (double)data[2][2];

	set_position ( x, y, z );
}

void mtGin::GL::Camera::turn (
	double	const	left_right,
	double	const	clock,
	double	const	down_up
	)
{
	set_angle ( m_rot_x + left_right, m_rot_y + clock, m_rot_z + down_up );
}

void mtGin::GL::Camera::turn_around ()
{
	set_angle ( (180.0 - m_rot_x), m_rot_y, (180.0 + m_rot_z) );
}

void mtGin::GL::Camera::update_matrix ()
{
	m_matrix.set_identity ();
	m_matrix.rotate ( (GLfloat)(180.0 - m_rot_x), 1, 0, 0 );
	m_matrix.rotate ( (GLfloat)m_rot_y, 0, 1, 0 );
	m_matrix.rotate ( (GLfloat)m_rot_z, 0, 0, 1 );
	m_matrix.translate (
		(GLfloat)-m_x,
		(GLfloat)-m_y,
		(GLfloat)-m_z );
}

