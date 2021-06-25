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

#include "lanter.h"



TerraGL::TerraGL ()
{
}

TerraGL::~TerraGL ()
{
}

void TerraGL::set_window_size (
	int	const	width,
	int	const	height,
	float	const	range
	)
{
	m_proj.set_identity ();
	m_proj.perspective ( 45.0f, float(width) / float(height), 0.1f, range );
}



class LandTriangles
{
public:
	LandTriangles () {}
	~LandTriangles () { clear(); }

	inline mtGin::GL::VertexRGBnormal const * get_data () const
		{ return m_data; }
	inline size_t get_vertex_count () const
		{ return m_vertex_count; }

	int init ( TerraMap const & map );

private:
	void clear ();

/// ----------------------------------------------------------------------------

	mtGin::GL::VertexRGBnormal	* m_data = nullptr;
	size_t				m_vertex_count = 0;
};

int LandTriangles::init ( TerraMap const & map )
{
	int const rows = (size_t)map.get_rows ();
	if ( rows < 2 )
	{
		std::cerr <<"Map not initialized, cannot build LandTriangles\n";
		return 1;
	}
	int const rows1 = rows - 1;

	clear ();

	// 4 triangles per square; 3 vertices per triangle.
	m_vertex_count = size_t(rows1 * rows1 * 3 * 4);
	m_data = (mtGin::GL::VertexRGBnormal *)calloc ( m_vertex_count,
		sizeof(m_data[0]));

	if ( ! m_data )
	{
		std::cerr <<"Unable to allocate LandTriangles buffer\n";
		return 1;
	}

	std::cout << "LandTriangles::init rows=" << rows
		<< " vertices=" << m_vertex_count
		<< " bytes=" << m_vertex_count * sizeof(m_data[0])
		<< "\n";

	mtGin::GL::VertexRGBnormal * dest = m_data;

	for ( int y = 0; y < rows1; y++ )
	{
		for ( int x = 0; x < rows1; x++ )
		{
			float const r = 0.2f;
			float const g = 0.5f;
			float const b = 0.4f;

			float const x0 = map.get_xy_real ( x );
			float const x2 = map.get_xy_real ( x+1 );
			float const y0 = map.get_xy_real ( y );
			float const y2 = map.get_xy_real ( y+1 );
			float const x1 = (x0+x2)/2;
			float const y1 = (y0+y2)/2;

			float const z00 = map.get_value_real ( x, y );
			float const z20 = map.get_value_real ( x+1, y );
			float const z22 = map.get_value_real ( x+1, y+1 );
			float const z02 = map.get_value_real ( x, y+1 );
			float const z11 = (z00 + z20 + z22 + z02) / 4;

			dest->set_xyz ( x0, y0, z00 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x2, y0, z20 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x1, y1, z11 );
			dest->set_rgb ( r, g, b );
			dest->calc_normal ( dest[-2], dest[-1] );
			dest++;

			dest->set_xyz ( x2, y0, z20 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x2, y2, z22 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x1, y1, z11 );
			dest->set_rgb ( r, g, b );
			dest->calc_normal ( dest[-2], dest[-1] );
			dest++;

			dest->set_xyz ( x2, y2, z22 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x0, y2, z02 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x1, y1, z11 );
			dest->set_rgb ( r, g, b );
			dest->calc_normal ( dest[-2], dest[-1] );
			dest++;

			dest->set_xyz ( x0, y2, z02 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x0, y0, z00 );
			dest->set_rgb ( r, g, b );
			dest++;
			dest->set_xyz ( x1, y1, z11 );
			dest->set_rgb ( r, g, b );
			dest->calc_normal ( dest[-2], dest[-1] );
			dest++;
		}
	}

	return 0;
}

void LandTriangles::clear ()
{
	free ( m_data );
	m_data = nullptr;
	m_vertex_count = 0;
}



class CubeLines
{
public:
	explicit CubeLines ( float size );

