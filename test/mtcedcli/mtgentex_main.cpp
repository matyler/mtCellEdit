/*
	Copyright (C) 2015 Mark Tyler

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

#include "mtgentex.h"



static char	const * arg_o		= NULL;
static int		exit_val	= 0;



char const * get_arg_o ( void )
{
	return arg_o;
}

static int file_func (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	char		* buf		= NULL;
	char		* errtxt	= NULL;
	mtUtreeNode	* rootnode	= NULL;
	int		buflen		= 0;


	buf = mtkit_file_load ( filename, &buflen, MTKIT_FILE_ZERO, NULL );

	if ( ! buf )
	{
		fprintf ( stderr, "Unable to load file '%s'\n", filename );

		exit_val = 1;
		goto finish;
	}

	rootnode = mtkit_utree_load_mem ( NULL, buf, (size_t)buflen, &errtxt );

	if ( ! rootnode || errtxt )
	{
		fprintf ( stderr, "Error: %s = %i bytes.  parsed %i bytes\n",
			filename, buflen, (int)(errtxt - buf) );
		fprintf ( stderr, "%s\n", errtxt );

		exit_val = 1;
		goto finish;
	}

	if ( arg_o )
	{
		mtFile		* mtf;
		void		* mtfbuf = NULL;
		int64_t		mtfbuflen = 0;


		mtf = mtkit_utree_save_file_mem ( rootnode,
			MTKIT_UTREE_OUTPUT_DEFAULT );

		if (	! mtf ||
			mtkit_file_get_mem ( mtf, &mtfbuf, &mtfbuflen ) ||
			mtkit_file_save ( arg_o, (char const *)mtfbuf,
				(int)mtfbuflen, 0 )
			)
		{
			mtkit_file_close ( mtf );

			fprintf ( stderr, "Error creating output file '%s'\n",
				arg_o );

			exit_val = 1;
			goto finish;
		}

		mtkit_file_close ( mtf );
	}

	parse_node ( rootnode );

finish:

	mtkit_utree_destroy_node ( rootnode );
	free ( buf );

	return 1;		// Stop parsing command line
}

static int error_func (
	int			const	error,
	int			const	arg,
	int			const	argc,
	char	const * const * const	argv,
	void			* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "Argument error %i: arg = %i/%i", error, arg, argc);

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n\n" );

	exit_val = 1;

	return 1;			// Stop parsing
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtArg	const	arg_list[] = {
				{ "o", MTKIT_ARG_STRING, &arg_o, 0, NULL },
				{ NULL, 0, NULL, 0, NULL }
				};


	srand ( (unsigned int)time ( NULL ) );

	mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func, NULL );

	return exit_val;
}

