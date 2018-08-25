/*
	Copyright (C) 2009-2018 Mark Tyler

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
	mtTree		* const	newtree;
	mtTreeFuncDup	const	callback;
} dupSTATE;



static int scan_cb_dup (
	mtTreeNode	* const	node,
	void		* const	user_data
	)
{
	dupSTATE const * const state = user_data;
	mtTreeNode newnode = { NULL };

	if ( state->callback ( node, &newnode ) )
	{
		return 1;		// Stop, error
	}

	if ( 0 == mtkit_tree_node_add ( state->newtree, newnode.key,
		newnode.data ) )
	{
		if ( state->newtree->del )
		{
			state->newtree->del ( &newnode );
		}

		return 1;		// Stop, error
	}

	return 0;			// Continue
}

mtTree * mtkit_tree_duplicate (
	mtTree		* const	tree,
	mtTreeFuncDup	const	duplicate
	)
{
	if ( ! tree || ! duplicate )
	{
		return NULL;
	}

	mtTree * const newtree = mtkit_tree_new ( tree->cmp, tree->del );
	if ( ! newtree )
	{
		return NULL;
	}

	dupSTATE state = { newtree, duplicate };

	if ( mtkit_tree_scan ( tree, scan_cb_dup, &state, 0 ) )
	{
		mtkit_tree_destroy ( newtree );

		return NULL;
	}

	return newtree;
}

