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


// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class Backend
{
public:
	Backend ( int const argc, char const * const * const argv )
	:
	m_argc		( argc ),
	m_argv		( argv )
	{}

	int command_line ();
		// 0 = Continue running
		// 1 = Terminate program with 0

	int load_pts_file ( char const * filename );

	inline int get_file_tot () const	{ return m_file_tot; };

/// ----------------------------------------------------------------------------

	pCloud		cloud;

private:

/// ----------------------------------------------------------------------------

	int			const	m_argc;
	char	const * const * const	m_argv;

	int		m_tmp		= 0;
	int		m_file_tot	= 0;
	char	const *	m_txt		= nullptr;
};



static int set_format (
	Backend		* const backend,
	char	const * const	input
	)
{
	int format = pCloud::FORMAT_PLY;
	mtKit::CharInt	const ctab[] = {
		{ "ply",	pCloud::FORMAT_PLY },
		{ "pts",	pCloud::FORMAT_PTS },
		{ NULL, 0 }
		};

	if ( mtKit::cli_parse_charint ( input, ctab, format ) )
	{
		std::cerr << "No such format name: " << input << "\n";

		return 1;
	}

	return backend->cloud.set_format ( format ) ? 1 : 0;
}

int Backend::command_line ()
{
	int	show_version	= 0;

	mtKit::Arg args ( [this]( char const * const filename )
		{
			load_pts_file ( filename );
			return 0;
		} );

	args.add ( "-help",	show_version, 2 );
	args.add ( "-version",	show_version, 1 );

	args.add ( "n", m_tmp, [this]
		{
			return (cloud.set_limit ( m_tmp ) ? 1 : 0);
		} );

	args.add ( "o", m_txt, [this]
		{
			cloud.set_output_filename ( m_txt );
			return 0;
		} );

	args.add ( "slices", m_tmp, [this]
		{
			return cloud.set_slices ( m_tmp );
		} );

	args.add ( "format",	m_txt, [this]
		{
			return set_format ( this, m_txt );
		} );

	args.parse ( m_argc, m_argv );

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

