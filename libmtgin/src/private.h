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

#include <math.h>

#include <sndfile.h>

#include <mtkit.h>
#include <mtpixy.h>

#include "mtgin.h"
#include "mtgin_sdl.h"
#include "mtgin_gl.h"



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API

enum
{
	GIN_MAIN_LOOP_NEST_MAX	= 10
};


#ifdef __cplusplus
}

// C++ API

namespace mtGin
{



}		// namespace mtGin
#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif

