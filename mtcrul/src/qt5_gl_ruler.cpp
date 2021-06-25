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

#include "qt5_cloudview.h"



RulerQtGL::RulerQtGL ()
	:
	m_active_ruler_id	( 0 ),
	m_show_lines		( true ),
	m_show_plane		( true ),
	m_line_butt_size	( 1.0 ),
	m_line_thickness	( 1.0 )
{
}

RulerQtGL::~RulerQtGL ()
{
	destroy ();
}

void RulerQtGL::destroy ()
{
	m_triangles.destroy ();
	m_lines.destroy ();
}

void RulerQtGL::init ( std::string const & version )
{
	m_triangles.init ( version );
	m_lines.init ( version );
}

void RulerQtGL::populate (
	std::map<int, Crul::Ruler> const & map,
	double		const	plane_range
	)
{
	// Setup our vertex buffer object.
	std::vector<mtGin::GL::VertexRGBnormal> triangles;
	std::vector<mtGin::GL::VertexRGB> lines;

	Crul::Ruler const * plane = NULL;

	for ( auto && i : map )
	{
		Crul::Ruler const & ruler = i.second;

		if ( ! ruler.get_visible () )
		{
			continue;		// Skip invisible rulers
		}

		if ( m_show_lines )
		{
			Crul::Line const & line = ruler.get_line ();
			mtGin::GL::VertexRGB vertex;

			vertex.x = line.x1;
			vertex.y = line.y1;
			vertex.z = line.z1;
			vertex.r = line.r;
			vertex.g = line.g;
			vertex.b = line.b;

			lines.push_back ( vertex );

			vertex.x = line.x2;
			vertex.y = line.y2;
			vertex.z = line.z2;

			lines.push_back ( vertex );
		}

		if ( ruler.get_id() == m_active_ruler_id )
		{
			ruler.create_ends_gl ( triangles, m_line_butt_size );

			if ( m_show_plane )
			{
				plane = &ruler;
			}
		}
	}

	if ( plane )
	{
		// Draw the transparent plane last to avoid blending issues
		plane->create_plane_gl ( lines, plane_range );
	}

	m_triangles.populate ( triangles.data(), triangles.size() );
	m_lines.populate ( lines.data(), lines.size() );
}

void RulerQtGL::render (
	mtGin::GL::Matrix4x4 const & camera,
	mtGin::GL::Matrix4x4 const & proj,
	mtGin::Vertex	const &	light
	) const
{
	if ( ! is_visible () )
	{
		return;
	}

	glLineWidth ( GLfloat ( m_line_thickness ) );
	m_lines.render ( camera, proj );

	m_triangles.render ( camera, proj, light );
}

