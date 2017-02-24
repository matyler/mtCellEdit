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



static int64_t mtkit_file_size (
	char	const * const	filename
	)
{
	struct stat	buf;


	if ( ! filename )
	{
		return -1;
	}

	if ( stat ( filename, &buf ) )
	{
		return -2;
	}

	return buf.st_size;
}

static void mtkit_print_errno (
	int	const	e
	)
{
	char	const *	se;


	if ( e == 0 )
	{
		return;
	}

	se = strerror ( e );
	if ( ! se )
	{
		se = "?";
	}

	fprintf ( stderr, "errno = %i - %s\n\n", e, se );
}

static double fio_get_filesize (
	char	const * const	filename
	)
{
	int64_t		s64;
	double		size;


	s64 = mtkit_file_size ( filename );
	size = (double)s64;

	if ( size < 0 )
	{
		// Store current errno value just in case it gets changed
		int		e = errno;


		fprintf ( stderr, "Unable to access file '%s'\n\n", filename );
		mtkit_print_errno ( e );

		return -1;
	}

	return size;
}

static void print_update_progress (
	int	const	verbose,
	double	const	btot,
	double	const	filesize
	)
{
	if ( verbose )
	{
		fprintf ( stderr, "\rProgress = %.1f%%", 100 * btot / filesize);
		fflush ( stderr );
	}
}

static void print_update_progress_nl (
	int	const	verbose
	)
{
	if ( verbose )
	{
		fprintf ( stderr, "\n" );
	}
}

static void print_title (
	char	const * const	txt
	)
{
	char		* ds;
	char		* src;


	ds = strdup ( txt );

	if ( ! ds )
	{
		puts ( txt );

		return;
	}

	for ( src = ds; src[0] != 0; src ++ )
	{
		src[0] = '-';
	}

	puts ( ds );
	puts ( txt );
	puts ( ds );

	free ( ds );
}



#define SEQ_MAX		5



static int print_analysis_real ( void )
{
	FILE		* fp	= NULL;
	unsigned char	* buf	= NULL;

	int		i,
			seq	= 0,
			last	= -1,
			byte,
			delta,
			res	= 1;
			;
	int64_t		freq[256] = { 0 },
			ldelt[256] = { 0 },
			numrep[ SEQ_MAX ] = { 0 }
			;
	double		numrep_exp[ SEQ_MAX ],
			div,
			diff,
			norm,
			max,
			cum,
			filesize = 0,
			btot = 0
			;
	unsigned char	* src,
			* end
			;
	size_t		buflen;

	char	const * const	filename	= get_arg_i ();
	size_t		const	bufsize		= 1000000;
	int		const	verbose		= 1;
	static char const * const numrep_str[ SEQ_MAX ] = {
			"2", "3", "4", "5", "6+" };


	buf = (unsigned char *)rdc_malloc ( bufsize, "analysis" );
	if ( ! buf )
	{
		goto finish;
	}

	fp = rdc_open_input ();
	if ( ! fp )
	{
		goto finish;
	}

	filesize = fio_get_filesize ( filename );
	if ( filesize < 0 )
	{
		goto finish;
	}

	printf ( "Analysing '%s'\n", filename );

	buflen = fread ( buf, 1, bufsize, fp );
	if ( buflen < 1 )
	{
		goto finish;		// EOF
	}

	src = buf;
	end = src + buflen - 1;
	btot += (double)buflen;

	byte = *src++;
	freq[ byte ] ++;
	last = byte;

	while ( 1 )
	{
		if ( src > end )
		{
			buflen = fread ( buf, 1, bufsize, fp );
			if ( buflen < 1 )
			{
				break;		// EOF
			}

			src = buf;
			end = src + buflen - 1;

			btot += (double)buflen;
			print_update_progress ( verbose, btot, filesize );
		}

		byte = *src++;

		freq[ byte ] ++;

		delta = byte - last;

		if ( delta < 0 )
		{
			delta += 256;
		}

		ldelt[ delta ] ++;

		if ( byte == last )
		{
			seq ++;
		}
		else
		{
			if ( seq )
			{
				// Record the sequence number in table:
				// 2,3,4,5,6+

				seq --;

				if ( seq >= SEQ_MAX )
				{
					seq = SEQ_MAX - 1;
				}

				numrep[ seq ] ++;
			}

			seq = 0;
		}

		last = byte;
	}

	if ( seq )
	{
		// Record the sequence number in table:
		// 2,3,4,5,6+

		seq --;

		if ( seq >= SEQ_MAX )
		{
			seq = SEQ_MAX - 1;
		}

		numrep[ seq ] ++;
	}

	res = 0;

finish:
	print_update_progress_nl ( verbose );

	fclose ( fp );
	free ( buf );

	printf ( "Bytes read = %.0f\n\n", btot );
	print_title ( "Byte Frequencies" );
	printf ( "\n" );

	if ( verbose )
	{
		printf ( "Byte\tTot\tNorm\tDiff%%\n" );
	}

	norm = btot / 256.0;
	max = 0;
	cum = 0;

	for ( i = 0; i < 256; i++ )
	{
		diff = -100 * ( norm - (double)freq[i] ) / norm;

		cum += fabs ( diff );
		max = MAX ( max, fabs ( diff ) );

		if ( verbose )
		{
			printf ( "%i\t%" PRId64 "\t%.1f\t%.3f\n",
				i, freq[i], norm, diff );
		}
	}

	if ( verbose )
	{
		puts ( "" );
	}

	printf ( "Maximum Variation = %.3f\n", max );
	printf ( "Average Variation = %.3f\n", cum / 256.0 );
	printf ( "\n" );
	print_title ( "Delta Byte Frequencies" );
	printf ( "\n" );

	if ( verbose )
	{
		printf ( "Delta\tTot\tNorm\tDiff%%\n" );
	}

	max = 0;
	cum = 0;

	for ( i = 0; i < 256; i++ )
	{
		diff = -100 * ( norm - (double)ldelt[i] ) / norm;

		cum += fabs ( diff );
		max = MAX ( max, fabs ( diff ) );

		if ( verbose )
		{
			printf ( "%i\t%" PRId64 "\t%.1f\t%.3f\n",
				i, ldelt[i], norm, diff );
		}
	}

	if ( verbose )
	{
		puts ( "" );
	}

	printf ( "Maximum Variation = %.3f\n", max );
	printf ( "Average Variation = %.3f\n", cum / 511.0 );
	printf ( "\n" );
	print_title ( "Identical Consecutive Bytes" );
	printf ( "\n" );
	printf ( "ICB's\tTot\tNorm\tDiff%%\n" );

	// Calculate expected number based on file size
	div = 256.0;

	for ( i = 0; i < SEQ_MAX; i++, div *= 256.0 )
	{
		numrep_exp[i] = btot / div;

		if ( numrep_exp[i] == 0 )
		{
			diff = 0.0;
		}
		else
		{
			diff = -100 * ( numrep_exp[i] - (double)numrep[i] ) /
				numrep_exp[i];
		}

		printf ( "%s\t%" PRId64 "\t%.1f\t%.3f\n", numrep_str[i],
			numrep[i], numrep_exp[i], diff );
	}

	printf ( "\n\n" );

	return res;
}

int print_analysis (
	mtArg	const * const	ARG_UNUSED ( mtarg ),
	int		const	ARG_UNUSED ( arg ),
	int		const	ARG_UNUSED ( argc ),
	char	const * const	ARG_UNUSED ( argv[] ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	if (	validate_arg_i ()	||
		print_analysis_real ()
		)
	{
		set_exit_fail ();

		return 1;
	}

	return 0;
}

