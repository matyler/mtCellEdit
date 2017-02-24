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

#include "private.h"



typedef int (* utFunc) ( void );



typedef struct
{
	char	const * const	name;
	utFunc		const	func;
} utComRec;

typedef struct
{
	char	const * const	name;
	int		const	id;
} utFTRec;



GLOBAL			global;

static utComRec const	* comrec;	// Current command record
					// (NULL = cedutils)



//	Command table is strictly alpha sorted for quick binary searching
static utComRec const	comtab[] = {
	{ "append",	cedut_append },
	{ "clear",	cedut_clear },
	{ "cut",	cedut_cut },
	{ "diff",	cedut_diff },
	{ "eval",	cedut_eval },
	{ "find",	cedut_find },
	{ "flip",	cedut_flip },
	{ "insert",	cedut_insert },
	{ "ls",		cedut_ls },
	{ "paste",	cedut_paste },
	{ "rotate",	cedut_rotate },
	{ "set",	cedut_set },
	{ "sort",	cedut_sort },
	{ "transpose",	cedut_transpose }
	};

//	File type table is strictly alpha sorted for quick binary searching
static utFTRec const ftypes[] = {
	{ "csv_content",	CED_FILE_TYPE_CSV_CONTENT },
	{ "csv_content_noq",	CED_FILE_TYPE_CSV_CONTENT_NOQ },
	{ "csv_value",		CED_FILE_TYPE_CSV_VALUE },
	{ "csv_value_noq",	CED_FILE_TYPE_CSV_VALUE_NOQ },
	{ "input",		CED_FILE_TYPE_NONE },
	{ "ledger",		CED_FILE_TYPE_LEDGER },
	{ "ledger_gz",		CED_FILE_TYPE_LEDGER_GZ },
	{ "ledger_val",		CED_FILE_TYPE_LEDGER_VAL },
	{ "ledger_val_gz",	CED_FILE_TYPE_LEDGER_VAL_GZ },
	{ "output_html",	CED_FILE_TYPE_OUTPUT_HTML },
	{ "output_tsv",		CED_FILE_TYPE_OUTPUT_TSV },
	{ "output_tsv_q",	CED_FILE_TYPE_OUTPUT_TSV_QUOTED },
	{ "tsv_content",	CED_FILE_TYPE_TSV_CONTENT },
	{ "tsv_content_gz",	CED_FILE_TYPE_TSV_CONTENT_GZ },
	{ "tsv_content_noq",	CED_FILE_TYPE_TSV_CONTENT_NOQ },
	{ "tsv_value",		CED_FILE_TYPE_TSV_VALUE },
	{ "tsv_value_gz",	CED_FILE_TYPE_TSV_VALUE_GZ },
	{ "tsv_value_noq",	CED_FILE_TYPE_TSV_VALUE_NOQ }
	};

// Note: must match ERROR_* in private.h
static char const * const ff_errtab[] = {
	"Unknown",				// 0
	"Unable to load file",			// 1
	"Unexpected libmtcelledit failure"	// 2
	};

#define COMTAB_LEN	( sizeof ( comtab ) / sizeof ( comtab[0] ) )
#define FTYPES_LEN	( sizeof ( ftypes ) / sizeof ( ftypes[0] ) )
#define FF_ERRTAB_LEN	( sizeof ( ff_errtab ) / sizeof ( ff_errtab[0] ) )



static const utComRec * get_comrec (
	char	const * const	command
	)
{
	int		a,
			b,
			c,
			cmp;


	a = 0;
	b = COMTAB_LEN - 1;

	while ( a <= b )
	{
		c = (a + b) / 2;
		cmp = strcmp ( command, comtab[c].name );

		if ( cmp == 0 )
		{
			return &comtab[c];	// Found
		}

		if ( cmp < 0 )
		{
			b = c - 1;
		}
		else	// cmp > 0
		{
			a = c + 1;
		}
	}


	return NULL;			// Not found
}

static int get_filetype ( void )
{
	int		a,
			b,
			c,
			cmp;


	a = 0;
	b = FTYPES_LEN - 1;

	while ( a <= b )
	{
		c = (a + b) / 2;
		cmp = strcmp ( global.s_arg, ftypes[c].name );

		if ( cmp == 0 )
		{
			global.i_ftype_out = ftypes[c].id;

			return 0;	// Found
		}

		if ( cmp < 0 )
		{
			b = c - 1;
		}
		else	// cmp > 0
		{
			a = c + 1;
		}
	}


	return 1;			// Not found
}

