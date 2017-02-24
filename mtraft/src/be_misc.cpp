/*
	Copyright (C) 2011-2016 Mark Tyler

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

#include "be.h"



static char	const	* scan_directory = NULL;



static int arg_file_func (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( ! scan_directory )
	{
		scan_directory = filename;
	}

	return 0;			// Keep parsing
}

static int arg_error_func (
	int			const	error,
	int			const	arg,
	int			const	argc,
	char	const * const * const	argv,
	void			* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "error_func: Argument ERROR! - num=%i arg=%i/%i",
		error, arg, argc );

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n" );

	return 0;			// Keep parsing
}

char const * raft_cline (
	int			const	argc,
	char	const * const * const	argv
	)
{
	int		show_version = 0;
	mtArg	const	arg_list[] = {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ NULL, 0, NULL, 0, NULL }
			};


	mtkit_arg_parse ( argc, argv, arg_list, arg_file_func, arg_error_func,
		NULL );

	switch ( show_version )
	{
	case 1:
		printf ( "%s\n\n", VERSION );

		exit ( 0 );

	case 2:
		printf ( "%s\n\n"
		"For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n"
		, VERSION, BIN_NAME );

		exit ( 0 );
	}

	return scan_directory;
}

char * raft_get_clipboard (
	CedSheet	* const	sheet
	)
{
	char		ch = 0,
			* tbuf = NULL;
	mtFile		* memfile;
	void		* vb;
	int64_t		vb_size;


	memfile = ced_sheet_save_mem ( sheet, CED_FILE_TYPE_OUTPUT_TSV );
	if ( ! memfile )
	{
		return NULL;
	}

	// NUL terminate so we can use as string
	if (	mtkit_file_write ( memfile, &ch, 1 )		||
		mtkit_file_get_mem ( memfile, &vb, &vb_size )	||
		vb_size < 0
		)
	{
		goto finish;
	}

	if ( ! mtkit_utf8_string_legal ( (unsigned char *)vb, 0 ) )
	{
		// Create proper UTF-8 for GTK+
		tbuf = mtkit_iso8859_to_utf8 ( (char *)vb, 0, NULL );
		if ( ! tbuf )
		{
			goto finish;
		}
	}
	else
	{
		tbuf = (char *)malloc ( (size_t)vb_size );
		if ( ! tbuf )
		{
			goto finish;
		}

		memcpy ( tbuf, vb, (size_t)vb_size );
	}

finish:
	mtkit_file_close ( memfile );

	return tbuf;
}

