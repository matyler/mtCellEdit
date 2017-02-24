/*
	Copyright (C) 2009-2016 Mark Tyler

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



typedef int (* strCMPfunc) (
	char	const *	s1,
	char	const *	s2
	);
	// -1 = s1 < s2
	//  0 = s1 == s2
	//  1 = s1 > s2



typedef struct
{
	int		key;
	CedCell		* cell;
} csortNODE;

typedef struct
{
	mtTree		** sort_array;	// active_rows items
	mtTreeNode	** tn_array;	// active_rows items
	int		row1;
	int		row2;
	int	const	* cols;
	int		col1;
	int		col2;
	int	const	* rows;
	int		* csorti;
	csortNODE	* csortn;
	int		active_rows;
	int		active_cols;
	int		i;		// Used by recurse_array_populate
	int		reverse;	// -1 = descending 1 = ascending
	int	const	* mode_list;
	CedSheet	* sheet;
	strCMPfunc	cmp_func;	// strcmp or strcasecmp
	mtTree		* ctree;	// Used by recurse_swap_row
} sortSTATE;



static sortSTATE	state;



static void recurse_row_count (
	mtTreeNode	* const	tnode
	)
{
	if ( tnode->left && ( (intptr_t)tnode->key > state.row1 ) )
	{
		recurse_row_count ( tnode->left );
	}

	if (	( (intptr_t)tnode->key >= state.row1 ) &&
		(state.row2 == 0 || (intptr_t)tnode->key <= state.row2)
		)
	{
		state.active_rows ++;
	}

	if (	tnode->right &&
		( state.row2 == 0 || (intptr_t)tnode->key < state.row2)
		)
	{
		recurse_row_count ( tnode->right );
	}
}

static void recurse_array_populate (
	mtTreeNode	* const	tnode
	)
{
	if ( tnode->left && ( (intptr_t)tnode->key > state.row1 ) )
	{
		recurse_array_populate ( tnode->left );
	}

	if (	( (intptr_t)tnode->key >= state.row1 ) &&
		(state.row2 == 0 || (intptr_t)tnode->key <= state.row2)
		)
	{
		state.sort_array[ state.i ] = (mtTree *)tnode->data;
		state.tn_array[ state.i ] = tnode;
		state.i ++;
	}

	if (	tnode->right &&
		( state.row2 == 0 || (intptr_t)tnode->key < state.row2)
		)
	{
		recurse_array_populate ( tnode->right );
	}
}

static int cmp_cells (
	CedCell		* const	c1,
	CedCell		* const	c2
	)
{
	static int	ctp[CED_CELL_TYPE_TOTAL] = { 0, -10, -100, -100, -100,
				0, -100 };


	if ( ! c1 && ! c2 )
	{
		return 0;		// equal as neither cells exist
	}

	if ( ! c1 )
	{
		return 1;		// empty a => a>b
	}
	else if ( ! c2 )
	{
		return -1;		// empty b => a<b
	}

	if ( ! c1->text && ! c2->text )
	{
		return 0;		// equal as neither cells exist
	}

	if ( ! c1->text )
	{
		return 1;		// empty a => a>b
	}
	else if ( ! c2->text )
	{
		return -1;		// empty b => a<b
	}


	// This should never happen unless corruption has occurred
	if (	c1->type < CED_CELL_TYPE_NONE ||
		c1->type >= CED_CELL_TYPE_TOTAL ||
		c2->type < CED_CELL_TYPE_NONE ||
		c2->type >= CED_CELL_TYPE_TOTAL
		)
	{
		return 0;
	}

	// Check cell type precedence
	if ( ctp[ c1->type ] < ctp[ c2->type ] )
	{
		return ( -1 * state.reverse );
	}
	if ( ctp[ c1->type ] > ctp[ c2->type ] )
	{
		return ( 1 * state.reverse );
	}

	// At this point we know that the 2 cells are of the same type so
	// directly compare
	switch ( c1->type )
	{
	case CED_CELL_TYPE_TEXT:
	case CED_CELL_TYPE_TEXT_EXPLICIT:
		return (state.reverse * state.cmp_func ( c1->text, c2->text ) );

	case CED_CELL_TYPE_VALUE:
	case CED_CELL_TYPE_FORMULA:
	case CED_CELL_TYPE_FORMULA_EVAL:
	case CED_CELL_TYPE_DATE:
		if ( c1->value < c2->value )
		{
			return (-1 * state.reverse);
		}
		else if ( c1->value > c2->value )
		{
			return (1 * state.reverse);
		}
		break;

	case CED_CELL_TYPE_ERROR:	// All errors are equal
	default:
		return 0;		// Unknown type - should never happen
	}

	return 0;
}

static int row_compare (
	void	const * const	a,
	void	const * const	b
	)
{
	mtTree		* r1,
			* r2;
	mtTreeNode	* tn1,
			* tn2;
	CedCell		* c1,
			* c2;
	int		i,
			res = 0;


	r1 = ( (mtTree * const *)a )[0];
	r2 = ( (mtTree * const *)b )[0];

	for ( i = 0; state.cols[i] > 0; i++ )
	{
		tn1 = mtkit_tree_node_find ( r1,
			(void *)(intptr_t)state.cols[i] );
		tn2 = mtkit_tree_node_find ( r2,
			(void *)(intptr_t)state.cols[i] );

		if ( ! tn1 && ! tn2 )
		{
			continue;	// equal as neither cells exist
		}

		if ( ! tn1 )
		{
			return 1;	// empty a => a > b
		}
		else if ( ! tn2 )
		{
			return -1;	// empty b => a < b
		}
		else
		{
			c1 = (CedCell *)tn1->data;
			c2 = (CedCell *)tn2->data;

			if ( state.mode_list )
			{
				if ( state.mode_list[i] &
					CED_SORT_MODE_DESCENDING )
				{
					state.reverse = -1;
				}
				else
				{
					state.reverse = 1;
				}

				if ( state.mode_list[i] & CED_SORT_MODE_CASE )
				{
					state.cmp_func = strcmp;
				}
				else
				{
					state.cmp_func = strcasecmp;
				}
			}

			res = cmp_cells ( c1, c2 );
			if ( res )
			{
				break;
			}
		}
	}

	return res;
}

int ced_sheet_sort_rows (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	rowtot,
	int	const	* const	cols,
	int		const	mode,
	int	const	* const	mode_list
	)
{
	int		r;


	if (	! sheet			||
		! cols			||
		row < 1			||
		row > CED_MAX_ROW	||
		rowtot > CED_MAX_ROW
		)
	{
		return 1;
	}

	if (	! sheet->rows		||
		! sheet->rows->root	||
		rowtot == 1
		)
	{
		return 0;		// Nothing to do
	}

	memset ( &state, 0, sizeof ( state ) );

	state.row1 = row;
	state.row2 = row + rowtot - 1;
	state.cols = cols;
	state.mode_list = mode_list;

	if ( rowtot == 0 )
	{
		state.row2 = 0;
	}

	if ( state.row2 > CED_MAX_ROW )
	{
		state.row2 = CED_MAX_ROW;
	}

	if ( mode & CED_SORT_MODE_DESCENDING )
	{
		state.reverse = -1;
	}
	else
	{
		state.reverse = 1;
	}

	if ( mode & CED_SORT_MODE_CASE )
	{
		state.cmp_func = strcmp;
	}
	else
	{
		state.cmp_func = strcasecmp;
	}

/*
NOTES

The idea is that we are sorting rows within a given section.  We don't
need to change the row tree structure for this, merely juggle the data
(i.e. the references to the cell trees) between tree nodes in sheet->rows.
Row keys become r1, r1 + 1, r1 + 2, ... because empty rows always go to the end.

M.Tyler 11-8-2009
*/

	// Recursively count number of active rows in the range
	recurse_row_count ( sheet->rows->root );

	if ( state.active_rows < 1 )
	{
		return 0;		// Nothing to do
	}

	// Create arrays for tree (of row cells) refs & treenodes
	state.sort_array = (mtTree **)calloc ( (size_t)state.active_rows,
		sizeof (mtTree *) );

	state.tn_array = (mtTreeNode **)calloc ( (size_t)state.active_rows,
		sizeof (mtTreeNode *) );

	if ( ! state.sort_array || ! state.tn_array )
	{
		free ( state.sort_array );
		free ( state.tn_array );

		return 1;
	}

	// Recursively populate arrays
	state.i = 0;
	recurse_array_populate ( sheet->rows->root );

	// Quicksort the array
	qsort ( state.sort_array, (size_t)state.active_rows, sizeof (mtTree *),
		row_compare );

	// Rework each node in tree by changing row # and tree pointer as per
	// the new array order.
	for ( r = 0; r < state.active_rows; r++ )
	{
		if ( ! state.tn_array[r] )
		{
			// Should never happen, but just in case.
			continue;
		}

		state.tn_array[r]->key = (void *)(intptr_t)(row + r);
		state.tn_array[r]->data = state.sort_array[r];
	}

	// Clean up
	free ( state.sort_array );
	free ( state.tn_array );

	return 0;
}

