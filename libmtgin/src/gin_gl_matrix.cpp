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

#include "private.h"


/*
NOTE: algorithms used from Qt6 sources used as a guide to some optimizations.
qmatrix4x4.cpp
https://www.qt.io/
*/


void mtGin::GL::Matrix3x3::set_identity ()
{
	m_data[0][0] = m_data[1][1] = m_data[2][2] = 1;

	m_data[0][1] = m_data[0][2] =
	m_data[1][0] = m_data[1][2] =
	m_data[2][0] = m_data[2][1] =
		0.0f;
}



/// ----------------------------------------------------------------------------



void mtGin::GL::Matrix4x4::set_identity ()
{
	m_data[0][0] = m_data[1][1] = m_data[2][2] = m_data[3][3] = 1;

	m_data[0][1] = m_data[0][2] = m_data[0][3] =
	m_data[1][0] = m_data[1][2] = m_data[1][3] =
	m_data[2][0] = m_data[2][1] = m_data[2][3] =
	m_data[3][0] = m_data[3][1] = m_data[3][2] =
		0.0f;
}

void mtGin::GL::Matrix4x4::copy_to_doubles ( Array4x4_double & dest ) const
{
	for ( int i = 0; i < 4; i++ )
	{
		for ( int j = 0; j < 4; j++ )
		{
			dest[i][j] = m_data[i][j];
		}
	}
}


namespace {

static inline double matrix_det_2x2 (
	mtGin::GL::Array4x4_double const m,
	int		const	col0,
	int		const	col1,
	int		const	row0,
	int		const	row1
	)
{
	return m[col0][row0] * m[col1][row1] - m[col0][row1] * m[col1][row0];
}

static inline double matrix_det_3x3 (
	mtGin::GL::Array4x4_double const m,
	int		const	col0,
	int		const	col1,
	int		const	col2,
	int		const	row0,
	int		const	row1,
	int		const	row2
	)
{
	return m[col0][row0] * matrix_det_2x2 ( m, col1, col2, row1, row2 )
		- m[col1][row0] * matrix_det_2x2 ( m, col0, col2, row1, row2 )
		+ m[col2][row0] * matrix_det_2x2 ( m, col0, col1, row1, row2 );
}

} // namespace



mtGin::GL::Matrix3x3 mtGin::GL::Matrix4x4::normal () const
{
	Matrix3x3 matrix;

	Array4x4_double d;
	copy_to_doubles ( d );

	double det = matrix_det_3x3 ( d, 0, 1, 2, 0, 1, 2 );

	if ( det == 0.0 )
	{
		return matrix;
	}

	det = 1.0 / det;

	Array3x3_float & i = matrix.data();

	// Invert and transpose
	i[0][0] = GLfloat( (d[1][1] * d[2][2] - d[2][1] * d[1][2]) * det );
	i[0][1] = GLfloat(-(d[1][0] * d[2][2] - d[1][2] * d[2][0]) * det );
	i[0][2] = GLfloat( (d[1][0] * d[2][1] - d[1][1] * d[2][0]) * det );

	i[1][0] = GLfloat(-(d[0][1] * d[2][2] - d[2][1] * d[0][2]) * det );
	i[1][1] = GLfloat( (d[0][0] * d[2][2] - d[0][2] * d[2][0]) * det );
	i[1][2] = GLfloat(-(d[0][0] * d[2][1] - d[0][1] * d[2][0]) * det );

	i[2][0] = GLfloat( (d[0][1] * d[1][2] - d[0][2] * d[1][1]) * det );
	i[2][1] = GLfloat(-(d[0][0] * d[1][2] - d[0][2] * d[1][0]) * det );
	i[2][2] = GLfloat( (d[0][0] * d[1][1] - d[1][0] * d[0][1]) * det );

	return matrix;
}

