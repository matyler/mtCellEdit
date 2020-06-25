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

#include "mtpts2ply.h"


class Backend;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class Backend
{
public:
	Backend ( int argc, char const * const * argv );

	int command_line ();
		// 0 = Continue running
		// 1 = Terminate program with 0

	int load_pts_file ( char const * filename );

	inline int get_tmp () const		{ return m_tmp; }
	inline int get_file_tot () const	{ return m_file_tot; };

/// ----------------------------------------------------------------------------

	pCloud		cloud;

private:

/// ----------------------------------------------------------------------------

	int			const	m_argc;
	char	const * const * const	m_argv;

	int		m_tmp;
	int		m_file_tot;
	char	const *	m_txt;
};



Backend::Backend (
	int			const	argc,
	char	const * const * const	argv
	)
	:
	m_argc		( argc ),
	m_argv		( argv ),
	m_tmp		( 0 ),
	m_file_tot	( 0 ),
	m_txt		( NULL )
{
}

static int file_func (
	char	const * const	filename,
	void		* const	user_data
	)
{
	Backend * backend = static_cast<Backend *>(user_data);

	backend->load_pts_file ( filename );

	return 0;		// Continue parsing
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

static int set_limit (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	user_data
	)
{
	Backend * backend = static_cast<Backend *>(user_data);

	return backend->cloud.set_limit ( backend->get_tmp () ) ? 1 : 0;
}

static int set_format (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	arg,
	int		const	ARG_UNUSED ( argc ),
	char	const * const	argv[],
	void		* const	user_data
	)
{
	Backend * backend = static_cast<Backend *>(user_data);

	int format = pCloud::FORMAT_PLY;
	mtKit::CharInt	const ctab[] = {
		{ "ply",	pCloud::FORMAT_PLY },
		{ "pts",	pCloud::FORMAT_PTS },
		{ NULL, 0 }
		};

	if ( mtKit::cli_parse_charint ( argv[ arg ], ctab, format ) )
	{
		std::cerr << "No such format name: " << argv[ arg ] << "\n";

		return 1;
	}

	return backend->cloud.set_format ( format ) ? 1 : 0;
}

static int set_slices (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	user_data
	)
{
	Backend * backend = static_cast<Backend *>(user_data);

	return backend->cloud.set_slices ( backend->get_tmp () );
}

static int set_output (
	mtArg	const *	const	ARG_UNUSED ( mtarg ),
	int		const	arg,
	int		const	ARG_UNUSED ( argc ),
	char	const * const	argv[],
	void		* const	user_data
	)
{
	Backend * backend = static_cast<Backend *>(user_data);

	backend->cloud.set_output_filename ( argv[ arg ] );

	return 0;
}

int Backend::command_line ()
{
	int	show_version	= 0;

	mtArg	const	arg_list[] = {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ "n",		MTKIT_ARG_INT, &m_tmp, 0, set_limit },
		{ "o",		MTKIT_ARG_STRING, &m_txt, 0, set_output },
		{ "slices",	MTKIT_ARG_INT, &m_tmp, 0, set_slices },
		{ "format",	MTKIT_ARG_STRING, &m_txt, 0, set_format },
		{ NULL, 0, NULL, 0, NULL }
		};


	mtkit_arg_parse( m_argc, m_argv, arg_list, file_func, error_func, this);

	if ( show_version )
	{
		printf ( "%s (part of %s)\n\n", m_argv[0], VERSION );

		if ( 2 == show_version )
		{
			printf (
				"For further information consult the man page "
				"%s(1) or the mtCellEdit Handbook.\n"
				"\n", m_argv[0] );
		}

		return 1;		// Quit program
	}

	return 0;			// Continue program
}

int Backend::load_pts_file ( char const * const filename )
{
	m_file_tot++;

	mtKit::LineFileRead file;

	if ( file.open ( filename ) )
	{
		return 1;
	}

	return cloud.load ( file );
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Backend backend ( argc, argv );

	switch ( backend.command_line () )
	{
	case 0: break;		// Normal termination
	case 1: return 0;	// Terminate now successfully as requested
	default: return 1;	// Terminate now with error (unknown reason)
	}

	if ( backend.get_file_tot () < 1 )
	{
		mtKit::LineFileRead file;

		file.open ( stdin );

		if ( backend.cloud.load ( file ) )
		{
			return 1;
		}
	}

	backend.cloud.print_data ();

	return 0;
}

