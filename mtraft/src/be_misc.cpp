/*
	Copyright (C) 2011-2020 Mark Tyler

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



static int print_version ()
{
	printf ( "%s\n\n", VERSION );

	return 1;		// Stop parsing
}

static int print_help ()
{
	print_version ();

	printf ("For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n"
		, BIN_NAME );

	return 1;		// Stop parsing
}

int raft_cline (
	int			const	argc,
	char	const * const * const	argv,
	char	const **	const	path
	)
{
	char const * scan_directory = nullptr;

	mtKit::Arg args ( [&scan_directory]( char const * const filename )
		{
			if ( ! scan_directory )
			{
				scan_directory = filename;
			}
			return 0; 	// Continue parsing
		} );

	int stop = 0;

	args.add ( "-help",	stop, 1, print_help );
	args.add ( "-version",	stop, 1, print_version );

	args.parse ( argc, argv );

	if ( stop )
	{
		return 1;		// Quit program
	}

	if ( path )
	{
		path[0] = scan_directory;
	}

	return 0;			// Success
}

char * raft_get_clipboard (
	CedSheet	* const	sheet
	)
{
	char		* tbuf = NULL;

	mtFile * memfile = ced_sheet_save_mem ( sheet, CED_FILE_TYPE_TSV_VALUE);
	if ( ! memfile )
	{
		return NULL;
	}

	char		ch = 0;
	void		* vb = NULL;
	int64_t		vb_size = -1;

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
		// Create proper UTF-8. WARNING! Assumes locale.
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
	memfile = NULL;

	return tbuf;
}

