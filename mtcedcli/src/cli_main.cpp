/*
	Copyright (C) 2012-2016 Mark Tyler

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

#include <readline/readline.h>
#include <readline/history.h>



static char * get_line ( void )
{
	char		* line_read = NULL;


	line_read = readline ( BIN_NAME" > " );

	if ( line_read && line_read[0] )
	{
		add_history ( line_read );
	}

	return line_read;
}

static void main_loop (
	CedCli_STATE	* const	state
	)
{
	char		* input;


	while ( state->exit == 0 )
	{
		input = get_line ();

		if ( ! input )
		{
			break;
		}

		if ( input[0] )
		{
			cedcli_parse ( input, state );
		}

		free ( input );
	}

	clear_history ();
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

	return 0;		// Keep parsing
}

static int state_init (
	CedCli_STATE	* const	global,
	int		const	tab_text
	)
{
	int		i;


	ced_init ();

	for ( i = 0; i <= CEDCLI_FILE_MAX; i++ )
	{
		global->file_list[i] = cui_file_new ();

		if ( cui_file_book_new ( global->file_list[i] ) )
		{
			return 1;
		}
	}

	global->clipboard = cui_clip_new ();

	if (	! global->clipboard ||
		cedcli_jumptable_init ( global ) ||
		cedcli_prefs_init ( global ) )
	{
		return 1;
	}

	if ( tab_text )
	{
		rl_bind_key ( '\t', rl_insert );
	}

	return 0;
}

static void state_cleanup (
	CedCli_STATE	* const	global
	)
{
	int		i;


	cui_clip_free ( global->clipboard );
	global->clipboard = NULL;

	for ( i = 0; i <= CEDCLI_FILE_MAX; i++ )
	{
		cui_file_free ( global->file_list[i] );
		global->file_list[i] = NULL;
	}

	cedcli_prefs_free ( global );
	cedcli_jumptable_free ( global );
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	CedCli_STATE	global;
	int		show_version	= 0,
			show_about	= 1,
			tab_text	= 0;

	mtArg	const	arg_list[]	= {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ "q",		MTKIT_ARG_SWITCH, &show_about, 0, NULL },
		{ "t",		MTKIT_ARG_SWITCH, &tab_text, 1, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};


	memset ( &global, 0, sizeof(global) );
	rl_variable_bind ( "expand-tilde", "on" );
	mtkit_arg_parse ( argc, argv, arg_list, NULL, error_func, NULL );

	switch ( show_version )
	{
	case 1:	printf ( "%s\n\n", VERSION );
		exit ( 0 );

	case 2:	printf ( "%s\n\n"
		"For further information consult the man page %s(1) "
		"or the mtCellEdit Handbook.\n"
		"\n", VERSION, BIN_NAME );

		exit ( 0 );
	}

	if ( show_about )
	{
		jtf_about ( NULL, NULL );
	}

	if ( state_init ( &global, tab_text ) == 0 )
	{
		main_loop ( &global );
	}

	state_cleanup ( &global );

	if ( global.exit == CEDCLI_USER_QUIT )
	{
		return 0;
	}

	return ( global.exit );
}

