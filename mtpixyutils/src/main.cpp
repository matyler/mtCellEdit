/*
	Copyright (C) 2016 Mark Tyler

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




Global			global;

static utComRec const	* comrec;	// Current command record
					// (NULL = cedutils)



static utComRec const	comtab[] = {
	{ "ls",		pixyut_ls },
	{ "new",	pixyut_new },
	{ "resize",	pixyut_resize },
	{ "scale",	pixyut_scale },
	{ NULL,		NULL }
	};

static utFTRec const ftypes[] = {
	{ "bmp",	mtPixy::File::BMP },
	{ "gif",	mtPixy::File::GIF },
	{ "gpl",	mtPixy::File::GPL },
	{ "jpeg",	mtPixy::File::JPEG },
	{ "none",	mtPixy::File::NONE },
	{ "png",	mtPixy::File::PNG },
	{ NULL,		0 }
	};

static utFTRec const imtypes[] = {
	{ "indexed",	mtPixy::Image::INDEXED },
	{ "rgb",	mtPixy::Image::RGB },
	{ NULL,		0 }
	};

// Note: must match ERROR_* in private.h
static char const * const ff_errtab[] = {
	"Unknown",				// 0
	"Unable to load file",			// 1
	"Unexpected libmtpixy failure",		// 2
	"Bad palette"				// 3
	};



#define FF_ERRTAB_LEN	( sizeof ( ff_errtab ) / sizeof ( ff_errtab[0] ) )



static const utComRec * get_comrec (
	char	const * const	command
	)
{
	int		i;


	for ( i = 0; comtab[i].name; i++ )
	{
		if ( 0 == strcmp ( command, comtab[i].name ) )
		{
			return &comtab[i];	// Found
		}
	}


	return NULL;			// Not found
}

static int get_filetype ( void )
{
	int		i;


	for ( i = 0; ftypes[i].name; i++ )
	{
		if ( 0 == strcmp ( global.s_arg, ftypes[i].name ) )
		{
			global.i_ftype_out = ftypes[i].id;

			return 0;	// Found
		}
	}


	return 1;			// Not found
}

static int get_imtype ( void )
{
	int		i;


	for ( i = 0; imtypes[i].name; i++ )
	{
		if ( 0 == strcmp ( global.s_arg, imtypes[i].name ) )
		{
			global.i_image_type = imtypes[i].id;

			return 0;	// Found
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
	if (	global.s_arg[0] == 'p' &&
		global.s_arg[1] == 'i' &&
		global.s_arg[2] == 'x' &&
		global.s_arg[3] == 'y'
		)
	{
		global.s_arg += 4;
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
		printf ( "pixy%s - Part of %s\n\n", comrec->name, VERSION );
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
		printf ( "pixy%s - Part of %s\n"
			"For further information consult the man page "
			"pixy%s(1) or the mtPixy Handbook.\n"
			"\n"
			, comrec->name, VERSION, comrec->name );
	}
	else
	{
		printf ( "%s\n\n"
			"For further information consult the man page "
			"%s(1) or the mtPixy Handbook.\n"
			"\n"
			, VERSION, BIN_NAME );
	}

	return 1;			// Stop parsing
}

Global::Global (
	)
	:
	i_comp_png	( 5 ),
	i_comp_jpeg	( 85 ),
	i_error		( 0 ),
	i_ftype_in	( mtPixy::File::BMP ),
	i_ftype_out	( mtPixy::File::NONE ),
	i_height	( 100 ),
	i_image_type	( mtPixy::Image::INDEXED ),
	i_palette	( 0 ),
	i_scale		( 0 ),
	i_tmp		( 0 ),
	i_verbose	( 0 ),
	i_width		( 100 ),
	i_x		( 0 ),
	i_y		( 0 ),
	s_arg		(),
	image		()
{
}

static int init_globals ( void )
{
	mtPixy::Image	* im;


	im = mtPixy::image_create( mtPixy::Image::INDEXED, 100, 100 );
	if ( ! im )
	{
		global.i_error = 1;

		return 1;
	}

	global.set_image ( im );

	return 0;			// Success
}

static void cleanup_globals ( void )
{
	global.set_image ( NULL );
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

int ut_load_file ( void )
{
	mtPixy::Image		* new_image = NULL;
	mtPixy::File::Type	new_type;


	if ( ! global.s_arg )
	{
		global.i_error = 1;

		return 1;		// Fail
	}

	if ( global.s_arg[0] == 0 )
	{
		// User sent us "" which means action the current image

		return 0;
	}

	new_image = mtPixy::image_load ( global.s_arg, &new_type );
	if ( ! new_image )
	{
		return 1;		// Fail
	}

	global.set_image ( new_image );

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
	int		ftype, compress = 0;


	if ( global.i_ftype_out == mtPixy::File::NONE )
	{
		ftype = global.i_ftype_in;
	}
	else
	{
		ftype = global.i_ftype_out;
	}

	switch ( ftype )
	{
	case mtPixy::File::PNG:
		compress = global.i_comp_png;
		break;

	case mtPixy::File::JPEG:
		compress = global.i_comp_jpeg;
		break;
	}

	if ( global.image->save ( global.s_arg, (mtPixy::File::Type)ftype,
		compress ) )
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

		for ( i = 0; ftypes[i].name ; i++ )
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

static int argcb_imtype (
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


		printf ( "Valid image types:\n\n" );

		for ( i = 0; imtypes[i].name ; i++ )
		{
			printf ( "%s\n", imtypes[i].name );
		}
	}
	else if ( get_imtype () )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such image format '%s'\n",
			global.s_arg );
		global.i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

void Global::set_image (
	mtPixy::Image	* const	im
	)
{
	delete image;
	image = im;
}

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
	mtArg const arg_list[] = {
		{ "-help", MTKIT_ARG_SWITCH, &global.i_tmp, 1, print_help },
		{ "-version", MTKIT_ARG_SWITCH, &global.i_tmp, 2,print_version},
		{ "com", MTKIT_ARG_STRING, &global.s_arg, 1, argcb_com },
		{ "comp_png", MTKIT_ARG_INT, &global.i_comp_png, 0, NULL },
		{ "comp_jpeg", MTKIT_ARG_INT, &global.i_comp_jpeg, 0, NULL },
		{ "height", MTKIT_ARG_INT, &global.i_height, 0, NULL },
		{ "i", MTKIT_ARG_STRING, &global.s_arg, 0, argcb_i },
		{ "imtype", MTKIT_ARG_STRING, &global.s_arg, 0, argcb_imtype },
		{ "o", MTKIT_ARG_STRING, &global.s_arg, 0, argcb_o },
		{ "otype", MTKIT_ARG_STRING, &global.s_arg, 0, argcb_otype },
		{ "palette", MTKIT_ARG_INT, &global.i_palette, 0, NULL },
		{ "scale_blocky", MTKIT_ARG_SWITCH, &global.i_scale, 1, NULL },
		{ "v", MTKIT_ARG_SWITCH, &global.i_verbose, 1, NULL },
		{ "width", MTKIT_ARG_INT, &global.i_width, 0, NULL },
		{ "x", MTKIT_ARG_INT, &global.i_x, 0, NULL },
		{ "y", MTKIT_ARG_INT, &global.i_y, 0, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};

	mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func, NULL );

	cleanup_globals ();

	return global.i_error;
}

