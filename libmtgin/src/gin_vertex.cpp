/*
	Copyright (C) 2019-2022 Mark Tyler

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



bool mtGin::Vertex::operator == ( Vertex const & a ) const
{
	if (	x == a.x &&
		y == a.y &&
		z == a.z
		)
	{
		return true;
	}

	return false;
}

mtGin::Vertex mtGin::Vertex::operator + ( Vertex const & a ) const
{
	return { x+a.x, y+a.y, z+a.z };
}

void mtGin::Vertex::operator += ( Vertex const & a )
{
	x += a.x;
	y += a.y;
	z += a.z;
}

mtGin::Vertex mtGin::Vertex::operator - ( Vertex const & a ) const
{
	return { x-a.x, y-a.y, z-a.z };
}

void mtGin::Vertex::operator -= ( Vertex const & a )
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
}

mtGin::Vertex mtGin::Vertex::operator * ( double const scaler ) const
{
	return { x * scaler, y * scaler, z * scaler };
}

void mtGin::Vertex::operator *= ( double const scaler )
{
	x *= scaler;
	y *= scaler;
	z *= scaler;
}

double mtGin::Vertex::distance () const
{
	return sqrt ( x*x + y*y + z*z );
}

double mtGin::Vertex::distance ( Vertex const &a ) const
{
	return (*this - a).distance ();
}

void mtGin::Vertex::normalize ( Vertex const &a )
{
	double const d = a.distance ();

	if ( d == 0.0 )
	{
		zero ();
	}
	else
	{
		x = a.x / d;
		y = a.y / d;
		z = a.z / d;
	}
}

double mtGin::Vertex::speed ( double const time ) const
{
	if ( time == 0.0 || isnan ( time ) || isinf ( time ) )
	{
		return 0.0;
	}

	return this->distance () / fabs ( time );
}

double mtGin::Vertex::speed (
	Vertex	const	& pos,
	double	const	time
	) const
{
	return (*this - pos).speed ( time );
}

int mtGin::Vertex::interpolate (
	double	const	tm,
	Vertex	const	& p1,
	Vertex	const	& p2,
	Vertex	const	& p3,
	Vertex	const	& p4
	)
{
	if ( tm < 0.0 || tm > 1.0 )
	{
		std::cerr << "Invalid tm=" << tm << "\n";

		return 1;
	}

//	Cubic bezier curve with control points 1/3rd length from P2 & P3

	double	const	lenny = p2.distance ( p3 ) / 3.0;
	Vertex		p12u, p23u, p43u, p32u;	// Unit vectors
	Vertex		cp2, cp3;		// Control points

	p12u = p2 - p1;
	p12u.normalize ();

	p23u = p3 - p2;
	p23u.normalize ();

	cp2 = p12u + p23u;
	cp2 *= ( 0.5 * lenny );
	cp2 += p2;

	p43u = p3 - p4;
	p43u.normalize ();

	p32u = p2 - p3;
	p32u.normalize ();

	cp3 = p43u + p32u;
	cp3 *= ( 0.5 * lenny );
	cp3 += p3;

	bezier ( tm, p2, cp2, p3, cp3 );
//	linear ( tm, p2, p3 );

	return 0;
}

double mtGin::Vertex::get_animation_time (
	double	const	speed1,
	double	const	speed2,
	double	const	speed3,
	double	const	time		// 0.0 - 1.0 Linear time
	)
{
	if ( speed2 <= 0.0 )
	{
		return time;
	}

	double const angle1 = atan ( speed1 / speed2 );
	double const cpy1 = sin ( angle1 ) / 2.0;

	double const angle2 = atan ( speed3 / speed2 );
	double const cpy2 = 1.0 - sin ( angle2 ) / 2.0;

	double const bez = 3.0 * time * (1.0-time) * (1.0-time) * cpy1 +
		3.0 * (1.0-time) * time * time * cpy2 +
		time * time * time;

	return bez * 1.0 + time * 0.0;
}

void mtGin::Vertex::bezier (
	double	const	tm,		// 0.0 - 1.0 Linear time
	Vertex	const	& p1,
	Vertex	const	& cp1,
	Vertex	const	& p2,
	Vertex	const	& cp2
	)
{
	if ( p1 == p2 )
	{
		*this = p1;
		return;
	}

	x = (1.0-tm) * (1.0-tm) * (1.0-tm) * p1.x +
		3.0 * tm * (1.0-tm) * (1.0-tm) * cp1.x +
		3.0 * (1.0-tm) * tm * tm * cp2.x +
		tm * tm * tm * p2.x;

	y = (1.0-tm) * (1.0-tm) * (1.0-tm) * p1.y +
		3.0 * tm * (1.0-tm) * (1.0-tm) * cp1.y +
		3.0 * (1.0-tm) * tm * tm * cp2.y +
		tm * tm * tm * p2.y;

	z = (1.0-tm) * (1.0-tm) * (1.0-tm) * p1.z +
		3.0 * tm * (1.0-tm) * (1.0-tm) * cp1.z +
		3.0 * (1.0-tm) * tm * tm * cp2.z +
		tm * tm * tm * p2.z;
}

void mtGin::Vertex::linear (
	double	const	tm,		// 0.0 - 1.0 Linear time
	Vertex	const	& p1,
	Vertex	const	& p2
	)
{
	x = (1.0 - tm) * p1.x + tm * p2.x;
	y = (1.0 - tm) * p1.y + tm * p2.y;
	z = (1.0 - tm) * p1.z + tm * p2.z;
}

