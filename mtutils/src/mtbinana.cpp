/*
	Copyright (C) 2018-2022 Mark Tyler

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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include <mtkit.h>


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



enum
{
	MODE_BYTE_SINGLE,
	MODE_BYTE_DOUBLE
};



static int m_analysis_mode = MODE_BYTE_SINGLE;



static void analyse_data_single (
	unsigned char	const * const	data,
	int			const	size
	)
{
	int	bits_used[8] = {0};
	int constexpr bit_mask[8] = {
			1 << 0,
			1 << 1,
			1 << 2,
			1 << 3,
			1 << 4,
			1 << 5,
			1 << 6,
			1 << 7
		};
	int	bytes_used[256] = {0};

	for ( int i = 0; i < size; i++ )
	{
		unsigned char const byte = data[i];

		bytes_used[ byte ]++;

		for ( int j = 0; j < 8; j++ )
		{
			if ( byte & bit_mask[j] )
			{
				bits_used[ j ]++;
			}
		}
	}

	double min_perc = 100.0;
	double max_perc = 0.0;

//	printf ( "Bit = 1:\n" );

	for ( int i = 0; i < 8; i++ )
	{
		double perc = 100.0 * (double)(bits_used[i]) / (double)size;

		printf ( "b_%i\t%.6f\n", i, perc );

		min_perc = MIN ( min_perc, perc );
		max_perc = MAX ( max_perc, perc );
	}

	min_perc = 100.0;
	max_perc = 0.0;

	int byte = 0;

	for ( int row = 0; row < 32; row ++ )
	{
		for ( int col = 0; col < 8; col ++ )
		{
			double const perc = 100.0 * (double)(bytes_used[byte]) /
					(double)size;

			printf ( "by_%03i\t%.6f\n", byte, perc );

			min_perc = MIN ( min_perc, perc );
			max_perc = MAX ( max_perc, perc );

			byte++;
		}
	}
}

static void analyse_data_double (
	unsigned char	const * const	data,
	int			const	size
	)
{
	int	tot[256][256];

	memset ( tot, 0, sizeof(tot) );

	for ( int i = 0; i < (size - 1); i++ )
	{
		int const r = data[ i ];
		int const c = data[ i+1 ];

		tot[r][c]++;
	}

	for ( int row = 0; row < 256; row ++ )
	{
		for ( int col = 0; col < 256; col ++ )
		{
			printf ( "%i\t", tot[row][col] );
		}

		printf ( "\n" );
	}
}

static int file_func ( char const * const filename )
{
	int size = 0;

	char * mem = mtkit_file_load ( filename, &size, 0, NULL );

	printf ( "%s\t%i\n", filename, size );

	if ( mem )
	{
		switch ( m_analysis_mode )
		{
		case MODE_BYTE_SINGLE:
			analyse_data_single ( (unsigned char *)mem, size );
			break;

		case MODE_BYTE_DOUBLE:
			analyse_data_double ( (unsigned char *)mem, size );
			break;

		default:
			fprintf ( stderr, "Bad mode = %i\n", m_analysis_mode );
			break;
		}

		free ( mem );
		mem = NULL;
	}
	else
	{
		printf ( "No data\n" );
	}

	puts ( "" );

	return 0;		// Continue parsing
}

int Backend::command_line ()
{
	int	show_version	= 0;

	mtKit::Arg args ( file_func );

	args.add ( "-help",	show_version, 2 );
	args.add ( "-version",	show_version, 1 );
	args.add ( "double",	m_analysis_mode, MODE_BYTE_DOUBLE, NULL );

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