static int col_compare (
	void	const * const	a,
	void	const * const	b
	)
{
	int		i, res = 0;
	csortNODE const	* node1;
	csortNODE const	* node2;
	CedCell		* c1, * c2;


	node1 = (csortNODE const *)a;
	node2 = (csortNODE const *)b;

	c1 = node1->cell;
	c2 = node2->cell;

	for ( i = 0; state.rows[i] > 0; i++ )
	{
		if ( state.mode_list )
		{
			if ( state.mode_list[i] & CED_SORT_MODE_DESCENDING )
			{
				state.reverse = -1;
			}
			else
			{
				state.reverse = 1;
			}

			if ( state.mode_list[i] & CED_SORT_MODE_CASE )
			{
				state.cmp_func = strcmp;
			}
			else
			{
				state.cmp_func = strcasecmp;
			}
		}

		res = cmp_cells ( c1, c2 );
		if ( res )
		{
			break;
		}

		c1 = ced_sheet_get_cell ( state.sheet, state.rows[ i + 1 ],
			node1->key + state.col1 );
		c2 = ced_sheet_get_cell ( state.sheet, state.rows[ i + 1 ],
			node2->key + state.col1 );
	}

	return res;
}

static void recurse_swap_col (
	mtTreeNode	* const	colnode
	)
{
	int		c = (int)(intptr_t) colnode->key;


	if ( colnode->left && c > state.col1 )
	{
		recurse_swap_col ( colnode->left );
	}

	if (	c >= state.col1 &&
		(state.col2 == 0 || c <= state.col2)
		)
	{
		state.tn_array[ c - state.col1 ] = colnode;
		state.csortn[ state.csorti[c - state.col1] ].cell =
			(CedCell *)colnode->data;
	}

	if (	colnode->right &&
		(state.col2 == 0 || c < state.col2)
		)
	{
		recurse_swap_col ( colnode->right );
	}
}

