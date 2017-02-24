/*
	Copyright (C) 2008-2016 Mark Tyler

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



typedef struct
{
	CedSheet	* sheet,
			* newsheet;
	int		row,
			column,
			max_row,
			max_col,
			rowli,
			colli,		// Row / Column loop in sheet dest
			crown;		// Current row # in paste src
	mtTree		* scol;		// Cell tree in paste sheet row

	CedCell		* newcell,	// New/Duplicated cell
			* oldcell;
	mtTree		* ccol;		// Cell tree in sheet dest

	int		paste_rowtot,
			paste_coltot;	// Used for max row/col in paste

	mtTreeNode	* tnode;	// Temp
	CedCellStack	* clear_root;

	CedCellPrefs	const	* default_prefs;

} pasteSTATE;



#define COL_RECURSE_START \
if ( (intptr_t)colnode->key <= state->paste_coltot ) \
{ \
	for (	state->colli = (int)(intptr_t)colnode->key + state->column - 1;\
		state->colli <= state->max_col; \
		state->colli += state->paste_coltot \
		) \
	{


#define COL_RECURSE_FINISH( FUNC ) \
	} \
} \
if ( colnode->left ) \
{ \
	FUNC ( colnode->left, state ); \
} \
if ( colnode->right && (intptr_t)colnode->key < state->paste_coltot ) \
{ \
	FUNC ( colnode->right, state ); \
} \
return 0;


#define ROW_RECURSE_START \
int		res; \
state->crown = (int)(intptr_t)rownode->key; \
if ( state->crown <= state->paste_rowtot ) \
{\
	state->scol = (mtTree *)rownode->data;		/* Row to paste */ \
	for (	state->rowli = state->crown + state->row - 1; \
		state->rowli <= state->max_row; \
		state->rowli += state->paste_rowtot \
		) \
	{ \
		state->tnode = mtkit_tree_node_find ( state->sheet->rows, \
			(void *)(intptr_t)(state->rowli) ); \
		if ( ! state->tnode ) \
		{ \
/* Add new row to sheet to hold this cell, populate ccol with new cell tree */ \
			state->ccol = mtkit_tree_new ( ced_cmp_cell, \
				ced_del_cell ); \
			if ( ! state->ccol ) \
			{ \
				return 2; \
			} \
			if ( ! mtkit_tree_node_add ( state->sheet->rows, \
				(void *)(intptr_t)(state->rowli), \
				state->ccol ) ) \
			{ \
				mtkit_tree_destroy ( state->ccol ); \
				state->ccol = NULL; \
				return 2; \
			} \
		} \
		else\
		{ \
			state->ccol = (mtTree *)state->tnode->data; \
		} \
		res = 0;


#define ROW_RECURSE_FINISH( FUNC )\
		/* Remove any row with no cells */ \
		if ( ! state->ccol->root ) \
		{ \
			mtkit_tree_node_remove ( state->sheet->rows, \
				(void *)(intptr_t)(state->rowli) ); \
		} \
		if ( res ) \
		{ \
			return res; \
		} \
	}\
}\
if ( rownode->left ) \
{ \
	FUNC ( rownode->left, state ); \
} \
if ( rownode->right && state->crown < state->paste_rowtot ) \
{ \
	FUNC ( rownode->right, state ); \
} \
return 0;



static int recurse_paste_col (
	mtTreeNode	* const	colnode,
	pasteSTATE	* const	state
	)
{
	COL_RECURSE_START

	state->newcell = ced_cell_duplicate ( (CedCell *)colnode->data );
	if ( ! state->newcell )
	{
		return 2;
	}

	if ( ! mtkit_tree_node_add ( state->ccol,
		(void *)(intptr_t)state->colli, state->newcell ) )
	{
		ced_cell_destroy ( state->newcell );

		return 2;
	}

	COL_RECURSE_FINISH ( recurse_paste_col )
}

