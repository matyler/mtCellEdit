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



static void test_write_multi_threaded (
	int	const	thrds
	)
{
	printf ( "Multi thread (%i) write\n", thrds );

	double const t1 = build_get_time ();
	CedSheet * sheet = build_sheet_num_mt ( ROW_TOT, COL_TOT, thrds );

	if ( ! sheet )
	{
		fprintf ( stderr, "build_sheet_num_mt failed.\n" );
		return;
	}

	double const t2 = build_get_time ();

	printf ( "  Finished. duration=%f\n\n", t2 - t1 );

	if ( ced_sheet_save ( sheet, "sheet_mt.tsv",
		CED_FILE_TYPE_TSV_CONTENT ) )
	{
		fprintf ( stderr, "ced_sheet_save failed.\n" );
	}

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
		test_write_multi_threaded ( t );
	}

	return 0;
}

