/*
	Copyright (C) 2020 Mark Tyler

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

#include "private.h"



typedef int (* utFunc) ( void );



typedef struct
{
	char	const * const	name;
	utFunc		const	func;
} utComRec;




Global			global;

static utComRec const	* comrec;	// Current command record
					// (NULL = cedutils)



static utComRec const	comtab[] = {
	{ "decbot",	dwut_decbot },
	{ "decfont",	dwut_decfont },
	{ "dechg",	dwut_dechg },
	{ "decsoda",	dwut_decsoda },
	{ "encbot",	dwut_encbot },
	{ "encfont",	dwut_encfont },
	{ "enchg",	dwut_enchg },
	{ "encsoda",	dwut_encsoda },
	{ NULL,		NULL }
	};

// Note: must match ERROR_* in private.h
static char const * const ff_errtab[] = {
	"Unknown",				// 0
	"No bottle selected",			// 1
	"Unable to encode soda",		// 2
	"Unable to decode soda",		// 3
	"Unable to encode bottle",		// 4
	"Unable to decode bottle",		// 5
	"Unable to open database",		// 6
	"Unable to encode font",		// 7
	"Unable to decode font",		// 8
	"Unable to encode homoglyphs",		// 9
	"Unable to decode homoglyphs"		// 10
	};



#define FF_ERRTAB_LEN	( sizeof ( ff_errtab ) / sizeof ( ff_errtab[0] ) )



static const utComRec * get_comrec (
	char	const *	command
	)
{
	if (	command[0] == 'd' &&
		command[1] == 'w'
		)
	{
		command += 2;
	}

	for ( int i = 0; comtab[i].name; i++ )
	{
		if ( 0 == strcmp ( command, comtab[i].name ) )
		{
			return &comtab[i];	// Found
		}
	}

	return NULL;			// Not found
}

static int argcb_com (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	comrec = get_comrec ( global.s_arg );

	if ( ! comrec )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such command '%s'\n",
			global.s_arg );

		global.exit.set_value ( 1 );

		return 1;
	}

	return 0;			// Continue parsing args
}


static int file_func (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( comrec && comrec->func )
	{
		global.s_arg = filename;

		// Check we have loaded a DB
		if ( ! global.db.get_butt () )
		{
			if ( global.db.open ( global.s_db_path ) )
			{
				global.exit.set_value ( 1 ); // Terminate ASAP
				return global.exit.value ();
			}

			global.s_db_path = "";
		}

		int res = comrec->func ();

		if ( res > 0 )
		{
			if ( (unsigned)res > (FF_ERRTAB_LEN - 1) )
			{
				res = 0;
			}

			fprintf ( stderr, "%s error: %s. arg = '%s'\n",
				comrec->name, ff_errtab[res], filename );

			global.exit.set_value ( 1 );	// Terminate ASAP
		}
	}

	return global.exit.value ();	// Keep parsing if no errors encountered
}

static int error_func (
	int		const	error,
	int		const	arg,
	int		const	argc,
	char	const * const	argv[],
	void		* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "error_func: Argument ERROR! - num = %i arg = %i/%i",
		error, arg, argc );

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n" );

	return 0;			// Keep parsing
}

static int print_version (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( comrec )
	{
		printf ( "dw%s - Part of %s\n\n", comrec->name, VERSION );
	}
	else
	{
		printf ( "%s\n\n", VERSION );
	}

	return 1;			// Stop parsing
}

static int print_help (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( comrec )
	{
		printf ( "dw%s - Part of %s\n"
			"For further information consult the man page "
			"dw%s(1) or the mtDataWell Handbook.\n"
			"\n"
			, comrec->name, VERSION, comrec->name );
	}
	else
	{
		printf ( "%s\n\n"
			"For further information consult the man page "
			"%s(1) or the mtDataWell Handbook.\n"
			"\n"
			, VERSION, BIN_NAME );
	}

	return 1;			// Stop parsing
}

int main (
	int		const	argc,
	char	const * const	argv[]
	)
{
	// Establish what command has been used to call this program
	global.s_arg = strrchr ( argv[0], MTKIT_DIR_SEP );

	if ( ! global.s_arg )
	{
		global.s_arg = argv[0];
	}
	else
	{
		global.s_arg ++;
	}

	comrec = get_comrec ( global.s_arg );

	// Parse & action the command line arguments
	int tmp = 0;
	mtArg const arg_list[] = {
		{ "-help", MTKIT_ARG_SWITCH, &tmp, 0, print_help },
		{ "-version", MTKIT_ARG_SWITCH, &tmp, 0,print_version },
		{ "bottle", MTKIT_ARG_STRING, &global.s_bottle, 0, NULL },
		{ "com", MTKIT_ARG_STRING, &global.s_arg, 1, argcb_com },
		{ "db", MTKIT_ARG_STRING, &global.s_db_path, 0, NULL },
		{ "font", MTKIT_ARG_INT, &global.i_font, 0, NULL },
		{ "o", MTKIT_ARG_STRING, &global.s_o, 0, NULL },
		{ "v", MTKIT_ARG_SWITCH, &global.i_verbose, 1, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};

	mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func, NULL );

	return global.exit.value ();
}

