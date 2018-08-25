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
	mtTreeFuncScan	callback;
	void		* user_data;
	int		error;
} scanSTATE;



static int tree_scan_recurse_forward (
	mtTreeNode	* const	node,
	scanSTATE	* const	state
	)
{
	if ( node->left )
	{
		state->error = tree_scan_recurse_forward ( node->left, state );
		if ( state->error )
		{
			return state->error;
		}
	}

	state->error = state->callback ( node, state->user_data );
	if ( state->error )
	{
		return 2;
	}

	if ( node->right )
	{
		tree_scan_recurse_forward ( node->right, state );
		if ( state->error )
		{
			return state->error;
		}
	}

	return 0;
}

static int tree_scan_recurse_backward (
	mtTreeNode	* const	node,
	scanSTATE	* const	state
	)
{
	if ( node->right )
	{
		tree_scan_recurse_backward ( node->right, state );
		if ( state->error )
		{
			return state->error;
		}
	}

	state->error = state->callback ( node, state->user_data );
	if ( state->error )
	{
		return 2;
	}

	if ( node->left )
	{
		state->error = tree_scan_recurse_backward ( node->left, state );
		if ( state->error )
		{
			return state->error;
		}
	}

	return 0;
}

int mtkit_tree_scan (
	mtTree		* const	tree,
	mtTreeFuncScan	const	callback,
	void		* const	user_data,
	int		const	direction
	)
{
	scanSTATE	state = { callback, user_data, 0 };


	if ( ! tree || ! callback )
	{
		return 1;
	}

	if ( ! tree->root )
	{
		return 0;		// Nothing to do
	}

	if ( direction == 1 )
	{
		return tree_scan_recurse_backward ( tree->root, &state );
	}

	return tree_scan_recurse_forward ( tree->root, &state );
}

