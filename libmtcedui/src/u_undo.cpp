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

#include "private.h"


#define FUNCTION_UNDO_HEADER \
	CuiUndoStep	* undo_data = NULL; \
	int		f = 1, \
			res = 0; \


#define FUNCTION_UNDO_SETUP( FN_ID ) \
	if ( ub->undo.max ) \
	{ \
		undo_data = cui_undo_step_new ( FN_ID, ub, sheet ); \
		if ( ! undo_data ) \
		{ \
			cui_book_undo_flush ( ub ); \
			res = CUI_ERROR_UNDO_LOST; \
		} \
	} \


#define FUNCTION_UNDO_BRANCH \
	if ( f ) \
	{ \
		if ( f == -1 ) \
		{ \
			cui_book_undo_flush ( ub ); \
			res = CUI_ERROR_CHANGES; \
		} \
		else \
		{ \
			res = CUI_ERROR_NO_CHANGES; \
		} \
		\
		goto error; \
	} \
	else \
	{ \
		if ( undo_data ) \
		{ \
			cui_undo_commit_step ( ub, undo_data ); \
		} \
	} \


#define FUNCTION_UNDO_FAIL \
	error_report: \
		res = CUI_ERROR_UNDO_OP; \
	\
	error: \
		if ( undo_data ) \
		{ \
			cui_undo_step_destroy ( undo_data ); \
		} \


#define FUNCTION_UNDO_FAIL_2 \
	error: \
		if ( undo_data ) \
		{ \
			cui_undo_step_destroy ( undo_data ); \
		} \


/*
	TEMPLATE:

... cui_function ( ... )
{
	FUNCTION_UNDO_HEADER

# Do any checks for locked cells - bail out if required
# Function declarations & startup tests put here
# NOTE: you must have 'sheet' (CedSheet) and 'ub' (undoable book CuBook)
# prepared before this macro

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_ADD )

	if ( undo_data )
	{
# Here the function prepares the undo structures and puts them into
# the prepared 'undo_data->atom'.
	}

# In the event of multiple atoms are required use core_undo_atom_new () and
# 'undo_data->atom_last'.


# Function does its thing here with 'f' exit flag:
#	 0 = success;
#	-1 = flush undo;
#	< -1 || >0 = fail

	f = core_function ( ... );

	FUNCTION_UNDO_BRANCH

# Function does its own cleanups for a safe exit here

	return res;

	FUNCTION_UNDO_FAIL

# Function does its own error cleanups here

	return res;
}


*/



enum	// Undo function ID's for atom
{
	UNDO_FN_SHEET_DESTROY,
	UNDO_FN_SHEET_PASTE,		// Including sort, clear content/prefs
	UNDO_FN_SHEET_ADD,
	UNDO_FN_SHEET_RENAME,
	UNDO_FN_ROW_INSERT,
	UNDO_FN_ROW_DELETE,
	UNDO_FN_COL_INSERT,
	UNDO_FN_COL_DELETE,
	UNDO_FN_CELL_SET,

	UNDO_FN_TOTAL
};



#define UNDO_ATOM_CHARP_MAX		2
#define UNDO_ATOM_CELLP_MAX		2
#define UNDO_ATOM_SHEETP_MAX		2
#define UNDO_ATOM_UINT_MAX		8



typedef struct CuiUndoAtom	CuiUndoAtom;



struct CuiUndoStep
{
	CuiUndoAtom	* atom,		// First atom
			* atom_last;	// Last atom added

	char		* sheet_name;	// Name of sheet for this step to action

	CedSheetPrefs	sheet_prefs;	// Cursor/Scrollbar stuff for this step

	CuiUndoStep	* undo_step,	// Next undo step
			* redo_step;	// Next redo step
};

struct CuiUndoAtom
{
	int		fn_id;		// Function that is to be carried out

	char		* charp[UNDO_ATOM_CHARP_MAX];
	CedCell		* cellp[UNDO_ATOM_CELLP_MAX];
	CedSheet	* sheetp[UNDO_ATOM_SHEETP_MAX];
	int		uint[UNDO_ATOM_UINT_MAX];

	CuiUndoAtom	* previous,	// Non-circular
			* next;
};

// UNDO_FN_SHEET_DESTROY	old sheet name,, old sheet,
// UNDO_FN_SHEET_PASTE		,, old area/paste|NULL = clear, r/c/rtot/ctot/paste_rowtot/paste_coltot/mode|~0 = col width/width (column widths)
// UNDO_FN_SHEET_ADD		current active_sheet name / new page name,, new sheet, 0 = on redo change active sheet name to new name 1 = don't
// UNDO_FN_SHEET_RENAME		old/new page name,,,
// UNDO_FN_ROW_INSERT		,, over-run rows|NULL, r/rtot
// UNDO_FN_ROW_DELETE		,, old row, r/rtot
// UNDO_FN_COL_INSERT		,, over-run cols|NULL, c/ctot
// UNDO_FN_COL_DELETE		,, old col, c/ctot
// UNDO_FN_CELL_SET		old/new cell text,,, r/c





static int cui_undo_atom_new (
	CuiUndoStep	* const	step,
	int		const	fn_id
	)
{
	CuiUndoAtom	* atom;


	atom = (CuiUndoAtom *)calloc ( 1, sizeof ( CuiUndoAtom ) );
	if ( ! atom )
	{
		return 1;
	}

	atom->fn_id = fn_id;

	// Add this atom to the end of the list of atoms for this step
	if ( step->atom_last )
	{
		step->atom_last->next = atom;
		atom->previous = step->atom_last;
	}
	else
	{
		step->atom = atom;
	}

	step->atom_last = atom;

	return 0;
}

