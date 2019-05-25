/*
	Copyright (C) 2008-2018 Mark Tyler

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



#define FREE_PREFS_TEXT(A)	free ( A->format_datetime );	\
				free ( A->num_thousands );	\
				free ( A->text_prefix );	\
				free ( A->text_suffix );	\



int ced_cell_destroy (
	CedCell		* const	cell
	)
{
	if ( ! cell )
	{
		return 1;
	}

	ced_cell_prefs_destroy ( cell->prefs );
	free ( cell->text );
	free ( cell );

	return 0;
}

int ced_cmp_cell (
	void	const * const	k1,
	void	const * const	k2
	)
{
	if ( k1 < k2 )
	{
		return -1;
	}

	if ( k1 > k2 )
	{
		return 1;
	}

	return 0;
}

void ced_del_cell (
	mtTreeNode	* const	node
	)
{
	ced_cell_destroy ( (CedCell *)(node->data) );
}



static CedCellPrefs const default_cell_prefs =
			{
			0, 16777215, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			NULL, NULL, NULL, NULL
			};



CedCell * ced_cell_new ( void )
{
	return (CedCell *)( calloc ( sizeof ( CedCell ), 1 ) );
}

CedCellPrefs * ced_cell_prefs_new ( void )
{
	CedCellPrefs * const prefs = (CedCellPrefs *)( calloc (
		sizeof(CedCellPrefs), 1 ) );

	if ( ! prefs )
	{
		return NULL;
	}

	memcpy ( prefs, &default_cell_prefs, sizeof(default_cell_prefs) );

	return prefs;
}

int ced_cell_prefs_destroy (
	CedCellPrefs	* const	prefs
	)
{
	if ( ! prefs )
	{
		return 1;
	}

	FREE_PREFS_TEXT ( prefs )
	free ( prefs );

	return 0;
}

CedCellPrefs const * ced_cell_prefs_default ( void )
{
	return &default_cell_prefs;
}



#define CELL_STRDUP( A ) \
	if ( A && A[0] ) \
	{ \
		A = strdup ( A ); \
		if ( ! A ) \
		{ \
			res = -1; \
		} \
	} \
	else \
	{ \
		A = NULL; \
	}



int ced_cell_set_prefs (
	CedCell		* const	cell,
	CedCellPrefs	* const	prefs
	)
{
	int	res = 0;


	if ( ! prefs )
	{
		ced_cell_prefs_destroy ( cell->prefs );
		cell->prefs = NULL;

		return 0;
	}

	if ( ! cell->prefs )
	{
		cell->prefs = ced_cell_prefs_new ();
		if ( ! cell->prefs )
		{
			return 1;
		}
	}
	else
	{
		// Empty old prefs strings
		FREE_PREFS_TEXT ( cell->prefs )
		// Dangling pointers are safe as they get set next
	}

	memcpy ( cell->prefs, prefs, sizeof ( CedCellPrefs ) );

	// Duplicate new prefs strings or NULL out dangling refs
	CELL_STRDUP ( cell->prefs->format_datetime )
	CELL_STRDUP ( cell->prefs->num_thousands )
	CELL_STRDUP ( cell->prefs->text_prefix )
	CELL_STRDUP ( cell->prefs->text_suffix )

	return res;
}

int ced_strtocellref (
	char		const	* const	input,
	CedCellRef		* const	result,
	char		const * * const	next,
	int			const	strict
	)
{
	int		ref,
			i,
			res[2][2];
	char	const	* s = input;
	char	const	c[] = "rRcC";
	char		* unsafe;


	if ( ! input || ! result )
	{
		return 1;
	}

	// Skip all leading spaces
	while ( isspace ( s[0] ) )
	{
		s++;
	}

	for ( i = 0; i < 2; i++ )
	{
		/* Must have input string to parse and begin with rR or cC */
		if (	! s ||
			( s[0] != c[2 * i] && s[0] != c[2 * i + 1] )
			)
		{
			return 1;
		}

		s++;

		if ( s[0] == '_' )
		{
			/* Check for maximum row/col shorthand - absolute ref */
			res[i][0] = 0;

			if ( i == 0 )
			{
				res[i][1] = CED_MAX_ROW;
			}
			else
			{
				res[i][1] = CED_MAX_COLUMN;
			}

			s++;
		}
		else if ( s[0] == '[' )
		{
			/* Check for relative open square bracket */
			s++;

			/* Get relative reference */
			if ( mtkit_strtoi ( s, &ref, &unsafe, 0 ) )
			{
				return 1;
			}

			s = unsafe;

			/* Get closing square bracket */
			if ( s[0] != ']' )
			{
				return 1;
			}

			s++;

			res[i][0] = 1;
			res[i][1] = ref;
		}
		else if ( s[0] >= '0' &&
			s[0] <= '9' &&
			! mtkit_strtoi ( s, &ref, &unsafe, 0 ) )
		{
			s = unsafe;

			/* Absolute cell reference */
			res[i][0] = 0;
			res[i][1] = ref;

			if (	( i == 0 && ref > CED_MAX_ROW ) ||
				( i == 1 && ref > CED_MAX_COLUMN )
				)
			{
			// Row/Column is out of range for an absolute reference
				return 1;
			}
		}
		else
		{
			/* No reference found so must be this row/column */
			res[i][0] = 1;
			res[i][1] = 0;
		}
	}

	if ( strict && mtkit_strnonspaces ( s ) )
	{
		return 1;
	}

	result->row_m = res[0][0];
	result->row_d = res[0][1];
	result->col_m = res[1][0];
	result->col_d = res[1][1];

	if ( next )
	{
		next[0] = s;
	}

	return 0;
}

