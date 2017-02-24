/*
	Copyright (C) 2014-2015 Mark Tyler

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



static int create_xor_real ( void )
{
	FILE		* fp_in		= NULL;
	FILE		* fp_out	= NULL;
	unsigned char	* buf_in	= NULL;
	unsigned char	* buf_pad	= NULL;
	rdcPadFile	* padfile	= NULL;

	int		res = 1;
	size_t		i;
	size_t		tot;

	size_t	const	buf_size = 1000000;


	if (	! ( padfile	= rdc_pad_open () )			||
		! ( fp_in	= rdc_open_input () )			||
		! ( fp_out	= rdc_open_output () )			||
		! ( buf_in	= (unsigned char *)rdc_malloc ( buf_size,
					"input" ) )	||
		! ( buf_pad	= (unsigned char *)rdc_malloc ( buf_size,
					"pad" ) )
		)
	{
		goto error;
	}

	while ( 1 )
	{
		tot = fread ( buf_in, 1, buf_size, fp_in );
		if ( tot == 0 )
		{
			break;
		}

		if ( rdc_pad_read ( padfile, buf_pad, buf_size ) )
		{
			goto error;
		}

		for ( i = 0; i < tot; i++ )
		{
			buf_in [ i ] ^= buf_pad [ i ];
		}

		if ( tot != fwrite ( buf_in, 1, tot, fp_out ) )
		{
			fprintf ( stderr, "Error writing to output file.\n\n" );

			goto error;
		}
	}

	res = 0;			// We have finished without failure

error:
	rdc_fclose ( fp_in );
	rdc_fclose ( fp_out );
	rdc_pad_close ( padfile );
	free ( buf_in );
	free ( buf_pad );

	return res;
}

int create_xor (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if (	validate_arg_o ()		||
		validate_arg_i ()		||
		validate_arg_pad ()		||
		validate_arg_pad_start ()	||
		create_xor_real ()
		)
	{
		set_exit_fail ();

		return 1;
	}

	return 0;
}