static int cui_undo_atom_destroy (
	CuiUndoAtom	* const	atom
	)
{
	int		i;


	for ( i = 0; i < UNDO_ATOM_CHARP_MAX; i++ )
	{
		free ( atom->charp[i] );
	}

	for ( i = 0; i < UNDO_ATOM_CELLP_MAX; i++ )
	{
		ced_cell_destroy ( atom->cellp[i] );
	}

	for ( i = 0; i < UNDO_ATOM_SHEETP_MAX; i++ )
	{
		ced_sheet_destroy ( atom->sheetp[i] );
	}

	free ( atom );

	return 0;
}

static int cui_undo_step_destroy (
	CuiUndoStep	* const	undo_step
	)
{
	if ( undo_step->atom )		// Destroy any atoms
	{
		CuiUndoAtom	* atom = undo_step->atom,
				* atom2;

		do
		{
			atom2 = atom->next;
			cui_undo_atom_destroy ( atom );
			atom = atom2;
		}
		while ( atom );
	}

	free ( undo_step->sheet_name );
	free ( undo_step );

	return 0;
}

static CuiUndoStep * cui_undo_step_new (
	int		const	fn_id,
	CuiBook		* const	ARG_UNUSED ( ub ),
	CedSheet	* const	sheet
	)
{
	CuiUndoStep	* step;


	step = (CuiUndoStep *)calloc ( 1, sizeof ( CuiUndoStep ) );
	if ( ! step )
	{
		return NULL;
	}

	if ( sheet )
	{
		step->sheet_prefs.cursor_r1 = sheet->prefs.cursor_r1;
		step->sheet_prefs.cursor_c1 = sheet->prefs.cursor_c1;
		step->sheet_prefs.cursor_r2 = sheet->prefs.cursor_r2;
		step->sheet_prefs.cursor_c2 = sheet->prefs.cursor_c2;
		step->sheet_prefs.split_r1 = sheet->prefs.split_r1;
		step->sheet_prefs.split_r2 = sheet->prefs.split_r2;
		step->sheet_prefs.split_c1 = sheet->prefs.split_c1;
		step->sheet_prefs.split_c2 = sheet->prefs.split_c2;
		step->sheet_prefs.start_row = sheet->prefs.start_row;
		step->sheet_prefs.start_col = sheet->prefs.start_col;

		step->sheet_name = strdup((char const *)sheet->book_tnode->key);
		if ( ! step->sheet_name )
		{
			goto error;
		}
	}

	if ( cui_undo_atom_new ( step, fn_id ) )
	{
		goto error;
	}

	return step;

error:
	cui_undo_step_destroy ( step );

	return NULL;
}

static void prune_undo_steps (
	CuiBook		* const	ub,
	int		const	skip
	)
{
	CuiUndoStep	* st1,
			* st2;
	int		i;


	for (	i = 0,		st1 = ub->undo.undo_step;
		i < skip &&	st1;
		i++,		st1 = st1->undo_step )
	{
	}

	if ( ! st1 )
	{
		return;			// Nothing to prune
	}

	if ( st1->redo_step )
	{
		// NULL any reference to st1
		st1->redo_step->undo_step = NULL;
	}

	do
	{
		st2 = st1->undo_step;
		cui_undo_step_destroy ( st1 );
		ub->undo.undo_tot --;
		st1 = st2;
	}
	while ( st1 );

	if ( skip < 1 )
	{
		ub->undo.undo_step = NULL;
	}
}

static void prune_redo_steps (
	CuiBook		* const	ub,
	int		const	skip
	)
{
	CuiUndoStep	* st1,
			* st2;
	int		i;


	for (	i = 0,		st1 = ub->undo.redo_step;
		i < skip &&	st1;
		i++,		st1 = st1->redo_step )
	{
	}


	if ( ! st1 )
	{
		return;			// Nothing to prune
	}

	if ( st1->undo_step )
	{
		// NULL any reference to st1
		st1->undo_step->redo_step = NULL;
	}

	do
	{
		st2 = st1->redo_step;
		cui_undo_step_destroy ( st1 );
		st1 = st2;
	}
	while ( st1 );

	if ( skip < 1 )
	{
		ub->undo.redo_step = NULL;
	}
}

static void prune_steps (
	CuiBook		* const	ub
	)
{
	if ( ub->undo.max < ub->undo.undo_tot )
	{
		prune_undo_steps ( ub, ub->undo.max );
	}

	if ( ub->undo.max < ub->undo.redo_tot )
	{
		prune_redo_steps ( ub, ub->undo.max );
	}
}


static int cui_undo_commit_step (
	CuiBook		* const	ub,
	CuiUndoStep	* const	undo_step
	)
{
	// Remove all redo steps
	prune_redo_steps ( ub, 0 );

	if ( ub->undo.undo_step )
	{
		ub->undo.undo_step->redo_step = undo_step;
		prune_undo_steps ( ub, ub->undo.max-1 );
	}

	undo_step->undo_step = ub->undo.undo_step;
	ub->undo.undo_step = undo_step;
	ub->undo.undo_tot ++;

	return 0;
}