static int recurse_paste_row (
	mtTreeNode	* const	rownode,
	pasteSTATE	* const	state
	)
{
	ROW_RECURSE_START

	if ( recurse_paste_col ( state->scol->root, state ) )
	{
		res = 2;
	}

	ROW_RECURSE_FINISH ( recurse_paste_row )
}

static int recurse_paste_col_content (
	mtTreeNode	* const	colnode,
	pasteSTATE	* const	state
	)
{
	COL_RECURSE_START

	state->oldcell = (CedCell *)colnode->data;	// Paste cell

	state->tnode = mtkit_tree_node_find ( state->ccol,
		(void *)(intptr_t)(state->colli) );

	if ( state->tnode )
	{
		// Current sheet cell
		state->newcell = (CedCell *)state->tnode->data;

		if (	! state->oldcell->text &&
			! state->newcell->prefs
			)
		{
			// Paste content = NULL && destination prefs = default
			// so remove cell

			if ( ced_cell_stack_push ( &state->clear_root,
				state->rowli, state->colli ) )
			{
				return 2;
			}
		}
		else
		{
			// Duplicate text/type/value
			if ( mtkit_strfreedup ( &state->newcell->text,
				state->oldcell->text ) )
			{
				return 2;
			}

			state->newcell->type = state->oldcell->type;
			state->newcell->value = state->oldcell->value;
		}
	}
	else	// No cell in this column
	{
		if ( state->oldcell->text )
		{
			// No current cell & paste has content so create empty
			// cell and duplicate text/type/value

			state->newcell = ced_cell_new ();
			if ( ! state->newcell )
			{
				return 2;
			}

			state->newcell->type = state->oldcell->type;
			state->newcell->value = state->oldcell->value;
			state->newcell->text = strdup ( state->oldcell->text );

			if (	! state->newcell->text ||
				! mtkit_tree_node_add ( state->ccol,
					(void *)(intptr_t)state->colli,
					state->newcell )
				)
			{
				ced_cell_destroy ( state->newcell );

				return 2;
			}
		}
	}

	COL_RECURSE_FINISH ( recurse_paste_col_content )
}

static int recurse_paste_row_content (
	mtTreeNode	* const	rownode,
	pasteSTATE	* const	state
	)
{
	ROW_RECURSE_START

	if ( recurse_paste_col_content ( state->scol->root, state ) )
	{
		res = 2;
	}

	ROW_RECURSE_FINISH ( recurse_paste_row_content )
}


static int recurse_paste_col_prefs (
	mtTreeNode	* const	colnode,
	pasteSTATE	* const	state
	)
{
	COL_RECURSE_START

	state->oldcell = (CedCell *)colnode->data;	// Paste cell

	state->tnode = mtkit_tree_node_find ( state->ccol,
		(void *)(intptr_t)(state->colli) );
	if ( state->tnode )
	{
		// Current sheet cell
		state->newcell = (CedCell *)state->tnode->data;

		if (	! state->newcell->text &&
			! state->oldcell->prefs
			)
		{
			// Paste prefs = default && destination content = empty
			// so remove cell
			if ( ced_cell_stack_push ( &state->clear_root,
				state->rowli, state->colli )
				)
			{
				return 2;
			}
		}
		else
		{
			// Set prefs in sheet from paste cell
			if ( ced_cell_set_prefs ( state->newcell,
				state->oldcell->prefs )
				)
			{
				return 2;
			}
		}
	}
	else	// No cell in this column
	{
		if ( state->oldcell->prefs )
		{
			// No current cell & paste has prefs so
			// create empty cell and duplicate prefs

			state->newcell = ced_cell_new ();
			if ( ! state->newcell )
			{
				return 2;
			}

			if (	ced_cell_set_prefs ( state->newcell,
					state->oldcell->prefs ) ||
				! mtkit_tree_node_add ( state->ccol,
					(void *)(intptr_t)state->colli,
					state->newcell )
				)
			{
				ced_cell_destroy ( state->newcell );

				return 2;
			}
		}
	}

	COL_RECURSE_FINISH ( recurse_paste_col_prefs )
}