void mtGin::GL::Matrix4x4::rotate (
	GLfloat		angle,
	GLfloat		x,
	GLfloat		y,
	GLfloat		z
	)
{
	angle = fmodf ( angle, 360 );
	if ( angle < 0.0f )
	{
		angle += 360.0f;
	}

	if ( angle == 0.0f )
	{
		return;
	}

	GLfloat c, s;

	if ( angle == 90.0f )
	{
		s = 1.0f;
		c = 0.0f;
	}
	else if ( angle == 180.0f )
	{
		s = 0.0f;
		c = -1.0f;
	}
	else if ( angle == 270.0f )
	{
		s = -1.0f;
		c = 0.0f;
	}
	else
	{
		c = (GLfloat)cos ( ((double)angle) / 180.0 * M_PI );
		s = (GLfloat)sin ( ((double)angle) / 180.0 * M_PI );
	}

	if ( x == 0.0f )
	{
		if ( y == 0.0f )
		{
			if ( z != 0.0f )
			{
				// Rotate around Z axis
				if ( z < 0.0f )
				{
					s = -s;
				}

				GLfloat tmp;

				tmp = m_data[0][0];
				m_data[0][0] = tmp * c + m_data[1][0] * s;
				m_data[1][0] = m_data[1][0] * c - tmp * s;

				tmp = m_data[0][1];
				m_data[0][1] = tmp * c + m_data[1][1] * s;
				m_data[1][1] = m_data[1][1] * c - tmp * s;

				tmp = m_data[0][2];
				m_data[0][2] = tmp * c + m_data[1][2] * s;
				m_data[1][2] = m_data[1][2] * c - tmp * s;

				tmp = m_data[0][3];
				m_data[0][3] = tmp * c + m_data[1][3] * s;
				m_data[1][3] = m_data[1][3] * c - tmp * s;

				return;
			}
		}
		else if ( z == 0.0f )
		{
			// Rotate around Y axis
			if ( y < 0.0f )
			{
				s = -s;
			}

			GLfloat tmp;

			tmp = m_data[2][0];
			m_data[2][0] = tmp * c + m_data[0][0] * s;
			m_data[0][0] = m_data[0][0] * c - tmp * s;

			tmp = m_data[2][1];
			m_data[2][1] = tmp * c + m_data[0][1] * s;
			m_data[0][1] = m_data[0][1] * c - tmp * s;

			tmp = m_data[2][2];
			m_data[2][2] = tmp * c + m_data[0][2] * s;
			m_data[0][2] = m_data[0][2] * c - tmp * s;

			tmp = m_data[2][3];
			m_data[2][3] = tmp * c + m_data[0][3] * s;
			m_data[0][3] = m_data[0][3] * c - tmp * s;

			return;
		}
	}
	else if ( y == 0.0f && z == 0.0f )
	{
		// Rotate around X axis
		if ( x < 0.0f )
		{
			s = -s;
		}

		GLfloat tmp;

		tmp = m_data[1][0];
		m_data[1][0] = tmp * c + m_data[2][0] * s;
		m_data[2][0] = m_data[2][0] * c - tmp * s;

		tmp = m_data[1][1];
		m_data[1][1] = tmp * c + m_data[2][1] * s;
		m_data[2][1] = m_data[2][1] * c - tmp * s;

		tmp = m_data[1][2];
		m_data[1][2] = tmp * c + m_data[2][2] * s;
		m_data[2][2] = m_data[2][2] * c - tmp * s;

		tmp = m_data[1][3];
		m_data[1][3] = tmp * c + m_data[2][3] * s;
		m_data[2][3] = m_data[2][3] * c - tmp * s;

		return;
	}

	double len =
		double(x) * double(x) +
		double(y) * double(y) +
		double(z) * double(z);

	if ( len != 0.0 && len != 1.0 )
	{
		x = GLfloat( double(x) / len );
		y = GLfloat( double(y) / len );
		z = GLfloat( double(z) / len );
	}

	GLfloat const ic = 1.0f - c;

	m_data[0][0] = x * x * ic + c;
	m_data[1][0] = x * y * ic - z * s;
	m_data[2][0] = x * z * ic + y * s;
	m_data[3][0] = 0.0f;
	m_data[0][1] = y * x * ic + z * s;
	m_data[1][1] = y * y * ic + c;
	m_data[2][1] = y * z * ic - x * s;
	m_data[3][1] = 0.0f;
	m_data[0][2] = x * z * ic - y * s;
	m_data[1][2] = y * z * ic + x * s;
	m_data[2][2] = z * z * ic + c;
	m_data[3][2] = 0.0f;
	m_data[0][3] = 0.0f;
	m_data[1][3] = 0.0f;
	m_data[2][3] = 0.0f;
	m_data[3][3] = 1.0f;
}

void mtGin::GL::Matrix4x4::translate (
	GLfloat	const	x,
	GLfloat	const	y,
	GLfloat	const	z
	)
{
	m_data[3][0] += m_data[0][0] * x + m_data[1][0] * y + m_data[2][0] * z;
	m_data[3][1] += m_data[0][1] * x + m_data[1][1] * y + m_data[2][1] * z;
	m_data[3][2] += m_data[0][2] * x + m_data[1][2] * y + m_data[2][2] * z;
	m_data[3][3] += m_data[0][3] * x + m_data[1][3] * y + m_data[2][3] * z;
}

void mtGin::GL::Matrix4x4::perspective (
	GLfloat	const	vert_angle,
	GLfloat	const	aspect_ratio,
	GLfloat	const	near,
	GLfloat	const	far
	)
{
	if ( near == far || aspect_ratio == 0.0f )
	{
		return;
	}

	GLfloat const r = vert_angle * GLfloat ( M_PI / 360.0 );
	GLfloat const s = sinf ( r );

	if ( s == 0.0f )
	{
		return;
	}

	GLfloat const cot = cosf ( r ) / s;
	GLfloat const clip = far - near;

	m_data[0][0] = cot / aspect_ratio;
	m_data[1][0] = 0.0f;
	m_data[2][0] = 0.0f;
	m_data[3][0] = 0.0f;
	m_data[0][1] = 0.0f;
	m_data[1][1] = cot;
	m_data[2][1] = 0.0f;
	m_data[3][1] = 0.0f;
	m_data[0][2] = 0.0f;
	m_data[1][2] = 0.0f;
	m_data[2][2] = -(near + far) / clip;
	m_data[3][2] = -(2.0f * near * far) / clip;
	m_data[0][3] = 0.0f;
	m_data[1][3] = 0.0f;
	m_data[2][3] = -1.0f;
	m_data[3][3] = 0.0f;
}

