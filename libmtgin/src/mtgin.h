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

#ifndef MTGIN_H_
#define MTGIN_H_

#include <mtkit.h>
#include <mtpixy.h>



//	mtGin = Mark Tyler's Graphical Interface Nexus



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API

namespace mtGin
{

class BezierCache;
class BezierNode;
class BezierPath;
class Vertex;



class Vertex
{
public:
	Vertex (
		double const x1 = 0.0,
		double const y1 = 0.0,
		double const z1 = 0.0
		)
		:
		x (x1), y (y1), z (z1)
	{}

	// Undefined result if contents are inf/nan.

	inline void zero () { x=0; y=0; z=0; }

	bool	operator ==	( Vertex const &a ) const;

	Vertex	operator +	( Vertex const &a ) const;
	void	operator +=	( Vertex const &a );
	Vertex	operator -	( Vertex const &a ) const;
	void	operator -=	( Vertex const &a );
	Vertex	operator *	( double scaler ) const;
	void	operator *=	( double scaler );

	double distance () const;			// this
	double distance ( Vertex const &a ) const;	// this -> a

	inline void normalize () { normalize ( *this ); }
	void normalize ( Vertex const &a );		// this = a normalized

	double speed ( double time ) const;	// distance / time
	double speed ( Vertex const & pos, double time ) const;

	int interpolate (		// this = p2 -> p3 at tm% position
		double tm,		// 0.0 <= tm <= 1.0
		Vertex const &p1,
		Vertex const &p2,
		Vertex const &p3,
		Vertex const &p4
		);

	// Calculate the time so the actor speed smoothly changes over time.
	static double get_animation_time (
		double speed1,
		double speed2,
		double speed3,
		double time		// 0.0 - 1.0 Linear time
		);
		// = animation time 0.0 - 1.0

	void bezier (
		double tm,		// 0.0 <= tm <= 1.0
		Vertex const &p1,
		Vertex const &cp1,
		Vertex const &p2,
		Vertex const &cp2
		);

	void linear (
		double tm,		// 0.0 <= tm <= 1.0
		Vertex const &p1,
		Vertex const &p2
		);

/// ----------------------------------------------------------------------------

	double		x;
	double		y;
	double		z;
};



// All Bezier* times are: 0.0 <= tm <= 1.0



class BezierCache	// Item for pre-calculated tm<->distance table
{
public:
	BezierCache ( Vertex & p, double const tm, double const len ) :
		m_p (p), m_tm (tm), m_len (len) {}

/// ----------------------------------------------------------------------------

	Vertex	const	m_p;		// Point on the curve at m_tm

	double	const	m_tm;
	double	const	m_len;
};



class BezierNode
{
public:
	void set (		// Also resets/recalculates the cache
		Vertex const & p1,
		Vertex const & cp1,
		Vertex const & p2,
		Vertex const & cp2
		);

	inline void get ( Vertex & p1, Vertex & cp1, Vertex & p2, Vertex & cp2 )
		const
		{
			p1 = m_p1;
			cp1 = m_cp1;
			p2 = m_p2;
			cp2 = m_cp2;
		}

	void get_position ( double tm, Vertex & v ) const;
		// Get a point on the curve

	int get_length ( double tm, double & len ) const;
		// Use the cache to get the length of Bezier: p1 to pn.
		// 0.0 <= len <= (total length)
		// tm=0.0 => len = 0.0; tm=1.0 => length of whole curve

	int get_time ( double tm_start, double length, double & tm ) const;
		// Use cache to go a certain length from a start point.
		// On success tm becomes the end point of this.

	int get_closest_time (
		Vertex const & p,
		double & tm,
		double & distance	// Distance from curve at tm
		) const;
		// Using the cache, find the tm value closest to this point.

	inline Vertex const & get_p2 () const { return m_p2; }

private:
	void create_cache ( int tot );
		// Prepare this many nodes in the cache

/// ----------------------------------------------------------------------------

	Vertex m_p1, m_cp1, m_p2, m_cp2;

	std::vector<BezierCache> m_cache;
};



class BezierPath	// Collection of Bezier nodes forming a full path
{
public:
	void clear () { m_node.clear(); }

	void add (
		Vertex const & p2,
		Vertex const & cp1,
		Vertex const & cp2
		);

	inline BezierNode const * get_node ( size_t n ) const
	{
		return &m_node[n];
	}

	inline size_t get_size () const { return m_node.size (); }

	// Single segment length
	double get_length ( size_t node, double a, double b ) const;

	// Multi segment length
	double get_length (
		size_t node1, double tm1,
		size_t node2, double tm2
		) const;

	int get_position ( size_t node, double tm, Vertex & v ) const;

	int get_closest_time (
		Vertex const & p,
		size_t & bez_node,	// Closest node
		double & tm,		// Time in node p1->p2
		size_t bn		// Start at BezNode
		) const;
		// Find the node/tm value closest to this point.

	int get_relative_time (
		size_t a_node,
		double a_tm,
		size_t b_node,
		double b_tm,
		double perc,	// 0.0 - 1.0 : % of distance from A->B
		size_t & node,
		double & tm
		) const;

	// Create new path based on these points, with auto smoothing
	void set_smooth_curve ( std::vector<Vertex> const & vertices );

private:
	std::vector<BezierNode> m_node;

/// ----------------------------------------------------------------------------

	int get_relative_forward (	// Caller validates 3 args
		size_t a_node,
		double a_tm,
		double delta,		// >0.0
		size_t & b_node,
		double & b_tm
		) const;
};



}		// namespace mtGin



#endif		// C++ API



#endif		// MTGIN_H_

