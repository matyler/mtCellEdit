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



///	PLY	----------------------------------------------------------------



inline static int ply_header (
	FILE		* const	fp,
	uint64_t	const	size
	)
{
	return fprintf ( fp,
		"ply\n"
		"format ascii 1.0\n"
		"element vertex %" PRIu64 "\n"
		"property float x\n"
		"property float y\n"
		"property float z\n"
		"property uchar intensity\n"
		"property uchar red\n"
		"property uchar green\n"
		"property uchar blue\n"
		"end_header\n"
		, size
		) < 0 ? 1 : 0;
}

inline static int fp_point (
	FILE	* const	fp,
	double	const	x,
	double	const	y,
	double	const	z,
	int	const	intensity,
	int	const	r,
	int	const	g,
	int	const	b
	)
{
	return fprintf ( fp, "%.15g %.15g %.15g %i %i %i %i\n",
		x, y, z, intensity, r, g, b ) < 0 ? 1 : 0;
}



class printCloud
{
public:
	explicit printCloud (
		std::map<pCloudNkey, pCloudNdata, pCloudNkey> &m_map
		)
	{
		std::map<pCloudNkey, pCloudNdata>::iterator it;

		for ( it = m_map.begin (); it != m_map.end (); ++it )
		{
			pCloudNkey const * const key = &it->first;
			pCloudNdata const * const data = &it->second;

			if ( fp_point ( stdout,
				key->get_x (),
				key->get_y (),
				key->get_z (),
				data->get_intensity (),
				data->get_r (),
				data->get_g (),
				data->get_b () )
				)
			{
				std::cerr << "printCloud::fp_point ERROR\n";
				throw 123;
			}
		}
	}
};


int pCloud::print_ply_stdout ()
{
	if ( ply_header ( stdout, m_map.size () ) )
	{
		return 1;
	}

	try
	{
		printCloud ( this->m_map );
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}



struct cmpBy_Y
{
	bool operator () ( pCloudNkey const * a, pCloudNkey const * b ) const
	{
		if ( a->get_y () != b->get_y () )
		{
			return a->get_y () < b->get_y ();
		}

		if ( a->get_z () != b->get_z () )
		{
			return a->get_z () < b->get_z ();
		}

		return a->get_x () < b->get_x ();
	}
};



class sliceCloud
{
public:
	sliceCloud (
		int		const	format,
		int		const	m_slices,
		std::map<pCloudNkey, pCloudNdata, pCloudNkey> &m_map,
		std::string	const	& m_output_filename
		)
	{
		char		const *	type = "???";
		size_t		const	slices = (size_t)m_slices;
		std::vector<int64_t>	lim_x ( slices + 1, 0 );
		std::vector<int64_t>	lim_y ( slices + 1, 0 );
		char			buf[128];
		mtKit::ByteFileWrite	file;
		int64_t		const	map_size = (int64_t)m_map.size ();


		switch ( format )
		{
		case pCloud::FORMAT_PLY:	type = "ply";	break;
		case pCloud::FORMAT_PTS:	type = "pts";	break;
		}

		for ( size_t n = 1; n <= slices; n++ )
		{
			lim_x[n] = ((int64_t)n) * map_size / m_slices;
		}

		std::map<pCloudNkey, pCloudNdata>::iterator it = m_map.begin ();

		for (	size_t file_num_x = 1;
			file_num_x <= slices;
			file_num_x ++
			)
		{
			int64_t const x_tot = lim_x[ file_num_x ] -
				lim_x[ file_num_x - 1 ];

			if ( x_tot < 1 )
			{
				continue;
			}

			// Map each of these X slice items, sorting by Y
			std::map<pCloudNkey const *, pCloudNdata const *,
				cmpBy_Y> y_map;

			for ( int64_t a = 0; a < x_tot; a++, ++it )
			{
				y_map.insert (
					std::pair<pCloudNkey const *,
						pCloudNdata const *>(
						&it->first, &it->second)
					);
			}

			for ( size_t n = 1; n <= slices; n++ )
			{
				lim_y[n] = ((int64_t)n) * x_tot / m_slices;
			}

			std::map<pCloudNkey const *, pCloudNdata const *>::
				iterator ity = y_map.begin ();

			for (	size_t file_num_y = 1;
				file_num_y <= slices;
				file_num_y ++
				)
			{
				int64_t const y_tot = lim_y[ file_num_y ] -
					lim_y[ file_num_y - 1 ];

				if ( y_tot < 1 )
				{
					continue;
				}

				std::string filename ( m_output_filename );

				snprintf ( buf, sizeof(buf), "%02i_%02i",
					(int)file_num_x, (int)file_num_y );
				filename += buf;
				filename += ".";
				filename += type;

				if ( file.open ( filename.c_str () ) )
				{
					std::cerr << "pCloud::print_" << type
						<< "_file "
						<< "ERROR - open\n";

					throw 123;
				}

				if ( format == pCloud::FORMAT_PLY &&
					ply_header ( file.get_fp (),
						(uint64_t)y_tot )
					)
				{
					std::cerr << "pCloud::print_" << type
						<< "_file "
						<< "ERROR - open/header\n";

					throw 123;
				}

				int64_t const end = lim_y[ file_num_y ];

				for (	int64_t j = lim_y[ file_num_y - 1 ];
					j < end;
					j++, ++ity
					)
				{
					pCloudNkey const * const key =
						ity->first;
					pCloudNdata const * const data =
						ity->second;

					if ( fp_point ( file.get_fp (),
						key->get_x (),
						key->get_y (),
						key->get_z (),
						data->get_intensity (),
						data->get_r (),
						data->get_g (),
						data->get_b () )
						)
					{
						std::cerr << "pCloud::"
							<< "print_" << type
							<< "_file ERROR - "
							<< "fp_point\n";

						throw 123;
					}
				}
			}
		}
	}
};	// sliceCloud



int pCloud::print_ply_file ()
{
	try
	{
		sliceCloud ( m_format, m_slices, m_map, m_output_filename );
	}
	catch (...)
	{
		std::cerr << "pCloud::print_ply_file EXCEPTION\n";

		return 1;
	}

	return 0;
}



///	PTS	----------------------------------------------------------------



int pCloud::print_pts_stdout ()
{
	try
	{
		printCloud ( this->m_map );
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}

int pCloud::print_pts_file ()
{
	try
	{
		sliceCloud ( m_format, m_slices, m_map, m_output_filename );
	}
	catch (...)
	{
		std::cerr << "pCloud::print_pts_file EXCEPTION\n";

		return 1;
	}

	return 0;
}

