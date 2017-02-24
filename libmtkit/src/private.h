/*
	Copyright (C) 2012-2016 Mark Tyler

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
	#include <errno.h>
	#include <math.h>
	#include <ctype.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <pwd.h>
	#include <langinfo.h>
	#include <locale.h>
	#include <iconv.h>

	#include <zlib.h>
}



#include "mtkit.h"



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



mtUtreeNode * mtkit_utree_load_file (
	mtUtreeNode		* parent,	// NULL = create new root
	char		const	* filename,
	int			* errors,	// Put error flag here,
						// NULL = don't. Result
						// 0 = success
	int			* filetype	// MTKIT_FILE_OUT_*
	);
	// NULL = nothing loaded, else something loaded

mtUtreeNode * mtkit_utree_new_text (
	mtUtreeNode		* parent,
	char		const	* text
	);

int mtkit_utree_save_file (
	mtUtreeNode		* node,		// Node to save
	char		const	* filename,
	int			output,		// MTKIT_UTREE_OUTPUT_*
	int			filetype	// MTKIT_FILE_*
	);



#ifndef DEBUG
#pragma GCC visibility pop
#endif

