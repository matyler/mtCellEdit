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



static int tree_cmp_val (
	void	const * const	k1,
	void	const * const	k2
	)
{
	CedCell	const * const	a = (CedCell const *)k1;
	CedCell	const * const	b = (CedCell const *)k2;


	if ( a->value < b->value )
	{
		return -1;
	}

	if ( a->value > b->value )
	{
		return 1;
	}

	return 0;
}

static void tree_del_item (
	mtTreeNode	* const	node
	)
{
	free ( node->data );
}

static int tree_cmp_text (
	void	const * const	k1,
	void	const * const	k2
	)
{
	CedCell	const * const	a = (CedCell const *)k1;
	CedCell	const * const	b = (CedCell const *)k2;


	return strcmp ( a->text, b->text );
}

CedIndex * ced_index_new (
	int		const	type
	)
{
	CedIndex	* index;
	mtTree		* tree;


	switch ( type )
	{
	case CED_INDEX_TYPE_VALUE:
		tree = mtkit_tree_new ( tree_cmp_val, tree_del_item );
		break;

	case CED_INDEX_TYPE_TEXT:
		tree = mtkit_tree_new ( tree_cmp_text, tree_del_item );
		break;

	default:
		return NULL;
	}

	if ( ! tree )
	{
		return NULL;
	}

	index = (CedIndex *)calloc ( sizeof ( CedIndex ), 1 );
	if ( ! index )
	{
		mtkit_tree_destroy ( tree );

		return NULL;
	}

	index->type = type;
	index->tree = tree;

	return index;
}

static int index_cell (
	CedSheet	* ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	CedIndex	* const	index = (CedIndex *)user_data;
	CedIndexItem	*	item;


	if ( ! cell->text )
	{
		return 0;		// Nothing to index
	}

	item = (CedIndexItem *)calloc ( sizeof ( CedIndexItem ), 1 );
	if ( ! item )
	{
		return 1;
	}

	item->row = row;
	item->col = col;

	if ( ! mtkit_tree_node_add ( index->tree, (void *)cell, (void *)item ) )
	{
		free ( item );

		return 1;
	}

	return 0;
}

int ced_index_add_items (
	CedIndex	* const	index,
	CedSheet	* const	sheet,
	int		const	row,
	int		const	col,
	int		const	rowtot,
	int		const	coltot
	)
{
	if (	! index ||
		! sheet ||
		row < 1 ||
		col < 1
		)
	{
		return 1;
	}

	return ced_sheet_scan_area ( sheet, row, col, rowtot, coltot,
		index_cell, (void *)index );
}

int ced_index_destroy (
	CedIndex	* const	index
	)
{
	if ( ! index )
	{
		return 1;
	}

	mtkit_tree_destroy ( index->tree );
	free ( index );

	return 0;
}

int ced_index_query (
	CedIndex	*	const	index,
	double			const	value,
	char		const	* const	text,
	CedIndexItem	*	* const	item
	)
{
	CedCell		* cell;
	mtTreeNode	* node;


	if ( ! index || ! item )
	{
		return -1;		// Error
	}

	cell = ced_cell_new ();
	if ( ! cell )
	{
		return -1;		// Error
	}

	cell->value = value;

	if ( ! text )
	{
		if ( index->type == CED_INDEX_TYPE_TEXT )
		{
			// Error - a text index requires a text query
			ced_cell_destroy ( cell );
			return -1;
		}

		mtkit_strfreedup ( &cell->text, "" );
	}
	else
	{
		if ( index->type == CED_INDEX_TYPE_VALUE )
		{
			// Error - a value index requires a value query
			ced_cell_destroy ( cell );
			return -1;
		}

		mtkit_strfreedup ( &cell->text, text );
	}

	if ( ! cell->text )
	{
		ced_cell_destroy ( cell );
		return -1;
	}

	node = mtkit_tree_node_find ( index->tree, (void *) cell );

	ced_cell_destroy ( cell );
	cell = NULL;

	if ( ! node )
	{
		return 0;		// Not found
	}

	item[0] = (CedIndexItem *)node->data;

	return 1;			// Found
}