static int check_for_locked_cells_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	CedCellRef	* const	cref = (CedCellRef *)user_data;


	cref->row_d = row;
	cref->col_d = col;

	if ( cell->prefs && cell->prefs->locked )
	{
		return 1;
	}

	return 0;
}

int cui_check_sheet_lock (
	CedSheet	* const	sheet
	)
{
	if ( ! sheet )
	{
		return 0;
	}

	if ( sheet->book && sheet->book->prefs.disable_locks )
	{
		return 0;
	}

	if ( sheet->prefs.locked )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	return 0;
}



static char	* cui_error_buf[2048];



char const * cui_error_str ( void )
{
	// Get the error string to report to user

	return (char const *)cui_error_buf;
}


static int cui_check_cell_lock (	// Are any of these cells locked?
	CedSheet	* const	sheet,
	int		const	r,
	int		const	c,
	int		const	rowtot,
	int		const	coltot
	)
	// 1 = locked
{
	int		i;
	CedCellRef	cref;


	if ( ! sheet )
	{
		return 0;
	}

	if ( sheet->book && sheet->book->prefs.disable_locks )
	{
		return 0;
	}

	i = ced_sheet_scan_area ( sheet, r, c, rowtot, coltot,
		check_for_locked_cells_cb, &cref );

	if ( i )
	{
		if ( i == 2 )
		{
			snprintf ( (char *)cui_error_buf,
				sizeof ( cui_error_buf ),
				"r%ic%i", cref.row_d, cref.col_d );
		}
		else
		{
			snprintf ( (char *)cui_error_buf,
				sizeof ( cui_error_buf ),
				"Problem scanning for locked cells." );
		}

		i = CUI_ERROR_LOCKED_CELL;
	}

	return i;
}



///	API FUNCS

static int cui_book_undo_set_max (	// Set maximum undo/redo steps,
					// clearing any excess
	CuiBook		* const	ub,
	int		const	max_steps	// 0..
	)
{
	if ( ! ub )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	ub->undo.max = max_steps;
	prune_steps ( ub );

	return 0;
}

CuiBook * cui_book_new ( void )
{
	CuiBook		* cubook;


	cubook = (CuiBook *)calloc ( sizeof ( CuiBook ), 1 );

	if ( cubook )
	{
		cui_book_undo_set_max ( cubook, CUI_DEFAULT_MAX_STEPS );
	}

	return cubook;
}

int cui_book_destroy (
	CuiBook		* const	ub
	)
{
	if ( ! ub )
	{
		return 0;		// Nothing to do
	}

	cui_book_undo_set_max ( ub, 0 );	// Remove old undo steps
	ced_book_destroy ( ub->book );
	free ( ub );

	return 0;
}

static int cui_book_undo_flush (	// Remove all undo/redo steps (but keep
					// the current max_steps)
	CuiBook		* const	ub
	)
{
	int		old;

	if ( ! ub )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	old = ub->undo.max;
	cui_book_undo_set_max ( ub, 0 );
	cui_book_undo_set_max ( ub, old );

	return 0;
}