static int file_func (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( comrec && comrec->func )
	{
		int		res;


		global.s_arg = filename;

		res = comrec->func ();

		if ( res > 0 )
		{
			if ( (unsigned)res > (FF_ERRTAB_LEN - 1) )
			{
				res = 0;
			}

			fprintf ( stderr, "%s error: %s. arg = '%s'\n",
				comrec->name, ff_errtab[res], filename );

			global.i_error = 1;	// Terminate ASAP
		}
	}

	return global.i_error;		// Keep parsing if no errors encountered
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

static void select_command ( void )
{
	if (	global.s_arg[0] == 'c' &&
		global.s_arg[1] == 'e' &&
		global.s_arg[2] == 'd'
		)
	{
		global.s_arg += 3;
	}

	comrec = get_comrec ( global.s_arg );
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
		printf ( "ced%s - Part of %s\n\n", comrec->name, VERSION );
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
		printf ( "ced%s - Part of %s\n"
			"For further information consult the man page "
			"ced%s(1) or the mtCellEdit Handbook.\n"
			"\n"
			, comrec->name, VERSION, comrec->name );
	}
	else
	{
		printf ( "%s\n\n"
			"For further information consult the man page "
			"%s(1) or the mtCellEdit Handbook.\n"
			"\n"
			, VERSION, BIN_NAME );
	}

	return 1;			// Stop parsing
}

static int init_globals ( void )
{
	ced_init ();

	memset ( &global, 0, sizeof ( global ) );

	global.i_range[0] = 1;
	global.i_range[1] = CED_MAX_ROW;
	global.i_range[2] = 1;
	global.i_range[3] = CED_MAX_COLUMN;
	global.i_dest[0] = 1;
	global.i_dest[1] = 1;
	global.i_dest[2] = 1;
	global.i_dest[3] = 1;
	global.i_start = 1;
	global.i_total = 1;
	global.i_ftype_in = CED_FILE_TYPE_TSV_CONTENT;
	global.i_ftype_out = CED_FILE_TYPE_TSV_CONTENT;

	global.sheet = ced_sheet_new ();
	if ( ! global.sheet )
	{
		global.i_error = 1;

		return 1;
	}

	return 0;			// Success
}

