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

#include "thrd_build.h"



static CedSheet * build_init (
	int	const	rows,
	int	const	cols
	)
{
	if ( rows < 1 || cols < 1 )
	{
		return NULL;
	}

	return ced_sheet_new ();
}

static int populate_row (
	CedSheet	* const	sheet,
	int		const	r,
	int		const	cols
	)
{
	int i;
	for ( i = 0; i < cols; i++ )
	{
		int const c = 1 + rand () % cols;
		double const num = ((double)(rand () % 1000000)) / 1000.0;

		if ( ! ced_sheet_set_cell_value ( sheet, r, c, num ) )
		{
			return 1;
		}
	}

	return 0;
}

CedSheet * build_sheet_num_st (
	int	const	rows,
	int	const	cols
	)
{
	CedSheet * sheet = build_init ( rows, cols );

	if ( ! sheet )
	{
		return NULL;
	}

	int r;
	for ( r = 1; r <= rows; r++ )
	{
		if ( populate_row ( sheet, r, cols ) )
		{
			ced_sheet_destroy ( sheet );
			sheet = NULL;

			return NULL;
		}
	}

	return sheet;
}



typedef struct	thrData		thrData;



struct thrData
{
	int		id;
	int		err;
	CedSheet	* sheet;
	int		rows;
	int		cols;
	int		thrd_tot;
	pthread_t	thread;
};



static void * thread_func (
	void	* const	arg
	)
{
	thrData	* const	data = arg;

	int r;
	for ( r = 1 + data->id; r <= data->rows; r += data->thrd_tot )
	{
		if ( populate_row ( data->sheet, r, data->cols ) )
		{
			data->err = 1;
			return NULL;
		}
	}

	return NULL;
}

static void thread_init (
	thrData		* const	mem,
	int		const	id,
	CedSheet	* const	sheet,
	int		const	rows,
	int		const	cols,
	int		const	thrds
	)
{
	mem->id = id;
	mem->err = 0;
	mem->sheet = sheet;
	mem->rows = rows;
	mem->cols = cols;
	mem->thrd_tot = thrds;
	pthread_create ( &mem->thread, NULL, thread_func, mem );
}

CedSheet * build_sheet_num_mt (
	int	const	rows,
	int	const	cols,
	int	const	thrds
	)
{
	if ( thrds < 1 || thrds > 10 )
	{
		return NULL;
	}

	CedSheet * sheet = build_init ( rows, cols );

	if ( ! sheet )
	{
		return NULL;
	}

	// Pre-allocate all of the row pointers
	int r;
	for ( r = 1; r <= rows; r++ )
	{
		if ( ! ced_sheet_set_cell_text ( sheet, r, 1, "" ) )
		{
			fprintf ( stderr, "Error pre-allocating rows\n" );

			ced_sheet_destroy ( sheet );
			sheet = NULL;

			return NULL;
		}
	}

	thrData mem[ thrds ];
	int i;

	for ( i = 0; i < thrds; i++ )
	{
		thread_init ( &mem[i], i, sheet, rows, cols, thrds );
	}

	for ( i = 0; i < thrds; i++ )
	{
		pthread_join ( mem[i].thread, NULL );
	}

	for ( i = 0; i < thrds; i++ )
	{
		if ( mem[i].err )
		{
			ced_sheet_destroy ( sheet );
			sheet = NULL;

			return NULL;
		}
	}

	return sheet;
}

double build_get_time ( void )
{
	struct timespec tp;

	if ( clock_gettime ( CLOCK_MONOTONIC, &tp ) )
	{
		return 0.0;
	}

	return ((double)tp.tv_sec + ((double)tp.tv_nsec) / 1000000000 );
}

