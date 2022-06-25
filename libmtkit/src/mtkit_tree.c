/*
	Copyright (C) 2008-2021 Mark Tyler

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

/*
This implements a standard AVL tree structure.
Much of the code inspiration comes from GLIB 2.2:

Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
* Modified by the GLib Team and others 1997-2000.  See the AUTHORS
* file for a list of people on the GLib Team.  See the ChangeLog
* files for a list of changes.  These files are distributed with
* GLib at ftp://ftp.gtk.org/pub/gtk/.
*/


mtTree * mtkit_tree_new (
	mtTreeFuncCmp	const	cmp_func,
	mtTreeFuncDel	const	del_func
	)
{
	if ( ! cmp_func )
	{
		return NULL;
	}

	mtTree * const tree = (mtTree *)( calloc ( sizeof(mtTree), 1 ) );

	if ( tree )
	{
		tree->cmp = cmp_func;
		tree->del = del_func;
	}

	return tree;
}


static void mtkit_tree_destroy_recurse (
	mtTree		* const	tree,
	mtTreeNode	* const	node
	)
{
	if ( ! node )
	{
		return;
	}

	mtkit_tree_destroy_recurse ( tree, node->left );
	node->left = NULL;
	mtkit_tree_destroy_recurse ( tree, node->right );
	node->right = NULL;

	if ( tree->del ) tree->del ( node );

	free ( node );
}

int mtkit_tree_destroy (
	mtTree		* const	tree
	)
{
	if ( ! tree )
	{
		return 1;
	}

	mtkit_tree_destroy_recurse ( tree, tree->root );
	free ( tree );

	return 0;
}

static mtTreeNode * tree_node_new (
	void		* const	key,
	void		* const	data
	)
{
	mtTreeNode * const node = (mtTreeNode *)(
		calloc ( sizeof ( mtTreeNode ), 1 ));

	if ( node )
	{
		node->key = key;
		node->data = data;
	}

	return node;
}

static mtTreeNode * tree_node_rotate_left (
	mtTreeNode	* const	node
	)
{
	mtTreeNode	* right;
	int		a_bal;
	int		b_bal;


	right = node->right;

	node->right = right->left;
	right->left = node;

	a_bal = node->balance;
	b_bal = right->balance;

	if ( b_bal <= 0 )
	{
		if ( a_bal >= 1 )
		{
			right->balance = b_bal - 1;
		}
		else
		{
			right->balance = a_bal + b_bal - 2;
		}

		node->balance = a_bal - 1;
	}
	else
	{
		if ( a_bal <= b_bal )
		{
			right->balance = a_bal - 2;
		}
		else
		{
			right->balance = b_bal - 1;
		}

		node->balance = a_bal - b_bal - 1;
	}

	return right;
}

static mtTreeNode * tree_node_rotate_right (
	mtTreeNode	* const	node
	)
{
	mtTreeNode	* left;
	int		a_bal;
	int		b_bal;


	left = node->left;

	node->left = left->right;
	left->right = node;

	a_bal = node->balance;
	b_bal = left->balance;

	if ( b_bal <= 0 )
	{
		if ( b_bal > a_bal )
		{
			left->balance = b_bal + 1;
		}
		else
		{
			left->balance = a_bal + 2;
		}

		node->balance = a_bal - b_bal + 1;
	}
	else
	{
		if ( a_bal <= -1 )
		{
			left->balance = b_bal + 1;
		}
		else
		{
			left->balance = a_bal + b_bal + 2;
		}

		node->balance = a_bal + 1;
	}

	return left;
}

static mtTreeNode * tree_node_balance (
	mtTreeNode	* const	node
	)
{
	if ( node->balance < -1 )
	{
		if ( node->left->balance > 0 )
		{
			node->left = tree_node_rotate_left ( node->left );
		}

		return tree_node_rotate_right ( node );
	}
	else if ( node->balance > 1 )
	{
		if ( node->right->balance < 0 )
		{
			node->right = tree_node_rotate_right ( node->right );
		}

		return tree_node_rotate_left ( node );
	}

	return node;
}

