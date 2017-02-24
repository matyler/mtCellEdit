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



FILE * rdc_open_input ( void )
{
	FILE		* fp = NULL;


	fp = fopen ( get_arg_i (), "r" );
	if ( ! fp )
	{
		fprintf ( stderr, "Unable to open input file.\n\n" );
	}

	return fp;
}

FILE * rdc_open_output ( void )
{
	FILE		* fp = NULL;


	fp = fopen ( get_arg_o (), "w" );
	if ( ! fp )
	{
		fprintf ( stderr, "Unable to open output file.\n\n" );
	}

	return fp;
}

void rdc_fclose (
	FILE		* const	fp
	)
{
	if ( fp )
	{
		fclose ( fp );
	}
}

void * rdc_malloc (
	size_t		const	buf_size,
	char	const * const	txt
	)
{
	void		* const buf = malloc ( buf_size );


	if ( ! buf )
	{
		fprintf ( stderr, "Unable to create %s memory buffer.\n\n",
			txt );
	}

	return buf;
}



struct rdcPadFile
{
	FILE		* fp;
};



rdcPadFile * rdc_pad_open ( void )
{
	rdcPadFile	* padfile;


	padfile = (rdcPadFile *)calloc ( 1, sizeof ( rdcPadFile ) );
	if ( ! padfile )
	{
		goto error;
	}

	padfile->fp = fopen ( get_arg_pad (), "r" );

	if (	! padfile->fp		||
		0 != fseek ( padfile->fp, (long)get_arg_pad_start (), SEEK_SET )
		)
	{
		rdc_pad_close ( padfile );

		goto error;
	}

	return padfile;

error:
	fprintf ( stderr, "Unable to open pad file.\n\n" );

	return NULL;
}

int rdc_pad_read (
	rdcPadFile	* const	padfile,
	unsigned char	* const	buf,
	size_t		const	buf_size
	)
{
	size_t		todo;


	todo = fread ( buf, 1, buf_size, padfile->fp );
	if ( todo != buf_size )
	{
		// No more data left in pad so fail
		return 1;
	}

	return 0;
}

int rdc_pad_close (
	rdcPadFile	* const	padfile
	)
{
	if ( ! padfile )
	{
		return 1;
	}

	if ( padfile->fp )
	{
		fclose ( padfile->fp );
	}

	free ( padfile );

	return 0;
}

