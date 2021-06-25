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



Crul::Cloud::Cloud ()
	:
	m_cpts		( & m_cpts_low )
{
	set_resampling_rates ( 0, -2 );
}

Crul::Cloud::~Cloud ()
{
}

void Crul::Cloud::clear ()
{
	m_cpts_low.clear ();
	m_cpts_medium.clear ();
	m_cpts_high.clear ();
}

void Crul::Cloud::set_resampling_rates (
	int	const	low,
	int	const	medium
	)
{
	if (	low < SAMPLE_RATE_MIN || low > SAMPLE_RATE_MAX		||
		medium < SAMPLE_RATE_MIN || medium > SAMPLE_RATE_MAX	||
		low < medium
		)
	{
		m_rate_low = 0;
		m_rate_medium = -2;
	}
	else
	{
		m_rate_low = low;
		m_rate_medium = medium;
	}
}

int Crul::Cloud::set_resolution (
	int		const	type,
	Crul::DB	* const db
	)
{
	switch ( type )
	{
	case DB::CACHE_TYPE_HIGH:	m_cpts = & m_cpts_high;		break;
	case DB::CACHE_TYPE_MEDIUM:	m_cpts = & m_cpts_medium;	break;
	case DB::CACHE_TYPE_LOW:	m_cpts = & m_cpts_low;		break;
	default:
		return 1;
	}

	if ( m_cpts->size () > 0 )
	{
		// Cloud is already populated so nothing to do
		return 0;
	}

	return db->load_cache ( type, m_cpts->get_points () );
}



///	PTS LOAD	--------------------------------------------------------



int Crul::Cloud::load_pts (
	std::string	const	& filename,
	DB		* const	db
	)
{
	mtKit::LineFileRead file;

	if ( file.open ( filename ) )
	{
		std::cerr << "Unable to open file '" << filename << "'\n";
		return ERROR_PTS_IMPORT;
	}

	int res = 0;

	try
	{
		clear ();

		db->clear_cache ();

		std::vector<mtGin::GL::VertexRGB> * cloud_high =
			m_cpts_high.get_points ();

		while ( 1 )
		{
			if ( file.read_line () )
			{
				break;
			}

			mtGin::GL::VertexRGB xyzrgb;

			double	x, y, z;
			int	intensity, r, g, b;

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

			xyzrgb.x = (GLfloat)x;
			xyzrgb.y = (GLfloat)y;
			xyzrgb.z = (GLfloat)z;
			xyzrgb.r = (GLfloat)(r / 255.0);
			xyzrgb.g = (GLfloat)(g / 255.0);
			xyzrgb.b = (GLfloat)(b / 255.0);

			cloud_high->push_back ( xyzrgb );
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
		if ( resample_cloud ( m_cpts_low.get_points (), m_rate_low )

			|| resample_cloud ( m_cpts_medium.get_points (),
				m_rate_medium )

			|| db->save_cache ( DB::CACHE_TYPE_HIGH,
				m_cpts_high.get_points () )

			|| db->save_cache ( DB::CACHE_TYPE_MEDIUM,
				m_cpts_medium.get_points () )

			|| db->save_cache ( DB::CACHE_TYPE_LOW,
				m_cpts_low.get_points () )
			)
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



///	RESAMPLING	--------------------------------------------------------



class pCloud;
class pCloudNkey;
class pCloudNdata;



class pCloudNkey
{
public:
	pCloudNkey (
		double	x = 0.0,
		double	y = 0.0,
		double	z = 0.0
		)
		:
		m_x	( x ),
		m_y	( y ),
		m_z	( z )
	{}

	bool operator () ( pCloudNkey const &a, pCloudNkey const &b ) const;

	inline double get_x () const { return m_x; }
	inline double get_y () const { return m_y; }
	inline double get_z () const { return m_z; }

private:
	double		m_x, m_y, m_z;
};



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



class pCloudNdata
{
public:
	pCloudNdata (
		double	r = 0,
		double	g = 0,
		double	b = 0
		)
		:
		m_r	( r ),
		m_g	( g ),
		m_b	( b ),
		m_count	( 1 )
	{}

	void add (
		double	r,
		double	g,
		double	b
		);

	inline double get_r () const { return m_count > 0 ? m_r / m_count : 0; }
	inline double get_g () const { return m_count > 0 ? m_g / m_count : 0; }
	inline double get_b () const { return m_count > 0 ? m_b / m_count : 0; }

private:
	double		m_r, m_g, m_b;
	int		m_count;
};



void pCloudNdata::add (
	double	const	r,
	double	const	g,
	double	const	b
	)
{
	m_r += r;
	m_g += g;
	m_b += b;

	m_count ++;
}



class pCloud
{
public:
	explicit pCloud ( int limit );

	int add_item (
		double x,
		double y,
		double z,
		double r,
		double g,
		double b
		);
	int to_vector ( std::vector<mtGin::GL::VertexRGB> * const dest );

	double xyz_sample ( double xyz ) const;

private:
	std::map<pCloudNkey, pCloudNdata, pCloudNkey> m_map;

	int	const	m_ltd;	// 0=Don't use m_lim, store x,y,z verbatim
	double	const	m_lim;
	double	const	m_lim_half;	// Cache of (m_lim / 2)
};



pCloud::pCloud ( int const limit )
	:
	m_ltd		( 1 ),
	m_lim		( pow ( 2.0, (double)limit ) ),
	m_lim_half	( m_lim / 2 )
{
}

int pCloud::add_item (
	double const x,
	double const y,
	double const z,
	double const r,
	double const g,
	double const b
	)
{
	try
	{
		pCloudNkey key ( x, y, z );
		std::map<pCloudNkey, pCloudNdata>::iterator it =m_map.find(key);

		if ( it == m_map.end () )
		{
			m_map.insert ( std::pair<pCloudNkey, pCloudNdata>(
				key, pCloudNdata ( r, g, b ) ) );
		}
		else
		{
			it->second.add ( r, g, b );
		}
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

double pCloud::xyz_sample ( double const xyz ) const
{
	if ( 0 == m_ltd )
	{
		return (GLfloat)xyz;
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

	return (GLfloat)(xyz + delta);
}

int pCloud::to_vector ( std::vector<mtGin::GL::VertexRGB> * const dest )
{
	mtGin::GL::VertexRGB rec;

	std::map<pCloudNkey, pCloudNdata>::iterator it;

	for ( it = m_map.begin (); it != m_map.end (); ++it )
	{
		pCloudNkey const * const key = &it->first;
		pCloudNdata const * const data = &it->second;

		rec.x = (GLfloat)key->get_x ();
		rec.y = (GLfloat)key->get_y ();
		rec.z = (GLfloat)key->get_z ();

		rec.r = (GLfloat)data->get_r ();
		rec.g = (GLfloat)data->get_g ();
		rec.b = (GLfloat)data->get_b ();

		dest->push_back ( rec );
	}

	return 0;
}



int Crul::Cloud::resample_cloud (
	std::vector<mtGin::GL::VertexRGB> * const dest,
	int	const	rate
	)
{
	pCloud map ( rate );

	// Pack cloud points into resampled map

	std::vector<mtGin::GL::VertexRGB> const & cloud =
		* m_cpts_high.get_points ();

	for ( auto && v : cloud )
	{
		if ( map.add_item (
			map.xyz_sample ( v.x ),
			map.xyz_sample ( v.y ),
			map.xyz_sample ( v.z ),
			v.r, v.g, v.b )
			)
		{
			std::cerr << "resample_cloud: Unable to add_item\n";
			return 1;
		}
	}

	return map.to_vector ( dest );
}

