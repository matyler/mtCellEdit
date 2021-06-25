/*
	Copyright (C) 2019-2020 Mark Tyler

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

#include "private.h"



#define FPS_MIN		10
#define FPS_MAX		480
#define SECONDS_MIN	1
#define SECONDS_MAX	60
#define F0_MIN		0
#define F0_MAX		100000



int Global::pixy_fade ()
{
	mtPixmap * img;

	switch ( m_image_pair.open_file ( s_arg, i_ftype_in, &img ) )
	{
	case 0:
		// Both images are loaded and match, so do some work.
		break;

	case -1:
		// Only one image loaded.
		return 0;

	case 1:
		i_error = 1;	// Error, so bail out
		return 0;
	}

	set_pixmap ( img );

	int const fps = MAX( FPS_MIN, MIN( FPS_MAX, i_fps ) );
	int const secs = MAX( SECONDS_MIN, MIN( SECONDS_MAX, i_seconds));
	int const f0 = MAX( F0_MIN, MIN( F0_MAX, i_frame0));

	int const frames = f0 + fps * secs;

	mtPixmap const * const pixmap_a = m_image_pair.get_pixmap_a ();
	mtPixmap const * const pixmap_b = m_image_pair.get_pixmap_b ();

	unsigned char const * const mem_a = pixy_pixmap_get_canvas ( pixmap_a );
	unsigned char const * const mem_b = pixy_pixmap_get_canvas ( pixmap_b );
	unsigned char * const dest = pixy_pixmap_get_canvas ( m_pixmap.get() );

	int64_t const pixels = pixy_pixmap_get_bytes_per_pixel ( pixmap_a ) *
		pixy_pixmap_get_width ( pixmap_a ) *
		pixy_pixmap_get_height( pixmap_a );

	std::string fn ( s_dir );
	fn += MTKIT_DIR_SEP;
	fn += std::string ( s_prefix );

	for ( int f = f0; f < frames; f++ )
	{
		double const m1 = 1.0 - ( (double)(f - f0) /
			(double)(frames - f0) );
		double const m2 = 1.0 - m1;

		for ( int64_t p = 0; p < pixels; p++ )
		{
			dest[p] = (unsigned char)(mem_a[p] * m1 +
				mem_b[p] * m2 + 0.5);
		}

		char num_txt[16];

		snprintf ( num_txt, sizeof(num_txt), "%06i.png", f );

		std::string const filename ( fn + num_txt );

		if ( i_verbose )
		{
			puts ( filename.c_str () );
		}

		if ( pixy_pixmap_save_png( m_pixmap.get(), filename.c_str(), 2))
		{
			fprintf ( stderr, "Error: Unable to save file '%s'\n",
				filename.c_str () );

			i_error = 1;

			return 0;
		}
	}

	return 0;
}

