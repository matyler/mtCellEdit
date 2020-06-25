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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include <map>
#include <vector>

#include <mtkit.h>



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
		);

	bool operator () ( pCloudNkey const &a, pCloudNkey const &b ) const;

	inline double get_x () const { return m_x; }
	inline double get_y () const { return m_y; }
	inline double get_z () const { return m_z; }

private:
	double		m_x, m_y, m_z;
};



class pCloudNdata
{
public:
	pCloudNdata (
		int	intensity = 0,
		int	r = 0,
		int	g = 0,
		int	b = 0
		);

	void add (
		int	intensity,
		int	r,
		int	g,
		int	b
		);

	inline int get_intensity () const
		{ return m_count > 0 ? m_intensity / m_count : 0; }
	inline int get_r () const { return m_count > 0 ? m_r / m_count : 0; }
	inline int get_g () const { return m_count > 0 ? m_g / m_count : 0; }
	inline int get_b () const { return m_count > 0 ? m_b / m_count : 0; }
	inline int get_count () const { return m_count; }

private:
	int		m_intensity, m_r, m_g, m_b, m_count;
};



class pCloud
{
public:
	pCloud ();
	~pCloud ();

	int load ( mtKit::LineFileRead & file );
	int add_item (
		double	x,
		double	y,
		double	z,
		int	intensity,
		int	r,
		int	g,
		int	b
		);

	int set_limit ( int n );	// x,y,z fixed sample to points 2^n
	int set_format ( int n );	// n = FORMAT_*
	double xyz_sample ( double xyz ) const;
	inline double get_limit () const { return m_lim; }

	void set_output_filename ( char const * filename );
	int set_slices ( int slices );

	void print_data ();

/// ----------------------------------------------------------------------------

	static int const LIMIT_MIN = -32;
	static int const LIMIT_MAX = 32;

	static int const SLICES_MIN = 1;
	static int const SLICES_MAX = 16;

	enum
	{
		FORMAT_ERROR		= -1,
		FORMAT_MIN		= 0,

		FORMAT_PLY		= 0,
		FORMAT_PTS		= 1,

		FORMAT_MAX		= 1
	};

private:
	int print_ply_stdout ();
	int print_ply_file ();

	int print_pts_stdout ();
	int print_pts_file ();

/// ----------------------------------------------------------------------------

	std::map<pCloudNkey, pCloudNdata, pCloudNkey> m_map;

	char	const	* m_output_filename;

	int	m_ltd;		// 0=Don't use m_lim, store x,y,z verbatim
	double	m_lim;
	double	m_lim_half;	// Cache of (m_lim / 2)
	int	m_format;	// FORMAT_*
	int	m_slices;
};

