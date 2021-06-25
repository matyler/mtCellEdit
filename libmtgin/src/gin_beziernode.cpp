/*
	Copyright (C) 2019-2021 Mark Tyler

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

#include <math.h>

#include "mtgin.h"



void mtGin::BezierNode::set (
	Vertex	const	& p1,
	Vertex	const	& cp1,
	Vertex	const	& p2,
	Vertex	const	& cp2
	)
{
	m_p1 = p1;
	m_cp1 = cp1;

	m_p2 = p2;
	m_cp2 = cp2;

// FIXME
//	create_cache ( 1 << 14 );	// High quality = slow
	create_cache ( 1 << 10 );	// Medium quality = ok
//	create_cache ( 1 << 7 );	// Low quality = fast
}

void mtGin::BezierNode::get_position (
	double	const	tm,
	Vertex		& v
	) const
{
	v.bezier ( tm, m_p1, m_cp1, m_p2, m_cp2 );
}

int mtGin::BezierNode::get_length (
	double	const	tm,
	double		& len
	) const
{
	if ( tm < 0.0 || tm > 1.0 || isinf ( tm ) || isnan ( tm ) )
	{
		return 1;
	}

	size_t a = 0, b, c = m_cache.size () - 1;

	for ( b = (a + c)/2; a < c; b = (a + c)/2 )
	{
		double const time_b = m_cache[ b ].m_tm;

		if ( tm == time_b )
		{
			break;
		}
		else if ( tm < time_b )
		{
			if ( b < 1 )
			{
				break;
			}

			c = b - 1;
		}
		else // tm > time_b
		{
			a = b + 1;
		}
	}

	if ( tm != m_cache[ b ].m_tm )
	{
		size_t bb1, bb2;

		if ( 0 == b )
		{
			bb1 = 0;
			bb2 = 1;
		}
		else if ( (m_cache.size () - 1) == b )
		{
			bb1 = b - 1;
			bb2 = b;
		}
		else
		{
			if ( tm > m_cache[ b ].m_tm )
			{
				bb1 = b;
				bb2 = b + 1;
			}
			else
			{
				bb1 = b - 1;
				bb2 = b;
			}
		}

		double const perc = (tm - m_cache[ bb1 ].m_tm) /
			(m_cache[ bb2 ].m_tm - m_cache[ bb1 ].m_tm);

		len = (1.0 - perc) * m_cache[ bb1 ].m_len +
			perc * m_cache[ bb2 ].m_len;
	}
	else
	{
		len = m_cache[ b ].m_len;
	}

	return 0;
}

int mtGin::BezierNode::get_time (
	double	const	tm_start,
	double	const	length,
	double		& tm
	) const
{
	size_t const end = m_cache.size ();

	if (	end < 2							||
		tm_start < 0.0 || tm_start > 1.0			||
		length < 0.0 || isinf ( length ) || isnan ( length )
		)
	{
		std::cerr << "BezierNode::get_time - INSANITY\n";

		return 1;
	}

	if ( length == 0.0 )
	{
		tm = tm_start;
		return 0;
	}

	double len1;

	if ( get_length ( tm_start, len1 ) )
	{
		std::cerr << "BezierNode::get_time - bad get_length\n";

		return 1;
	}

	size_t n;

	for ( n = 1; n < end; n++ )
	{
		if ( m_cache[n].m_len >= (len1 + length) )
		{
			break;
		}
	}

	if ( n >= end )
	{
		std::cerr << "BezierNode::get_time - n Not found. "
			<< "END forced\n";

		n = end - 1;
	}

	if ( m_cache[n].m_len == m_cache[n-1].m_len )
	{
		tm = tm_start;
		return 0;
	}

	double const excess = (len1 + length) - m_cache[n-1].m_len;
	double const perc = excess / (m_cache[n].m_len - m_cache[n-1].m_len);

	tm = (1.0 - perc)*m_cache[n-1].m_tm + perc * m_cache[n].m_tm;

#ifdef DEBUGZ
//#ifdef DEBUG
	std::cout << "		excess=" << excess
		<< " perc=" << perc
		<< " tm=" << tm
		<< " len1=" << len1
		<< " m_cache[n-1].m_len=" << m_cache[n-1].m_len
		<< " m_cache[n].m_len=" << m_cache[n].m_len
		<< "\n";
#endif

	return 0;
}

int mtGin::BezierNode::get_closest_time (
	Vertex	const	& p,
	double		& tm,
	double		& distance
	) const
{
	size_t const end = m_cache.size ();

	if ( end < 1 )
	{
		return 1;
	}

	// Find the closest point in the cache

	tm = m_cache[0].m_tm;
	distance = m_cache[0].m_p.distance ( p );

	for ( size_t i = 1; i < end; i++ )
	{
		double const d = m_cache[i].m_p.distance ( p );

		if ( d < distance )
		{
			distance = d;
			tm = m_cache[i].m_tm;
		}

		if ( distance == 0.0 )
		{
			return 0;
		}
	}

	double const span = 0.5 / (double)end;
	double a_tm, b_tm, c_tm, a_dist, b_dist, c_dist;
	Vertex est;

	if ( 0 == tm )
	{
		a_tm = tm;
		a_dist = distance;

		c_tm = tm + span;
		get_position ( c_tm, est );
		c_dist = est.distance ( p );

		b_tm = (a_tm + c_tm) / 2.0;
		get_position ( b_tm, est );
		b_dist = est.distance ( p );
	}
	else if ( 1 == tm )
	{
		a_tm = tm - span;
		get_position ( a_tm, est );
		a_dist = est.distance ( p );

		c_tm = tm;
		c_dist = distance;

		b_tm = (a_tm + c_tm) / 2.0;
		get_position ( b_tm, est );
		b_dist = est.distance ( p );
	}
	else
	{
		a_tm = tm - span;
		get_position ( a_tm, est );
		a_dist = est.distance ( p );

		c_tm = tm + span;
		get_position ( c_tm, est );
		c_dist = est.distance ( p );

		b_tm = tm;
		b_dist = distance;
	}

	// Drill in by 8 binary iterations to find a closer point than the cache
	for ( int i = 0; i < 8; i++ )
	{
		if ( a_dist < c_dist )
		{
			// Drill into A/B
			c_tm = b_tm;
			c_dist = b_dist;
		}
		else
		{
			// Drill into B/C
			a_tm = b_tm;
			a_dist = b_dist;
		}

		b_tm = (a_tm + c_tm) / 2.0;
		get_position ( b_tm, est );
		b_dist = est.distance ( p );

		if ( b_dist < distance )
		{
			distance = b_dist;
			tm = b_tm;
		}
	}

	return 0;
}

void mtGin::BezierNode::create_cache ( int const tot )
{
	size_t const sz = (size_t)tot;
	m_cache.clear ();
	m_cache.reserve ( sz + 1 );

	for ( size_t i = 0; i <= sz; i++ )
	{
		double	const	tm = (double)i / (double)tot;
		Vertex		p;
		double		len = 0;

		get_position ( tm, p );

		if ( i > 0 )
		{
			BezierCache const &cache = m_cache[ i - 1 ];
			Vertex const p2 = cache.m_p;

			len = cache.m_len + p.distance ( p2 );
		}

		BezierCache const bc ( p, tm, len );

		m_cache.push_back ( bc );
	}
}

