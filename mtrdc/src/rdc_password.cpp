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



static int create_passwords_real ( void )
{
	rdcPadFile	* padfile	= NULL;
	FILE		* fp_out	= NULL;
	char		* buf_pass	= NULL;
	unsigned char	* buf_pad	= NULL;

	int		res = 1;
	int		i, j, k;

	char	const * const passchar	= get_arg_password_chars ();
	int	const	passchar_len	= (int)strlen ( passchar );
	size_t	const	buf_pass_size	= (size_t)get_arg_password_len ();
	size_t	const	buf_pad_size	= buf_pass_size;


	// + 2 is for \n\0 string temination
	buf_pass = (char *)rdc_malloc ( buf_pass_size + 2, "password" );
	buf_pad = (unsigned char *)rdc_malloc ( buf_pad_size, "pad" );

	if (	! buf_pass				||
		! buf_pad				||
		! ( fp_out = rdc_open_output () )	||
		! ( padfile = rdc_pad_open () )
		)
	{
		goto error;
	}

	for ( i = get_arg_iterations (); i > 0; i-- )
	{
		// Get next batch of pad data

		if ( rdc_pad_read ( padfile, buf_pad, buf_pad_size ) )
		{
			fprintf ( stderr, "Not enough data in pad.\n\n" );

			goto error;
		}

		// Turn pad numbers into password

		for (	j = 0;
			j < (int)buf_pass_size;
			j++
			)
		{
			k = buf_pad [ j ];
			buf_pass [ j ] = passchar [ k % passchar_len ];
		}

		buf_pass [ j ]	= '\n';
		buf_pass [ j + 1 ]	= 0;

		k = fputs ( buf_pass, fp_out );

		if ( k == EOF || k < 0 )
		{
			fprintf ( stderr, "Error writing to output file.\n\n" );

			goto error;
		}
	}

	res = 0;			// We have finished without failure

error:
	rdc_fclose ( fp_out );
	rdc_pad_close ( padfile );
	free ( buf_pass );
	free ( buf_pad );

	return res;
}

int create_passwords (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if (	validate_arg_o ()		||
		validate_arg_iterations ()	||
		validate_arg_pad ()		||
		validate_arg_pad_start ()	||
		validate_arg_password_chars ()	||
		validate_arg_password_len ()	||
		create_passwords_real ()
		)
	{
		set_exit_fail ();

		return 1;
	}

	return 0;
}

