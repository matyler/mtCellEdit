/*
	Copyright (C) 2008-2014 Mark Tyler

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



int ced_init ( void )
{
	char		buf[] = "LANG=C";


	// This forces consistent numbers, i.e. 1.23 != 1,23
	putenv ( buf );

	// Ensure random numbers really are random
	srand ( (unsigned int)time ( NULL ) );

	return 0;
}



typedef struct
{
	int		rc,		// Row/column
			rctot;		// Total inserts (+) removals (-)
} insremSTATE;



static void recurse_rc_reindex (
	mtTreeNode	* const	node,
	insremSTATE	* const	state
	)
{
	if ( (intptr_t)node->key >= state->rc )
	{
		// Re-index
		node->key = ( (char *)node->key + state->rctot );
	}

	if ( node->left && (intptr_t)node->key > state->rc )
	{
		recurse_rc_reindex ( node->left, state );
	}

	if ( node->right )
	{
		recurse_rc_reindex ( node->right, state );
	}
}

int ced_sheet_insert_row (
	CedSheet	* const	sheet,
	int		const	row,
	int			rowtot
	)
{
	int		res;
	insremSTATE	state = { row, 0 };


	if (	! sheet			||
		row < 1			||
		rowtot < 1		||
		row > CED_MAX_ROW	||
		rowtot > CED_MAX_ROW
		)
	{
		return 1;
	}

	if ( (row + rowtot - 1) > CED_MAX_ROW )
	{
		rowtot = CED_MAX_ROW + 1 - row;
	}

	state.rctot = rowtot;

	// Pastry cut the end rows before recursing
	res = ced_sheet_clear_area ( sheet, CED_MAX_ROW + 1 - rowtot,
		0, 0, 0, 0 );

	if ( res )
	{
		return res;
	}

	if ( sheet->rows && sheet->rows->root )
	{
		recurse_rc_reindex ( sheet->rows->root, &state );
	}

	return 0;
}

int ced_sheet_delete_row (
	CedSheet	* const	sheet,
	int		const	row,
	int			rowtot
	)
{
	int		res;
	insremSTATE	state = { row, 0 };


	if (	! sheet			||
		row < 1			||
		rowtot < 1		||
		row > CED_MAX_ROW	||
		rowtot > CED_MAX_ROW
		)
	{
		return 1;
	}

	if ( (row + rowtot - 1) > CED_MAX_ROW )
	{
		rowtot = CED_MAX_ROW + 1 - row;
	}

	state.rctot = -rowtot;

	res = ced_sheet_clear_area ( sheet, row, 0, rowtot, 0, 0 );
	if ( res )
	{
		return res;
	}

	if ( sheet->rows && sheet->rows->root )
	{
		recurse_rc_reindex ( sheet->rows->root, &state );
	}

	return 0;
}


static void recurse_col_reindex (
	mtTreeNode	* const	node,
	insremSTATE	* const	state
	)
{
	recurse_rc_reindex ( ( (mtTree *)node->data )->root, state );

	if ( node->left )
	{
		recurse_col_reindex ( node->left, state );
	}

	if ( node->right )
	{
		recurse_col_reindex ( node->right, state );
	}
}

int ced_sheet_insert_column (
	CedSheet	* const	sheet,
	int		const	col,
	int			coltot
	)
{
	int		res;
	insremSTATE	state = { col, 0 };


	if (	! sheet			||
		col < 1			||
		coltot < 1		||
		col > CED_MAX_COLUMN	||
		coltot > CED_MAX_COLUMN
		)
	{
		return 1;
	}

	if ( (col + coltot - 1) > CED_MAX_COLUMN )
	{
		coltot = CED_MAX_COLUMN + 1 - col;
	}

	state.rctot = coltot;

	// Pastry cut the end columns before recursing
	res = ced_sheet_clear_area ( sheet, 0, CED_MAX_COLUMN + 1 - coltot, 0,
		0, 0 );

	if ( res )
	{
		return res;
	}

	if ( sheet->rows && sheet->rows->root )
	{
		recurse_col_reindex ( sheet->rows->root, &state );
	}

	return 0;
}

int ced_sheet_delete_column (
	CedSheet	* const	sheet,
	int		const	col,
	int			coltot
	)
{
	int		res;
	insremSTATE	state = { col, 0 };


	if (	! sheet			||
		col < 1			||
		coltot < 1		||
		col > CED_MAX_COLUMN	||
		coltot > CED_MAX_COLUMN
		)
	{
		return 1;
	}

	if ( (col + coltot - 1) > CED_MAX_COLUMN )
	{
		coltot = CED_MAX_COLUMN + 1 - col;
	}

	state.rctot = -coltot;

	res = ced_sheet_clear_area ( sheet, 0, col, 0, coltot, 0 );

	if ( res )
	{
		return res;
	}

	if ( sheet->rows && sheet->rows->root )
	{
		recurse_col_reindex ( sheet->rows->root, &state );
	}

	return 0;
}



typedef struct
{
	CedSheet		* sheet,
				* newsheet;
	int			row,
				column,
				rowtot,
				coltot;

	int			crown;		// Current row # in newsheet
	mtTree			* scol;		// Cell tree in sheet row

	CedCell			* newcell;	// Duplicated cell
	mtTree			* ccol;		// Cell tree in newsheet

	int			newkey;

	CedCellStack		* clear_root;

	mtTreeNode		* tnode;	// Temp

	CedCellPrefs	const	* default_prefs;
} copySTATE;

/*
NOTE
I prefer a state structure as it saves having to wastefully create and destroy
variables for each invocation of the following recursive functions.
M.Tyler 14-8-2009
*/