	mtGin::GL::VertexRGB const * get_data () const { return point; }
	static size_t get_data_size () { return point_tot; }

private:
	// Cube = 12 lines is 24 vertices
	static size_t const point_tot = 24;
	mtGin::GL::VertexRGB point[ point_tot ];
};

class CubeCorners
{
public:
	explicit CubeCorners ( GLfloat size, GLfloat r, GLfloat g, GLfloat b );

	mtGin::GL::VertexRGB point[ 8 ];
/*
  6-----7
 /|    /|
4-----5 |
| |   | |
| 2---|-3
|/    |/
0-----1
*/
};

CubeCorners::CubeCorners (
	GLfloat	const	size,
	GLfloat	const	r,
	GLfloat	const	g,
	GLfloat	const	b
	)
{
	for ( int i=0; i<8; i++ )
	{
		point[i].x = size * ((((i/1)%2)==0) ? -1 : 1);
		point[i].y = size * ((((i/2)%2)==0) ? -1 : 1);
		point[i].z = size * ((((i/4)%2)==0) ? -1 : 1);

		point[i].r = r;
		point[i].g = g;
		point[i].b = b;
	}
}

CubeLines::CubeLines ( float const size )
{
	CubeCorners corner ( size, 0.6f, 0.6f, 0.6f );

	mtGin::GL::VertexRGB * dest = point;

	// BOTTOM square
	*dest++ = corner.point[0];
	*dest++ = corner.point[1];
	*dest++ = corner.point[1];
	*dest++ = corner.point[3];
	*dest++ = corner.point[3];
	*dest++ = corner.point[2];
	*dest++ = corner.point[2];
	*dest++ = corner.point[0];

	// Middle supports
	for ( int i=0; i<4; i++ )
	{
		*dest++ = corner.point[i];
		*dest++ = corner.point[i+4];
	}

	// TOP square
	*dest++ = corner.point[4];
	*dest++ = corner.point[5];
	*dest++ = corner.point[5];
	*dest++ = corner.point[7];
	*dest++ = corner.point[7];
	*dest++ = corner.point[6];
	*dest++ = corner.point[6];
	*dest++ = corner.point[4];
}

void TerraGL::init_buffers ( TerraMap const & map )
{
	std::string const version ( mtGin::GL::get_shader_language_version() );

	m_gl_lines.init ( version );

	CubeLines cube ( GLfloat(LANDSCAPE_VIEW_RANGE)/2.0f );
	m_gl_lines.populate ( cube.get_data(), cube.get_data_size() );

	LandTriangles land_tri;
	if ( land_tri.init ( map ) )
	{
		return;
	}

	m_gl_triangles.init ( version );
	m_gl_triangles.populate ( land_tri.get_data(),
		land_tri.get_vertex_count() );

	std::vector<mtGin::Vertex> bezpo = {
		{-1000,	-500,	600 },
		{ 0,	-500,	700 },
		{ 0,	-100,	1000 },
		{ 500,	-500,	700 },
		{ 700,	-500,	900 }
		};

	mtGin::BezierPath bez;
	bez.set_smooth_curve ( bezpo );

	std::vector< mtGin::GL::VertexRGB > bezls;
	size_t const ntot = bez.get_size();

	if ( ntot > 1 )
	{
		mtGin::Vertex p1, p2, cp1, cp2;

		for ( size_t i = 1; i < ntot; i++ )
		{
			mtGin::BezierNode const * const n1 = bez.get_node ( i );
			n1->get ( p1, cp1, p2, cp2 );

			// Create line from point to point
			bezls.push_back ( { (GLfloat)p1.x, (GLfloat)p1.y,
				(GLfloat)p1.z, 0.7f, 0.7f, 0.5f } );
			bezls.push_back ( { (GLfloat)p2.x, (GLfloat)p2.y,
				(GLfloat)p2.z, 0.7f, 0.7f, 0.5f } );

			// Create lines depicting vertex control points
			bezls.push_back ( { (GLfloat)p1.x, (GLfloat)p1.y,
				(GLfloat)p1.z, 0.3f, 0.3f, 1.0f } );
			bezls.push_back ( { (GLfloat)cp1.x, (GLfloat)cp1.y,
				(GLfloat)cp1.z, 0.3f, 0.3f, 1.0f } );

			bezls.push_back ( { (GLfloat)p2.x, (GLfloat)p2.y,
				(GLfloat)p2.z, 0.3f, 0.3f, 1.0f } );
			bezls.push_back ( { (GLfloat)cp2.x, (GLfloat)cp2.y,
				(GLfloat)cp2.z, 0.3f, 0.3f, 1.0f } );

			// Create smooth bezier with 100 subdivisions
			for ( int tm = 0; tm < 100; tm++ )
			{
				if (	bez.get_position ( i, tm/100.0, p1 )
					|| bez.get_position(i, (tm+1)/100.0, p2)
					)
				{
					continue;
				}

				bezls.push_back ( {(GLfloat)p1.x, (GLfloat)p1.y,
					(GLfloat)p1.z, 1.0f, 0.5f, 1.0f } );
				bezls.push_back ( {(GLfloat)p2.x, (GLfloat)p2.y,
					(GLfloat)p2.z, 1.0f, 0.5f, 1.0f } );
			}
		}

		m_gl_lines_bez.init ( version );
		m_gl_lines_bez.populate ( bezls.data(), bezls.size() );
	}
}

