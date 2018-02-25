/*
	Copyright (C) 2009-2017 Mark Tyler

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



static int compare_func (
	void	const * const	k1,
	void	const * const	k2
	)
{
	return strcmp ( (char const *)k1, (char const *)k2 );
}

static void delete_func (
	mtTreeNode	* const	node
	)
{
	free ( node->key );
}

int mtkit_arg_parse (
	int			const	argc,
	char	const *	const * const	argv,
	mtArg		const *	const	arg_list,
	mtArgFileFunc		const	file_func,
	mtArgErrorFunc		const	error_func,
	void		*	const	user_data
	)
{
	mtTree		* tree;
	mtTreeNode	* node;
	char	const	** cp;
	char		* st;
	int		i,
			j,
			err,
			* ip;
	double		* fp;


/*
NOTE - I am using a tree to hold all of the arguments available.  This is
slightly slower than a sequential search for small sets of arguments, or small
arg_lists's. However in the situation when I use many switches and/or many
arguments it avoids the potentially huge slowdown that a sequential search would
cost.

I can live with many tiny slowdowns, but not the occasional huge one, ergo the
tree is the best choice.
*/

	tree = mtkit_tree_new ( compare_func, delete_func );
	if ( ! tree )
	{
		goto error;
	}

	for ( j = 0; arg_list[j].argument; j++ )
	{
		st = strdup ( arg_list[j].argument );
		if ( ! st )
		{
			goto error;
		}

		if ( ! mtkit_tree_node_add ( tree, st, (void *)(intptr_t)j ) )
		{
			free ( st );

			goto error;
		}
	}

	for ( i = 1; i < argc; i++ )	// Parse each argument
	{
		if ( argv[i][0] != '-' )
		{
			// This argument is not a switch so it must be an input
			// file

			if ( file_func )
			{
				if ( file_func ( argv[i], user_data ) )
				{
					goto callback_terminate;
				}
			}

			continue;
		}

		// Switch found starting with '-'

		node = mtkit_tree_node_find ( tree, argv[i] + 1 );

		if ( ! node )
		{
			// No such node found so argument hasn't been defined
			// in arg_list

			if ( error_func )
			{
				if ( error_func ( MTKIT_ARG_ERROR_UNKNOWN, i,
					argc, argv, user_data ) )
				{
					goto callback_terminate;
				}
			}

			continue;
		}

		j = (int)(intptr_t)node->data;

		if (	! arg_list[j].variable			||
			arg_list[j].type <= MTKIT_ARG_NONE	||
			arg_list[j].type >= MTKIT_ARG_TOTAL
			)
		{
			// Error in arg list - bad type or no variable

			if ( error_func )
			{
				if ( error_func ( MTKIT_ARG_ERROR_BAD_TYPE, i,
					argc, argv, user_data ) )
				{
					goto callback_terminate;
				}
			}

			continue;
		}

		if ( arg_list[j].type == MTKIT_ARG_SWITCH )
		{
			ip = (int *) arg_list[j].variable;
			ip[0] = arg_list[j].value;

			if ( arg_list[j].callback &&
				arg_list[j].callback ( &arg_list[j], i, argc,
					argv, user_data )
				)
			{
				goto callback_terminate;
			}

			continue;
		}

		i++;

		if ( i >= argc )	// Ensure another arg exists
		{
			if ( error_func )
			{
				if ( error_func ( MTKIT_ARG_ERROR_TERMINATED,
					i, argc, argv, user_data ) )
				{
					goto callback_terminate;
				}
			}

			break;		// Stop here
		}

		err = 0;

		switch ( arg_list[j].type )
		{
		case MTKIT_ARG_INT:
			ip = (int *) arg_list[j].variable;
			err = mtkit_strtoi ( argv[i], ip, NULL, 1 );
			break;

		case MTKIT_ARG_STRING:
			cp = (char const **) arg_list[j].variable;
			cp[0] = argv[i];
			break;

		case MTKIT_ARG_DOUBLE:
			fp = (double *) arg_list[j].variable;
			err = mtkit_strtod ( argv[i], fp, NULL, 1 );
			break;

		default:
			break;		// Should never happen
		}

		if ( err )
		{
			if (	error_func &&
				error_func ( MTKIT_ARG_ERROR_DATA, i, argc,
					argv, user_data )
				)
			{
				goto callback_terminate;
			}
		}
		else
		{
			if (	arg_list[j].callback &&
				arg_list[j].callback ( &arg_list[j], i, argc,
					argv, user_data )
				)
			{
				goto callback_terminate;
			}
		}
	}

	mtkit_tree_destroy ( tree );
	return 0;

error:
	mtkit_tree_destroy ( tree );
	return 1;

callback_terminate:
	mtkit_tree_destroy ( tree );
	return 2;
}

int mtkit_arg_int_boundary_check (
	char	const * const	arg_name,
	int		const	arg_val,
	int		const	min,
	int		const	max
	)
{
	if ( ! arg_name || min > max )
	{
		return -1;		// Invalid argument
	}

	if (	arg_val < min ||
		arg_val > max
		)
	{
		fprintf ( stderr, "Argument '%s' (%i) out of bounds [%i,%i]\n",
			arg_name, arg_val, min, max );

		return 1;		// Invalid
	}

	return 0;			// Valid
}

/*
int mtkit_arg_double_boundary_check (
	char	const * const	arg_name,
	double		const	arg_val,
	double		const	min,
	double		const	max
	)
{
	if ( ! arg_name || min > max )
	{
		return -1;		// Invalid argument
	}

	if (	arg_val < min ||
		arg_val > max
		)
	{
		fprintf ( stderr, "Argument '%s' (%f) out of bounds [%f,%f]\n",
			arg_name, arg_val, min, max );

		return 1;		// Invalid
	}

	return 0;			// Valid
}
*/

int mtkit_arg_string_boundary_check (
	char	const * const	arg_name,
	char	const * const	arg_val,
	int		const	min,
	int		const	max
	)
{
	if ( ! arg_name || ! arg_val )
	{
		return -1;		// Invalid argument
	}

	if ( min >= 0 || max >= 0 )
	{
		size_t		len = strlen ( arg_val );


		if (	(min >= 0 && len < (size_t)min)	||
			(max >= 0 && len > (size_t)max)
			)
		{
			fprintf ( stderr,
				"Argument '%s' length out of bounds [%i,%i]\n",
				arg_name, min, max );

			return 1;	// Invalid
		}
	}

	return 0;			// Valid
}

