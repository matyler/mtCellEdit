/*
	Copyright (C) 2016 Mark Tyler

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
	// Key not destroyed here: is a reference to data in (CliTabItem *)
	delete ( (mtKit::CliItem *)node->data );
}

mtKit::CliItem::CliItem ()
	:
	m_key		(),
	m_func		(),
	m_arg_min	( 0 ),
	m_arg_max	( 0 ),
	m_arg_help	(),
	m_arg_scale	( 1 )
{
	m_tree = mtkit_tree_new ( tab_cmp, tab_del );
}

mtKit::CliItem::~CliItem ()
{
	mtkit_tree_destroy ( m_tree );
	m_tree = NULL;

	free ( m_key );
	m_key = NULL;

	free ( m_arg_help );
	m_arg_help = NULL;
}

int mtKit::CliItem::add_item (
	CliItem		* item
	)
{
	if ( 0 == mtkit_tree_node_add ( m_tree, (void *)item->m_key,
		(void *)item ) )
	{
		return 1;
	}

	return 0;
}

int mtKit::CliItem::set_data (
	char	const * const	key,
	CliFunc		const	func,
	int		const	arg_min,
	int		const	arg_max,
	char	const * const	arg_help,
	int		const	arg_scale
	)
{
	if ( arg_min < 0 || arg_max < arg_min || arg_scale < 1 )
	{
		return 1;
	}

	if ( key && mtkit_strfreedup ( &m_key, key ) )
	{
		return 1;
	}

	if ( func )
	{
		m_func = func;
	}

	m_arg_min = arg_min;
	m_arg_max = arg_max;
	m_arg_scale = arg_scale;

	if ( arg_help && mtkit_strfreedup ( &m_arg_help, arg_help ) )
	{
		return 1;
	}

	return 0;
}

mtKit::CliItem * mtKit::CliItem::find_item (
	char	const * const	key
	) const
{
	mtTreeNode * node = mtkit_tree_node_find ( m_tree, (void const *)key );
	if ( node )
	{
		return (CliItem *)node->data;
	}

	return NULL;
}

mtKit::CliItem const * mtKit::CliItem::match_args (
	char	** const	argv,
	int	* const		cli_error,
	int	* const		ncargs
	) const
{
	if ( ! argv )
	{
		return NULL;
	}

	int		i, errnum = -3;
	CliItem	const	* match = this;

	for ( i = 0; argv[i]; i++ )
	{
		match = match->find_item ( argv[i] );
		if ( ! match )
		{
			// Unmatched command
			errnum = i;

			goto finish;
		}

		if ( ! match->m_tree->root )
		{
			// No more commands in this line so rest must be args
			// (not commands)

			i++;
			break;
		}
	}

	if ( match == this || ! match->m_func )
	{
		// Not enough commands were matched to reach a valid
		// item

		errnum = -1;
		goto finish;
	}

	if ( ncargs )
	{
		ncargs[0] = i;
	}

	int		st;

	// Count args remaining
	for ( st = i; argv[st]; st ++ )
	{
	}

	st = st - i;

	if (	st < ( match->m_arg_min * match->m_arg_scale )	||
		0 != (st % match->m_arg_scale )
		)
	{
		// Too few args
		errnum = -1;
	}
	else if ( st > ( match->m_arg_max * match->m_arg_scale ) )
	{
		// Too many args
		errnum = -2;
	}
	else
	{
		errnum = 0;
	}

finish:
	if ( cli_error )
	{
		cli_error[0] = errnum;
	}

	return match;
}

int mtKit::CliItem::callback (
	char	** const argv
	) const
{
	if ( ! argv || ! m_func )
	{
		return 1;
	}

	return m_func ( argv );
}

int mtKit::CliItem::print_help_item () const
{
	char	const	* info = " ... ";

	if ( m_func )
	{
		if ( m_arg_help )
		{
			info = m_arg_help;
		}
		else
		{
			// Do sub-functions also exist?

			if ( m_tree->root )
			{
				info = "[...]";
			}
			else
			{
				info = "";
			}
		}
	}

	printf ( "%-14s %s\n", m_key, info );

	return 0;
}

static int cb_help (
	mtTreeNode	* const	node,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	mtKit::CliItem	const	* citem = (mtKit::CliItem const *)node->data;

	if ( citem )
	{
		return citem->print_help_item ();
	}

	return 0;			// Continue
}

int mtKit::CliItem::print_help () const
{
	printf ( "\n" );
	mtkit_tree_scan ( m_tree, cb_help, NULL, 0 );
	printf ( "\n" );

	return 0;
}

