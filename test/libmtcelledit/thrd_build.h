/*
	Copyright (C) 2017 Mark Tyler

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

#include <mtkit.h>
#include <mtcelledit.h>



enum
{
	ROW_TOT		= 1000,
	COL_TOT		= 1000
};



// NOTE: suffix st=Single Threaded, mt=Multi Threaded

// Build an array of random numbers, in random positions

CedSheet * build_sheet_num_st (
	int rows,
	int cols
	);

CedSheet * build_sheet_num_mt (
	int rows,
	int cols,
	int thrds		// 1..10
	);

// Simple wrapper function for measuring elapsed time

double build_get_time ( void );		// Monotonic time, measured in seconds

