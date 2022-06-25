/*
	Copyright (C) 2009-2021 Mark Tyler

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

#include "ced_sort.h"



class RowSort : public Sort_State
{
public:
	RowSort (
		CedSheet	* const	sheet,
		int		const	mode,
		int	const * const	mode_list
		)
		:
		Sort_State	( sheet, mode, mode_list )
	{
	}

	~RowSort ()
	{
		free_data ();
	}

	// All args and state is checked here, before allocating
	int sort ( int row, int rowtot, int const * cols );

	inline int get_cols ( int i ) const { return m_cols[i]; }

private:
	int alloc_data ();
	void free_data ();

	void recurse_array_populate ( mtTreeNode * tnode );
	void recurse_row_count ( mtTreeNode const * tnode );

/// ----------------------------------------------------------------------------

	int		m_row1		= 0;
	int		m_row2		= 0;
	int	const	* m_cols	= nullptr;
	int		m_i		= 0;
	int		m_active_rows	= 0;

	// Owned memory allocation pointers, so we use rule of 5
	mtTree		** m_rowtree_array = nullptr;	// m_active_rows items
	mtTreeNode	** m_treenode_array = nullptr;	// m_active_rows items

	MTKIT_RULE_OF_FIVE ( RowSort )
};



namespace {
static int rowsort_compare (
	void	const * const	a,
	void	const * const	b,
	void		* const	user
	)
{
	RowSort * const rs = static_cast<RowSort *>( user );

	mtTree * const r1 = ( (mtTree * const *)a )[0];
	mtTree * const r2 = ( (mtTree * const *)b )[0];

	for ( int i = 0; rs->get_cols( i ) > 0; i++ )
	{
		mtTreeNode const * const tn1 = mtkit_tree_node_find ( r1,
			(void *)(intptr_t)rs->get_cols( i ) );
		mtTreeNode const * const tn2 = mtkit_tree_node_find ( r2,
			(void *)(intptr_t)rs->get_cols( i ) );

		rs->set_mode_from_list ( i );

		int const res = rs->cmp_cells (
			tn1 ? (CedCell const *)(tn1->data) : nullptr,
			tn2 ? (CedCell const *)(tn2->data) : nullptr
			);
		if ( res )
		{
			return res;
		}
	}

	return 0;
}
} // namespace

int RowSort::sort (
	int		const	row,
	int		const	rowtot,
	int	const * const	cols
	)
{
	if (	! sheet()		||
		! cols			||
		row < 1			||
		row > CED_MAX_ROW	||
		rowtot > CED_MAX_ROW
		)
	{
		return 1;
	}

	if (	! sheet()->rows		||
		! sheet()->rows->root	||
		rowtot == 1
		)
	{
		return 0;		// Nothing to do
	}

	m_row1 = row;
	m_row2 = row + rowtot - 1;
	m_cols = cols;

	if ( rowtot == 0 )
	{
		m_row2 = 0;
	}

	if ( m_row2 > CED_MAX_ROW )
	{
		m_row2 = CED_MAX_ROW;
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
	m_active_rows = 0;
	recurse_row_count ( sheet()->rows->root );

	if ( m_active_rows < 1 )
	{
		return 0;		// Nothing to do
	}

	if ( alloc_data () )
	{
		return 1;
	}

	// Recursively populate arrays
	m_i = 0;
	recurse_array_populate ( sheet()->rows->root );

	// Quicksort the array
	qsort_r ( m_rowtree_array, (size_t)m_active_rows, sizeof (mtTree *),
		rowsort_compare, this );

	// Rework each node in tree by changing row # and tree pointer as per
	// the new array order.
	for ( int r = 0; r < m_active_rows; r++ )
	{
		if ( ! m_treenode_array[r] )
		{
			// Should never happen, but just in case.
			continue;
		}

		m_treenode_array[r]->key = (void *)(intptr_t)(row + r);
		m_treenode_array[r]->data = m_rowtree_array[r];
	}

	return 0;
}

int RowSort::alloc_data ()
{
	if ( m_rowtree_array || m_treenode_array )
	{
		free_data ();
	}

	// Create arrays for tree (of row cells) refs & treenodes
	m_rowtree_array = (mtTree **)calloc ( (size_t)m_active_rows,
		sizeof (mtTree *) );
	m_treenode_array = (mtTreeNode **)calloc ( (size_t)m_active_rows,
		sizeof (mtTreeNode *) );

	return (m_rowtree_array && m_treenode_array) ? 0 : 1;
}

void RowSort::free_data ()
{
	free ( m_rowtree_array );
	m_rowtree_array = nullptr;

	free ( m_treenode_array );
	m_treenode_array = nullptr;
}

void RowSort::recurse_array_populate ( mtTreeNode * const tnode )
{
	if ( tnode->left && ( (intptr_t)tnode->key > m_row1 ) )
	{
		recurse_array_populate ( tnode->left );
	}

	if (	( (intptr_t)tnode->key >= m_row1 ) &&
		(m_row2 == 0 || (intptr_t)tnode->key <= m_row2)
		)
	{
		m_rowtree_array[ m_i ] = (mtTree *)tnode->data;
		m_treenode_array[ m_i ] = tnode;
		m_i ++;
	}

	if (	tnode->right &&
		( m_row2 == 0 || (intptr_t)tnode->key < m_row2)
		)
	{
		recurse_array_populate ( tnode->right );
	}
}

void RowSort::recurse_row_count ( mtTreeNode const * const tnode )
{
	if ( tnode->left && ( (intptr_t)tnode->key > m_row1 ) )
	{
		recurse_row_count ( tnode->left );
	}

	if (	( (intptr_t)tnode->key >= m_row1 ) &&
		(m_row2 == 0 || (intptr_t)tnode->key <= m_row2)
		)
	{
		m_active_rows ++;
	}

	if (	tnode->right &&
		( m_row2 == 0 || (intptr_t)tnode->key < m_row2)
		)
	{
		recurse_row_count ( tnode->right );
	}
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
	RowSort rs ( sheet, mode, mode_list );

	return rs.sort ( row, rowtot, cols );
}