int ced_strtocellrange (
	char		const	* const	input,
	CedCellRef		* const	r1,
	CedCellRef		* const	r2,
	char		const *	* const	next,
	int			const	strict
	)
{
	char	const	* col = NULL;


	if ( ced_strtocellref ( input, r1, &col, 0 ) || ! col )
	{
		return 1;
	}

	for ( ;; col++ )
	{
		if ( isspace ( col[0] ) )
		{
			continue;
		}

		if ( col[0] == ':' )
		{
			break;
		}

		return 1;	// Invalid char, including NUL
	}

	if ( ced_strtocellref ( col + 1, r2, next, strict ) )
	{
		return 1;
	}

	return 0;
}

int ced_cellreftostr (
	char		* const	output,
	CedCellRef	* const	cellref
	)
{
	char		rt[32] = {0},
			ct[32] = {0};


	if ( ! output || ! cellref )
	{
		return 1;
	}

	if ( cellref->row_m )
	{
		if ( cellref->row_d )
		{
			/* Relative */
			snprintf ( rt, sizeof ( rt ), "[%i]", cellref->row_d );
		}
	}
	else
	{
		/* Absolute */
		snprintf ( rt, sizeof ( rt ), "%i", cellref->row_d );
	}

	if ( cellref->col_m )
	{
		if ( cellref->col_d )
		{
			/* Relative */
			snprintf ( ct, sizeof ( ct ), "[%i]", cellref->col_d );
		}
	}
	else
	{
		/* Absolute */
		snprintf ( ct, sizeof ( ct ), "%i", cellref->col_d );
	}

	snprintf ( output, 128, "R%sC%s", rt, ct );

	return 0;
}

int ced_cell_paste (
	CedCell		* const	dest,
	CedCell		* const	src
	)
{
	if (	! dest ||
		! src ||
		mtkit_strfreedup ( &dest->text, src->text ) ||
		ced_cell_set_prefs ( dest, src->prefs )
		)
	{
		return 1;	// Fail
	}

	dest->type = src->type;
	dest->value = src->value;

	return 0;		// Success
}

CedCell * ced_cell_duplicate (
	CedCell		* const	cell
	)
{
	CedCell		* newcell;


	if ( ! cell )
	{
		return NULL;
	}

	newcell = ced_cell_new ();
	if ( ! newcell )
	{
		return NULL;
	}

	if ( ced_cell_paste ( newcell, cell ) )
	{
		ced_cell_destroy ( newcell );
		return NULL;
	}

	return newcell;
}

static int col_dup_func (
	mtTreeNode	* const	old,
	mtTreeNode	* const	nw
	)
{
	nw->key = old->key;
	nw->data = ced_cell_duplicate ( (CedCell *)(old->data) );

	if ( ! nw->data )
	{
		return 1;	// Fail
	}

	return 0;		// Success
}

static int row_dup_func (
	mtTreeNode	* const	old,
	mtTreeNode	* const	nw
	)
{
	nw->key = old->key;
	nw->data = mtkit_tree_duplicate ( (mtTree *)old->data, col_dup_func );

	if ( ! nw->data )
	{
		return 1;	// Fail
	}

	return 0;		// Success
}

CedSheet * ced_sheet_duplicate (
	CedSheet	* const	sheet
	)
{
	CedSheet	* newsheet;


	if ( ! sheet )
	{
		return NULL;
	}

	newsheet = ced_sheet_new ();
	if ( ! newsheet )
	{
		return NULL;
	}

	newsheet->prefs = sheet->prefs;

	if ( sheet->rows )
	{
		// Remove empty tree
		mtkit_tree_destroy ( newsheet->rows );
		newsheet->rows = mtkit_tree_duplicate ( sheet->rows,
			row_dup_func );

		if ( ! newsheet->rows )
		{
			ced_sheet_destroy ( newsheet );

			return NULL;
		}
	}

	return newsheet;
}



typedef struct
{
	int		row,		// Current row
			min_row,
			min_col,
			max_row,
			max_col
			;
	CedFuncScanArea	callback;
	void		* user_data;
	CedSheet	* sheet;
} scanSTATE;



