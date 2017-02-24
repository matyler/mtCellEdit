/*
	Copyright (C) 2010-2016 Mark Tyler

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
	#include <unistd.h>
	#include <time.h>
}

#include <mtcelledit.h>



#define NETWORK_FILENAME	"/proc/net/dev"

#define SHEET_ROW_TOTALS	1
#define SHEET_ROW_HEADER	2

#define SHEET_COL_TIME_START	1
#define SHEET_COL_TIME_END	2
#define SHEET_COL_TIME_TOTAL	3
#define SHEET_COL_IN_BYTES	4
#define SHEET_COL_OUT_BYTES	5
#define SHEET_COL_TOTAL_BYTES	6
#define SHEET_COL_IN_MBS	7
#define SHEET_COL_OUT_MBS	8
#define SHEET_COL_TOTAL_MBS	9



static CedSheet		* log_sheet = NULL;
static int		log_next_row,
			sleep_delay = 5;
static char		current_time[128];
static char	const	* log_filename = "mtnetlog.tsv";
static char	const	* log_network = "ppp0";
static double		last_in = 0,
			last_out = 0,
			last_in_ = 0,
			last_out_ = 0;



static void get_time_string (
	int		const	col
	)
{
	time_t		now;
	struct tm	* now_tm;


	now = time ( NULL );
	now_tm = localtime ( &now );

	strftime ( current_time, 120, "%F %T", now_tm );
	ced_sheet_set_cell ( log_sheet, log_next_row, col, current_time );
}

static void create_titles ( void )
{
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_TOTALS, SHEET_COL_TIME_TOTAL,
		"=sum( r[2]c:r_c )" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_TOTALS, SHEET_COL_IN_BYTES,
		"=sum( r[2]c:r_c )" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_TOTALS, SHEET_COL_OUT_BYTES,
		"=sum( r[2]c:r_c )" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_TOTALS, SHEET_COL_TOTAL_BYTES,
		"=sum( r[2]c:r_c )" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_TOTALS, SHEET_COL_IN_MBS,
		"=sum( r[2]c:r_c )" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_TOTALS, SHEET_COL_OUT_MBS,
		"=sum( r[2]c:r_c )" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_TOTALS, SHEET_COL_TOTAL_MBS,
		"=sum( r[2]c:r_c )" );

	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_TIME_START,
		"Start" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_TIME_END,
		"End" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_TIME_TOTAL,
		"Hours" );

	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_IN_BYTES,
		"Bytes In" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_OUT_BYTES,
		"Bytes Out" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_TOTAL_BYTES,
		"Bytes Total" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_IN_MBS,
		"MB In" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_OUT_MBS,
		"MB Out" );
	ced_sheet_set_cell ( log_sheet, SHEET_ROW_HEADER, SHEET_COL_TOTAL_MBS,
		"MB Total" );
}

static int open_log ( void )
{
	int		row_max,
			col_max;


	log_sheet = ced_sheet_load ( log_filename, NULL, NULL );

	if ( ! log_sheet )
	{
		log_sheet = ced_sheet_new ();
	}

	if ( ! log_sheet )
	{
		return 1;
	}

	create_titles ();

	// Find the next empty row to populate
	if ( ced_sheet_get_geometry ( log_sheet, &row_max,  &col_max ) )
	{
		return 1;
	}

	log_next_row = row_max + 1;

	return 0;
}

static void save_log ( void )
{
	ced_sheet_recalculate ( log_sheet, NULL, 0 );
	if ( ced_sheet_save ( log_sheet, log_filename, CED_FILE_TYPE_TSV_VALUE )
		)
	{
		fprintf ( stderr, "%s: Unable to save log file '%s'.\n",
			BIN_NAME, log_filename );
	}
}

static char * poll_network ( void )
{
	char		* file;
	FILE		* fp;


	sleep ( (unsigned int)sleep_delay );

	fp = fopen ( NETWORK_FILENAME, "r" );
	if ( ! fp )
	{
		fprintf ( stderr, "%s: Unable to find file %s\n", BIN_NAME,
			NETWORK_FILENAME );

		return NULL;
	}

	while ( 1 )
	{
		file = mtkit_file_readline ( fp, NULL, NULL );

		if (	! file ||
			file[0] == 0 ||
			strstr ( file, log_network )
			)
		{
			break;
		}

		free ( file );
		file = NULL;
	}

	fclose ( fp );

	return file;
}

static void wait_for_network ( void )
{
	char		* file;


	while ( 1 )
	{
		file = poll_network ();
		if ( file )
		{
			break;
		}
	}

	get_time_string ( SHEET_COL_TIME_START );

	free ( file );
}

static void monitor_network ( void )
{
	char		* file,
			* txt;
	double		v1,
			v2;
	int		i,
			stopped = 0;


	while ( 1 )
	{
		file = poll_network ();
		if ( ! file )
		{
			stopped = 1;
			break;		// Network is no longer available
		}

		txt = strstr ( file, log_network );
		if ( ! txt )
		{
			// Network is no longer available
			free ( file );
			stopped = 1;

			break;
		}

		txt += strlen ( log_network ) + 1;

		if ( mtkit_strtod ( txt, &v1, &txt, 0 ) )
		{
			fprintf ( stderr, "%s: Unable to parse '%s'.\n",
				BIN_NAME, txt );
			free ( file );

			goto finish;
		}

		for ( i = 0; i < 8; i++ )
		{
			if ( mtkit_strtod ( txt, &v2, &txt, 0 ) )
			{
				fprintf ( stderr, "%s: Unable to parse '%s'.\n",
					BIN_NAME, txt );
				free ( file );

				goto finish;
			}
		}

		// Look for numerical wrap-around such as when a 32 bit system
		// exceeds 2^32 bytes.  Or if the network is closed then opened
		// again quickly, thus restarting it from 0

		if ( v1 < last_in )
		{
			last_in = 0;
			last_in_ = 0;

			// This stops the number being counted twice
			// NOTE: in the unlikely event that v2 flipped also at
			// this point, it doesn't matter as both get set to
			// last_out_ = last_out = 0 in a few instructions time

			last_out_ = last_out;
			stopped = 2;
		}

		if ( v2 < last_out )
		{
			last_out = 0;
			last_out_ = 0;

			// This stops the number being counted twice
			// NOTE: in the unlikely event that v1 flipped also at
			// this point, it doesn't matter as already:
			// last_in_ = last_in = 0

			last_in_ = last_in;
			stopped = 2;
		}

		if ( stopped )
		{
			goto finish;
		}

		last_in = v1;
		last_out = v2;

		ced_sheet_set_cell_value ( log_sheet, log_next_row,
			SHEET_COL_IN_BYTES, v1 - last_in_ );
		ced_sheet_set_cell_value ( log_sheet, log_next_row,
			SHEET_COL_IN_MBS, (v1 - last_in_) / 1024 / 1024 );

		ced_sheet_set_cell_value ( log_sheet, log_next_row,
			SHEET_COL_OUT_BYTES, v2 - last_out_ );
		ced_sheet_set_cell_value ( log_sheet, log_next_row,
			SHEET_COL_OUT_MBS, (v2 - last_out_) / 1024 / 1024 );

		ced_sheet_set_cell_value ( log_sheet, log_next_row,
			SHEET_COL_TOTAL_BYTES, v1 + v2 - last_in_ - last_out_ );
		ced_sheet_set_cell_value ( log_sheet, log_next_row,
			SHEET_COL_TOTAL_MBS,
			(v1 + v2 - last_in_ - last_out_ ) / 1024 / 1024 );

		get_time_string ( SHEET_COL_TIME_END );
		ced_sheet_set_cell ( log_sheet, log_next_row,
			SHEET_COL_TIME_TOTAL, "=24*(rc2 - rc1)" );
		free ( file );

		save_log ();
	}

finish:

	if ( stopped == 1 )
	{
		// Don't do this if stopped via '2'
		last_in = 0;
		last_out = 0;
		last_in_ = 0;
		last_out_ = 0;
	}

	log_next_row ++;
}

static int file_func (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	log_filename = filename;

	return 0;			// Keep parsing
}

static int error_func (
	int			const	error,
	int			const	arg,
	int			const	argc,
	char	const * const *	const	argv,
	void			* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "error_func: Argument ERROR! - num=%i arg=%i/%i",
		error, arg, argc );

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n" );

	return 0;			// Keep parsing
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	int		show_version = 0;
	mtArg	const	arg_list[] = {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ "delay",	MTKIT_ARG_INT,	&sleep_delay, 0, NULL },
		{ "network",	MTKIT_ARG_STRING, &log_network, 0, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};


	mtkit_arg_parse ( argc, argv, arg_list, file_func, error_func, NULL );

	switch ( show_version )
	{
	case 1:
		printf ( "%s\n\n", VERSION );
		goto finish;

	case 2:
		printf ( "%s\n\n"
		"For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n"
		, VERSION, BIN_NAME );

		goto finish;
	}

	ced_init ();

	if ( open_log () )
	{
		goto finish;
	}

	while ( 1 )
	{
		wait_for_network ();
		monitor_network ();
	}

finish:
	return 0;
}

