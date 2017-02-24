/*
	Copyright (C) 2004-2016 Mark Tyler

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

#include "be.h"



typedef struct
{
	double		files;
	double		bytes;
	double		sdirs;
	double		other;
} scanTot;

typedef struct
{
	CedSheet	* sheet;
	raftScanFunc	callback;
	void		* user_data;
	int		row;
} scanState;



static double get_cell_value (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column
	)
{
	CedCell		* cell;


	cell = ced_sheet_get_cell ( sheet, row, column );
	if ( ! cell )
	{
		return 0;
	}

	return cell->value;
}

static void add_total (			// a += b
	scanTot		* const	a,
	scanTot		* const	b
	)
{
	a->files += b->files;
	a->bytes += b->bytes;
	a->sdirs += b->sdirs;
	a->other += b->other;
}

static void set_row_prefs (
	scanState	* state
	)
{
	CedCellPrefs	* pref_num,
			* pref_decimal,
			* pref_percent;


	pref_num = ced_cell_prefs_new ();
	pref_decimal = ced_cell_prefs_new ();
	pref_percent = ced_cell_prefs_new ();

	if ( ! pref_num || ! pref_decimal || ! pref_percent )
	{
		goto finish;
	}

	pref_num->format = 2;
	mtkit_strfreedup ( &pref_num->num_thousands, "," );

	pref_decimal->format = 2;
	pref_decimal->num_decimal_places = 2;
	mtkit_strfreedup ( &pref_decimal->num_thousands, "," );

	pref_percent->format = 2;
	pref_percent->num_decimal_places = 1;

	ced_sheet_set_cell_prefs ( state->sheet, state->row, RAFT_COL_FILES,
		pref_num, NULL );
	ced_sheet_set_cell_prefs ( state->sheet, state->row, RAFT_COL_BYTES,
		pref_num, NULL );
	ced_sheet_set_cell_prefs ( state->sheet, state->row, RAFT_COL_SUBDIRS,
		pref_num, NULL );
	ced_sheet_set_cell_prefs ( state->sheet, state->row, RAFT_COL_OTHER,
		pref_num, NULL );

	ced_sheet_set_cell_prefs ( state->sheet, state->row, RAFT_COL_MB,
		pref_decimal, NULL );

	ced_sheet_set_cell_prefs ( state->sheet, state->row,
		RAFT_COL_FILES_PERCENT, pref_percent, NULL );
	ced_sheet_set_cell_prefs ( state->sheet, state->row,
		RAFT_COL_MB_PERCENT, pref_percent, NULL );

finish:
	ced_cell_prefs_destroy ( pref_num );
	ced_cell_prefs_destroy ( pref_decimal );
	ced_cell_prefs_destroy ( pref_percent );
}

static void add2table (
	scanState	* const	state,
	char	const	* const	dirname,
	scanTot		* const	tot
	)
{
	ced_sheet_set_cell_text ( state->sheet, state->row, RAFT_COL_NAME,
		dirname );

	ced_sheet_set_cell_value ( state->sheet, state->row, RAFT_COL_FILES,
		tot->files );

	ced_sheet_set_cell_value ( state->sheet, state->row, RAFT_COL_BYTES,
		tot->bytes );

	ced_sheet_set_cell_value ( state->sheet, state->row, RAFT_COL_MB,
		tot->bytes / 1024 / 1024 );

	ced_sheet_set_cell_value ( state->sheet, state->row, RAFT_COL_SUBDIRS,
		tot->sdirs );

	ced_sheet_set_cell_value ( state->sheet, state->row, RAFT_COL_OTHER,
		tot->other );

	set_row_prefs ( state );

	state->row ++;
}

static void add_percentages (
	scanState	* const	state
	)
{
	int		i;
	double		tot_files,
			tot_bytes,
			perc;


	tot_files = get_cell_value ( state->sheet, state->row, RAFT_COL_FILES );
	tot_bytes = get_cell_value ( state->sheet, state->row, RAFT_COL_BYTES );

	for ( i = 1; i < state->row; i++ )
	{
		perc = get_cell_value ( state->sheet, i, RAFT_COL_FILES );
		perc = 100 / tot_files * perc;

		ced_sheet_set_cell_value ( state->sheet, i,
			RAFT_COL_FILES_PERCENT, perc );

		perc = get_cell_value ( state->sheet, i, RAFT_COL_BYTES );
		perc = 100 / tot_bytes * perc;

		ced_sheet_set_cell_value ( state->sheet, i,
			RAFT_COL_MB_PERCENT, perc );
	}
}

static void add_totals (
	scanState	* const	state
	)
{
	int		i;


	ced_sheet_set_cell_text ( state->sheet, state->row, RAFT_COL_NAME,
		" < TOTAL >" );

	// Set up totals formulas
	for ( i = RAFT_COL_FILES; i < RAFT_COL_TOTAL; i++ )
	{
		ced_sheet_set_cell ( state->sheet, state->row, i,
			"=sum ( r1c:r[-1]c )" );
	}

	// Calculate Totals
	ced_sheet_recalculate ( state->sheet, NULL, 0 );
	ced_sheet_recalculate ( state->sheet, NULL, 1 );

	// Convert formulas into numbers
	for ( i = RAFT_COL_FILES; i < RAFT_COL_TOTAL; i++ )
	{
		ced_sheet_set_cell_value ( state->sheet, state->row, i,
			get_cell_value ( state->sheet, state->row, i )
			);
	}

	ced_sheet_set_cell_value ( state->sheet, state->row,
		RAFT_COL_FILES_PERCENT, 100 );

	ced_sheet_set_cell_value ( state->sheet, state->row,
		RAFT_COL_MB_PERCENT, 100 );

	set_row_prefs ( state );
}

static int scan_recurse (
	char	const	* const	path,
	scanState	* const	state,
	int		const	recurse,	// 1 = No Recurse <.>
					// 2 = Top level dirs
					// 3 = Recurse
	scanTot		* sum		// Subdirectory total for (2->3)
	)
{
	int		res = 0;
	scanTot		tot = { 0, 0, 0, 0 };
	DIR		* dp;
	struct dirent	* ep;
	struct stat	buf;
	char		* tmp;


	if ( state->callback ( state->sheet, state->row, state->user_data ) )
	{
		return 1;		// User termination
	}

	// Open pathname given
	dp = opendir ( path );
	if ( ! dp )
	{
		fprintf ( stderr, "Unable to opendir '%s'\n", path );

		return 0;
	}

	while ( res == 0 && ( ep = readdir ( dp ) ) )
	{
		if (	! strcmp ( ep->d_name, "." ) ||
			! strcmp ( ep->d_name, ".." )
			)
		{
			// Quietly ignore "." and ".." directories
			continue;
		}

		tmp = mtkit_string_join ( path, ep->d_name, NULL, NULL );
		if ( ! tmp )
		{
			res = -1;
			break;
		}

		// Get full name of file/directory

		if ( lstat ( tmp, &buf ) )	// Get file details
		{
			free ( tmp );
			continue;
		}

		free ( tmp );

		if ( S_ISDIR ( buf.st_mode ) )
		{
			if ( recurse != 1 )
			{
				scanTot		ltot = { 0, 0, 0, 0 };


				tmp = mtkit_string_join ( path, ep->d_name, "/",
					NULL);

				if ( ! tmp )
				{
					return -1;
				}

				res = scan_recurse ( tmp, state, 3,
					recurse == 2 ? &ltot : sum );

				free ( tmp );

				if ( recurse == 2 )
				{
					// Top level recursion

					add2table ( state, ep->d_name, &ltot );
				}
			}

			if ( recurse != 2 )
			{
				tot.sdirs ++;
			}
		}
		else
		{
			if (	S_ISLNK ( buf.st_mode )		||
				! S_ISREG ( buf.st_mode )
				)
			{
				tot.other ++;
			}
			else
			{
				tot.files ++;
				tot.bytes += (double)buf.st_size;
			}
		}
	}

	if ( recurse == 1 )
	{
		add2table ( state, " <.>", &tot );
	}
	else if ( recurse == 3 )
	{
		add_total ( sum, &tot );
	}

	closedir ( dp );

	return res;
}

int raft_scan_sheet (
	char	const	*	const	path,
	CedSheet	**	const	sheet,
	raftScanFunc		const	callback,
	void		*	const	user_data
	)
{
	scanState	state = { NULL, callback, user_data, 1 };
	int		res = 0;


	if ( ! path || ! sheet )
	{
		return -1;		// Fail
	}

	sheet[0] = ced_sheet_new ();
	if ( ! sheet[0] )
	{
		return -1;		// Fail
	}

	state.sheet = sheet[0];

	res = scan_recurse ( path, &state, 1, NULL );
	if ( res == 0 )
	{
		res = scan_recurse ( path, &state, 2, NULL );
	}

	if ( res )
	{
		ced_sheet_destroy ( sheet[0] );
		sheet[0] = NULL;

		return res;
	}

	add_totals ( &state );
	add_percentages ( &state );

	return 0;
}



#define MAX_PATH_LEN		10000




char * raft_path_check (
	char	const	* path
	)
{
	char		* new_path;
	size_t		plen;


	if ( ! path || path[0] == 0 )
	{
		return NULL;		// Argument error
	}

	plen = strlen ( path );
	if ( plen >= MAX_PATH_LEN )
	{
		return NULL;		// Path too long
	}

	if ( path [ plen - 1 ] != '/' )
	{
		// Ensure path has '/' at end

		new_path = mtkit_string_join ( path, "/", NULL, NULL );
	}
	else
	{
		new_path = strdup ( path );
	}

	return new_path;
}

char * raft_path_merge (
	char	const	* const	path,
	CedSheet	* const	sheet,
	int		const	row
	)
{
	char		* new_path = NULL;
	CedCell		* cell;


	if ( ! path )
	{
		return NULL;
	}

	cell = ced_sheet_get_cell ( sheet, row, RAFT_COL_NAME );
	if ( cell && cell->text )
	{
		new_path = mtkit_string_join ( path, cell->text, "/", NULL );
	}

	return new_path;
}

