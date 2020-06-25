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

#ifndef CRUL_H
#define CRUL_H



// C++
#include <vector>
#include <map>

// C
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// OS
#include <GL/gl.h>		// GLfloat
#include <sqlite3.h>		// Unlock mtKit Sqlite API
#include <mtkit.h>
#include <mtpixy.h>

// Internal
//#include "mtkit_.h"
//#include "be.h"




namespace Crul
{

class DB;
class Camera;
class Cloud;
class CloudPTS;
class Model;
class Ruler;

struct Line;
struct PointGL;		// RGB opaque
struct PtsExtent;
struct VertexGL;	// RGBA



enum
{
	ERROR_MIN		= -999999999,

	ERROR_EXCEPTION,
	ERROR_USER_ABORT,
	ERROR_PTS_IMPORT,
	ERROR_MODEL_IMPORT,

	ERROR_NONE		= 0,

	LABEL_LENGTH_MAX	= 64,

	CAM_A_XROT_DEFAULT	= 275,
	CAM_A_YROT_DEFAULT	= 0,
	CAM_A_ZROT_DEFAULT	= 80,
	CAM_A_X_DEFAULT		= 0,
	CAM_A_Y_DEFAULT		= 0,
	CAM_A_Z_DEFAULT		= 0,

	CAM_B_XROT_DEFAULT	= 180,
	CAM_B_YROT_DEFAULT	= 0,
	CAM_B_ZROT_DEFAULT	= 0,
	CAM_B_X_DEFAULT		= 0,
	CAM_B_Y_DEFAULT		= 0,
	CAM_B_Z_DEFAULT		= 300,

	VIEW_NUDGE_DEFAULT	= 1,
	VIEW_NUDGE_MIN		= -10,
	VIEW_NUDGE_MAX		= 10,

	VIEW_POINT_SIZE_MIN	= 1,
	VIEW_POINT_SIZE_MAX	= 20,

	VIEW_LINE_BUTT_SIZE_MIN	= -10,
	VIEW_LINE_BUTT_SIZE_MAX	= 10,

	VIEW_LINE_THICKNESS_MIN	= 1,
	VIEW_LINE_THICKNESS_MAX	= 20
};



double normalize_angle ( double angle );
int get_float_rgb_int ( GLfloat val );		// 0.0->0 .. 1.0->255

void create_gl_grid (
	std::vector<Crul::VertexGL> & lines,
	int count,
	VertexGL const & v1,
	VertexGL const & v2,
	VertexGL const & v3,
	VertexGL const & v4
	);

void create_gl_face (
	std::vector<Crul::VertexGL> & vertices,
	VertexGL & v1,
	VertexGL & v2,
	VertexGL & v3,
	VertexGL & v4
	);

void create_gl_triangle (
	std::vector<Crul::VertexGL> & vertices,
	VertexGL & v1,
	VertexGL & v2,
	VertexGL & v3
	);



class DB
{
public:
	DB ();
	~DB ();

	int open ( std::string const & filename );
	inline std::string const & get_filename () const { return m_filename; }

	void clear_cache ();
	int load_cache ( int type, std::vector<PointGL> * cloud );
	int save_cache ( int type, std::vector<PointGL> const * cloud );

	static char const * get_cache_name ( int type );

	enum
	{
		CACHE_TYPE_ERROR	= -1,
		CACHE_TYPE_MIN		= 0,

		CACHE_TYPE_HIGH		= 0,
		CACHE_TYPE_MEDIUM	= 1,
		CACHE_TYPE_LOW		= 2,

		CACHE_TYPE_MODEL	= 3,

		CACHE_TYPE_MAX		= 3
	};

	int load_rulers ( std::map<int, Crul::Ruler> * map );
	int save_rulers ( std::map<int, Crul::Ruler> const * map );

	int load_cameras ( std::map<int, Crul::Camera> * map );
	int save_cameras ( std::map<int, Crul::Camera> const * map );

	void clear_model ();
	int load_model ( std::vector<VertexGL> & model );
	int save_model ( std::vector<VertexGL> const & model );

private:
	void clear_cache_index ( int id );

/// ----------------------------------------------------------------------------

	std::string	m_filename;

	mtKit::Sqlite	m_db;
};



struct Line
{
	static size_t const FLOAT_TOT = 9;

	int get_rgb_int () const;

	inline int get_r_int () const { return get_float_rgb_int ( r ); }
	inline int get_g_int () const { return get_float_rgb_int ( g ); }
	inline int get_b_int () const { return get_float_rgb_int ( b ); }

	double get_length () const;
	void get_unit_vector ( double &x, double &y, double &z ) const;
	double get_angle_xy () const;
	double get_angle_z () const;

	void create_gl (
		std::vector<Crul::VertexGL> & vertices,
		double line_size
		) const;

/// ----------------------------------------------------------------------------

	GLfloat	x1, y1, z1;
	GLfloat	x2, y2, z2;
	GLfloat	r, g, b;	// 0.0 -> 1.0
};



struct PointGL
{
	static size_t const FLOAT_TOT = 6;

	GLfloat	x, y, z;
	GLfloat	r, g, b;	// 0.0 -> 1.0
};



struct VertexGL
{
	static size_t const FLOAT_TOT = 9;

	inline void set_rgb ( GLfloat rr, GLfloat gg, GLfloat bb )
	{ r=rr; g=gg; b=bb; }

	inline void set_rgb ( VertexGL const & v )
	{ r=v.r; g=v.g; b=v.b; }

