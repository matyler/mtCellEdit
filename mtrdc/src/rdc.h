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

extern "C" {

	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <errno.h>
}

#include <mtcelledit.h>



typedef struct rdcPadFile	rdcPadFile;



#define RDC_ITERATIONS_MIN		1
#define RDC_ITERATIONS_MAX		1000000000
#define RDC_PASSWORD_MIN_LEN		4
#define RDC_PASSWORD_MAX_LEN		128
#define RDC_PASSWORD_CHARS_MIN_LEN	2
#define RDC_PASSWORD_CHARS_MAX_LEN	128



char	const *	get_arg_i		( void );
int		get_arg_iterations	( void );
int		get_arg_matrix_cols	( void );
int		get_arg_matrix_rows	( void );
char	const *	get_arg_o		( void );
char	const *	get_arg_pad		( void );
int		get_arg_pad_start	( void );
char	const *	get_arg_password_chars	( void );
int		get_arg_password_len	( void );

void		set_exit_fail		( void );


// Functions return: 0 = success, NULL = fail; unless otherwise stated.

int validate_arg_matrix_cols	( void );
int validate_arg_matrix_rows	( void );
int validate_arg_i		( void );
int validate_arg_iterations	( void );
int validate_arg_o		( void );
int validate_arg_pad		( void );
int validate_arg_pad_start	( void );
int validate_arg_password_chars	( void );
int validate_arg_password_len	( void );

// Action calls

int create_matrix	(mtArg const *, int, int, char const * const *, void *);
int create_passwords	(mtArg const *, int, int, char const * const *, void *);
int create_prng		(mtArg const *, int, int, char const * const *, void *);
int create_shuffle	(mtArg const *, int, int, char const * const *, void *);
int create_unshuffle	(mtArg const *, int, int, char const * const *, void *);
int create_xor		(mtArg const *, int, int, char const * const *, void *);
int print_analysis	(mtArg const *, int, int, char const * const *, void *);

// No validation of the arg_* values is done in the functions below so it must
// be done in the main Action calls above.

FILE * rdc_open_input	( void );
FILE * rdc_open_output	( void );
void rdc_fclose		( FILE * fp );

void * rdc_malloc (
	size_t		buf_size,
	char	const *	txt
	);

rdcPadFile * rdc_pad_open ( void );

int rdc_pad_read (
	rdcPadFile	* padfile,
	unsigned char	* buf,
	size_t		buf_size
	);

int rdc_pad_close (
	rdcPadFile	* padfile
	);

