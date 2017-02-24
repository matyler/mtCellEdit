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

extern "C" {

	#include <stdlib.h>
	#include <string.h>
}

#include <mtkit.h>
#include <mtcelledit.h>



typedef struct
{
	int
			i_csv,		// Input data format: 0 = tsv 1 = csv
			i_clock,	// 0 = clockwise 1 = anti
			i_dest[4],	// Start row, last row, start col,
					// end col
			i_error,	// 0 = success 1 = error
			i_ftype_in,
			i_ftype_out,	// If CED_FILE_TYPE_NONE use i_ftype_in
			i_num,		// 0 = Text 1 = Number
			i_rowcol,	// 0 = row 1 = col
			i_range[4],	// Start row, last row, start col,
					// end col
			i_start,
			i_case,		// 0 = Case insenstive
					// 1 = Case sensitive
			i_total,
			i_tmp,		// Temp scratch for several args
			i_verbose,	// 1 = verbose 0 = normal
			i_wildcard	// 0 = none 1 = use * and ? as wildcards
			;

	char	const	* s_arg;	// Current command line argument

	CedSheet	* sheet;	// Current sheet
} GLOBAL;



// Note: must match ff_errtab in main.c
enum
{
	ERROR_LOAD_FILE		= 1,
	ERROR_LIBMTCELLEDIT	= 2
};



extern GLOBAL		global;



int ut_load_file ( void );		// Loads sheet file (name in
					// global.s_arg). Can be a sheet only
					// (tsv|csv|ledger).
	// 0 = success. Failure is not sent to stderr, but global.i_error is
	// set.

/*	Command functions

	Return 0 = success.
	Return > 0 = Generic error to be reported by caller (main.c ff_errtab).
*/

int cedut_append ( void );
int cedut_cat ( void );
int cedut_clear ( void );
int cedut_cut ( void );
int cedut_diff ( void );
int cedut_eval ( void );
int cedut_find ( void );
int cedut_flip ( void );
int cedut_insert ( void );
int cedut_ls ( void );
int cedut_paste ( void );
int cedut_rotate ( void );
int cedut_set ( void );
int cedut_sort ( void );
int cedut_transpose ( void );

