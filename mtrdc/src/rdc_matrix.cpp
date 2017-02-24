/*
	Copyright (C) 2013-2015 Mark Tyler

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

#include "rdc.h"



static int create_matrix_real ( void )
{
	CedSheet	* sheet = NULL;
	int		i, j, r, c;
	double		dnum;
	char		txt [ 8 ];

	char	const * const	arg_o		= get_arg_o ();
	int	const		matrix_cols	= get_arg_matrix_cols ();
	int	const		matrix_rows	= get_arg_matrix_rows ();
	int	const		iterations	= get_arg_iterations ();


	sheet = ced_sheet_new ();

	if ( ! sheet )
	{
		fprintf ( stderr, "Unable to create new sheet.\n" );

		return 1;
	}

	txt [ sizeof(txt) - 1 ] = 0;

	for ( i = 0; i < iterations; i++ )
	{
		r = 1 + rand () % matrix_rows;
		c = 1 + rand () % matrix_cols;

		dnum = rand () % 1000000;
		dnum = dnum / 1000;

		ced_sheet_set_cell_value ( sheet, r, c, dnum );

		r = 1 + rand () % matrix_rows;
		c = 1 + rand () % matrix_cols;

		for ( j = 0; j < (int)(sizeof(txt) - 1); j++ )
		{
			txt[j] = (char)(32 + rand () % 95);
		}

		ced_sheet_set_cell_text ( sheet, r, c, txt );
	}

	if ( ced_sheet_save ( sheet, arg_o, CED_FILE_TYPE_TSV_VALUE ) )
	{
		fprintf ( stderr, "Unable to save matrix file %s.\n", arg_o );

		ced_sheet_destroy ( sheet );

		return 1;
	}

	ced_sheet_destroy ( sheet );

	return 0;
}

int create_matrix (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if (	validate_arg_o ()		||
		validate_arg_iterations ()	||
		validate_arg_matrix_rows ()	||
		validate_arg_matrix_cols ()	||
		create_matrix_real ()
		)
	{
		set_exit_fail ();

		return 1;
	}

	return 0;
}

