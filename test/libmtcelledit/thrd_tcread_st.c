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
	double		total;
	int		celltot;
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

static void test_read_single_threaded ( void )
{
	printf ( "Single thread read\n" );

	CedSheet * sheet = build_sheet_num_st ( ROW_TOT, COL_TOT );

	if ( ! sheet )
	{
		fprintf ( stderr, "build_tree_num_st failed.\n" );
		return;
	}

	thrData		mem = { 0.0, 0 };
	double const	t1 = build_get_time ();

	if ( ced_sheet_scan_area ( sheet, 1, 1, 0, 0, scan_sheet, &mem ) )
	{
		fprintf ( stderr, "Error scanning sheet.\n" );
	}

	double const	t2 = build_get_time ();

	printf ( "  Finished. duration=%f total=%f cells=%i\n\n", t2 - t1,
		mem.total, mem.celltot );

	ced_sheet_destroy ( sheet );
	sheet = NULL;
}

int main (
	int			const	ARG_UNUSED ( argc ),
	char	const * const * const	ARG_UNUSED ( argv )
	)
{
	srand ( (unsigned int)time ( NULL ) );

	test_read_single_threaded ();

	return 0;
}

