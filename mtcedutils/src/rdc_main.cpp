/*
	Copyright (C) 2013-2020 Mark Tyler

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



static char	const *	arg_i			= NULL;
static int		arg_iterations		= 1000000;
static int		arg_matrix_cols		= 1000;
static int		arg_matrix_rows		= 1000;
static char	const *	arg_o			= NULL;
static char	const *	arg_pad			= NULL;
static int		arg_pad_start		= 0;
static char	const *	arg_password_chars	=
						"0123456789"
						"abcdefghijklm"
						"nopqrstuvwxyz"
						;
static int		arg_password_len	= 10;
static int		arg_seed		= 0;
static int		arg_verbose		= 1;

static int		exit_val		= 0;



void set_exit_fail ()
{
	exit_val = 1;
}

char	const *	get_arg_i ( void ) {		return arg_i;		}
int		get_arg_iterations ( void ) {	return arg_iterations;	}
int		get_arg_matrix_cols ( void ) {	return arg_matrix_cols;	}
int		get_arg_matrix_rows ( void ) {	return arg_matrix_rows;	}
char	const *	get_arg_o ( void ) {		return arg_o;		}
char	const *	get_arg_pad ( void ) {		return arg_pad;		}
int		get_arg_pad_start ( void ) {	return arg_pad_start;	}
char	const *	get_arg_password_chars ( void ) { return arg_password_chars; }
int		get_arg_password_len ( void ) {	return arg_password_len; }
int		get_arg_verbose ( void ) {	return arg_verbose;	}



int validate_arg_o ()
{
	if ( ! arg_o )
	{
		fprintf ( stderr, "No output filename.\n\n" );

		return 1;
	}

	if ( mtkit_file_readable ( arg_o ) )
	{
		fprintf ( stderr, "Output file already exists.\n\n" );

		return 1;
	}

	return 0;			// File OK
}

static int validate_input_file (
	char	const * const	filename,
	char	const * const	txt_noname,
	char	const * const	txt_unreadable
	)
{
	if ( ! filename )
	{
		fputs ( txt_noname, stderr );

		return 1;
	}

	if ( ! mtkit_file_readable ( filename ) )
	{
		fputs ( txt_unreadable, stderr );

		return 1;
	}

	return 0;			// File OK
}

int validate_arg_i ()
{
	return validate_input_file (
		arg_i,
		"No input filename.\n\n",
		"Input file not readable.\n\n" );
}

int validate_arg_pad ()
{
	return validate_input_file (
		arg_pad,
		"No pad filename.\n\n",
		"Pad file not readable.\n\n" );
}

int validate_arg_iterations ()
{
	return mtkit_arg_int_boundary_check ( "iterations", arg_iterations,
		RDC_ITERATIONS_MIN, RDC_ITERATIONS_MAX );
}

int validate_arg_matrix_cols ()
{
	return mtkit_arg_int_boundary_check ( "matrix-cols", arg_matrix_cols, 1,
		CED_MAX_COLUMN );
}

int validate_arg_matrix_rows ()
{
	return mtkit_arg_int_boundary_check ( "matrix-rows", arg_matrix_rows, 1,
		CED_MAX_ROW );
}

int validate_arg_pad_start ()
{
	return mtkit_arg_int_boundary_check ( "pad-start", arg_pad_start, 0,
		INT_MAX );
}

int validate_arg_password_chars ()
{
	return mtkit_arg_string_boundary_check ( "password-chars",
		arg_password_chars, RDC_PASSWORD_CHARS_MIN_LEN,
		RDC_PASSWORD_CHARS_MAX_LEN );
}

int validate_arg_password_len ()
{
	return mtkit_arg_int_boundary_check ( "password-len", arg_password_len,
		RDC_PASSWORD_MIN_LEN, RDC_PASSWORD_MAX_LEN );
}

static int print_version ( char const * const app )
{
	printf ( "%s (part of %s)\n\n", app, VERSION );

	return 1;	// Stop parsing
}

static int print_help ( char const * const app )
{
	print_version ( app );

	printf ( "For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n", app );

	return 1;	// Stop parsing
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	ced_init ();

	mtKit::Arg args ( []( char const * const filename )
		{
			fprintf ( stderr, "Argument error: "
				"unexpected filename '%s'.\n\n", filename );

			set_exit_fail ();

			return 1;	// Stop parsing
		} );

	args.add ( "-help",	[argv]() { return print_help ( argv[0] ); } );
	args.add ( "-version",	[argv]() { return print_version ( argv[0] ); });
	args.add ( "create-matrix",	create_matrix );
	args.add ( "create-passwords",	create_passwords );
	args.add ( "create-prng",	create_prng );
	args.add ( "create-shuffle",	create_shuffle );
	args.add ( "create-unshuffle",	create_unshuffle );
	args.add ( "create-xor",	create_xor );
	args.add ( "i",			arg_i );
	args.add ( "iterations",	arg_iterations );
	args.add ( "matrix-cols",	arg_matrix_cols );
	args.add ( "matrix-rows",	arg_matrix_rows );
	args.add ( "o",			arg_o );
	args.add ( "pad",		arg_pad );
	args.add ( "pad-start",		arg_pad_start );
	args.add ( "password-chars",	arg_password_chars );
	args.add ( "password-len",	arg_password_len );
	args.add ( "print-analysis",	print_analysis );
	args.add ( "seed",		arg_seed, []
		{
			srand ( (unsigned int)arg_seed );
			return 0;
		} );

	args.parse ( argc, argv );

	return exit_val;
}

