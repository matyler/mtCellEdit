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

#include "mtpts2ply.h"



pCloud::pCloud ()
	:
	m_output_filename ( NULL ),
	m_ltd		( 0 ),
	m_lim		( 0.0 ),
	m_lim_half	( 0.0 ),
	m_format	( FORMAT_PLY ),
	m_slices	( 1 )
{
}

pCloud::~pCloud ()
{
}

int pCloud::load ( mtKit::LineFileRead & file )
{
	while ( 1 )
	{
		if ( file.read_line () )
		{
			break;
		}

		double	x = 0.0;
		double	y = 0.0;
		double	z = 0.0;
		int	intensity = 0;
		int	r = 0;
		int	g = 0;
		int	b = 0;

		if (	file.get_double ( x )
			|| file.get_double ( y )
			|| file.get_double ( z )
			|| file.get_int ( intensity )
			|| file.get_int ( r )
			|| file.get_int ( g )
			|| file.get_int ( b )
			)
		{
			continue;
		}

		if ( m_ltd )
		{
			x = xyz_sample ( x );
			y = xyz_sample ( y );
			z = xyz_sample ( z );
		}

		add_item ( x, y, z, intensity, r, g, b );
	}

	return 0;
}

int pCloud::add_item (
	double	const	x,
	double	const	y,
	double	const	z,
	int	const	intensity,
	int	const	r,
	int	const	g,
	int	const	b
	)
{
	try
	{
		pCloudNkey key ( x, y, z );
		std::map<pCloudNkey, pCloudNdata>::iterator it =m_map.find(key);

		if ( it == m_map.end () )
		{
			m_map.insert ( std::pair<pCloudNkey, pCloudNdata>(
				key, pCloudNdata ( intensity, r, g, b ) ) );
		}
		else
		{
			it->second.add ( intensity, r, g, b );
		}
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

int pCloud::set_limit ( int const n )
{
	if ( n < LIMIT_MIN || n > LIMIT_MAX )
	{
		std::cerr << "Error - pCloud::set_limit n = " << n << "\n";
		return 1;
	}

	m_lim = pow ( 2.0, (double)n );
	m_lim_half = m_lim / 2;
	m_ltd = 1;

	return 0;
}

int pCloud::set_format ( int const n )
{
	if ( n < FORMAT_MIN || n > FORMAT_MAX )
	{
		std::cerr << "Error - pCloud::set_format n = " << n << "\n";
		return 1;
	}

	m_format = n;

	return 0;
}

double pCloud::xyz_sample ( double const xyz ) const
{
	if ( 0 == m_ltd )
	{
		return xyz;
	}

	double const m = fmod ( xyz, m_lim );
	double delta;

	if ( fabs(m) <= m_lim_half )
	{
		delta = -m;
	}
	else
	{
		delta = m_lim - fabs(m);

		if ( m < 0 )
		{
			delta = -delta;
		}
	}

	return xyz + delta;
}

void pCloud::set_output_filename ( char const * const filename )
{
	m_output_filename = filename;
}

int pCloud::set_slices ( int const slices )
{
	if ( slices >= SLICES_MIN && slices <= SLICES_MAX )
	{
		m_slices = slices;
		return 0;
	}

	return 1;
}

void pCloud::print_data ()
{
	if ( m_output_filename )
	{
		switch ( m_format )
		{
		case FORMAT_PLY:	print_ply_file ();	break;
		case FORMAT_PTS:	print_pts_file ();	break;
		}
	}
	else
	{
		switch ( m_format )
		{
		case FORMAT_PLY:	print_ply_stdout ();	break;
		case FORMAT_PTS:	print_pts_stdout ();	break;
		}
	}
}



/// ----------------------------------------------------------------------------



pCloudNkey::pCloudNkey (
	double	const	x,
	double	const	y,
	double	const	z
	)
	:
	m_x		(x),
	m_y		(y),
	m_z		(z)
{
}

bool pCloudNkey::operator () (
	pCloudNkey	const	&a,
	pCloudNkey	const	&b
	) const
{
	if ( a.m_x < b.m_x ) { return true; }
	if ( a.m_x > b.m_x ) { return false; }

	if ( a.m_y < b.m_y ) { return true; }
	if ( a.m_y > b.m_y ) { return false; }

	if ( a.m_z < b.m_z ) { return true; }
//	if ( a.m_z > b.m_z ) { return false; }

	return false;
}



/// ----------------------------------------------------------------------------



pCloudNdata::pCloudNdata (
	int	const	intensity,
	int	const	r,
	int	const	g,
	int	const	b
	)
	:
	m_intensity	( intensity ),
	m_r		( r ),
	m_g		( g ),
	m_b		( b ),
	m_count		( 1 )
{
}

void pCloudNdata::add (
	int	const	intensity,
	int	const	r,
	int	const	g,
	int	const	b
	)
{
	m_intensity += intensity;
	m_r += r;
	m_g += g;
	m_b += b;

	m_count ++;
}

