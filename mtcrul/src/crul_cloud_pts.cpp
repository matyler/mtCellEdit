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



void Crul::PtsExtent::clear ()
{
	x_min = GLfloat(0); x_max = GLfloat(0);
	y_min = GLfloat(0); y_max = GLfloat(0);
	z_min = GLfloat(0); z_max = GLfloat(0);
	r_min = GLfloat(0); r_max = GLfloat(0);
	g_min = GLfloat(0); g_max = GLfloat(0);
	b_min = GLfloat(0); b_max = GLfloat(0);
}

bool Crul::PtsExtent::is_clear () const
{
	if (	x_min == GLfloat(0) && x_max == GLfloat(0) &&
		y_min == GLfloat(0) && y_max == GLfloat(0) &&
		z_min == GLfloat(0) && z_max == GLfloat(0) &&
		r_min == GLfloat(0) && r_max == GLfloat(0) &&
		g_min == GLfloat(0) && g_max == GLfloat(0) &&
		b_min == GLfloat(0) && b_max == GLfloat(0)
		)
	{
		return true;
	}

	return false;
}



///	CloudPTS	--------------------------------------------------------


Crul::CloudPTS::CloudPTS ()
	:
	m_extent	()
{
}

void Crul::CloudPTS::clear ()
{
	m_points.clear ();
	m_extent.clear ();
}

Crul::PtsExtent const * Crul::CloudPTS::extents_ready () const
{
	if ( m_extent.is_clear () )
	{
		return NULL;
	}

	return & m_extent;
}

Crul::PtsExtent const * Crul::CloudPTS::extents_calculate ()
{
	size_t const tot = m_points.size ();

	if ( tot < 1 )
	{
		return & m_extent;
	}

	mtGin::GL::VertexRGB const * const src = m_points.data ();

	m_extent.x_min = src[0].x;
	m_extent.x_max = src[0].x;

	m_extent.y_min = src[0].y;
	m_extent.y_max = src[0].y;

	m_extent.z_min = src[0].z;
	m_extent.z_max = src[0].z;

	m_extent.r_min = src[0].r;
	m_extent.r_max = src[0].r;

	m_extent.g_min = src[0].g;
	m_extent.g_max = src[0].g;

	m_extent.b_min = src[0].b;
	m_extent.b_max = src[0].b;

	for ( size_t i = 1; i < tot; i++ )
	{
		m_extent.x_min = MIN ( src[i].x, m_extent.x_min );
		m_extent.x_max = MAX ( src[i].x, m_extent.x_max );

		m_extent.y_min = MIN ( src[i].y, m_extent.y_min );
		m_extent.y_max = MAX ( src[i].y, m_extent.y_max );

		m_extent.z_min = MIN ( src[i].z, m_extent.z_min );
		m_extent.z_max = MAX ( src[i].z, m_extent.z_max );

		m_extent.r_min = MIN ( src[i].r, m_extent.r_min );
		m_extent.r_max = MAX ( src[i].r, m_extent.r_max );

		m_extent.g_min = MIN ( src[i].g, m_extent.g_min );
		m_extent.g_max = MAX ( src[i].g, m_extent.g_max );

		m_extent.b_min = MIN ( src[i].b, m_extent.b_min );
		m_extent.b_max = MAX ( src[i].b, m_extent.b_max );
	}

	return & m_extent;
}