	inline void set_normal ( VertexGL const & v )
	{ nx=v.nx; ny=v.ny; nz=v.nz; }

	void normal ( VertexGL & p2, VertexGL & p3 );

	GLfloat	x, y, z;
	GLfloat	r, g, b;	// 0.0 -> 1.0
	GLfloat	nx, ny, nz;	// Normal
};



struct PtsExtent
{
	void clear ();
	bool is_clear () const;

	GLfloat		x_min, x_max;
	GLfloat		y_min, y_max;
	GLfloat		z_min, z_max;
	GLfloat		r_min, r_max;
	GLfloat		g_min, g_max;
	GLfloat		b_min, b_max;
};



class CloudPTS
{
public:
	CloudPTS ();

	void clear ();			// Clear points & extents

	inline size_t size ()		const	{ return m_points.size (); }
	inline PointGL const * data ()	const	{ return m_points.data (); }
	inline std::vector<PointGL> * get_points () { return & m_points; }

	PtsExtent const * extents_ready () const;	// NULL = unset
	PtsExtent const * extents_calculate ();

private:
	std::vector<PointGL>	m_points;
	PtsExtent		m_extent;
};



class Cloud
{
public:
	Cloud ();
	~Cloud ();

	inline CloudPTS * get_cpts () const { return m_cpts; }

	void clear ();

	int set_resolution (
		int type,		// DB::CACHE_TYPE_*
		DB * db
		);

	void set_resampling_rates ( int low, int medium );

	// Load pts into high; populate low/medium; store cache on DB.
	int load_pts ( std::string const & filename, DB * db );

	inline PointGL const * data ()	const	{ return m_cpts->data (); }
	inline size_t size ()		const	{ return m_cpts->size (); }

/// ----------------------------------------------------------------------------

	static int const SAMPLE_RATE_MIN	= -1024;
	static int const SAMPLE_RATE_MAX	= 1023;

private:
	int resample_cloud (
		std::vector<Crul::PointGL> * dest,
		int rate
		);

/// ----------------------------------------------------------------------------

	CloudPTS	* m_cpts;

	CloudPTS	m_cpts_low;
	CloudPTS	m_cpts_medium;
	CloudPTS	m_cpts_high;

	int		m_rate_low;
	int		m_rate_medium;
};



class Model
{
public:
	Model ();
	~Model ();

	int import_file ( std::string const & filename, DB * db );

	int load_db_pts ( DB * db );

	inline std::vector<VertexGL> const & get_pts () { return m_pts; }

private:
	std::vector<VertexGL> m_pts;
};



class Camera
{
public:
	Camera ();

	inline int get_id () const			{ return m_id; }
	inline std::string const & get_label () const	{ return m_label; }
	inline double get_x () const			{ return m_x; }
	inline double get_y () const			{ return m_y; }
	inline double get_z () const			{ return m_z; }
	inline double get_rotX () const			{ return m_rotX; }
	inline double get_rotY () const			{ return m_rotY; }
	inline double get_rotZ () const			{ return m_rotZ; }
	inline bool get_read_only () const		{ return m_read_only; }

	inline void set_id ( int const i )		{ m_id = i; }
	inline void set_label ( std::string const & l )	{ m_label = l; }
	inline void set_x ( double const x )		{ m_x = x; }
	inline void set_y ( double const y )		{ m_y = y; }
	inline void set_z ( double const z )		{ m_z = z; }
	inline void set_rotX ( double const r )		{ m_rotX = r; }
	inline void set_rotY ( double const r )		{ m_rotY = r; }
	inline void set_rotZ ( double const r )		{ m_rotZ = r; }
	inline void set_read_only ( bool const r )	{ m_read_only = r; }

/// ----------------------------------------------------------------------------

private:
	int		m_id;
	std::string	m_label;
	double		m_x, m_y, m_z;
	double		m_rotX, m_rotY, m_rotZ;
	bool		m_read_only;
};



class Ruler
{
public:
	Ruler ();

	inline int get_id () const			{ return m_id; }
	inline Line const & get_line () const		{ return m_line; }
	inline std::string const & get_label () const	{ return m_label; }
	inline bool get_visible () const		{ return m_visible; }
	inline bool get_read_only () const		{ return m_read_only; }
	inline int get_plane () const			{ return m_plane; }

	inline void set_id ( int const i )		{ m_id = i; }
	inline void set_label ( std::string const & l )	{ m_label = l; }
	inline void set_visible ( bool const v )	{ m_visible = v; }
	inline void set_read_only ( bool const r )	{ m_read_only = r; }
	inline void set_plane ( int const p )		{ m_plane = p; }

	void set_line ( double x1, double y1, double z1,
		double x2, double y2, double z2 );
	void set_line_rgb ( int rgb );
	void set_line_rgb ( int r, int g, int b );

	int change_length ( double delta, double min ); // 0=No change 1=changed

	void swap_ab ();

	void create_ends_gl (
		std::vector<Crul::VertexGL> & vertices,
		double end_size
		) const;

	void create_plane_gl (
		std::vector<Crul::VertexGL> & lines,
		double plane_range
		) const;

	enum
	{
		PLANE_XY	= 0,
		PLANE_XZ	= 1,
		PLANE_YZ	= 2
	};

/// ----------------------------------------------------------------------------

private:
	int		m_id;
	Line		m_line;
	std::string	m_label;
	bool		m_visible;
	bool		m_read_only;
	int		m_plane;
};



}	// namespace Crul



#endif		// CRUL_H