static int copy_col_recurse (
	mtTreeNode	* const	colnode,
	copySTATE	* const	state
	)
{
	if ( colnode->left && (intptr_t)colnode->key > state->column )
	{
		if ( copy_col_recurse ( colnode->left, state ) )
		{
			return 1;
		}
	}

	if ( (intptr_t)colnode->key >= state->column &&
		( state->coltot == 0 ||
			(intptr_t)colnode->key <=
			(state->column + state->coltot - 1)
			)
		)
	{
		// This cell is in the desired column range so duplicate it
		state->newcell = ced_cell_duplicate ( (CedCell *)colnode->data
			);

		if ( ! state->newcell )
		{
			return 1;
		}

		if ( ! state->ccol )
		{
			// Add new row to newsheet to hold this cell, populate
			// ccol with new cell tree

			state->ccol = mtkit_tree_new ( ced_cmp_cell,
				ced_del_cell );

			if ( ! state->ccol )
			{
				ced_cell_destroy ( state->newcell );

				return 1;
			}

			// Add new row to newsheet
			if ( ! mtkit_tree_node_add ( state->newsheet->rows,
				(void *)(intptr_t)state->crown, state->ccol )
				)
			{
				mtkit_tree_destroy ( state->ccol );
				state->ccol = NULL;

				return 1;
			}
		}

		state->newkey = (int)(intptr_t)colnode->key - state->column + 1;
		if ( ! mtkit_tree_node_add ( state->ccol,
			(void *)(intptr_t)state->newkey, state->newcell )
			)
		{
			ced_cell_destroy ( state->newcell );

			return 1;
		}
	}

	if (	colnode->right &&
		( state->coltot == 0 ||
			(intptr_t)colnode->key <
				(state->column + state->coltot - 1)
			)
		)
	{
		if ( copy_col_recurse ( colnode->right, state ) )
		{
			return 1;
		}
	}

	return 0;
}

static int copy_row_recurse (
	mtTreeNode	* const	rownode,
	copySTATE	* const	state
	)
{
	if ( rownode->left && (intptr_t)rownode->key > state->row )
	{
		if ( copy_row_recurse ( rownode->left, state ) )
		{
			return 1;
		}
	}

	state->crown = (int)(intptr_t)rownode->key - state->row + 1;
	state->ccol = NULL;
	state->scol = (mtTree *)rownode->data;

	if (	state->scol->root &&
		(intptr_t)rownode->key >= state->row &&
		( state->rowtot == 0 ||
			(intptr_t)rownode->key <=
				(state->row + state->rowtot - 1)
			)
		)
	{
		// This row is in the desired range and it contains cells, so
		// recurse it column-wise

		if ( copy_col_recurse ( state->scol->root, state ) )
		{
			return 1;
		}
	}

	if ( rownode->right &&
		( state->rowtot == 0 ||
			(intptr_t)rownode->key <
				(state->row + state->rowtot - 1)
			)
		)
	{
		if ( copy_row_recurse ( rownode->right, state ) )
		{
			return 1;
		}
	}

	return 0;	// Success
}

