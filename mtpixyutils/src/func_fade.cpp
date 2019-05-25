/*
	Copyright (C) 2019 Mark Tyler

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



int pixyut_fade ()
{
	if ( global.image_pair.open_file ( global.s_arg ) )
	{
		return 0;
	}

	int const fps = MAX( FPS_MIN, MIN( FPS_MAX, global.i_fps ) );
	int const secs = MAX( SECONDS_MIN, MIN( SECONDS_MAX, global.i_seconds));
	int const f0 = MAX( F0_MIN, MIN( F0_MAX, global.i_frame0));

	int const frames = f0 + fps * secs;

	mtPixy::Image * const image_a = global.image_pair.get_image_a ();
	mtPixy::Image * const image_b = global.image_pair.get_image_b ();

	unsigned char const * const mem_a = image_a->get_canvas ();
	unsigned char const * const mem_b = image_b->get_canvas ();
	unsigned char * const dest = global.image->get_canvas ();

	int64_t const pixels = image_a->get_canvas_bpp () *
		(image_a->get_width () * image_a->get_height ());

	std::string fn ( global.s_dir );
	fn += MTKIT_DIR_SEP;
	fn += std::string(global.s_prefix);

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

		if ( global.i_verbose )
		{
			puts ( filename.c_str () );
		}

		if ( global.image->save_png ( filename.c_str (), 2 ) )
		{
			fprintf ( stderr, "Error: Unable to save file '%s'\n",
				filename.c_str () );

			global.i_error = 1;

			return 0;
		}
	}

	return 0;
}

