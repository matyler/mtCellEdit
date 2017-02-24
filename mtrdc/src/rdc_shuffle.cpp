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



static int shuffle_start (
	FILE	**	const	fp_in,
	FILE	**	const	fp_out,
	rdcPadFile **	const	padfile,
	unsigned char ** const	buf_in,
	unsigned char ** const	buf_pad,
	size_t		const	buf_in_size,
	size_t		const	buf_pad_size
	)
{
	if (	validate_arg_i ()		||
		validate_arg_o ()		||
		validate_arg_pad ()		||
		validate_arg_pad_start ()	||
		! ( buf_in[0]	= (unsigned char *)rdc_malloc ( buf_in_size,
			"input" ) )	||
		! ( buf_pad[0]	= (unsigned char *)rdc_malloc ( buf_pad_size,
			"pad" ) )	||
		! ( fp_in[0]	= rdc_open_input () )			||
		! ( fp_out[0]	= rdc_open_output () )			||
		! ( padfile[0]	= rdc_pad_open () )
		)
	{
		set_exit_fail ();

		return 1;
	}

	return 0;
}

static void shuffle_finish (
	FILE		* const	fp_in,
	FILE		* const	fp_out,
	rdcPadFile	* const	padfile,
	unsigned char * const	buf_in,
	unsigned char * const	buf_pad
	)
{
	rdc_fclose ( fp_in );
	rdc_fclose ( fp_out );
	rdc_pad_close ( padfile );
	free ( buf_in );
	free ( buf_pad );
}



#define SHUF_BITS_SCALE		4



static void shuf_byte_buf (
	unsigned char * const	buf_in,
	unsigned char * const	buf_pad,	// Same size as buf_in
	int		const	buf_in_size
	)
{
	unsigned char	* padmem = buf_pad;
	int		va, oa, ob;


	for ( oa = 0; oa < buf_in_size; oa++ )
	{
		ob = (*padmem++) % buf_in_size;

		va		= buf_in [ oa ];
		buf_in [ oa ]	= buf_in [ ob ];

		buf_in [ ob ]	= (unsigned char)va;
	}
}

static void unshuf_byte_buf (
	unsigned char * const	buf_in,
	unsigned char * const	buf_pad,	// Same size as buf_in
	int		const	buf_in_size
	)
{
	unsigned char	* padmem = buf_pad + buf_in_size - 1;
	int		va, oa, ob;


	for ( oa = buf_in_size - 1; oa >= 0; oa-- )
	{
		ob = (*padmem--) % buf_in_size;

		va		= buf_in [ oa ];
		buf_in [ oa ]	= buf_in [ ob ];

		buf_in [ ob ]	= (unsigned char)va;
	}
}

static int shuffle_bytes_real (
	int	const	unshuffle
	)
{
	FILE		* fp_in		= NULL;
	FILE		* fp_out	= NULL;
	rdcPadFile	* padfile	= NULL;
	unsigned char	* buf_in	= NULL;
	unsigned char	* buf_pad	= NULL;

	int		res = 1;
	size_t		tot;

	size_t	const	buf_in_size	= 256;
	size_t	const	buf_pad_size	= buf_in_size;


	if ( shuffle_start ( &fp_in, &fp_out, &padfile, &buf_in, &buf_pad,
		buf_in_size, buf_pad_size ) )
	{
		return 1;
	}

	while ( 1 )
	{
		tot = fread ( buf_in, 1, buf_in_size, fp_in );
		if ( tot < 1 )
		{
			break;
		}

		if ( rdc_pad_read ( padfile, buf_pad, tot ) )
		{
			fprintf ( stderr, "Not enough data in pad.\n\n" );

			goto error;
		}

		if ( unshuffle )
		{
			unshuf_byte_buf ( buf_in, buf_pad, (int)tot );
		}
		else	// Shuffle
		{
			shuf_byte_buf ( buf_in, buf_pad, (int)tot );
		}

		if ( tot != fwrite ( buf_in, 1, tot, fp_out ) )
		{
			fprintf ( stderr, "Error writing to output file.\n\n" );

			goto error;
		}
	}

	res = 0;

error:
	shuffle_finish ( fp_in, fp_out, padfile, buf_in, buf_pad );

	return res;
}

int create_shuffle (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	return shuffle_bytes_real ( 0 );
}

int create_unshuffle (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	return shuffle_bytes_real ( 1 );
}