CedSheet * ced_sheet_copy_area (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	int			rowtot,
	int			coltot
	)
{
	copySTATE	state = { sheet, NULL, row, column, rowtot, coltot,
				0, NULL, NULL, NULL, 0, NULL, NULL, NULL };


	if (	! sheet ||
		row > CED_MAX_ROW ||
		column > CED_MAX_COLUMN ||
		rowtot > (CED_MAX_ROW + 1) ||
		coltot > (CED_MAX_COLUMN + 1)
		)
	{
		return NULL;
	}

	if ( (column + coltot) > (CED_MAX_COLUMN + 1) )
	{
		coltot = CED_MAX_COLUMN + 1 - column;
	}

	if ( (row + rowtot) > (CED_MAX_ROW + 1) )
	{
		rowtot = CED_MAX_ROW + 1 - row;
	}

	state.coltot = coltot;
	state.rowtot = rowtot;

	state.newsheet = ced_sheet_new ();
	if ( ! state.newsheet )
	{
		return NULL;
	}

	if ( sheet->rows && sheet->rows->root )
	{
		if ( copy_row_recurse ( sheet->rows->root, &state ) )
		{
			ced_sheet_destroy ( state.newsheet );

			return NULL;
		}
	}

	return state.newsheet;
}

CedSheet * ced_sheet_copy_selection (
	CedSheet	* const	sheet,
	int		* const	rowtot,
	int		* const	coltot
	)
{
	int		r1,
			c1,
			r2,
			c2;
	CedSheet	* new_sheet;


	if ( ! sheet || ! rowtot || ! coltot )
	{
		return NULL;
	}

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	rowtot[0] = r2 - r1 + 1;
	coltot[0] = c2 - c1 + 1;

	new_sheet = ced_sheet_copy_area ( sheet, r1, c1, rowtot[0], coltot[0] );

	return new_sheet;
}

static int clear_cell_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	if ( ced_cell_stack_push ( & ( (copySTATE *)user_data )->clear_root,
		row, col )
		)
	{
		return 1;	// Stop
	}

	return 0;		// Continue
}

static int clear_content_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	copySTATE	* const	state = (copySTATE *)user_data;


	if ( ! cell->prefs )
	{
		// The prefs are the default and we are about to remove the
		// content so the cell can be deleted

		if ( ced_cell_stack_push ( &state->clear_root, row, col ) )
		{
			return 1;	// Stop
		}
	}
	else
	{
		// The prefs have been changed so we can only remove the cell
		// content

		free ( cell->text );
		cell->text = NULL;
		cell->type = CED_CELL_TYPE_NONE;
		cell->value = 0;
	}

	return 0;			// Continue
}

static int clear_prefs_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	copySTATE	* const	state = (copySTATE *)user_data;


	if ( ! cell->text )
	{
		// There is no cell content so the cell can be deleted

		if ( ced_cell_stack_push ( &state->clear_root, row, col ) )
		{
			return 1;	// Stop
		}
	}
	else
	{
		// There is content so simply set the prefs to default

		ced_cell_prefs_destroy ( cell->prefs );
		cell->prefs = NULL;
	}

	return 0;			// Continue
}


int ced_sheet_clear_area (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	int		const	rowtot,
	int		const	coltot,
	int		const	mode
	)
{
	copySTATE	state = { sheet, NULL, row, column, rowtot, coltot,
				0, NULL, NULL, NULL, 0, NULL, NULL, NULL };
	CedFuncScanArea	func;


	switch ( mode )
	{
	case CED_PASTE_CONTENT:
		func = clear_content_scan;
		break;

	case CED_PASTE_PREFS:
		func = clear_prefs_scan;
		break;

	default:
		func = clear_cell_scan;
	}

	state.default_prefs = ced_cell_prefs_default ();

	if ( ced_sheet_scan_area ( sheet, row, column, rowtot, coltot, func,
		&state )
		)
	{
		ced_cell_stack_destroy ( state.clear_root );

		return 1;
	}

	if ( ced_cell_stack_del_cells ( state.clear_root, sheet ) )
	{
		ced_cell_stack_destroy ( state.clear_root );

		return 2;
	}

	ced_cell_stack_destroy ( state.clear_root );

	return 0;
}

