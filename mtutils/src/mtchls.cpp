/*
	Copyright (C) 2017 Mark Tyler

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

#include "mtchls.h"


class Backend;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class Backend
{
public:
	Backend ( int argc, char const * const * argv );

	int command_line ();
		// 0 = Continue running
		// 1 = Terminate program with 0

/// ----------------------------------------------------------------------------

private:

/// ----------------------------------------------------------------------------

	int			const	m_argc;
	char	const * const * const	m_argv;
};



Backend::Backend (
	int			const	argc,
	char	const * const * const	argv
	)
	:
	m_argc		( argc ),
	m_argv		( argv )
{
}

static void str_2_ascii (
	char	const * const	id,
	char		* const	id_ascii,
	size_t			len
	)
{
	for ( size_t i = 0; i < len; i++ )
	{
		if ( isascii ( id[i] ) )
		{
			id_ascii[i] = id[i];
		}
		else
		{
			id_ascii[i] = ' ';
		}
	}
}

static void id_print_ascii (
	char	const * const	id_ascii,
	char	const * const	txt
	)
{
	printf ( " |  %s Header = '%c%c%c%c' \n", txt, id_ascii[0], id_ascii[1],
		id_ascii[2], id_ascii[3] );
}

static int file_func (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	puts ( filename );

	mtKit::ChunkFile::Load file;

	char		id [ mtKit::ChunkFile::CHUNK_HEADER_SIZE ];
	char		id_ascii [ sizeof(id) ];

	if ( file.open ( filename, id ) )
	{
		printf ( " |-- Error - Unable to open file\n" );
		goto finish;
	}

	str_2_ascii ( id, id_ascii, sizeof(id_ascii) );
	id_print_ascii ( id_ascii, "" );

	for ( int chunk = 1; ; chunk++ )
	{
		uint32_t buflen, buflen_enc;

		switch ( file.get_chunk ( NULL, &buflen, id, &buflen_enc ) )
		{
		case 0:
			printf ( " |---Chunk %i\n", chunk );
			str_2_ascii ( id, id_ascii, sizeof(id) );
			id_print_ascii ( id_ascii, "  |" );
			printf ( " |    | Size = %" PRIu32 " (%" PRIu32
				" %.1f%%)\n", buflen, buflen_enc,
				(100 * (double)buflen_enc / buflen ) );
			break;

		case mtKit::ChunkFile::INT_EOF:
			printf ( " |---EOF\n" );
			goto finish;

		case mtKit::ChunkFile::INT_ERROR:
			printf ( " |---Error - continuing\n" );
			break;

		case mtKit::ChunkFile::INT_ERROR_FATAL:
			printf ( " |---Error - fatal\n" );
			goto finish;

		default:
			printf ( " |---Error - unknown\n" );
			goto finish;
		}
	}

finish:
	puts ( "" );

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

int Backend::command_line ()
{
	int	show_version	= 0;

	mtArg	const	arg_list[] = {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};


	mtkit_arg_parse( m_argc, m_argv, arg_list, file_func, error_func, NULL);

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

	return 0;
}

