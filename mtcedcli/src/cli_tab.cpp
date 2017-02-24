/*
	Copyright (C) 2012-2016 Mark Tyler

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



struct mtCliTab
{
	mtCliItem const	* item;		// Item in the main static list this tab
					// refers to.  In root tab this is the
					// first item in static array.
	mtTree		* tree;		// Next level commands:
					// key = name (char *)
					// data = (mtCliTab *)
};



static int tab_cmp (
	void	const * const	k1,
	void	const * const	k2
	)
{
	return strcmp ( (char const *)k1, (char const *)k2 );
}

static void tab_del (
	mtTreeNode	* const	node
	)
{
	free ( node->key );
	mtkit_cli_free ( (mtCliTab *)node->data );
}

mtCliTab * mtkit_clitab_new ( void )
{
	mtCliTab	* tab;


	tab = (mtCliTab *)calloc ( sizeof ( mtCliTab ), 1 );
	if ( ! tab )
	{
		return NULL;
	}

	tab->tree = mtkit_tree_new ( tab_cmp, tab_del );
	if ( ! tab->tree )
	{
		free ( tab );

		return NULL;
	}

	return tab;
}

mtCliTab * mtkit_cli_new (
	const mtCliItem	* const	list
	)
{
	int		i, tokn;
	char		* tc;
	mtCliTab	* tab, * tmptab, * newtab;
	mtTreeNode	* tnode;


	if ( ! list )
	{
		return NULL;
	}

	tab = mtkit_clitab_new ();
	if ( ! tab )
	{
		return NULL;
	}

	tab->item = list;

	for ( i = 0; list[i].command; i++ )
	{
		tmptab = tab;

		for ( tokn = 0; ; tokn ++ )
		{
			tc = mtkit_strtok ( list[i].command, " ", tokn );
			if ( ! tc )
			{
				// No more items in command list so place this
				// ref here

				if ( tmptab->item )
				{
					fprintf ( stderr, "mtkit_cli_new: %s"
						" already exists\n",
						list[i].command );

					goto fail;
				}

				tmptab->item = &list[i];
				break;
			}

			tnode = mtkit_tree_node_find ( tmptab->tree, tc );
			if ( tnode )
			{
				// Node found so drill down
				tmptab = (mtCliTab *)tnode->data;
				free ( tc );
			}
			else
			{
				// Node missing so create it
				newtab = mtkit_clitab_new ();
				if ( ! newtab )
				{
					free ( tc );

					goto fail;
				}

				if ( mtkit_tree_node_add ( tmptab->tree, tc,
					newtab ) == 0 )
				{
					mtkit_cli_free ( newtab );
					free ( tc );

					goto fail;
				}

				tmptab = newtab;
			}
		}
	}

	return tab;

fail:
	mtkit_cli_free ( tab );
	return NULL;
}

int mtkit_cli_free (
	mtCliTab	* const	table
	)
{
	if ( ! table )
	{
		return 1;
	}

	mtkit_tree_destroy ( table->tree );
	free ( table );

	return 0;
}

const mtCliItem * mtkit_cli_match (
	mtCliTab	* const	table,
	char		** const argv,
	int		* const	cli_error,
	int		* const	ncargs
	)
{
	int		i, errnum = -3;
	mtCliItem const	* match = NULL;
	mtCliTab	* tab = table;
	mtTreeNode	* tnode;


	if ( ! table || ! argv )
	{
		return NULL;
	}

	for ( i = 0; argv[i]; i++ )
	{
		tnode = mtkit_tree_node_find ( tab->tree, argv[i] );
		if ( ! tnode )
		{
			// Unmatched command
			errnum = i + 1;

			goto finish;
		}

		tab = (mtCliTab *)tnode->data;

		if ( ! tab->tree->root )
		{
			// No more commands in this line so rest must be args
			// (not commands)

			i++;
			break;
		}
	}

	if ( tab != table )
	{
		match = tab->item;
		if ( ! match )
		{
			// Not enough commands were matched to reach a valid
			// item

			errnum = -1;
		}
	}

	if ( match )
	{
		if ( ncargs )
		{
			ncargs[0] = i;
		}

		if ( match->func_args == -1 )
		{
			// 0 or more args valid
			errnum = 0;
		}
		else if ( match->func_args == -2 )
		{
			// 1 or more args valid
			if ( argv[i] )
			{
				errnum = 0;
			}
			else
			{
				// Not enough args found
				errnum = -1;
			}
		}
		else if (  match->func_args >= 0 )
		{
			int		st;


			// Exact args must remain
			for ( st = i; argv[st]; st ++ )
			{
			}

			st = st - i;

			if ( st == match->func_args )
			{
				errnum = 0;
			}
			else if ( st > match->func_args )
			{
				// Too many args
				errnum = -2;
			}
			else	// ( st < match->func_args )
			{
				// Too few args
				errnum = -1;
			}
		}
		else
		{
			// Only happens for bogus func_args value
			errnum = -3;
		}
	}

finish:
	if ( cli_error )
	{
		cli_error[0] = errnum;
	}

	return match;
}

static int cb_help (
	mtTreeNode	* const	node,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	char	const	* info = " ... ";
	mtCliTab	* ctab = (mtCliTab *)node->data;


	if ( ctab->item )		// Function is called here
	{
		if ( ctab->item->help_args )
		{
			info = ctab->item->help_args;
		}
		else
		{
			// Do sub-functions also exist?

			if ( ctab->tree->root )
			{
				info = "[...]";
			}
			else
			{
				info = "";
			}
		}
	}

	printf ( "%-10s %s\n", (char *)node->key, info );

	return 0;			// Continue
}

int mtkit_cli_help (
	mtCliTab	*	table,
	char		** const argv
	)
{
	int		i;
	mtTreeNode	* tnode;


	for ( i = 0; argv[i]; i++ )
	{
		tnode = mtkit_tree_node_find ( table->tree, argv[i] );
		if ( ! tnode )
		{
			fprintf ( stderr, "Unknown command: %s\n", argv[i] );

			return 1;
		}

		table = (mtCliTab *)tnode->data;
	}

	if ( table->tree->root )
	{
		printf ( "\n" );
		mtkit_tree_scan ( table->tree, cb_help, NULL, 0 );
		printf ( "\n" );
	}

	return 0;
}

