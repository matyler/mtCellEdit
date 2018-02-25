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



typedef struct	thrData		thrData;



struct thrData
{
	int		id;
	CedSheet	* sheet;
	double		total;
	int		celltot;
	int		r1;
	int		rtot;
	pthread_t	thread;
};



static int scan_sheet (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	thrData		* data = user_data;

	data->total += cell->value;
	data->celltot ++;

	return 0;		// Continue
}

static void * thread_func (
	void	* const	arg
	)
{
	thrData	* const	data = arg;

	if ( ced_sheet_scan_area ( data->sheet, data->r1, 1, data->rtot, 0,
		scan_sheet, arg ) )
	{
		fprintf ( stderr, "  thread=%i Error scanning sheet.\n",
			data->id );
	}

	return NULL;
}

static void thread_init (
	thrData		* const	mem,
	int		const	id,
	CedSheet	* const	sheet,
	int		const	r1,
	int		const	rtot
	)
{
	mem->id = id;
	mem->sheet = sheet;
	mem->total = 0.0;
	mem->celltot = 0;
	mem->r1 = r1;
	mem->rtot = rtot;
	pthread_create ( &mem->thread, NULL, thread_func, mem );
}

static void test_read_multi_threaded (
	int	const	thrds
	)
{
	printf ( "Multi thread (%i) read \n", thrds );

	CedSheet * sheet = build_sheet_num_mt ( ROW_TOT, COL_TOT, thrds );

	if ( ! sheet )
	{
		fprintf ( stderr, "build_sheet_num_mt failed.\n" );
		return;
	}

	thrData		mem[ thrds ];
	double const	t1 = build_get_time ();

	int i;
	for ( i = 0; i < thrds; i++ )
	{
		int const r1 = 1 + i * ROW_TOT / thrds;
		int const r2 = 1 + (i + 1) * ROW_TOT / thrds;
		int const rtot = r2 - r1;

		thread_init ( &mem[i], i, sheet, r1, rtot );
	}

	for ( i = 0; i < thrds; i++ )
	{
		pthread_join ( mem[i].thread, NULL );
	}

	double const	t2 = build_get_time ();
	double		tot = 0.0;
	int		celltot = 0;

	for ( i = 0; i < thrds; i++ )
	{
		tot += mem[i].total;
		celltot += mem[i].celltot;
	}

	printf ( "  Finished. duration=%f total=%f cells=%i\n\n", t2 - t1,
		tot, celltot );

	ced_sheet_destroy ( sheet );
	sheet = NULL;
}

int main (
	int			const	ARG_UNUSED ( argc ),
	char	const * const * const	ARG_UNUSED ( argv )
	)
{
	srand ( (unsigned int)time ( NULL ) );

	int t;
	for ( t = 1; t <= 10; t++ )
	{
		test_read_multi_threaded ( t );
	}

	return 0;
}