int ced_cell_stack_push (
	CedCellStack	**	const	clear_root,
	int			const	row,
	int			const	col
	)
{
	CedCellStack	* cltmp;


	cltmp = (CedCellStack *)calloc ( sizeof ( CedCellStack ), 1 );
	if ( ! cltmp )
	{
		return 1;
	}

	cltmp->next = clear_root[0];
	cltmp->row = row;
	cltmp->col = col;
	clear_root[0] = cltmp;

	return 0;
}

int ced_cell_stack_del_cells (
	CedCellStack	* const	clear_root,
	CedSheet	* const	sheet
	)
{
	int		rown = 0;
	mtTree		* cell_tree = NULL;
	mtTreeNode	* rnode;
	CedCellStack	* cl;


	for ( cl = clear_root; cl; cl = cl->next )
	{
		if ( ! cell_tree || cl->row != rown )
		{
			rown = cl->row;
			rnode = mtkit_tree_node_find ( sheet->rows,
				(void *)(intptr_t)rown );

			if ( ! rnode )
			{
				return 1;
			}

			cell_tree = (mtTree *)rnode->data;
		}

		// Remove the cell
		mtkit_tree_node_remove ( cell_tree, (void *)(intptr_t)cl->col
			);

		if ( ! cell_tree->root )
		{
			// We have just deleted the last cell in this row so
			// destroy the row tree

			mtkit_tree_node_remove ( sheet->rows,
				(void *)(intptr_t)rown );

			rown = 0;
			cell_tree = NULL;
		}
	}

	return 0;
}

void ced_cell_stack_destroy (
	CedCellStack	* const	clear_root
	)
{
	CedCellStack	* cl,
			* tmp;


	for ( cl = clear_root; cl; cl = tmp )
	{
		tmp = cl->next;
		free ( cl );
	}
}

int ced_cell_set_2dyear (
	CedCell		* const	cell,
	int		const	year_start
	)
{
	int
			cent = 0,
			centm = 0,
			day = 1,
			divi,
			hour = 0,
			minute = 0,
			month = 1,
			new_year,
			second = 0,
			year = 0
			;
	char		buf[32] = {0},
			* newtxt,
			* src,
			* dest;
	size_t		buflen,
			len;


	if (	! cell ||
		year_start < MTKIT_DDT_MIN_DATE_YEAR ||
		year_start > (MTKIT_DDT_MAX_DATE_YEAR - 99) )
	{
		return -1;	// Arg Error
	}

	if ( cell->type != CED_CELL_TYPE_DATE || ! cell->text )
	{
		return 0;	// No Change
	}

	if ( mtkit_ddttoi ( cell->value, &day, &month, &year, &hour, &minute,
		&second )
		)
	{
		return -2;	// Error
	}

	if ( year > 100 || year < 0 )
	{
		return 0;	// No Change
	}

	centm = year_start % 100;
	cent = year_start - centm;

	if ( year < centm )
	{
		cent += 100;
	}

	new_year = cent + year;

	snprintf ( buf, sizeof ( buf ), "%i", new_year );

	buflen = strlen ( buf );
	len = strlen ( cell->text );

	// Check for overflow
	if ( len > (SIZE_MAX - 1 - buflen) )
	{
		return 0;	// No Change
	}

	newtxt = (char *)calloc ( 1, len + 1 + buflen );
	if ( ! newtxt )
	{
		return -2;	// Error
	}

	// Copy text to left of old year
	src = cell->text;
	dest = newtxt;
	for ( divi = 2; divi > 0 ; )
	{
		switch ( src[0] )
		{
		case 0:
			free ( newtxt );
			return -2;	// Error

		case '-':
		case '/':
			divi --;
			break;
		}

		dest[0] = src[0];
		dest ++;
		src ++;
	}

	// Copy whitespace
	while ( isspace ( src[0] ) )
	{
		dest[0] = src[0];
		dest ++;
		src ++;
	}

	// Insert new year
	memcpy ( dest, buf, buflen );
	dest += buflen;

	// Skip old year chars
	while ( isdigit ( src[0] ) )
	{
		src ++;
	}

	// Copy text to right of old year
	while ( src[0] )
	{
		dest[0] = src[0];
		dest ++;
		src ++;
	}

	// Create new DDT
	if ( mtkit_itoddt ( day, month, new_year, hour, minute, second,
		&cell->value ) )
	{
		free ( newtxt );

		return -2;
	}

	// NOTE: newtxt has been terminated by the calloc

	free ( cell->text );
	cell->text = newtxt;

	return 1;		// Successful change
}

