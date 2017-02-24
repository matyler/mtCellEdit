/*
	Copyright (C) 2016 Mark Tyler

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

extern "C" {

	#include <stdlib.h>
	#include <string.h>
}

#include <mtkit.h>
#include <mtpixy.h>



class Global
{
public:
	Global ();
	void set_image ( mtPixy::Image * im );

///	------------------------------------------------------------------------

	int
			i_comp_png,
			i_comp_jpeg,
			i_error,	// 0 = success 1 = error
			i_ftype_in,
			i_ftype_out,	// If PIXY_FILE_TYPE_NONE use i_ftype_in
			i_height,
			i_image_type,
			i_palette,	// 0=default 1=grey 2-6=uniform
			i_scale,	// 0=default 1=blocky (RGB)
			i_tmp,
			i_verbose,	// 1 = verbose 0 = normal
			i_width,
			i_x,
			i_y
			;

	char	const	* s_arg;	// Current command line argument

	mtPixy::Image	* image;	// Current image
};



// Note: must match ff_errtab in main.c
enum
{
	ERROR_LOAD_FILE		= 1,
	ERROR_LIBMTPIXY		= 2,
	ERROR_BAD_PALETTE	= 3
};



extern Global		global;



int ut_load_file ( void );		// Loads image file (name in
					// global.s_arg).
	// 0 = success. Failure is not sent to stderr, but global.i_error is
	// set.

/*	Command functions

	Return 0 = success.
	Return > 0 = Generic error to be reported by caller (main.c ff_errtab).
*/

int pixyut_ls ( void );
int pixyut_new ( void );
int pixyut_resize ( void );
int pixyut_scale ( void );

