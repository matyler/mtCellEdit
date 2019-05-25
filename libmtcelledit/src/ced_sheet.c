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



static int cmp_row (
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

static void del_row (
	mtTreeNode	* const	node
	)
{
	mtkit_tree_destroy ( (mtTree *)node->data );
}

CedSheet * ced_sheet_new ( void )
{
	CedSheet * const sheet = (CedSheet *)( calloc (
		sizeof ( CedSheet ), 1 ) );

	if ( ! sheet )
	{
		return NULL;
	}

	sheet->rows = mtkit_tree_new ( cmp_row, del_row );
	if ( ! sheet->rows )
	{
		free ( sheet );

		return NULL;
	}

	sheet->prefs.cursor_r1 = 1;
	sheet->prefs.cursor_c1 = 1;
	sheet->prefs.cursor_r2 = 1;
	sheet->prefs.cursor_c2 = 1;
	sheet->prefs.start_row = 1;
	sheet->prefs.start_col = 1;

	return sheet;
}

int ced_sheet_destroy (
	CedSheet	* const	sheet
	)
{
	if ( ! sheet )
	{
		return 1;
	}

	if ( sheet->book )
	{
		// Detach this sheet from the book that contains it
		ced_book_detach_sheet ( sheet );
	}

	mtkit_tree_destroy ( sheet->rows );
	free ( sheet );

	return 0;
}

static int stat_parse_cell_data (	// Populate the cell with the text +
					// parse as required
	CedSheet	* const	sheet,
	CedCell		* const	cell,	// MUST be a cell in the sheet
	char	const	* const	text,	// MUST always contain something,
					// i.e. never NULL or "" or "'"
	int		const	row,
	int		const	column
	)
{
	char		* tx;


	if ( text[0] == '\'' )
	{
		tx = strdup ( text + 1 );
	}
	else
	{
		tx = strdup ( text );
	}

	if ( ! tx )
	{
		return 1;
	}

	free ( cell->text );
	cell->text = tx;

	if ( text[0] == '\'' )
	{
		cell->type = CED_CELL_TYPE_TEXT_EXPLICIT;
	}
	else if ( text[0] == '=' )
	{
		cell->type = CED_CELL_TYPE_FORMULA;
		ced_sheet_parse_text ( sheet, row, column, text, cell );

		return 0;
	}
	else if ( ! mtkit_strtod ( cell->text, &cell->value, NULL, 1 ) &&
			! isnan ( cell->value ) &&
			! isinf ( cell->value )
		)
	{
		cell->type = CED_CELL_TYPE_VALUE;

		return 0;
	}
	else if ( ! mtkit_strtoddt ( cell->text, &cell->value ) )
	{
		cell->type = CED_CELL_TYPE_DATE;

		return 0;
	}
	else
	{
		cell->type = CED_CELL_TYPE_TEXT;
	}

	cell->value = 0.0;

	return 0;
}

CedCell * ced_cell_set_find (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column
	)
{
	CedCell		* cell = NULL;
	mtTree		* ctree;
	mtTreeNode	* rnode;


	// Find the row
	rnode = mtkit_tree_node_find ( sheet->rows, (void *)(intptr_t)row );
	if ( ! rnode )
	{
		// No such row so create it

		// Create cell tree
		ctree = mtkit_tree_new ( ced_cmp_cell, ced_del_cell );
		if ( ! ctree )
		{
			return NULL;
		}

		cell = ced_cell_new ();
		if ( ! cell )
		{
			mtkit_tree_destroy ( ctree );

			return NULL;
		}

		if ( ! mtkit_tree_node_add ( ctree, (void *)(intptr_t)column,
			(void *)cell ) )
		{
			ced_cell_destroy ( cell );
			mtkit_tree_destroy ( ctree );

			return NULL;
		}

		if ( ! mtkit_tree_node_add ( sheet->rows,
			(void *)(intptr_t)row, (void *)ctree )
			)
		{
			mtkit_tree_destroy ( ctree );

			return NULL;
		}
	}
	else
	{
		// Find the cell in the row
		ctree = (mtTree *)rnode->data;

		mtTreeNode * const cnode = mtkit_tree_node_find ( ctree,
			(void *)(intptr_t)column );

		if ( ! cnode )
		{
			// No such cell so create it
			cell = ced_cell_new ();
			if ( ! cell )
			{
				return NULL;
			}

			if ( ! mtkit_tree_node_add ( ctree,
				(void *)(intptr_t)column, (void *)cell )
				)
			{
				ced_cell_destroy ( cell );

				return NULL;
			}
		}
		else
		{
			cell = (CedCell *)(cnode->data);
		}
	}

	return cell;
}

static int stat_destroy_cell_ref (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column
	)
{
	mtTreeNode	* rnode;


	// Find the row
	rnode = mtkit_tree_node_find ( sheet->rows, (void *)(intptr_t)row );
	if ( ! rnode )
	{
		return 0;			// No such row
	}

	// Remove the cell
	mtkit_tree_node_remove ( (mtTree *)rnode->data,
		(void *)(intptr_t)column );

	if ( ! ( (mtTree *)rnode->data )->root )
	{
		// We have just deleted the last cell in this row so destroy
		// the row tree

		return mtkit_tree_node_remove ( sheet->rows,
			(void *)(intptr_t)row );
	}

	return 0;
}

int ced_sheet_delete_cell (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column
	)
{
	if (	! sheet			||
		row < 0			||
		row > CED_MAX_ROW	||
		column < 0		||
		column > CED_MAX_COLUMN
		)
	{
		return -1;
	}

	return stat_destroy_cell_ref ( sheet, row, column );
}

static CedCell * stat_sheet_set_cell_real (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	char	const	* const	text,
	int		const	txt_type
	)
{
	CedCell		* cell = NULL;


	if (	! sheet			||
		row < 1			||
		row > CED_MAX_ROW	||
		column < 1		||
		column > CED_MAX_COLUMN
		)
	{
		return NULL;
	}

	cell = ced_cell_set_find ( sheet, row, column );
	if ( ! cell )
	{
		return NULL;
	}

	if ( ! text )
	{
		free ( cell->text );
		cell->text = NULL;
		cell->type = 0;
		cell->value = 0;

		// If the prefs are default then delete the cell as empty
		if ( ! cell->prefs )
		{
			stat_destroy_cell_ref ( sheet, row, column );

			return NULL;
		}

		return cell;
	}

	if ( txt_type )
	{
		char * tx = strdup ( text );

		if ( ! tx )
		{
			goto fail_cell;
		}

		free ( cell->text );
		cell->text = tx;
		cell->value = 0.0;
		cell->type = CED_CELL_TYPE_TEXT_EXPLICIT;
	}
	else
	{
		if ( stat_parse_cell_data ( sheet, cell, text, row, column ) )
		{
			goto fail_cell;
		}
	}

	return cell;

fail_cell:
	if ( ! cell->text )
	{
		stat_destroy_cell_ref ( sheet, row, column );
	}

	return NULL;
}

CedCell * ced_sheet_set_cell (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	char	const	* const	text
	)
{
	return stat_sheet_set_cell_real ( sheet, row, column, text, 0 );
}

CedCell * ced_sheet_set_cell_value (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	double		const	value
	)
{
	char txt[128];


	snprintf ( txt, sizeof ( txt ), CED_PRINTF_NUM, value );

	return stat_sheet_set_cell_real ( sheet, row, column, txt, 0 );
}

CedCell * ced_sheet_set_cell_text (
	CedSheet	*	const	sheet,
	int			const	row,
	int			const	column,
	char	const	*	const	text
	)
{
	return stat_sheet_set_cell_real ( sheet, row, column, text, 1 );
}

int ced_sheet_set_cell_prefs (
	CedSheet	*	const	sheet,
	int			const	row,
	int			const	column,
	CedCellPrefs	*		prefs,
	CedCell		**	const	cellp
	)
{
	CedCell		* cell;


	if (	! sheet			||
		row < 1			||
		row > CED_MAX_ROW	||
		column < 1		||
		column > CED_MAX_COLUMN
		)
	{
		return 1;
	}

	// If prefs are default set to NULL
	if ( prefs && ! memcmp ( prefs, ced_cell_prefs_default (),
		sizeof ( CedCellPrefs ) )
		)
	{
		prefs = NULL;
	}

	cell = ced_sheet_get_cell ( sheet, row, column );
	if ( ! cell && ! prefs )
	{
		return 0;		// Nothing to do
	}

	if ( ! prefs && cell && ! cell->text )
	{
		// Delete the cell if no text in it and the prefs are default
		stat_destroy_cell_ref ( sheet, row, column );

		return 0;
	}

	if ( ! cell )
	{
		cell = ced_cell_set_find ( sheet, row, column );
		if ( ! cell )
		{
			return 1;
		}
	}

	if ( ced_cell_set_prefs ( cell, prefs ) )
	{
		return 2;
	}

	if ( cellp )
	{
		cellp[0] = cell;
	}

	return 0;
}

int ced_sheet_set_column_width (
	CedSheet	* const	sheet,
	int			column,
	int			coltot,
	int			width
	)
{
	if (	! sheet				||
		column < 1			||
		column > CED_MAX_COLUMN		||
		coltot < 1			||
		coltot > CED_MAX_COLUMN		||
		(column + coltot - 1) > CED_MAX_COLUMN
		)
	{
		return 1;
	}

	if ( width == 0 )
	{
		if ( ced_sheet_clear_area ( sheet, 0, column, 1, coltot, 0 ) )
		{
			return 1;
		}

		return 0;
	}

	if ( width > CED_MAX_COLUMN_WIDTH )
	{
		width = CED_MAX_COLUMN_WIDTH;
	}

	if ( width < CED_MIN_COLUMN_WIDTH )
	{
		width = CED_MIN_COLUMN_WIDTH;
	}

	for ( ; coltot > 0; coltot --, column ++ )
	{
		CedCell * const cell = ced_cell_set_find ( sheet, 0, column );

		if ( ! cell )
		{
			return -1;
		}

		if ( ! cell->prefs )
		{
			cell->prefs = ced_cell_prefs_new ();
			if ( ! cell->prefs )
			{
				return 1;
			}
		}

		cell->prefs->width = width;
	}

	return 0;
}

CedCell * ced_sheet_get_cell (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column
	)
{
	if (	! sheet			||
		row < 0			||
		row > CED_MAX_ROW	||
		column < 0		||
		column > CED_MAX_COLUMN
		)
	{
		return NULL;
	}

	// Find the row
	mtTreeNode * const rnode = mtkit_tree_node_find ( sheet->rows,
		(void *)(intptr_t)row );

	if ( ! rnode )
	{
		return NULL;		// No such row
	}

	// Find the cell in the row
	mtTreeNode * const cnode = mtkit_tree_node_find ( (mtTree *)rnode->data,
		(void *)(intptr_t)column );

	if ( ! cnode )
	{
		return NULL;		// No such cell
	}

	return (CedCell *)(cnode->data);
}



typedef struct	recalcSTATE	recalcSTATE;

struct recalcSTATE
{
	int		update_tot;
	CedParser	parser;
};



static int recalc_cb (
	CedSheet	* const	sheet,
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	ud
	)
{
	recalcSTATE	* const	state = (recalcSTATE *)(ud);


	if (	cell->type == CED_CELL_TYPE_FORMULA_EVAL ||
		cell->type == CED_CELL_TYPE_ERROR
		)
	{
		char	const *	const	formula = cell->text;
		double			start = cell->value;


		if ( formula && formula[0] == '=' )
		{
			state->parser.cell = cell;

			ced_sheet_parse_text ( sheet, row, col, formula, cell );

			// For the purposes of this function nan == nan
			if (	start != cell->value &&
				! ( isnan ( start ) && isnan ( cell->value ) )
				)
			{
				state->update_tot ++;
			}
		}
	}

	return 0;		// Continue
}

int ced_sheet_recalculate (
	CedSheet	* const	sheet,
	int		* const	updates,
	int		const	mode
	)
{
	recalcSTATE	state = { 0 };


	if ( ! sheet )
	{
		return 1;
	}

	if ( sheet->rows && sheet->rows->root )
	{
		int		res = 0;

		state.parser.sheet = sheet;

		if ( mode == 0 )
		{
			res = ced_sheet_scan_area ( sheet, 1, 1, 0, 0,
				recalc_cb, &state );
		}
		else
		{
			res = ced_sheet_scan_area_backwards ( sheet,
				CED_MAX_ROW, CED_MAX_COLUMN,
				CED_MAX_ROW, CED_MAX_COLUMN,
				recalc_cb, &state );
		}

		if ( res )
		{
			return 1;
		}
	}

	if ( updates )
	{
		updates[0] = state.update_tot;
	}

	return 0;
}


static int recurse_max_col (
	mtTreeNode	* const	row_node,
	int			max
	)
{
	mtTree		* cell_tree;
	mtTreeNode	* cell_node;


	if ( ! row_node )
	{
		return max;
	}

	if ( row_node->left )
	{
		max = recurse_max_col ( row_node->left, max );
	}

	cell_tree = (mtTree *)row_node->data;
	if ( cell_tree )
	{
		int		new_max = 0;

		if ( cell_tree->root )
		{
			cell_node = cell_tree->root;
			while ( cell_node->right )
			{
				cell_node = cell_node->right;
			}

			new_max = (int)(intptr_t)cell_node->key;
		}

		if ( new_max > max )
		{
			max = new_max;
		}
	}

	if ( row_node->right )
	{
		max = recurse_max_col ( row_node->right, max );
	}

	return max;
}

int ced_sheet_get_geometry (
	CedSheet	* const	sheet,
	int		* const	row_max,
	int		* const	col_max
	)
{
	if ( ! sheet )
	{
		return 1;
	}

	if ( row_max )
	{
		int		rows = 0;

		if ( sheet->rows && sheet->rows->root )
		{
			mtTreeNode const * node = sheet->rows->root;

			while ( node->right )
			{
				node = node->right;
			}

			rows = (int)(intptr_t)node->key;
		}

		row_max[0] = rows;
	}

	if ( col_max )
	{
		int		cols = 0;

		if ( sheet->rows && sheet->rows->root )
		{
			cols = recurse_max_col ( sheet->rows->root, 0 );
		}

		col_max[0] = cols;
	}

	return 0;
}

int ced_sheet_tsvmem_geometry (
	char	const	* const	mem,
	size_t		const	bytes,
	int		* const	rows,
	int		* const	cols
	)
{
	int		row,
			col,
			colmax = 0;
	char	const	* end,
			* s1,
			* s2;
	char		c = 0;


	if (	! mem		||
		bytes < 1	||
		mem [ bytes - 1 ] != 0
		)
	{
		return 1;
	}

	row = 1;
	col = 1;
	s1 = s2 = mem;
	end = mem + bytes;

	for ( ; s1 < end; s1 = s2 + 1 )
	{
		s2 = s1;

		while (	s2[0] != '\t' &&
			s2[0] != '\n' &&
			s2[0] != '\r' &&
			s2[0] != '\0' )
		{
			s2 ++;
		}

		c = s2[0];

		if ( c == '\0' )
		{
			break;
		}
		else if ( c == '\t' )
		{
			col ++;
		}
		else
		{
			if ( c == '\r' && s2[1] == '\n' )
			{
				// DOS newline
				s2 ++;
			}

			if ( c == '\n' || c == '\r' )
			{
				// Row delimiter
				if ( col > colmax )
				{
					colmax = col;
				}

				col = 1;
				row ++;
			}
		}
	}

	if (	c == '\n' ||
		c == '\r' ||
		s2 == mem
		)
	{
		// Remove empty row added with trailing newline; or file
		// containing single '0' byte

		row --;
	}
	else if ( c == 0 &&
		(s2[-1] == '\n' || s2[-1] == '\r')
		)
	{
		// Remove empty row added with trailing newline followed by a
		// '0' byte

		row --;
	}

	if ( row == 0 )
	{
		colmax = 0;
	}
	else
	{
		if ( col > colmax )
		{
			colmax = col;
		}
	}

	if ( rows )
	{
		rows[0] = row;
	}

	if ( cols )
	{
		cols[0] = colmax;
	}

	return 0;
}



typedef struct
{
	int		col_start,
			* width_list;
} autocolSTATE;



static int colauto_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	autocolSTATE	* const	state = (autocolSTATE *)user_data;
	char		* txt = NULL;
	int		ci;


	if ( ! cell->text )
	{
		return 0;
	}

	txt = ced_cell_create_output ( cell, NULL );
	if ( ! txt )
	{
		return 0;
	}

	// Is this text larger than previous max for this column?
	ci = col - state->col_start;


	size_t		w = strlen ( txt );


	if ( w < INT_MAX )
	{
		state->width_list[ci] = MAX ( (int)w, state->width_list[ci] );
	}

	free ( txt );

	return 0;
}