int cui_book_undo_step (
	CuiBook		* const	ub
	)
{
	CedSheet	* sheet;
	CuiUndoAtom	* atom;
	CuiUndoStep	* step;
	char		* next_active_sheet = NULL;


	if (	! ub ||
		! ub->undo.undo_step ||
		! ub->book
		)
	{
		return CUI_ERROR_NO_CHANGES;
	}

	step = ub->undo.undo_step;

	if ( step->sheet_name )
	{
		sheet = ced_book_get_sheet ( ub->book, step->sheet_name );
		if ( ! sheet )
		{
			goto error;
		}
	}
	else
	{
		// This sheet might be NULL if we are undo'ing a sheet add

		sheet = NULL;
	}

	next_active_sheet = step->sheet_name;

	for ( atom = step->atom_last; atom; atom = atom->previous )
	{
		switch ( atom->fn_id )
		{
		case UNDO_FN_SHEET_DESTROY:
			sheet = ced_sheet_duplicate ( atom->sheetp[0] );
			if ( ! sheet )
			{
				goto error;
			}

			if ( ced_book_add_sheet ( ub->book, sheet,
				atom->charp[0] ) )
			{
				ced_sheet_destroy ( sheet );
				sheet = NULL;

				goto error;
			}
			else
			{
				next_active_sheet = atom->charp[0];
			}

			sheet = NULL;
			break;

		case UNDO_FN_SHEET_PASTE:
			if ( ced_sheet_paste_area ( sheet, atom->sheetp[0],
				atom->uint[0], atom->uint[1], atom->uint[2],
				atom->uint[3], atom->uint[2], atom->uint[3],
				0 ) )
			{
				goto error;
			}
			break;

		case UNDO_FN_SHEET_ADD:
			next_active_sheet = atom->charp[0];

			if ( ced_book_destroy_sheet ( ub->book, atom->charp[1]
				) )
			{
				goto error;
			}
			break;

		case UNDO_FN_SHEET_RENAME:
			sheet = ced_book_get_sheet ( ub->book, atom->charp[1] );
			if ( ! sheet )
			{
				goto error;
			}

			if ( ced_book_page_rename ( sheet, atom->charp[0] ) )
			{
				sheet = NULL;

				goto error;
			}

			sheet = NULL;
			next_active_sheet = atom->charp[0];
			break;

		case UNDO_FN_ROW_INSERT:
			if (	ced_sheet_delete_row ( sheet, atom->uint[0],
					atom->uint[1] ) ||
				(atom->sheetp[0] && ced_sheet_paste_area (
					sheet, atom->sheetp[0],
					CED_MAX_ROW + 1 - atom->uint[1], 0,
					atom->uint[1], CED_MAX_COLUMN + 1,
					atom->uint[1], CED_MAX_COLUMN + 1, 0 )
				) )
			{
				goto error;
			}
			break;

		case UNDO_FN_ROW_DELETE:
			if (	ced_sheet_insert_row ( sheet, atom->uint[0],
					atom->uint[1] ) ||
				ced_sheet_paste_area ( sheet, atom->sheetp[0],
					atom->uint[0], 0, atom->uint[1],
					CED_MAX_COLUMN + 1, atom->uint[1],
					CED_MAX_COLUMN + 1, 0 )
				)
			{
				goto error;
			}
			break;

		case UNDO_FN_COL_INSERT:
			if (	ced_sheet_delete_column ( sheet, atom->uint[0],
					atom->uint[1] ) ||
				(atom->sheetp[0] &&
					ced_sheet_paste_area ( sheet,
					atom->sheetp[0],
					0, CED_MAX_COLUMN + 1 - atom->uint[1],
					CED_MAX_ROW + 1, atom->uint[1],
					CED_MAX_ROW + 1, atom->uint[1],
					0 )
				) )
			{
				goto error;
			}
			break;

		case UNDO_FN_COL_DELETE:
			if (	ced_sheet_insert_column ( sheet, atom->uint[0],
					atom->uint[1] ) ||
				ced_sheet_paste_area ( sheet, atom->sheetp[0],
					0, atom->uint[0],
					CED_MAX_ROW + 1, atom->uint[1],
					CED_MAX_ROW + 1, atom->uint[1], 0 )
				)
			{
				goto error;
			}
			break;

		case UNDO_FN_CELL_SET:
			if ( ! sheet )
			{
				goto error;
			}

			if ( ! ced_sheet_set_cell ( sheet, atom->uint[0],
					atom->uint[1], atom->charp[0] ) &&
				atom->charp[0]
				)
			{
				goto error;
			}
			break;

		default:
			goto error;
		}
	}

	if ( sheet )
	{
		sheet->prefs.cursor_r1 = step->sheet_prefs.cursor_r1;
		sheet->prefs.cursor_c1 = step->sheet_prefs.cursor_c1;
		sheet->prefs.cursor_r2 = step->sheet_prefs.cursor_r2;
		sheet->prefs.cursor_c2 = step->sheet_prefs.cursor_c2;
		sheet->prefs.split_r1 = step->sheet_prefs.split_r1;
		sheet->prefs.split_r2 = step->sheet_prefs.split_r2;
		sheet->prefs.split_c1 = step->sheet_prefs.split_c1;
		sheet->prefs.split_c2 = step->sheet_prefs.split_c2;
		sheet->prefs.start_row = step->sheet_prefs.start_row;
		sheet->prefs.start_col = step->sheet_prefs.start_col;
	}

	if ( mtkit_strfreedup ( &ub->book->prefs.active_sheet,
		next_active_sheet )
		)
	{
		goto error;
	}

	// Commit step change by going backwards through undo
	ub->undo.redo_step = step;
	ub->undo.undo_step = step->undo_step;

	ub->undo.undo_tot --;
	ub->undo.redo_tot ++;

	return 0;

error:
	cui_book_undo_flush ( ub );	// Fatal error - remove all undo steps

	return CUI_ERROR_CHANGES;
}



/*
This function finds the left adjacent node (LAN) to a sheet name (if it exists)
*/

static char * get_previous_sheetname (
	CedBook		* const	book,
	char		*	name
	)
{
	mtTreeNode	* node,
			* rp = NULL;
	int		c;


	if (	! book ||
		! name ||
		! book->sheets
		)
	{
		return name;
	}

	node = book->sheets->root;

	// Traverse the tree looking for 'name' sheet
	while ( 1 )
	{
		if ( ! node )
		{
			return name;
		}

		c = strcmp ( name, (char const *)node->key );
		if ( c == 0 )
		{
			break;		// We have found the 'name' sheet
		}

		if ( c < 0 )		// name < node->key
		{
			node = node->left;
		}
		else			// name > node->key
		{
			rp = node;
			node = node->right;
		}
	}

	if ( node->left )
	{
		// The left child or the its rightmost descendant is the LAN
		node = node->left;

		while ( node->right )
		{
			node = node->right;
		}

		name = (char *)node->key;
	}
	else
	{
		// Most recent right branching ancestor is the LAN
		if ( rp )
		{
			name = (char *)rp->key;
		}
	}

	return name;
}

