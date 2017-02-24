/*
	Copyright (C) 2008-2016 Mark Tyler

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



static int file_func (
	char	const * const	filename,
	void		* const	user_data
	)
{
	((char const **) user_data)[0] = filename;

	return 0;			// Keep parsing
}

static int error_func (
	int		const	error,
	int		const	arg,
	int		const	argc,
	char	const * const	argv[],
	void		* const	ARG_UNUSED ( user_data )
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



#define RECENT_FILE	"recent.file"



Backend::Backend ()
	:
	recent_file	( RECENT_FILE, 20 ),
	force_tsvcsv	(0),
	cline_filename	(),
	prefs_filename	()
{
}

Backend::~Backend ()
{
	preferences.save ();
}

char const * Backend::get_cline_filename () const
{
	return cline_filename;
}

int Backend::get_force_tsvcsv () const
{
	return force_tsvcsv;
}

int Backend::command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	int		show_version = 0;
	mtArg	const	arg_list[] = {
{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
{ "csv",	MTKIT_ARG_SWITCH, &force_tsvcsv, CED_FILE_FORCE_CSV, NULL },
{ "prefs",	MTKIT_ARG_STRING, &prefs_filename, 0, NULL },
{ "tsv",	MTKIT_ARG_SWITCH, &force_tsvcsv, CED_FILE_FORCE_TSV, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};


	mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func,
		&cline_filename );

	switch ( show_version )
	{
	case 1:
		printf ( "%s\n\n", VERSION );
		return 1;

	case 2:
		printf (
		"%s\n\n"
		"For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n"
		, VERSION, BIN_NAME );

		return 1;
	}

	prefs_init ();

	return 0;
}