static int recurse_scan_col_back (
	scanSTATE	* const	state,
	mtTreeNode	* const	col_node
	)
{
	int		col = (int)(intptr_t)col_node->key;


	if (	col_node->right &&
		col < state->max_col &&
		recurse_scan_col_back ( state, col_node->right ) )
	{
		return 2;
	}

	if ( col >= state->min_col && col <= state->max_col )
	{
		if ( state->callback ( state->sheet,
			(CedCell *)(col_node->data),
			state->row, col, state->user_data ) )
		{
			return 2;
		}
	}

	if (	col_node->left &&
		col > state->min_col &&
		recurse_scan_col_back ( state, col_node->left ) )
	{
		return 2;
	}

	return 0;
}

static int recurse_scan_row_back (
	scanSTATE	* const	state,
	mtTreeNode	* const	row_node
	)
{
	int		row = (int)(intptr_t)row_node->key;


	if (	row_node->right &&
		row < state->max_row &&
		recurse_scan_row_back ( state, row_node->right )
		)
	{
		return 2;
	}

	if ( row >= state->min_row && row <= state->max_row )
	{
		mtTreeNode	* cnode;


		cnode = ( (mtTree *) row_node->data )->root;
		state->row = row;

		if ( cnode && recurse_scan_col_back ( state, cnode ) )
		{
			return 2;
		}
	}

	if (	row_node->left &&
		row > state->min_row &&
		recurse_scan_row_back ( state, row_node->left )
		)
	{
		return 2;
	}

	return 0;
}

int ced_sheet_scan_area_backwards (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	col,
	int		const	rowtot,
	int		const	coltot,
	CedFuncScanArea	const	callback,
	void		* const	user_data
	)
{
	scanSTATE	state = { 0, 0, 0, row, col, callback, user_data,
				sheet };


	if (	! sheet			||
		! callback		||
		row < 0			||
		row > CED_MAX_ROW	||
		col < 0			||
		col > CED_MAX_COLUMN	||
		rowtot < 0		||
		coltot < 0
		)
	{
		return 1;
	}

	if ( ! sheet->rows || ! sheet->rows->root )
	{
		return 0;
	}

	// No need to check for coltot/rowtot upper limit

	if ( ! row || ! rowtot || rowtot > row )
	{
		state.min_row = 0;
	}
	else
	{
		state.min_row = row - rowtot + 1;
	}

	if ( ! col || ! coltot || coltot > col )
	{
		state.min_col = 0;
	}
	else
	{
		state.min_col = col - coltot + 1;
	}

	return recurse_scan_row_back ( &state, sheet->rows->root );
}


static int recurse_scan_col (
	scanSTATE	* const	state,
	mtTreeNode	* const	col_node
	)
{
	int		col = (int)(intptr_t)col_node->key;


	if (	col_node->left &&
		col > state->min_col &&
		recurse_scan_col ( state, col_node->left )
		)
	{
		return 2;
	}

	if ( col >= state->min_col && col <= state->max_col )
	{
		if ( state->callback ( state->sheet,
			(CedCell *)(col_node->data),
			state->row, col, state->user_data ) )
		{
			return 2;
		}
	}

	if ( col_node->right && col < state->max_col &&
		recurse_scan_col ( state, col_node->right ) )
	{
		return 2;
	}

	return 0;
}

static int recurse_scan_row (
	scanSTATE	* const	state,
	mtTreeNode	* const	row_node
	)
{
	int		row = (int)(intptr_t)row_node->key;


	if (	row_node->left &&
		row > state->min_row &&
		recurse_scan_row ( state, row_node->left )
		)
	{
		return 2;
	}

	if ( row >= state->min_row && row <= state->max_row )
	{
		mtTreeNode	* cnode;


		cnode = ( (mtTree *) row_node->data )->root;
		state->row = row;

		if ( cnode && recurse_scan_col ( state, cnode ) )
		{
			return 2;
		}
	}

	if (	row_node->right &&
		row < state->max_row &&
		recurse_scan_row ( state, row_node->right )
		)
	{
		return 2;
	}

	return 0;
}

int ced_sheet_scan_area (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	col,
	int			rowtot,
	int			coltot,
	CedFuncScanArea	const	callback,
	void		* const	user_data
	)
{
	scanSTATE	state = { 0, row, col, 0, 0, callback, user_data,
				sheet };


	if (	! sheet			||
		! callback		||
		row < 0			||
		row > CED_MAX_ROW	||
		col < 0			||
		col > CED_MAX_COLUMN	||
		rowtot < 0		||
		coltot < 0
		)
	{
		return 1;
	}

	if ( ! sheet->rows || ! sheet->rows->root )
	{
		return 0;
	}

	if ( rowtot > CED_MAX_ROW )
	{
		rowtot = CED_MAX_ROW;
	}

	if ( coltot > CED_MAX_COLUMN )
	{
		coltot = CED_MAX_COLUMN;
	}

	if ( ! rowtot || (row + rowtot - 1) > CED_MAX_ROW )
	{
		state.max_row = CED_MAX_ROW;
	}
	else
	{
		state.max_row = row + rowtot - 1;
	}

	if ( ! coltot || (col + coltot - 1) > CED_MAX_COLUMN )
	{
		state.max_col = CED_MAX_COLUMN;
	}
	else
	{
		state.max_col = col + coltot - 1;
	}

	return recurse_scan_row ( &state, sheet->rows->root );
}

