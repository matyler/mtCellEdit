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



Crul::Model::Model ()
{
}

Crul::Model::~Model ()
{
}

int Crul::Model::import_file (
	std::string	const	& filename,
	DB		* const	db
	)
{
	mtKit::LineFileRead file;

	if ( file.open ( filename ) )
	{
		std::cerr << "Unable to open file '" << filename << "'\n";
		return ERROR_MODEL_IMPORT;
	}

	int res = 0;

	try
	{
		m_pts.clear ();
		db->clear_model ();

		while ( 1 )
		{
			if ( file.read_line () )
			{
				break;
			}

			double	x1, y1, z1;
			double	x2, y2, z2;
			double	x3, y3, z3;
			double	intensity;
			int	r, g, b;

			if (	file.get_double ( x1 )
				|| file.get_double ( y1 )
				|| file.get_double ( z1 )
				|| file.get_double ( x2 )
				|| file.get_double ( y2 )
				|| file.get_double ( z2 )
				|| file.get_double ( x3 )
				|| file.get_double ( y3 )
				|| file.get_double ( z3 )
				|| file.get_double ( intensity )
				|| file.get_int ( r )
				|| file.get_int ( g )
				|| file.get_int ( b )
				)
			{
				continue;
			}

			VertexGL p1, p2, p3;

			p1.x = GLfloat (x1);
			p1.y = GLfloat (y1);
			p1.z = GLfloat (z1);
			p1.r = GLfloat (r / 255.0);
			p1.g = GLfloat (g / 255.0);
			p1.b = GLfloat (b / 255.0);

			p2.x = GLfloat (x2);
			p2.y = GLfloat (y2);
			p2.z = GLfloat (z2);
			p2.set_rgb ( p1 );

			p3.x = GLfloat (x3);
			p3.y = GLfloat (y3);
			p3.z = GLfloat (z3);
			p3.set_rgb ( p1 );

			p1.normal ( p2, p3 );

			m_pts.push_back ( p1 );
			m_pts.push_back ( p2 );
			m_pts.push_back ( p3 );
		}
	}
	catch (...)
	{
		res = ERROR_EXCEPTION;
	}

	if ( res )
	{
		return res;
	}

	try
	{
		if ( db->save_model ( m_pts ) )
		{
			return ERROR_PTS_IMPORT;
		}
	}
	catch (...)
	{
		return ERROR_EXCEPTION;
	}

	return 0;
}

int Crul::Model::load_db_pts ( DB * const db )
{
	return db->load_model ( m_pts );
}

