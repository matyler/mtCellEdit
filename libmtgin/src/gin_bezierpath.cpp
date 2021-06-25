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

#include "mtgin.h"



void mtGin::BezierPath::add (
	Vertex	const	& p2,
	Vertex	const	& cp1,
	Vertex	const	& cp2
	)
{
	size_t const i = m_node.size ();

	m_node.resize ( i + 1 );

	Vertex p1;

	if ( i == 0 )
	{
		p1 = p2;
	}
	else
	{
		p1 = m_node[i - 1].get_p2 ();
	}

	m_node[i].set ( p1, cp1, p2, cp2 );
}

double mtGin::BezierPath::get_length (
	size_t	const	node,
	double	const	a,
	double	const	b
	) const
{
	if ( node < m_node.size () )
	{
		double len_a, len_b;

		if (	0 == m_node[ node ].get_length ( a, len_a )	&&
			0 == m_node[ node ].get_length ( b, len_b )
			)
		{
			return abs (len_b - len_a);
		}
	}

	std::cerr << "BezierPath::get_length1 argument insanity\n";

	return 0.0;
}

double mtGin::BezierPath::get_length (
	size_t	const	node1,
	double	const	tm1,
	size_t	const	node2,
	double	const	tm2
	) const
{
	if ( node1 == node2 )
	{
		return get_length ( node1, tm1, tm2 );
	}
	else if ( node1 > node2 )
	{
		std::cerr << "BezierPath::get_length2 argument insanity\n";
		return 0.0;
	}

	// We have now proven that (node1 < node2)

	// First segment
	double len = get_length ( node1, tm1, 1.0 );

	// Middle segments (if any exist)
	for ( size_t n = node1 + 1; n < node2; n++ )
	{
		len += get_length ( n, 0.0, 1.0 );
	}

	// Last segment
	len += get_length ( node2, 0.0, tm2 );

	return len;
}

int mtGin::BezierPath::get_position (
	size_t	const	node,
	double	const	tm,
	Vertex		& v
	) const
{
	if ( node >= m_node.size () )
	{
		return 1;
	}

	m_node[ node ].get_position ( tm, v );

	return 0;
}

int mtGin::BezierPath::get_closest_time (
	Vertex	const	& p,
	size_t		& bez_node,
	double		& tm,
	size_t	const	bn
	) const
{
	size_t	const	end = m_node.size ();
	double		shortest = -1.0;

	for ( size_t i = bn; i < end; i++ )
	{
		double ct = 0.0;
		double dist = 0.0;

		if ( m_node[i].get_closest_time ( p, ct, dist ) )
		{
			continue;
		}

		if ( shortest < 0.0 || dist < shortest )
		{
			shortest = dist;
			bez_node = i;
			tm = ct;
		}
	}

	return (shortest < 0.0) ? 1 : 0;
}

int mtGin::BezierPath::get_relative_time (
	size_t	const	a_node,
	double	const	a_tm,
	size_t	const	b_node,
	double	const	b_tm,
	double	const	perc,
	size_t		& node,
	double		& tm
	) const
{
	size_t	const	end = m_node.size ();

	if (	a_node >= end	|| b_node >= end	||
		a_node > b_node				||
		(a_node == b_node && a_tm > b_tm)	||
		a_tm < 0.0	|| a_tm > 1.0		||
		b_tm < 0.0	|| b_tm > 1.0		||
		perc < 0.0	|| perc > 1.0
		)
	{
		std::cerr << "get_relative_time insanity:"
			<< " a_node=" << a_node
			<< " a_tm=" << a_tm
			<< " b_node=" << b_node
			<< " b_tm=" << b_tm
			<< " perc=" << perc
			<< "\n";

		return 1;
	}

	double const a_to_b = get_length ( a_node, a_tm, b_node, b_tm );
	double const len = a_to_b * perc;

	if ( get_relative_forward ( a_node, a_tm, len, node, tm ) )
	{
		return 1;
	}

	if ( node > b_node )
	{
		std::cerr << "get_relative_time node > b_node\n";
		return 1;
	}

	return 0;
}

int mtGin::BezierPath::get_relative_forward (
	size_t	const	a_node,
	double	const	a_tm,
	double	const	delta,
	size_t		& b_node,
	double		& b_tm
	) const
{
	size_t	const	end = m_node.size ();

	double done = 0;	// Distance so far travelled in loop
	size_t n;		// a_node <= n <= b_node
	double t;		// 0.0 <= n <= 1.0

	for (	n = a_node, t = a_tm;
		n < end;
		n++, t = 0.0
		)
	{
		double const remaining = get_length ( n, t, 1.0 );

#ifdef DEBUGZ
//#ifdef DEBUG
	std::cout << "	n=" << n
		<< " t=" << t
		<< " remaining=" << remaining
		<< "\n";
#endif

		if ( (done + remaining) >= delta )
		{
			// The ending in somewhere in this segment
			double const dist = delta - done;

			if ( m_node[n].get_time ( t, dist, t ) )
			{
				std::cerr << "get_relative_forward ERROR "
					"get_time"
					<< " n=" << n
					<< " t=" << t
					<< " dist=" << dist
					<< "\n";

				return 1;
			}

			done = delta;

			break;
		}

		done += remaining;
	}

	if ( done < delta )
	{
		std::cerr << "	get_relative_forward ERROR:"
			<< " delta=" << delta
			<< "\n";

		return 1;
	}

	b_node = n;
	b_tm = t;

	return 0;
}


namespace
{

static mtGin::Vertex turn_unitvector (
	mtGin::Vertex	const	& p1,
	mtGin::Vertex	const	& p3
	)
{
	mtGin::Vertex turn = p1 - p3;
	turn.normalize ();

	return turn;
}

}	// namespace

void mtGin::BezierPath::set_smooth_curve (
	std::vector<Vertex> const & vertices
	)
{
	size_t const tot = vertices.size();

	m_node.clear();

	if ( tot < 1 )
	{
		return;
	}

	// Add first node
	add ( vertices[0], vertices[0], vertices[0] );

	Vertex p1, p2, p3, p4;		// Points

	p2 = p3 = vertices[0];
	if ( tot > 1 )
	{
		p4 = vertices[1];
	}
	else
	{
		p4 = p3;
	}

	// Add remaining nodes
	for ( size_t v = 1; v < tot; ++v )
	{
		p1 = p2;
		p2 = p3;
		p3 = p4;

		double const p2_p3_len = p2.distance ( p3 );
		Vertex const tuv1 = ::turn_unitvector ( p1, p3 );
		Vertex const cp1 = p2 - tuv1 * (p2_p3_len/3.0);

		if ( (v+1) < tot )
		{
			p4 = vertices[v+1];
		}

		Vertex const tuv2 = ::turn_unitvector ( p2, p4 );
		Vertex const cp2 = p3 + tuv2 * (p2_p3_len/3.0);

		add ( p3, cp1, cp2 );
	}
}