int cui_book_redo_step (
	CuiBook		* const	ub
	)
{
	CedSheet	* sheet,
			* ts;
	CuiUndoAtom	* atom;
	CuiUndoStep	* step;
	char		* next_active_sheet = NULL;


	if (	! ub ||
		! ub->undo.redo_step ||
		! ub->book )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	step = ub->undo.redo_step;

	if ( step->sheet_name )
	{
		sheet = ced_book_get_sheet ( ub->book, step->sheet_name );
		if ( ! sheet )
		{
			goto error;
		}
	}
	else
	{
		// This sheet might be NULL if we are undo'ing a sheet add

		sheet = NULL;
	}

	next_active_sheet = step->sheet_name;

	for ( atom = step->atom; atom; atom = atom->next )
	{
		switch ( atom->fn_id )
		{
		case UNDO_FN_SHEET_DESTROY:
			next_active_sheet = get_previous_sheetname ( ub->book,
				atom->charp[0] );

			if ( ced_book_destroy_sheet ( ub->book, atom->charp[0]
				) )
			{
				goto error;
			}

			break;

		case UNDO_FN_SHEET_PASTE:
			if ( ! sheet )
			{
				goto error;
			}

			if ( atom->sheetp[1] )
			{
				// Redo paste
				if ( ced_sheet_paste_area ( sheet,
					atom->sheetp[1], atom->uint[0],
					atom->uint[1], atom->uint[2],
					atom->uint[3], atom->uint[4],
					atom->uint[5], atom->uint[6] )
					)
				{
					goto error;
				}
			}
			else if ( atom->uint[6] == ~0 )
			{
				// Redo column width
				if ( ced_sheet_set_column_width ( sheet,
					atom->uint[1], atom->uint[3],
					atom->uint[7] )
					)
				{
					goto error;
				}
			}
			else
			{
				// Redo clear
				if ( ced_sheet_clear_area ( sheet,
					atom->uint[0], atom->uint[1],
					atom->uint[2], atom->uint[3],
					atom->uint[6] )
					)
				{
					goto error;
				}
			}
			break;

		case UNDO_FN_SHEET_ADD:
			if ( ! atom->uint[0] )
			{
				next_active_sheet = atom->charp[1];
			}
			else
			{
				next_active_sheet = atom->charp[0];
			}

			ts = ced_sheet_duplicate ( atom->sheetp[0] );
			if ( ! ts )
			{
				goto error;
			}

			if ( ced_book_add_sheet ( ub->book, ts, atom->charp[1]
				) )
			{
				ced_sheet_destroy ( ts );

				goto error;
			}
			break;

		case UNDO_FN_SHEET_RENAME:
			sheet = ced_book_get_sheet ( ub->book, atom->charp[0] );
			if ( ! sheet )
			{
				goto error;
			}

			if ( ced_book_page_rename ( sheet, atom->charp[1] ) )
			{
				sheet = NULL;

				goto error;
			}

			sheet = NULL;
			next_active_sheet = atom->charp[1];
			break;

		case UNDO_FN_ROW_INSERT:
			if ( ced_sheet_insert_row ( sheet, atom->uint[0],
				atom->uint[1] ) )
			{
				goto error;
			}
			break;

		case UNDO_FN_ROW_DELETE:
			if ( ced_sheet_delete_row ( sheet, atom->uint[0],
				atom->uint[1] ) )
			{
				goto error;
			}
			break;

		case UNDO_FN_COL_INSERT:
			if ( ced_sheet_insert_column ( sheet, atom->uint[0],
				atom->uint[1] ) )
			{
				goto error;
			}
			break;

		case UNDO_FN_COL_DELETE:
			if ( ced_sheet_delete_column ( sheet, atom->uint[0],
				atom->uint[1] ) )
			{
				goto error;
			}
			break;

		case UNDO_FN_CELL_SET:
			if ( ! sheet )
			{
				goto error;
			}

			if (	! ced_sheet_set_cell ( sheet, atom->uint[0],
					atom->uint[1], atom->charp[1] ) &&
				atom->charp[1]
				)
			{
				goto error;
			}

			break;

		default:
			goto error;
		}
	}

	if ( sheet )
	{
		sheet->prefs.cursor_r1 = step->sheet_prefs.cursor_r1;
		sheet->prefs.cursor_c1 = step->sheet_prefs.cursor_c1;
		sheet->prefs.cursor_r2 = step->sheet_prefs.cursor_r2;
		sheet->prefs.cursor_c2 = step->sheet_prefs.cursor_c2;
		sheet->prefs.split_r1 = step->sheet_prefs.split_r1;
		sheet->prefs.split_r2 = step->sheet_prefs.split_r2;
		sheet->prefs.split_c1 = step->sheet_prefs.split_c1;
		sheet->prefs.split_c2 = step->sheet_prefs.split_c2;
		sheet->prefs.start_row = step->sheet_prefs.start_row;
		sheet->prefs.start_col = step->sheet_prefs.start_col;
	}

	if ( mtkit_strfreedup ( &ub->book->prefs.active_sheet,
		next_active_sheet )
		)
	{
		goto error;
	}

	// Commit step change by going forewards through redo
	ub->undo.undo_step = step;
	ub->undo.redo_step = step->redo_step;

	ub->undo.undo_tot ++;
	ub->undo.redo_tot --;

	return 0;

error:
	cui_book_undo_flush ( ub );	// Fatal error - remove all undo steps

	return CUI_ERROR_CHANGES;
}



/*
	--------------------------------------------
	Undoable wrapper functions for libmtcelledit
	--------------------------------------------
*/

