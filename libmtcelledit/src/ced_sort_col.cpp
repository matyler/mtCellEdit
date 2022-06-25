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



typedef struct
{
	int		key;
	CedCell		* cell;
} csortNODE;



class ColSort : public Sort_State
{
public:
	ColSort (
		CedSheet	* const	sheet,
		int		const	mode,
		int	const * const	mode_list
		)
		:
		Sort_State	( sheet, mode, mode_list )
	{
	}

	~ColSort ()
	{
		free_data ();
	}

	// All args and state is checked here, before allocating
	int sort ( int column, int coltot, int const * rows );

	inline int col1() const { return m_col1; }
	inline int col2() const { return m_col2; }
	inline int get_rows ( int i ) const { return m_rows[i]; }

private:
	int alloc_data ();
	void free_data ();

	void recurse_swap_col ( mtTreeNode * colnode );
	void recurse_swap_row ( mtTreeNode * rownode );

/// ----------------------------------------------------------------------------

	int		m_col1		= 0;
	int		m_col2		= 0;
	int	const	* m_rows	= nullptr;
	int		m_active_cols	= 0;

	// Owned memory allocation pointers, so we use rule of 5
	int		* m_csorti	= nullptr;
	csortNODE	* m_csortn	= nullptr;
	mtTreeNode	** m_treenode_array = nullptr;	// m_active_cols items

	MTKIT_RULE_OF_FIVE ( ColSort )
};



namespace {
static int colsort_compare (
	void	const * const	a,
	void	const * const	b,
	void		* const	user
	)
{
	ColSort * const cs = static_cast<ColSort *>( user );

	csortNODE const	* const node1 = (csortNODE const *)a;
	csortNODE const	* const node2 = (csortNODE const *)b;

	CedCell * c1 = node1->cell;
	CedCell * c2 = node2->cell;

	for ( int i = 0; cs->get_rows( i ) > 0; i++ )
	{
		cs->set_mode_from_list ( i );

		int const res = cs->cmp_cells ( c1, c2 );
		if ( res )
		{
			return res;
		}

		c1 = ced_sheet_get_cell ( cs->sheet(), cs->get_rows( i + 1 ),
			node1->key + cs->col1() );
		c2 = ced_sheet_get_cell ( cs->sheet(), cs->get_rows( i + 1 ),
			node2->key + cs->col1() );
	}

	return 0;
}
} // namespace



int ColSort::sort (
	int		const	column,
	int		const	coltot,
	int	const * const	rows
	)
{
	if (	! sheet()	||
		! rows		||
		column < 1	||
		rows[0] == 0	||
		column > CED_MAX_COLUMN
		)
	{
		return 1;
	}

	if (	! sheet()->rows		||
		! sheet()->rows->root	||
		coltot == 1
		)
	{
		return 0;		// Nothing to do
	}

	m_col1 = column;
	m_col2 = column + coltot - 1;
	m_rows = rows;

	if ( coltot == 0 )
	{
		int c;

		if ( ced_sheet_get_geometry ( sheet(), nullptr, &c ) )
		{
			return 1;
		}

		m_active_cols = c - column + 1;
		m_col2 = 0;
	}
	else
	{
		m_active_cols = coltot;
	}

	if ( m_col2 > CED_MAX_COLUMN )
	{
		m_col2 = CED_MAX_COLUMN;
	}

	if ( m_active_cols < 1 )
	{
		return 0;			// Nothing to do
	}

	if ( alloc_data () )
	{
		return 1;
	}

	// Initialize current order in csort1 lookup table
	for ( int r = 0; r < m_active_cols; r++ )
	{
		m_csortn[r].key = r;
		m_csortn[r].cell = ced_sheet_get_cell ( sheet(), rows[0],
			r + column );
	}

	qsort_r ( m_csortn, (size_t)m_active_cols, sizeof ( csortNODE ),
		colsort_compare, this );

	// Create csort2 reverse lookup table (reference later with:
	// new_index = csorti[ old_index - col1 ];)
	for ( int r = 0; r < m_active_cols; r++ )
	{
		m_csorti[ m_csortn[r].key ] = r;
	}

	// Recursively traverse rows, and then col cells to switch cell keys
	// around
	recurse_swap_row ( sheet()->rows->root );

	return 0;
}

int ColSort::alloc_data ()
{
	if ( m_csortn || m_csorti || m_treenode_array )
	{
		free_data ();
	}

	m_csortn = (csortNODE *)calloc ( (size_t)m_active_cols,
		sizeof (csortNODE));
	m_csorti = (int *)calloc ( (size_t)m_active_cols, sizeof (int) );
	m_treenode_array = (mtTreeNode **)calloc ( (size_t)m_active_cols,
		sizeof (mtTreeNode *) );

	return (m_csortn && m_csorti && m_treenode_array) ? 0 : 1;
}

void ColSort::free_data ()
{
	free ( m_csortn );
	m_csortn = nullptr;

	free ( m_csorti );
	m_csorti = nullptr;

	free ( m_treenode_array );
	m_treenode_array = nullptr;
}

void ColSort::recurse_swap_col ( mtTreeNode * const colnode )
{
	int	const	c = (int)(intptr_t)colnode->key;


	if ( colnode->left && c > m_col1 )
	{
		recurse_swap_col ( colnode->left );
	}

	if (	c >= m_col1 &&
		(m_col2 == 0 || c <= m_col2)
		)
	{
		m_treenode_array[ c - m_col1 ] = colnode;
		m_csortn[ m_csorti[c - m_col1] ].cell =
			(CedCell *)(colnode->data);
	}

	if (	colnode->right &&
		(m_col2 == 0 || c < m_col2)
		)
	{
		recurse_swap_col ( colnode->right );
	}
}

void ColSort::recurse_swap_row ( mtTreeNode * const rownode )
{
	if ( rownode->left )
	{
		recurse_swap_row ( rownode->left );
	}

	memset ( m_treenode_array, 0,
		(unsigned)m_active_cols * sizeof (mtTreeNode *) );

	memset ( m_csortn, 0,
		(unsigned)m_active_cols * sizeof ( csortNODE ) );

	recurse_swap_col ( ( (mtTree *)rownode->data )->root );

	for ( int i = 0, j = 0; i < m_active_cols; i++ )
	{
		// Set cell + key
		if ( m_treenode_array[i] )
		{
			while ( ! m_csortn[j].cell )
			{
				j++;
			}

			m_treenode_array[i]->key =
				(void *)(intptr_t)(m_col1 + j);
			m_treenode_array[i]->data =
				(void *)(m_csortn[j].cell);

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
	ColSort cs ( sheet, mode, mode_list );

	return cs.sort ( column, coltot, rows );
}