void TerraGL::set_light (
	double	const	x,
	double	const	y,
	double	const	z
	)
{
	m_light.x = x;
	m_light.y = y;
	m_light.z = z;
}

void TerraGL::render (
	mtGin::GL::Matrix4x4 const	& camera
	) const
{
	m_gl_triangles.render ( camera, m_proj, m_light );
	m_gl_lines.render ( camera, m_proj );
	m_gl_lines_bez.render ( camera, m_proj );
}

void TerraGL::render_pyramid (
	GLfloat	const	x,
	GLfloat	const	y,
	GLfloat	const	z,
	GLfloat	const	size
	)
{
	GLfloat const s = size * GLfloat(1.5);

	glBegin ( GL_LINES );
	glVertex3f( x + size, y + size, z ); glVertex3f( x + size, y - size, z);
	glVertex3f( x + size, y - size, z ); glVertex3f( x - size, y - size, z);
	glVertex3f( x - size, y - size, z ); glVertex3f( x - size, y + size, z);
	glVertex3f( x - size, y + size, z ); glVertex3f( x + size, y + size, z);
	glEnd ();

	glBegin ( GL_LINES );
	glVertex3f ( x + size, y + size, z ); glVertex3f ( x, y, z + s );
	glVertex3f ( x, y, z + s ); glVertex3f ( x - size, y - size, z );
	glEnd ();

	glBegin ( GL_LINES );
	glVertex3f ( x + size, y - size, z ); glVertex3f ( x, y, z + s );
	glVertex3f ( x, y, z + s ); glVertex3f ( x - size, y + size, z );
	glEnd ();
}

void TerraGL::render_light () const
{
	glColor3f ( 1.0f, 1.0f, 0.0f );
	render_pyramid ( GLfloat(m_light.x), GLfloat(m_light.y),
		GLfloat(m_light.z), 100.0f );
}

void TerraGL::render_axis ()
{
	float const s = LANDSCAPE_VIEW_RANGE * 2;

	glColor3f ( 1.0f, 0.0f, 0.0f );
	glBegin ( GL_LINES );
	glVertex3f ( -s, 0, 0 ); glVertex3f ( s, 0, 0);
	glEnd ();

	glColor3f ( 0.0f, 1.0f, 0.0f );
	glBegin ( GL_LINES );
	glVertex3f ( 0, -s, 0 ); glVertex3f ( 0, s, 0);
	glEnd ();

	glColor3f ( 0.0f, 0.0f, 1.0f );
	glBegin ( GL_LINES );
	glVertex3f ( 0, 0, -s ); glVertex3f ( 0, 0, s);
	glEnd ();
}

