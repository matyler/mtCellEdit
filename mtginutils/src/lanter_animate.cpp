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

#include "lanter.h"



Lanimate::Lanimate (
	Mainwindow	& mw,
	Core		& core
	)
	:
	m_mainwindow	( mw ),
	m_core		( core )
{
}

Lanimate::~Lanimate ()
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



int Lanimate::init ()
{
	int		const	fps = m_core.get_anim_fps ();
	char	const * const	tsv_file = m_core.get_anim_tsv ();
	char	const * const	output_dir = m_core.get_anim_dir ();

	if (	! tsv_file
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

	std::vector<mtGin::Vertex> points_camera;
	std::vector<mtGin::Vertex> points_focus;
	mtGin::Vertex vertex;
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

		vertex.x = file.value (row, 2);
		vertex.y = file.value (row, 3);
		vertex.z = file.value (row, 4);
		points_camera.push_back ( vertex );

		vertex.x = file.value (row, 5);
		vertex.y = file.value (row, 6);
		vertex.z = file.value (row, 7);
		points_focus.push_back ( vertex );

		m_time_path.push_back ( time );

		seconds = time;
	}

	m_frame_total = (int)(fps * seconds);

	m_camera_path.set_smooth_curve ( points_camera );
	m_focus_path.set_smooth_curve ( points_focus );

	if ( output_dir )
	{
		m_output_dir = output_dir;
		m_output_dir += MTKIT_DIR_SEP;
		m_output_dir += "f_";
	}

	m_fps = fps;

	return 0;
}

int Lanimate::prepare_frame ( int const frame )
{
	if (	frame < 0
		|| frame > m_frame_total
		|| m_camera_path.get_size() < 2
		)
	{
		return 1;
	}

	if ( m_camera_path.get_size() != m_time_path.size() )
	{
		std::cerr << "prepare_frame error: bad vectors\n";
		return 1;
	}

	double const secs = (double)frame / (double)m_fps;
	size_t const nodetot = m_camera_path.get_size();

	size_t node = 0;
	size_t n1 = 0;
	size_t n2 = 1;	// Needed for m_time[0] >= secs
	double tm = 0.0;

	if ( m_time_path[0] < secs )
	{
		for ( node = 1; node < nodetot; node++ )
		{
			if ( secs <= m_time_path[node] )
			{
				tm =	(secs - m_time_path[node-1]) /
					(m_time_path[node] -
						m_time_path[node-1]);
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

	// Calculate the speeds of these three segments to smoothly change speed
	// as we travel from one to another.
	double s1 = 0.0, s2 = 0.0, s3 = 0.0;

	if ( n1 > 0 )
	{
		double const t = m_time_path[n1] - m_time_path[n1 - 1];
		double const d = m_camera_path.get_length ( n1, 0.0, 1.0 );
		s1 = d / t;
	}

	{
		double const t = m_time_path[n2] - m_time_path[n1];
		double const d = m_camera_path.get_length ( n2, 0.0, 1.0 );
		s2 = d / t;
	}

	if ( (n2+1) < nodetot )
	{
		double const t = m_time_path[n2+1] - m_time_path[n2];
		double const d = m_camera_path.get_length ( n2+1, 0.0, 1.0 );
		s3 = d / t;
	}

	tm = mtGin::Vertex::get_animation_time ( s1, s2, s3, tm );

	if ( m_camera_path.get_position ( node, tm, m_view_position ) )
	{
		std::cerr << "prepare_frame error: path position unknown\n";
		return 1;
	}

	if ( m_focus_path.get_position ( node, tm, m_view_focus ) )
	{
		std::cerr << "prepare_frame error: focus position unknown\n";
		return 1;
	}

	return 0;
}

int Lanimate::save_frame ( int const frame ) const
{
	if ( frame < 0 || frame > m_frame_total )
	{
		return 1;
	}

	if ( m_output_dir.size() < 1 )
	{
		return 0;
	}

	char buf[32];
	snprintf ( buf, sizeof(buf), "%06i", frame );

	std::string filename ( m_output_dir );
	filename += buf;
	filename += ".png";

	return m_mainwindow.export_image ( filename );
}

void Lanimate::clear ()
{
	m_time_path.clear ();
	m_camera_path.clear ();
	m_focus_path.clear ();

	m_output_dir.clear ();
	m_frame_total = 0;
	m_fps = ANIM_FPS_DEFAULT;
}