static void cleanup_globals ( void )
{
	ced_sheet_destroy ( global.sheet );
	global.sheet = NULL;
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
		global.i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

static int parse_cellrange (
	int		idata[4]
	)
{
	CedCellRef	r1,
			r2;


	if ( strchr ( global.s_arg, ':' ) )
	{
		// User has entered a range

		if ( ced_strtocellrange ( global.s_arg, &r1, &r2, NULL, 1 ) )
		{
			goto error;
		}
	}
	else
	{
		// User has entered a cell reference

		if ( ced_strtocellref ( global.s_arg, &r1, NULL, 1 ) )
		{
			goto error;
		}

		r2 = r1;
	}

	// Disallow relative r1/r2
	if (	r1.row_m ||
		r1.col_m ||
		r2.row_m ||
		r2.col_m
		)
	{
		goto error;
	}

	// Lowest row/col first for deterministic calculations later
	idata[0] = MIN ( r1.row_d, r2.row_d );
	idata[1] = MAX ( r1.row_d, r2.row_d );
	idata[2] = MIN ( r1.col_d, r2.col_d );
	idata[3] = MAX ( r1.col_d, r2.col_d );

	return 0;

error:

	fprintf ( stderr, "Error: Bad cell range or reference '%s'\n",
		global.s_arg );
	global.i_error = 1;

	return 1;
}

static int argcb_dest (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	return parse_cellrange ( global.i_dest );
}

static int argcb_range (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	return parse_cellrange ( global.i_range );
}

static int check_stdinout (
	char	const * const	filename
	)
{
	if (	filename		&&
		filename[0] == '-'	&&
		filename[1] == 0
		)
	{
		return 0;
	}

	return -1;
}

static int check_load_stdin (
	char	const * const	filename,
	char	**	const	buf,
	size_t		* const	buflen
	)
{
	int		res = -1;


	if ( 0 == check_stdinout ( filename ) )
	{
		res = mtkit_file_load_stdin ( buf, buflen );
		if ( res )
		{
			fprintf ( stderr, "Error during mtkit_file_load_stdin."
				"\n" );
		}
	}

	return res;
}

int ut_load_file ( void )
{
	CedSheet	* new_sheet = NULL;
	int		new_type;
	int		p;
	char		* buf = NULL;
	size_t		buflen;


	if ( ! global.s_arg )
	{
		global.i_error = 1;

		return 1;		// Fail
	}

	if ( global.s_arg[0] == 0 )
	{
		// User sent us "" which means action the current sheet

		return 0;
	}

	p = check_load_stdin ( global.s_arg, &buf, &buflen );

	if ( global.i_csv )
	{
		switch ( p )
		{
		case -1:	// No stdin requested
			new_sheet = ced_sheet_load_csv ( global.s_arg, NULL );
			break;

		case 0:		// Loaded stdin
			new_sheet = ced_sheet_load_csv_mem ( buf, buflen, NULL);
			break;
		}

		new_type = CED_FILE_TYPE_CSV_CONTENT;
	}
	else
	{
		switch ( p )
		{
		case -1:	// No stdin requested
			new_sheet = ced_sheet_load ( global.s_arg, NULL,
				&new_type );
			break;

		case 0:		// Loaded stdin
			new_sheet = ced_sheet_load_mem ( buf, buflen, NULL,
				&new_type );
			break;
		}
	}

	free ( buf );

	if ( ! new_sheet )
	{
		global.i_error = 1;

		return 1;		// Fail
	}

	ced_sheet_destroy ( global.sheet );
	global.sheet = new_sheet;
	global.i_ftype_in = new_type;

	return 0;			// Success
}

static int argcb_i (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( ut_load_file () )
	{
		fprintf ( stderr, "Error: Unable to load file '%s'\n",
			global.s_arg );

		global.i_error = 1;

		return 1;		// Failure, so stop parsing
	}

	return 0;			// Continue parsing args
}

static int argcb_o (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	int		ftype;


	if ( global.i_ftype_out == CED_FILE_TYPE_NONE )
	{
		ftype = global.i_ftype_in;
	}
	else
	{
		ftype = global.i_ftype_out;
	}

	if ( 0 == check_stdinout ( global.s_arg ) )
	{
		int		res = 0;
		mtFile		* file;
		void		* buf;


		file = ced_sheet_save_mem ( global.sheet, ftype );
		if (	mtkit_file_write ( file, "", 1 )	||
			mtkit_file_get_mem ( file, &buf, NULL )
			)
		{
			fprintf ( stderr, "Error: Unable to save file to stdout"
				"\n" );

			global.i_error = 1;
			res = 1;
		}
		else
		{
			fputs ( (char const *)buf, stdout );
		}

		mtkit_file_close ( file );

		return res;
	}

	if ( ced_sheet_save ( global.sheet, global.s_arg, ftype ) )
	{
		fprintf ( stderr, "Error: Unable to save file '%s'\n",
			global.s_arg );

		global.i_error = 1;

		return 1;		// Failure, so stop parsing
	}

	return 0;			// Continue parsing args
}

static int argcb_otype (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if ( strcmp ( global.s_arg, "list" ) == 0 )
	{
		size_t		i;


		printf ( "Valid file types:\n\n" );

		for ( i = 0; i < FTYPES_LEN ; i++ )
		{
			printf ( "%s\n", ftypes[i].name );
		}
	}
	else if ( get_filetype () )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such file format '%s'\n",
			global.s_arg );

		global.i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

static int argcb_recalc (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	int		i;


	for ( i = 0; i < global.i_tmp; i++ )
	{
		ced_sheet_recalculate ( global.sheet, NULL, 0 );
		ced_sheet_recalculate ( global.sheet, NULL, 1 );
	}

	return 0;			// Continue parsing args
}



static mtArg const arg_list[] = {
{ "-help",	MTKIT_ARG_SWITCH,	&global.i_tmp, 1, print_help },
{ "-version",	MTKIT_ARG_SWITCH,	&global.i_tmp, 2, print_version },

{ "anti",	MTKIT_ARG_SWITCH,	&global.i_clock, 1, NULL },
{ "case",	MTKIT_ARG_SWITCH,	&global.i_case, 1, NULL },
{ "clock",	MTKIT_ARG_SWITCH,	&global.i_clock, 0, NULL },
{ "col",	MTKIT_ARG_SWITCH,	&global.i_rowcol, 1, NULL },
{ "com",	MTKIT_ARG_STRING,	&global.s_arg, 1, argcb_com },
{ "csv",	MTKIT_ARG_SWITCH,	&global.i_csv, 1, NULL },
{ "dest",	MTKIT_ARG_STRING,	&global.s_arg, 0, argcb_dest },
{ "i",		MTKIT_ARG_STRING,	&global.s_arg, 0, argcb_i },
{ "num",	MTKIT_ARG_SWITCH,	&global.i_num, 1, NULL },
{ "o",		MTKIT_ARG_STRING,	&global.s_arg, 0, argcb_o },
{ "otype",	MTKIT_ARG_STRING,	&global.s_arg, 0, argcb_otype },
{ "range",	MTKIT_ARG_STRING,	&global.s_arg, 0, argcb_range },
{ "recalc",	MTKIT_ARG_INT,		&global.i_tmp, 0, argcb_recalc },
{ "row",	MTKIT_ARG_SWITCH,	&global.i_rowcol, 0, NULL },
{ "start",	MTKIT_ARG_INT,		&global.i_start, 0, NULL },
{ "total",	MTKIT_ARG_INT,		&global.i_total, 0, NULL },
{ "tsv",	MTKIT_ARG_SWITCH,	&global.i_csv, 0, NULL },
{ "v",		MTKIT_ARG_SWITCH,	&global.i_verbose, 1, NULL },
{ "wildcard",	MTKIT_ARG_SWITCH,	&global.i_wildcard, 1, NULL },

{ NULL, 0, NULL, 0, NULL }
};



int main (
	int		const	argc,
	char	const * const	argv[]
	)
{
	if ( init_globals () )
	{
		return 1;
	}

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

	select_command ();

	// Parse & action the command line arguments
	mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func, NULL );

	cleanup_globals ();

	return global.i_error;
}

