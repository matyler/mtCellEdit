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
	#include <math.h>
	#include <time.h>
	#include <ctype.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
	#include <errno.h>

	#include <zlib.h>
	#include <cairo.h>
	#include <pango/pango.h>
	#include <pango/pangoft2.h>
}

#include <mtkit.h>

#include "mtpixy.h"




#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifndef DEBUG
#pragma GCC visibility pop
#endif