static void recurse_swap_row (
	mtTreeNode	* const	rownode
	)
{
	int		i,
			j;


	if ( rownode->left )
	{
		recurse_swap_row ( rownode->left );
	}

	memset ( state.tn_array, 0,
		(unsigned)state.active_cols * sizeof (mtTreeNode *) );

	memset ( state.csortn, 0,
		(unsigned)state.active_cols * sizeof ( csortNODE ) );

	recurse_swap_col ( ( (mtTree *)rownode->data )->root );

	for ( i = 0, j = 0; i < state.active_cols; i++ )
	{
		// Set cell + key
		if ( state.tn_array[i] )
		{
			while ( ! state.csortn[j].cell )
			{
				j++;
			}

			state.tn_array[i]->key =
				(void *)(intptr_t)(state.col1 + j);
			state.tn_array[i]->data =
				(void *)(state.csortn[j].cell);

			j++;
		}
	}

	if ( rownode->right )
	{
		recurse_swap_row ( rownode->right );
	}
}

int ced_sheet_sort_columns (
	CedSheet	* const	sheet,
	int		const	column,
	int		const	coltot,
	int	const	* const	rows,
	int		const	mode,
	int	const	* const	mode_list
	)
{
	int		r;


	if (	! sheet		||
		! rows		||
		column < 1	||
		rows[0] == 0	||
		column > CED_MAX_COLUMN
		)
	{
		return 1;
	}

	if (	! sheet->rows		||
		! sheet->rows->root	||
		coltot == 1
		)
	{
		return 0;		// Nothing to do
	}

	memset ( &state, 0, sizeof ( state ) );

	state.col1 = column;
	state.col2 = column + coltot - 1;
	state.rows = rows;
	state.sheet = sheet;
	state.mode_list = mode_list;

	if ( mode & CED_SORT_MODE_DESCENDING )
	{
		state.reverse = -1;
	}
	else
	{
		state.reverse = 1;
	}

	if ( mode & CED_SORT_MODE_CASE )
	{
		state.cmp_func = strcmp;
	}
	else
	{
		state.cmp_func = strcasecmp;
	}

	if ( coltot == 0 )
	{
		if ( ced_sheet_get_geometry ( sheet, NULL, &r ) )
		{
			return 1;
		}

		state.active_cols = r - column + 1;
		state.col2 = 0;
	}
	else
	{
		state.active_cols = coltot;
	}

	if ( state.col2 > CED_MAX_COLUMN )
	{
		state.col2 = CED_MAX_COLUMN;
	}

	if ( state.active_cols < 1 )
	{
		return 0;			// Nothing to do
	}

	state.csortn = (csortNODE *)calloc ( (size_t)state.active_cols,
		sizeof ( csortNODE));
	state.csorti = (int *)calloc ( (size_t)state.active_cols,
		sizeof ( int ) );
	state.tn_array = (mtTreeNode **)calloc ( (size_t)state.active_cols,
		sizeof (mtTreeNode *) );

	if (	! state.csortn ||
		! state.csorti ||
		! state.tn_array
		)
	{
		free ( state.csortn );
		free ( state.csorti );
		free ( state.tn_array );

		return 1;
	}

	// Initialize current order in csort1 lookup table
	for ( r = 0; r < state.active_cols; r++ )
	{
		state.csortn[r].key = r;
		state.csortn[r].cell = ced_sheet_get_cell ( sheet, rows[0],
			r + column );
	}

	qsort ( state.csortn, (size_t)state.active_cols, sizeof ( csortNODE ),
		col_compare );

	// Create csort2 reverse lookup table (reference later with:
	// new_index = csorti[ old_index - col1 ];)
	for ( r = 0; r < state.active_cols; r++ )
	{
		state.csorti[ state.csortn[r].key ] = r;
	}

	// Recursively traverse rows, and then col cells to switch cell keys
	// around
	recurse_swap_row ( sheet->rows->root );

	// Clean up
	free ( state.csortn );
	free ( state.csorti );
	free ( state.tn_array );

	return 0;
}

