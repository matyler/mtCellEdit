/*
	Copyright (C) 2014-2016 Mark Tyler

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



static int create_prng_real ( void )
{
	int		const	bufsize		= 1000000;
	char	const *	const	arg_o		= get_arg_o ();
	int		const	iterations	= get_arg_iterations ();

	unsigned char		* buf		= NULL;
	mtFile			* file		= NULL;
	int			tot, i, todo;


	buf = (unsigned char *)malloc ( (size_t)bufsize );
	if ( ! buf )
	{
		fprintf ( stderr, "Unable to allocate memory for PRNG buffer."
			"\n\n" );

		return 1;
	}

	file = mtkit_file_open_disk ( arg_o );
	if ( ! file )
	{
		fprintf ( stderr, "Unable to open output file.\n\n" );

		goto error;
	}

	for ( tot = iterations; tot > 0; tot -= bufsize )
	{
		todo = MIN ( tot, bufsize );

		for ( i = 0; i < todo; i++ )
		{
			buf[i] = (unsigned char)rand ();
		}

		if ( mtkit_file_write ( file, buf, todo ) )
		{
			fprintf ( stderr,"Unable to write to output file.\n\n");

			goto error;
		}
	}

	mtkit_file_close ( file );
	free ( buf );

	return 0;

error:
	mtkit_file_close ( file );
	free ( buf );

	return 1;
}

int create_prng (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if (	validate_arg_o ()		||
		validate_arg_iterations ()	||
		create_prng_real ()
		)
	{
		set_exit_fail ();

		return 1;
	}

	return 0;
}

