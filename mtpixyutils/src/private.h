/*
	Copyright (C) 2016-2019 Mark Tyler

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

#include <mtkit.h>
#include <mtpixy.h>



class Global;
class ImagePair;



class ImagePair
{
public:
	int open_file ( char const * filename );
		// 0 = Loaded 2nd image + duplicated A into global.image
		// 1 = Loaded 1st image or an error

	inline mtPixy::Image * get_image_a () { return m_image_a.get (); }
	inline mtPixy::Image * get_image_b () { return m_image_b.get (); }

private:
	bool both_loaded ();	// Both images exist
	bool both_match ();
		// Both images exist, have canvas, same type/geometry

	int prepare_output ();

/// ----------------------------------------------------------------------------

	mtKit::unique_ptr<mtPixy::Image> m_image_a;
	mtKit::unique_ptr<mtPixy::Image> m_image_b;
};



class Global
{
public:
	Global ();
	void set_image ( mtPixy::Image * im );

/// ----------------------------------------------------------------------------

	int
			i_comp_png,
			i_comp_jpeg,
			i_error,	// 0 = success 1 = error
			i_fps,
			i_frame0,
			i_ftype_in,
			i_ftype_out,	// If PIXY_FILE_TYPE_NONE use i_ftype_in
			i_height,
			i_image_type,
			i_palette,	// 0=default 1=grey 2-6=uniform
			i_scale,	// 0=default 1=blocky (RGB)
			i_seconds,
			i_tmp,
			i_verbose,	// 1 = verbose 0 = normal
			i_width,
			i_x,
			i_y
			;

	char	const	* s_dir;
	char	const	* s_prefix;

	char	const	* s_arg;	// Current command line argument

	mtPixy::Image	* image;	// Current image

	ImagePair	image_pair;	// 2 working images
};



// Note: must match ff_errtab in main.c
enum
{
	ERROR_LOAD_FILE		= 1,
	ERROR_LIBMTPIXY		= 2,
	ERROR_BAD_PALETTE	= 3
};



extern Global		global;



int ut_load_file ();			// Loads image file (name in
					// global.s_arg).
	// 0 = success. Failure is not sent to stderr, but global.i_error is
	// set.

int * create_cube_analysis (
	int			i,	// 8=full cube 1=corners of cube
	unsigned char	const *	rgb,
	int			w,
	int			h
	);


/*	Command functions

	Return 0 = success.
	Return > 0 = Generic error to be reported by caller (main.c ff_errtab).
*/

int pixyut_cmp ();
int pixyut_delta ();
int pixyut_fade ();
int pixyut_ls ();
int pixyut_new ();
int pixyut_pica ();
int pixyut_resize ();
int pixyut_riba ();
int pixyut_rida ();
int pixyut_risa ();
int pixyut_scale ();
int pixyut_twit ();