static mtTreeNode * tree_node_insert (
	mtTree		* const	tree,
	mtTreeNode	* const	node,
	void		* const	key,
	void		* const	data,
	int		* const	inserted
	)
{
	int		compare,
			old_balance;


	if ( ! node )
	{
		inserted[0] = 1;

		return tree_node_new ( key, data );
	}

	compare = tree->cmp ( key, node->key );

	if ( compare == 0 )
	{
		inserted[0] = 2;

		if ( tree->del )
		{
			tree->del ( node );
		}

		node->key = key;
		node->data = data;

		return node;
	}
	else if ( compare < 0 )
	{
		if ( node->left )
		{
			old_balance = node->left->balance;
			node->left = tree_node_insert ( tree, node->left, key,
				data, inserted );

			if (	old_balance != node->left->balance &&
				node->left->balance
				)
			{
				node->balance --;
			}
		}
		else
		{
			inserted[0] = 1;
			node->left = tree_node_new ( key, data );
			node->balance --;
		}
	}
	else /*if ( compare > 0 ) NOTE: always true as per 2 tests before */
	{
		if ( node->right )
		{
			old_balance = node->right->balance;
			node->right = tree_node_insert ( tree, node->right, key,
				data, inserted );

			if (	old_balance != node->right->balance &&
				node->right->balance
				)
			{
				node->balance += 1;
			}
		}
		else
		{
			inserted[0] = 1;
			node->right = tree_node_new ( key, data );
			node->balance ++;
		}
	}

	if ( inserted[0] == 1 )
	{
		if (	node->balance < -1 ||
			node->balance > 1
			)
		{
			return tree_node_balance ( node );
		}
	}

	return node;
}

int mtkit_tree_node_add (
	mtTree		* const	tree,
	void		* const	key,
	void		* const	data
	)
{
	int		inserted = 0;


	if ( tree )
	{
		tree->root = tree_node_insert ( tree, tree->root, key, data,
			&inserted );
	}

	return inserted;
}

mtTreeNode * mtkit_tree_node_find (
	mtTree		* const	tree,
	void	const	* const	key
	)
{
	if ( ! tree )
	{
		return NULL;
	}

	mtTreeNode * node = tree->root;

	while ( node )
	{
		int const compare = tree->cmp ( key, node->key );

		if ( compare == 0 )
		{
			break;
		}

		if ( compare < 0 )
		{
			node = node->left;
		}

		if ( compare > 0 )
		{
			node = node->right;
		}
	}

	return node;
}

static mtTreeNode * tree_node_restore_left_balance (
	mtTreeNode	* const	node,
	int		const	old_balance
	)
{
	if ( ! node->left )
	{
		node->balance += 1;
	}
	else if (	node->left->balance != old_balance &&
			node->left->balance == 0
		)
	{
		node->balance += 1;
	}

	if ( node->balance > 1 )
	{
		return tree_node_balance ( node );
	}

	return node;
}

static mtTreeNode * tree_node_remove_leftmost (
	mtTreeNode	* const		node,
	mtTreeNode	** const	leftmost
	)
{
	int		old_balance;


	if ( ! node->left )
	{
		leftmost[0] = node;

		return node->right;
	}

	old_balance = node->left->balance;
	node->left = tree_node_remove_leftmost ( node->left, leftmost );

	return tree_node_restore_left_balance ( node, old_balance );
}

static mtTreeNode * tree_node_restore_right_balance (
	mtTreeNode	* const	node,
	int		const	old_balance
	)
{
	if ( ! node->right )
	{
		node->balance -= 1;
	}
	else if (	node->right->balance != old_balance &&
			node->right->balance == 0
		)
	{
		node->balance -= 1;
	}

	if ( node->balance < -1 )
	{
		return tree_node_balance ( node );
	}

	return node;
}

static mtTreeNode * tree_node_remove (
	mtTree			* const	tree,
	mtTreeNode		*	node,
	void		const	* const	key,
	int			* const	removed
	)
{
	mtTreeNode	* new_root;
	int		old_balance;
	int		cmp;


	if ( ! node )
	{
		return NULL;
	}

	cmp = tree->cmp ( key, node->key );
	if ( cmp == 0 )
	{
		mtTreeNode	* garbage = node;


		removed[0] = 1;

		if ( ! node->right )
		{
			node = node->left;
		}
		else
		{
			old_balance = node->right->balance;
			node->right = tree_node_remove_leftmost ( node->right,
				&new_root );
			new_root->left = node->left;
			new_root->right = node->right;
			new_root->balance = node->balance;
			node = tree_node_restore_right_balance ( new_root,
				old_balance );
		}

		if ( tree->del )
		{
			tree->del ( garbage );
		}

		free ( garbage );
	}
	else if ( cmp < 0 )
	{
		if ( node->left )
		{
			old_balance = node->left->balance;
			node->left = tree_node_remove ( tree, node->left, key,
				removed );
			node = tree_node_restore_left_balance ( node,
				old_balance );
		}
	}
	else /*if ( cmp > 0 ) NOTE: always true as per 2 tests before */
	{
		if ( node->right )
		{
			old_balance = node->right->balance;
			node->right = tree_node_remove ( tree, node->right,
				key, removed );
			node = tree_node_restore_right_balance ( node,
				old_balance );
		}
	}

	return node;
}

int mtkit_tree_node_remove (
	mtTree			* const	tree,
	void		const	* const	key
	)
{
	int		removed = 0;


	if ( ! tree )
	{
		return 0;
	}

	tree->root = tree_node_remove ( tree, tree->root, key, &removed );

	return removed;
}