int ced_sheet_get_column_width_list (
	CedSheet	* const	sheet,
	int		const	col,
	int		const	coltot,
	int	**	const	width_list
	)
{
	int		res;
	autocolSTATE	state = { col, NULL };


	// Other args are validated by ced_sheet_scan_area () below
	if ( coltot < 1 || ! width_list )
	{
		return 1;
	}

	width_list[0] = (int *)calloc ( (size_t)coltot, sizeof ( int ) );
	if ( ! width_list[0] )
	{
		return 1;
	}

	state.width_list = width_list[0];

	res = ced_sheet_scan_area ( sheet, 1, col, 0, coltot, colauto_cb,
		&state );
	if ( res )
	{
		free ( width_list[0] );
		width_list[0] = NULL;

		return 1;
	}

	return 0;
}

int ced_sheet_set_column_width_list (
	CedSheet	* const	sheet,
	int		const	col,
	int		const	coltot,
	int		* const	width_list
	)
{
	if (	! sheet					||
		col < 1					||
		col > CED_MAX_COLUMN			||
		coltot < 1				||
		coltot > CED_MAX_COLUMN			||
		(col + coltot - 1) > CED_MAX_COLUMN	||
		! width_list
		)
	{
		return 1;
	}

	int i;

	for ( i = 0; i < coltot; i++ )
	{
		if ( width_list[i] < 1 )
		{
			continue;
		}

		// Note: we set to one greater than the text width to account
		// for the cell padding

		if ( ced_sheet_set_column_width( sheet, col + i, 1,
			width_list[i] + 1 )
			)
		{
			return 1;
		}
	}

	return 0;	// success
}

void ced_sheet_cursor_max_min (
	CedSheet	* const	sheet,
	int		* const	r1,
	int		* const	c1,
	int		* const r2,
	int		* const	c2
	)
{
	int const tr1 = MIN ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	int const tr2 = MAX ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	int const tc1 = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	int const tc2 = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );

	r1[0] = MAX ( tr1, 1 );
	r2[0] = MAX ( tr2, 1 );
	c1[0] = MAX ( tc1, 1 );
	c2[0] = MAX ( tc2, 1 );
}

CedSheetPrefs * ced_sheet_prefs_new ( void )
{
	return (CedSheetPrefs *)( calloc ( sizeof ( CedSheetPrefs ), 1 ) );
}

int ced_sheet_prefs_free (
	CedSheetPrefs	* const	prefs
	)
{
	free ( prefs );

	return 0;
}

int ced_sheet_prefs_copy (
	CedSheetPrefs	* const	dest,
	CedSheetPrefs	* const	src
	)
{
	if ( ! src || ! dest )
	{
		return 1;
	}

	memcpy ( dest, src, sizeof ( CedSheetPrefs ) );

	return 0;
}

