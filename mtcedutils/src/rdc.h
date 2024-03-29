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

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <mtkit.h>
#include <mtcelledit.h>



#ifdef __cplusplus
extern "C" {
#endif


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
int		get_arg_verbose		( void );

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

int create_matrix	( void );
int create_passwords	( void );
int create_prng		( void );
int create_shuffle	( void );
int create_unshuffle	( void );
int create_xor		( void );
int print_analysis	( void );

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



#ifdef __cplusplus
}
#endif