static int recurse_paste_row_prefs (
	mtTreeNode	* const	rownode,
	pasteSTATE	* const	state
	)
{
	ROW_RECURSE_START

	if ( recurse_paste_col_prefs ( state->scol->root, state ) )
	{
		res = 2;
	}

	ROW_RECURSE_FINISH ( recurse_paste_row_prefs )
}

int ced_sheet_paste_area (
	CedSheet	* const	sheet,
	CedSheet	* const	paste,
	int		const	row,
	int		const	column,
	int			rowtot,
	int			coltot,
	int			paste_rowtot,
	int			paste_coltot,
	int		const	mode
	)
{
	pasteSTATE	state = { sheet, paste, row, column, 0, 0, 0, 0,
			0, NULL, NULL, NULL, NULL, 0, 0, NULL, NULL, NULL
			};
	int		res,
			action;


	if (	! sheet			||
		! paste			||
		row > CED_MAX_ROW	||
		column > CED_MAX_COLUMN
		)
	{
		return 1;
	}

	// Expand any auto-detects
	if (	! rowtot		||
		! coltot		||
		! paste_rowtot		||
		! paste_coltot
		)
	{
		int		r, c;


		ced_sheet_get_geometry ( paste, &r, &c );

		if ( ! r ) r = 1;
		if ( ! c ) c = 1;

		if ( ! rowtot ) rowtot = r;
		if ( ! coltot ) coltot = c;
		if ( ! paste_rowtot ) paste_rowtot = r;
		if ( ! paste_coltot ) paste_coltot = c;
	}

	if ( mode & CED_PASTE_CONTENT )
	{
		if ( ! (mode & CED_PASTE_ACTIVE_CELLS) )
		{
			// Clear old content
			res = ced_sheet_clear_area ( sheet, row, column, rowtot,
				coltot, CED_PASTE_CONTENT );
			if ( res )
			{
				return res;
			}
		}

		action = 0;
	}
	else if ( mode & CED_PASTE_PREFS )
	{
		if ( ! (mode & CED_PASTE_ACTIVE_CELLS) )
		{
			// Clear old prefs
			res = ced_sheet_clear_area ( sheet, row, column, rowtot,
				coltot, CED_PASTE_PREFS );
			if ( res )
			{
				return res;
			}
		}

		action = 1;
	}
	else
	{
		if ( ! (mode & CED_PASTE_ACTIVE_CELLS) )
		{
			// Clear cells
			res = ced_sheet_clear_area ( sheet, row, column, rowtot,
				coltot, 0 );
			if ( res )
			{
				return res;
			}
		}

		action = 2;
	}

	res = 0;
	if ( paste->rows && paste->rows->root )
	{
		state.default_prefs = ced_cell_prefs_default ();

		// This avoids wasting time when pasting a small area from a
		// large 'paste'
		state.paste_rowtot = MIN ( rowtot, paste_rowtot );
		state.paste_coltot = MIN ( coltot, paste_coltot );

		state.max_row = row + rowtot - 1;
		state.max_col = column + coltot - 1;

		if ( state.max_row > CED_MAX_ROW )
		{
			state.max_row = CED_MAX_ROW;
		}

		if ( state.max_col > CED_MAX_COLUMN )
		{
			state.max_col = CED_MAX_COLUMN;
		}

		switch ( action )
		{
		case 0:
			res = recurse_paste_row_content ( paste->rows->root,
				&state );
			break;

		case 1:
			res = recurse_paste_row_prefs ( paste->rows->root,
				&state );
			break;

		case 2:
			res = recurse_paste_row ( paste->rows->root, &state );
			break;
		}

		if ( ced_cell_stack_del_cells ( state.clear_root, sheet ) )
		{
			res = 2;
		}

		ced_cell_stack_destroy ( state.clear_root );
	}

	return res;
}