int cui_book_add_sheet (
	CuiBook		* const	ub,
	CedSheet	* const	new_sheet,
	char	const	* const	page
	)
{
	CedSheet	* sheet = NULL;	// Dummy to fool FUNCTION_UNDO_SETUP


	FUNCTION_UNDO_HEADER

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_ADD )

	if ( undo_data )
	{
		if ( ub->book->prefs.active_sheet )
		{
			undo_data->atom->charp[0] = strdup (
				ub->book->prefs.active_sheet );

			if ( ! undo_data->atom->charp[0] )
			{
				goto error_report;
			}
		}

		undo_data->atom->charp[1] = strdup ( page );
		if ( ! undo_data->atom->charp[1] )
		{
			goto error_report;
		}

		undo_data->atom->sheetp[0] = ced_sheet_duplicate ( new_sheet );
		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}
	}

	f = ced_book_add_sheet ( ub->book, new_sheet, page );

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_book_destroy_sheet (
	CuiBook		* const	ub,
	char	const	* const	page
	)
{
	CedSheet	* sheet = NULL,
			* ds;


	FUNCTION_UNDO_HEADER

	ds = ced_book_get_sheet ( ub->book, page );
	if (	! ds ||
		! ds->book_tnode ||
		! ds->book_tnode->key
		)
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( ds ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_DESTROY )

	if ( undo_data )
	{
		undo_data->atom->charp[0] = strdup ( page );
		if ( ! undo_data->atom->charp[0] )
		{
			goto error_report;
		}

		f = ced_book_detach_sheet ( ds );
		if ( ! f )
		{
			undo_data->atom->sheetp[0] = ds;
		}
	}
	else
	{
		f = ced_book_destroy_sheet ( ub->book, page );
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_book_page_rename (
	CuiBook		* const	ub,
	CedSheet	* const	shold,
	char	const	* const	name
	)
{
	CedSheet	* sheet = NULL;	// Dummy to fool FUNCTION_UNDO_SETUP


	FUNCTION_UNDO_HEADER

	if (	! shold ||
		! shold->book_tnode ||
		! shold->book_tnode->key ||
		! name
		)
	{
		return CUI_ERROR_NO_CHANGES;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_RENAME )

	if ( undo_data )
	{
		undo_data->atom->charp[0] = strdup (
			(char const *)shold->book_tnode->key );
		if ( ! undo_data->atom->charp[0] )
		{
			goto error_report;
		}

		undo_data->atom->charp[1] = strdup ( name );
		if ( ! undo_data->atom->charp[1] )
		{
			goto error_report;
		}
	}

	f = ced_book_page_rename ( shold, name );
	if ( f == -1 )
	{
		ced_sheet_destroy ( shold );
	}
	else if ( f == 2 )
	{
		f = 1;
	}

	FUNCTION_UNDO_BRANCH

	mtkit_strfreedup ( &ub->book->prefs.active_sheet, name );

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}



typedef struct
{
	int		* sheet_tot,
			* sheet_fail,
			* file_tot,
			* file_fail,
			res;
	CuiUndoStep	* undo_step;
} mergeSTATE;



static int cui_book_merge_cb (
	CedBook		* const	ARG_UNUSED ( book_dest ),
	CedBook		* const	ARG_UNUSED ( book_insert ),
	void		* const	item,		// Pointer to sheet/bookfile
	int		const	type,		// 0 = sheet 1 = file
	char	const	* const	name,		// Name of sheet or file
	int		const	already_exists,	// 1 = This name already exists
						// in book_dest
	void		* const	user_data
	)
{
	mergeSTATE	* const	state = (mergeSTATE *)user_data;


	if ( already_exists )
	{
		switch ( type )
		{
		case 0:
			state->sheet_fail[0] ++;
			break;

		case 1:
			state->file_fail[0] ++;
			break;
		}

		return 1;		// Don't move this sheet/file
	}

	if ( type == 1 )		// File
	{
		/*
		NOTE: We don't bother with putting files into the undo step as
		this undo stack is only interested in changes to the sheets in
		the book, not anything else.
		*/

		state->file_tot[0] ++;

		return 0;		// Move this file
	}

	if ( type != 0 )
	{
		return 1;		// Unknown type so do nothing
	}

	// We are here so this must be a sheet to be moved

	if ( state->undo_step )
	{
		// Only do this if we have an undo step to populate

		if ( state->undo_step->atom->charp[1] )
		{
			// Only add an atom if not the first invocation

			if ( cui_undo_atom_new ( state->undo_step,
				UNDO_FN_SHEET_ADD ) )
			{
				return 2;
			}
		}

		state->undo_step->atom_last->charp[1] = strdup ( name );
		if ( ! state->undo_step->atom_last->charp[1] )
		{
			return 2;
		}

		state->undo_step->atom_last->sheetp[0] = ced_sheet_duplicate (
			(CedSheet *)item );
		if ( ! state->undo_step->atom_last->sheetp[0] )
		{
			return 2;
		}

		state->undo_step->atom_last->uint[0] = 1;
		// New sheet must not become the active sheet
	}

	state->sheet_tot[0] ++;

	return 0;			// Move this sheet
}

int cui_book_merge (
	CuiBook		* const	ub,
	CedBook		* const	book,
	int		* const	sheet_tot,
	int		* const	sheet_fail,
	int		* const	file_tot,
	int		* const	file_fail
	)
{
	CedSheet	* sheet = NULL;
	mergeSTATE	state = { sheet_tot, sheet_fail, file_tot, file_fail,
				0, NULL };


	FUNCTION_UNDO_HEADER

	sheet_tot[0] = 0;
	sheet_fail[0] = 0;
	file_tot[0] = 0;
	file_fail[0] = 0;

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_ADD )

	state.undo_step = undo_data;
	f = ced_book_merge ( ub->book, book, cui_book_merge_cb, &state );
	if ( f > 1 )
	{
		f = -1;
	}

	if ( sheet_tot[0] == 0 )
	{
		// Nothing imported so don't bother with an undo step
		if ( undo_data )
		{
			cui_undo_step_destroy ( undo_data );
		}

		return 0;
	}

	if ( undo_data )
	{
		sheet = ced_book_get_sheet ( ub->book,
			ub->book->prefs.active_sheet );

		if ( sheet )
		{
			// Set up the current sheet name (if we have one)

			undo_data->atom_last->charp[0] = strdup (
				ub->book->prefs.active_sheet );
			sheet = NULL;
		}
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL_2

	return res;
}

int cui_sheet_set_cell (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	char	const	* const	text
	)
{
	CedCell		* cell;


	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	if ( cui_check_cell_lock ( sheet, row, column, 1, 1 ) )
	{
		return CUI_ERROR_LOCKED_CELL;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_CELL_SET )

	if ( undo_data )
	{
		cell = ced_sheet_get_cell ( sheet, row, column );

		if ( cell && cell->text )
		{
			undo_data->atom->charp[0] = strdup ( cell->text );
			if ( ! undo_data->atom->charp[0] )
			{
				goto error_report;
			}
		}
		else
		{
			// Leave NULL in atom cell pointer
		}

		if ( text )
		{
			undo_data->atom->charp[1] = strdup ( text );
			if ( ! undo_data->atom->charp[1] )
			{
				goto error_report;
			}
		}

		undo_data->atom->uint[0] = row;
		undo_data->atom->uint[1] = column;
	}

	if (	ced_sheet_set_cell ( sheet, row, column, text ) ||
		! text
		)
	{
		f = 0;
	}
	else
	{
		f = 1;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_set_column_width (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	column,
	int		const	coltot,
	int		const	width
	)
{
	FUNCTION_UNDO_HEADER


	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_PASTE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet, 0,
			column, 1, coltot );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = 0;
		undo_data->atom->uint[1] = column;
		undo_data->atom->uint[2] = 1;
		undo_data->atom->uint[3] = coltot;
		undo_data->atom->uint[4] = 1;
		undo_data->atom->uint[5] = coltot;
		undo_data->atom->uint[6] = ~0;
		undo_data->atom->uint[7] = width;
	}

	if ( ced_sheet_set_column_width ( sheet, column, coltot, width ) )
	{
		f = -1;
	}
	else
	{
		f = 0;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

static int cui_sheet_set_column_width_list (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	column,
	int		const	coltot,
	int		* const	width_list
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_PASTE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet, 0,
			column, 1, coltot );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = 0;
		undo_data->atom->uint[1] = column;
		undo_data->atom->uint[2] = 1;
		undo_data->atom->uint[3] = coltot;
		undo_data->atom->uint[4] = 1;
		undo_data->atom->uint[5] = coltot;
		undo_data->atom->uint[6] = 0;
	}

	if ( ced_sheet_set_column_width_list ( sheet, column, coltot,
		width_list )
		)
	{
		f = 1;
	}
	else
	{
		f = 0;
	}

	if ( undo_data && f == 0 )
	{
		undo_data->atom->sheetp[1] = ced_sheet_copy_area ( sheet, 0,
			column, 1, coltot );

		if ( ! undo_data->atom->sheetp[1] )
		{
			f = -1;
		}
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_set_column_width_auto (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	column,
	int		const	coltot
	)
{
	int		res,
			* width_list;


	if ( ! ub || ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( ced_sheet_get_column_width_list ( sheet, column, coltot,
		&width_list )
		)
	{
		return CUI_ERROR_NO_CHANGES;
	}

	res = cui_sheet_set_column_width_list ( ub, sheet, column, coltot,
		width_list );

	free ( width_list );

	return res;
}

int cui_sheet_sort_rows (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	rowtot,
	int	const	* const	cols,
	int		const	mode,
	int	const	* const	mode_list
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_PASTE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet, row,
			1, rowtot, CED_MAX_COLUMN );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = row;
		undo_data->atom->uint[1] = 1;
		undo_data->atom->uint[2] = rowtot;
		undo_data->atom->uint[3] = CED_MAX_COLUMN;
		undo_data->atom->uint[4] = rowtot;
		undo_data->atom->uint[5] = CED_MAX_COLUMN;
		undo_data->atom->uint[6] = 0;
	}

	if ( ced_sheet_sort_rows ( sheet, row, rowtot, cols, mode, mode_list ) )
	{
		f = -1;
	}
	else
	{
		f = 0;
	}

	if ( undo_data )
	{
		undo_data->atom->sheetp[1] = ced_sheet_copy_area ( sheet, row,
			1, rowtot, CED_MAX_COLUMN );

		if ( ! undo_data->atom->sheetp[1] )
		{
			goto error;
		}
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_sort_columns (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	column,
	int		const	coltot,
	int	const	* const	rows,
	int		const	mode,
	int	const	* const	mode_list
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_PASTE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet, 1,
			column, CED_MAX_ROW, coltot );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = 1;
		undo_data->atom->uint[1] = column;
		undo_data->atom->uint[2] = CED_MAX_ROW;
		undo_data->atom->uint[3] = coltot;
		undo_data->atom->uint[4] = CED_MAX_ROW;
		undo_data->atom->uint[5] = coltot;
		undo_data->atom->uint[6] = 0;
	}

	if ( ced_sheet_sort_columns ( sheet, column, coltot, rows, mode,
		mode_list ) )
	{
		f = -1;
	}
	else
	{
		f = 0;
	}

	if ( undo_data )
	{
		undo_data->atom->sheetp[1] = ced_sheet_copy_area ( sheet, 1,
			column, CED_MAX_ROW, coltot );
		if ( ! undo_data->atom->sheetp[1] )
		{
			goto error;
		}
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_insert_row (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	rowtot
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_ROW_INSERT )

	if ( undo_data )
	{
		// Store the area that get lost in the pastry cut
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet,
			CED_MAX_ROW + 1 - rowtot, 0,
			rowtot, CED_MAX_COLUMN + 1 );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		if (	! undo_data->atom->sheetp[0]->rows ||
			! undo_data->atom->sheetp[0]->rows->root )
		{
			// Nothing in the pasty cut area so lose it

			ced_sheet_destroy ( undo_data->atom->sheetp[0] );
			undo_data->atom->sheetp[0] = NULL;
		}

		undo_data->atom->uint[0] = row;
		undo_data->atom->uint[1] = rowtot;
	}

	f = ced_sheet_insert_row ( sheet, row, rowtot );
	if ( f == 2 )
	{
		f = -1;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_delete_row (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	rowtot
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	if ( cui_check_cell_lock ( sheet, row, 0, rowtot, 0 ) )
	{
		return CUI_ERROR_LOCKED_CELL;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_ROW_DELETE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet, row,
			0, rowtot, CED_MAX_COLUMN + 1 );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = row;
		undo_data->atom->uint[1] = rowtot;
	}

	f = ced_sheet_delete_row ( sheet, row, rowtot );
	if ( f == 2 )
	{
		f = -1;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_insert_column (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	col,
	int		const	coltot
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_COL_INSERT )

	if ( undo_data )
	{
		// Store the area that get lost in the pastry cut
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet,
			0, CED_MAX_COLUMN + 1 - coltot,
			CED_MAX_ROW + 1, coltot );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		if (	! undo_data->atom->sheetp[0]->rows ||
			! undo_data->atom->sheetp[0]->rows->root )
		{
			// Nothing in the pasty cut area so lose it

			ced_sheet_destroy ( undo_data->atom->sheetp[0] );
			undo_data->atom->sheetp[0] = NULL;
		}

		undo_data->atom->uint[0] = col;
		undo_data->atom->uint[1] = coltot;
	}

	f = ced_sheet_insert_column ( sheet, col, coltot );
	if ( f == 2 )
	{
		f = -1;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_delete_column (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	col,
	int		const	coltot
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	if ( cui_check_cell_lock ( sheet, 0, col, 0, coltot ) )
	{
		return CUI_ERROR_LOCKED_CELL;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_COL_DELETE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet, 0,
			col, CED_MAX_ROW + 1, coltot );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = col;
		undo_data->atom->uint[1] = coltot;
	}

	f = ced_sheet_delete_column ( sheet, col, coltot );
	if ( f == 2 )
	{
		f = -1;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_paste_area (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	CedSheet	* const	paste,
	int		const	row,
	int		const	column,
	int		const	rowtot,
	int		const	coltot,
	int		const	paste_rowtot,
	int		const	paste_coltot,
	int		const	mode
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet || ! paste )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	if ( cui_check_cell_lock ( sheet, row, column, rowtot, coltot ) )
	{
		return CUI_ERROR_LOCKED_CELL;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_PASTE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area ( sheet, row,
			column, rowtot, coltot );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->sheetp[1] = ced_sheet_duplicate ( paste );
		if ( ! undo_data->atom->sheetp[1] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = row;
		undo_data->atom->uint[1] = column;
		undo_data->atom->uint[2] = rowtot;
		undo_data->atom->uint[3] = coltot;
		undo_data->atom->uint[4] = paste_rowtot;
		undo_data->atom->uint[5] = paste_coltot;
		undo_data->atom->uint[6] = mode;
	}

	f = ced_sheet_paste_area ( sheet, paste, row, column, rowtot, coltot,
		paste_rowtot, paste_coltot, mode );

	if ( f == 2 )
	{
		f = -1;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

int cui_sheet_clear_area (
	CuiBook		* const	ub,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	int		const	rowtot,
	int		const	coltot,
	int		const	mode
	)
{
	FUNCTION_UNDO_HEADER

	if ( ! sheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	if ( cui_check_sheet_lock ( sheet ) )
	{
		return CUI_ERROR_LOCKED_SHEET;
	}

	if ( cui_check_cell_lock ( sheet, row, column, rowtot, coltot ) )
	{
		return CUI_ERROR_LOCKED_CELL;
	}

	FUNCTION_UNDO_SETUP ( UNDO_FN_SHEET_PASTE )

	if ( undo_data )
	{
		undo_data->atom->sheetp[0] = ced_sheet_copy_area( sheet, row,
			column, rowtot, coltot );

		if ( ! undo_data->atom->sheetp[0] )
		{
			goto error_report;
		}

		undo_data->atom->uint[0] = row;
		undo_data->atom->uint[1] = column;
		undo_data->atom->uint[2] = rowtot;
		undo_data->atom->uint[3] = coltot;
		undo_data->atom->uint[4] = rowtot;
		undo_data->atom->uint[5] = coltot;
		undo_data->atom->uint[6] = mode;
	}

	f = ced_sheet_clear_area ( sheet, row, column, rowtot, coltot, mode );
	if ( f == 2 )
	{
		f = -1;
	}

	FUNCTION_UNDO_BRANCH

	return res;

	FUNCTION_UNDO_FAIL

	return res;
}

