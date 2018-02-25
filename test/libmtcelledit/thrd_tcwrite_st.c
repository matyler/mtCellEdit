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



static void test_write_single_threaded ( void )
{
	printf ( "Single thread write\n" );

	double const t1 = build_get_time ();
	CedSheet * sheet = build_sheet_num_st ( ROW_TOT, COL_TOT );

	if ( ! sheet )
	{
		fprintf ( stderr, "build_tree_num_st failed.\n" );
		return;
	}

	double const t2 = build_get_time ();

	printf ( "  Finished. duration=%f\n\n", t2 - t1 );

	if ( ced_sheet_save ( sheet, "sheet_st.tsv",
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

	test_write_single_threaded ();

	return 0;
}

