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

#include "mandy.h"



Manimate::Manimate (
	Mainwindow	& mw,
	Core		& core
	)
	:
	m_mainwindow	( mw ),
	m_core		( core )
{
}

Manimate::~Manimate ()
{
}



class TSVread
{
public:
	TSVread () {}
	~TSVread () { close (); }

	int load ( char const * filename )
	{
		close ();
		sheet = ced_sheet_load ( filename, nullptr, nullptr );
		return sheet ? 0 : 1;
	}

	double value ( int row, int column )
	{
		CedCell const * const cell = ced_sheet_get_cell ( sheet, row,
			column );

		if ( ! cell )
		{
			return 0.0;
		}

		return cell->value;
	}

	void close ()
	{
		ced_sheet_destroy ( sheet );
		sheet = nullptr;
	}

	CedSheet * sheet = nullptr;
};



int Manimate::init ( Mandelbrot * const mandy )
{
	int		const	fps = m_core.get_anim_fps ();
	char	const * const	tsv_file = m_core.get_anim_tsv ();
	char	const * const	output_dir = m_core.get_anim_dir ();

	if (	! mandy
		|| ! tsv_file
		|| ! output_dir
		|| fps < ANIM_FPS_MIN
		|| fps > ANIM_FPS_MAX
		)
	{
		return 1;
	}

	clear ();

	TSVread file;

	if ( file.load ( tsv_file ) )
	{
		std::cerr << "Unable to load TSV file " << tsv_file << "\n";
		return 1;
	}

	std::vector<mtGin::Vertex> points;
	double seconds = 0.0;
	int rowtot = 0;

	ced_sheet_get_geometry ( file.sheet, &rowtot, nullptr );

	for ( int row = 1; row <= rowtot; row++ )
	{
		double const time = file.value ( row, 1 );
		if ( time < seconds )
		{
			std::cerr << "Error: row=" << row << " time=" << time
				<< " is bad\n";
			break;
		}

		mtGin::Vertex v (
			mtkit_double_bound( file.value(row, 2), AXIS_CXY_MIN,
				AXIS_CXY_MAX ),
			mtkit_double_bound( file.value(row, 3), AXIS_CXY_MIN,
				AXIS_CXY_MAX ),
			0.0
			);

		double const range = mtkit_double_bound ( file.value(row, 4),
			AXIS_RANGE_MIN, AXIS_RANGE_MAX );

		points.push_back ( v );
		m_time.push_back ( time );
		m_range.push_back ( range );

		seconds = time;
	}

	m_frame_total = (int)(fps * seconds);

	m_path.set_smooth_curve ( points );

	m_output_dir = output_dir;
	m_output_dir += MTKIT_DIR_SEP;
	m_output_dir += "f_";

	m_fps = fps;
	m_mandy = mandy;
	m_verbose = m_core.get_verbose();

	return 0;
}

int Manimate::prepare_frame ( int const frame ) const
{
	if (	frame < 0
		|| frame > m_frame_total
		|| ! m_mandy
		|| m_path.get_size() < 2
		)
	{
		return 1;
	}

	if ( m_path.get_size() != m_time.size() )
	{
		std::cerr << "prepare_frame error: bad vectors\n";
		return 1;
	}

	double const secs = (double)frame / (double)m_fps;
	size_t const nodetot = m_path.get_size();

	size_t node = 0;
	size_t n1 = 0;
	size_t n2 = 1;	// Needed for m_time[0] >= secs
	double tm = 0.0;

	if ( m_time[0] < secs )
	{
		for ( node = 1; node < nodetot; node++ )
		{
			if ( secs <= m_time[node] )
			{
				tm =	(secs - m_time[node-1]) /
					(m_time[node] - m_time[node-1]);
				break;
			}
		}

		n1 = node - 1;
		n2 = node;
	}

	if ( node >= nodetot )
	{
		std::cerr << "prepare_frame error: frame outside range\n";
		return 1;
	}

	// Geometric interpolation to get smooth travel during this time segment
	double const t_delta = m_time[n2] - m_time[n1];
	double const pw = pow ( m_range[n2] / m_range[n1], 1/t_delta );
	double const range = m_range[n1] * pow ( pw, tm * t_delta );

/*
NOTES:
now = time point of animation
t = [time_a .. time_b] = animation segment range
tm = [0.0 .. 1.0] = (now - time_a) / (time_b - time_a)
t_delta = time_b - time_a
m * x^0.0 = range_a
m * x^t_delta = range_b
m = range_a
x = (range_b / range_a) ^ (1 / t_delta)

range = range_a * x^(tm * t_delta)
*/

	mtGin::Vertex pos;
	if ( m_path.get_position ( node, tm, pos ) )
	{
		std::cerr << "prepare_frame error: path position unknown\n";
		return 1;
	}

	m_mandy->zoom_cxyrange ( pos.x, pos.y, range );

	return m_mandy->build_mandelbrot_set ( m_core.get_thread_total () );
}

int Manimate::save_frame ( int const frame ) const
{
	if ( frame < 0 || frame > m_frame_total || ! m_mandy )
	{
		return 1;
	}

	char buf[32];
	snprintf ( buf, sizeof(buf), "%06i", frame );

	std::string filename ( m_output_dir );
	filename += buf;
	filename += ".png";

	return m_mainwindow.export_image ( filename );
}

void Manimate::clear ()
{
	m_time.clear ();
	m_path.clear ();
	m_range.clear ();

	m_output_dir.clear();
	m_mandy = nullptr;
	m_frame_total = 0;
	m_fps = ANIM_FPS_DEFAULT;
}

