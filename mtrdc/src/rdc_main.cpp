/*
	Copyright (C) 2013-2016 Mark Tyler

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

static int		exit_val		= 0;



static int print_version (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	printf ( "%s\n\n", VERSION );

	return 1;			// Stop parsing
}

static int print_help (
	mtArg		const *	const	mtarg,
	int			const	arg,
	int			const	argc,
	char	const * const * const	argv,
	void			* const	user_data
	)
{
	print_version ( mtarg, arg, argc, argv, user_data );

	printf ( "For further information consult the man page"
		" %s(1) or the mtCellEdit Handbook.\n\n"
		, BIN_NAME );

	return 1;			// Stop parsing
}

static int file_func (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "Argument error: unexpected filename '%s'.\n\n",
		filename );

	set_exit_fail ();

	return 1;
}

static int error_func (
	int			const	error,
	int			const	arg,
	int			const	argc,
	char	const * const * const	argv,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "Argument error %i: arg = %i/%i", error, arg, argc);

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n\n" );

	set_exit_fail ();

	return 1;			// Stop parsing
}

void set_exit_fail ( void )
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

static int set_seed (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	srand ( (unsigned int)arg_seed );

	return 0;
}

int validate_arg_o ( void )
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

int validate_arg_i ( void )
{
	return validate_input_file (
		arg_i,
		"No input filename.\n\n",
		"Input file not readable.\n\n" );
}

int validate_arg_pad ( void )
{
	return validate_input_file (
		arg_pad,
		"No pad filename.\n\n",
		"Pad file not readable.\n\n" );
}

int validate_arg_iterations ( void )
{
	return mtkit_arg_int_boundary_check ( "iterations", arg_iterations,
		RDC_ITERATIONS_MIN, RDC_ITERATIONS_MAX );
}

int validate_arg_matrix_cols ( void )
{
	return mtkit_arg_int_boundary_check ( "matrix-cols", arg_matrix_cols, 1,
		CED_MAX_COLUMN );
}

int validate_arg_matrix_rows ( void )
{
	return mtkit_arg_int_boundary_check ( "matrix-rows", arg_matrix_rows, 1,
		CED_MAX_ROW );
}

int validate_arg_pad_start ( void )
{
	return mtkit_arg_int_boundary_check ( "pad-start", arg_pad_start, 0,
		INT_MAX );
}

int validate_arg_password_chars ( void )
{
	return mtkit_arg_string_boundary_check ( "password-chars",
		arg_password_chars, RDC_PASSWORD_CHARS_MIN_LEN,
		RDC_PASSWORD_CHARS_MAX_LEN );
}

int validate_arg_password_len ( void )
{
	return mtkit_arg_int_boundary_check ( "password-len", arg_password_len,
		RDC_PASSWORD_MIN_LEN, RDC_PASSWORD_MAX_LEN );
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	int		tmp;

	mtArg	const	arg_list[] = {
	{ "-help",		MTKIT_ARG_SWITCH, &tmp, 0, print_help },
	{ "-version",		MTKIT_ARG_SWITCH, &tmp, 0, print_version },
	{ "create-matrix",	MTKIT_ARG_SWITCH, &tmp, 0, create_matrix },
	{ "create-passwords",	MTKIT_ARG_SWITCH, &tmp, 0, create_passwords },
	{ "create-prng",	MTKIT_ARG_SWITCH, &tmp, 0, create_prng },
	{ "create-shuffle",	MTKIT_ARG_SWITCH, &tmp, 0, create_shuffle },
	{ "create-unshuffle",	MTKIT_ARG_SWITCH, &tmp, 0, create_unshuffle },
	{ "create-xor",		MTKIT_ARG_SWITCH, &tmp, 0, create_xor },
	{ "i",			MTKIT_ARG_STRING, &arg_i, 0, NULL },
	{ "iterations",		MTKIT_ARG_INT, &arg_iterations, 0, NULL },
	{ "matrix-cols",	MTKIT_ARG_INT, &arg_matrix_cols, 0, NULL },
	{ "matrix-rows",	MTKIT_ARG_INT, &arg_matrix_rows, 0, NULL },
	{ "o",			MTKIT_ARG_STRING, &arg_o, 0, NULL },
	{ "pad",		MTKIT_ARG_STRING, &arg_pad, 0, NULL },
	{ "pad-start",		MTKIT_ARG_INT, &arg_pad_start, 0, NULL },
	{ "password-chars",	MTKIT_ARG_STRING, &arg_password_chars, 0, NULL},
	{ "password-len",	MTKIT_ARG_INT, &arg_password_len, 0, NULL },
	{ "print-analysis",	MTKIT_ARG_SWITCH, &tmp, 0, print_analysis },
	{ "seed",		MTKIT_ARG_INT, &arg_seed, 0, set_seed },
	{ NULL, 0, NULL, 0, NULL }
	};


	if ( ced_init () )
	{
		fprintf ( stderr, "Unable to start libmtcelledit.\n\n" );

		set_exit_fail ();
	}
	else
	{
		mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func,
			NULL );
	}

	return exit_val;
}

