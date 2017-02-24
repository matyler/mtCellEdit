/*
	Copyright (C) 2015 Mark Tyler

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

#include "mtgentex.h"



typedef struct
{
	int		tot;
	int		totmin;
	int		totmax;
} nodeTot;



static void print_node_error (
	mtUtreeNode	* const	node,
	char	const * const	errtext
	)
{
	fprintf ( stderr, "Error %s - node '%s'\n", errtext, node->text );
}

static int nodetot_populate (
	nodeTot		* const nt,
	mtUtreeNode	* const	node
	)
{
	if ( ! nt || ! node )
	{
		return 1;
	}

	nt->tot = 1;
	nt->totmin = 0;
	nt->totmax = 0;

	mtBulkInt bulki[] = {
		{ "tot",	&nt->tot	},
		{ "totmin",	&nt->totmin	},
		{ "totmax",	&nt->totmax	},
		{ NULL, NULL }
		};

	if ( mtkit_utree_bulk_get ( node, bulki, NULL, NULL ) )
	{
		print_node_error ( node, "getting tot's" );
		return 1;
	}

	if (	nt->totmin > 0 &&
		nt->totmax > 0 &&
		nt->totmin < nt->totmax
		)
	{
		nt->tot = nt->totmin + ( rand() % (nt->totmax - nt->totmin ) );
	}

	return 0;
}

static int output_int (
	mtUtreeNode	* const	node
	)
{
	int		min = -1000;
	int		max = 1000;


	if (	mtkit_utree_get_attribute_int ( node, "min", &min ) > 0 ||
		mtkit_utree_get_attribute_int ( node, "max", &max ) > 0
		)
	{
		print_node_error ( node, "getting int's min/max" );
		return 1;
	}

	if ( min > max )
	{
		print_node_error ( node, "(min > max)" );
		return 1;
	}

	// Promote to double to check within bounds
	double const drr = (double)max - (double)min;

	if ( drr > RAND_MAX )
	{
		print_node_error ( node, "(max - min) > RAND_MAX" );
		return 1;
	}

	// We now know this won't overflow
	int	const	irr = (int)drr;

	int		a;

	if ( irr > 0 )
	{
		a = min + ( rand() % irr );
	}
	else
	{
		a = min;
	}

	printf ( "%i", a );

	return 0;
}

static int output_double (
	mtUtreeNode	* const	node
	)
{
	double		min = -1000;
	double		max = 1000;


	if (	mtkit_utree_get_attribute_double ( node, "min", &min ) > 0 ||
		mtkit_utree_get_attribute_double ( node, "max", &max ) > 0
		)
	{
		print_node_error ( node, "getting double's min/max" );
	}

	if ( min > max )
	{
		print_node_error ( node, "(min > max)" );
	}

	double	const	lim = DBL_MAX / 2;

	if (	min < -lim || min > lim ||
		max < -lim || max > lim
		)
	{
		print_node_error ( node, "max/min out of bounds" );
		return 1;
	}

	// We now know this won't overflow
	double	const	drr = (double)max - (double)min;

	// Percent of drr
	double		p;

	p = ( (double)( rand() % 1000001 ) ) / 1000000;

	printf ( "%.15g", min + p * drr );

	return 0;
}

static int output_char (
	mtUtreeNode	* const	node,
	int		const	tot
	)
{
	int		res = 1;
	char	const *	range= " !\"#$%&\'()*+,-./0123456789:;<=>?"
			"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			"`abcdefghijklmnopqrstuvwxyz{|}~";


	mtkit_utree_get_attribute_str ( node, "range", &range );

	size_t cht = strlen ( range );

	if ( cht > 1000 )
	{
		print_node_error ( node, "length too large" );
		return 1;
	}

	if ( cht < 1 )
	{
		print_node_error ( node, "length too small" );
		return 1;
	}

	int		ictot = (int)cht;
	int		* blen	= NULL;	// Byte Length for this character
	int		* boff	= NULL;	// Byte offset for this character
	int	const	utf8	= mtkit_utf8_string_legal (
					(unsigned char const *)range, 0 );
	int		i;

	if ( utf8 )
	{
		ictot = mtkit_utf8_len ( (unsigned char const *)range, 0 );

		if ( ictot < 1 || ictot > 1000 )
		{
			print_node_error ( node, "UTF8 error" );
			goto finish;
		}
	}
	else
	{
		// NOT a UTF-8 string - each byte is a character
		// ch_tot already set earlier
	}

	blen = (int *)calloc ( (size_t)ictot, sizeof ( *blen ) );
	boff = (int *)calloc ( (size_t)ictot, sizeof ( *boff ) );

	if ( ! blen || ! boff )
	{
		print_node_error ( node, "memory allocation failure" );
		goto finish;
	}

	if ( utf8 )
	{
		int		bo = 0;
		int		l;
		unsigned char const * tstr = (unsigned char const *)range;


		for ( i = 0; i < ictot; i++ )
		{
			l = mtkit_utf8_offset ( tstr, 1 );

			if ( l < 1 )
			{
				print_node_error ( node, "parsing range" );
				goto finish;
			}

			blen[i] = l;
			boff[i] = bo;

			tstr += l;
			bo += l;
		}
	}
	else
	{
		// NOT a UTF-8 string - each byte is a character

		for ( i = 0; i < ictot; i++ )
		{
			blen[i] = 1;
			boff[i] = i;
		}
	}

	int		r;
	int		j;

	for ( i = tot; i > 0; i-- )
	{
		r = rand () % ictot;

		for ( j = 0; j < blen[r]; j++ )
		{
			putchar ( range[ boff[r] + j ] );
		}
	}

	res = 0;		// Success

finish:
	free ( blen );
	free ( boff );

	return res;
}

static int output_newline (
	int		const	tot
	)
{
	int		i;


	for ( i = tot; i > 0; i-- )
	{
		putchar ( '\n' );
	}

	return 0;
}

static int action_node (
	mtUtreeNode	* const	node
	)
{
	if ( ! node->text )
	{
		return 0;
	}

	nodeTot nt;

	if ( nodetot_populate ( &nt, node ) )
	{
		return 1;
	}

	int res = 0;

	if ( 0 == strcmp ( node->text, "int" ) )
	{
		res = output_int ( node );
	}
	else if ( 0 == strcmp ( node->text, "double" ) )
	{
		res = output_double ( node );
	}
	else if ( 0 == strcmp ( node->text, "char" ) )
	{
		res = output_char ( node, nt.tot );

		if ( res == 0 )
		{
			res = -1;	// Success but stop further repeats
		}
	}
	else if ( 0 == strcmp ( node->text, "newline" ) )
	{
		res = output_newline ( nt.tot );

		if ( res == 0 )
		{
			res = -1;	// Success but stop further repeats
		}
	}

	return res;
}

int parse_node (
	mtUtreeNode	* const	node
	)
{
	if ( ! node )
	{
		return 0;
	}


	int		res = 1;
	int		i;
	mtUtreeNode	* n;
	nodeTot		nt;


	for ( n = node; n; n = n->next )
	{
		if ( nodetot_populate ( &nt, n ) )
		{
			goto finish;
		}

		for ( i = nt.tot; i > 0; i-- )
		{
			switch ( n->type )
			{
			case MTKIT_UTREE_NODE_TYPE_ROOT:
			case MTKIT_UTREE_NODE_TYPE_ELEMENT:

				switch ( action_node ( n ) )
				{
				case -1:	i = 0; break;
				case 1:		goto finish;

				// Fall through for success
				}

				if ( parse_node ( n->child ) )
				{
					goto finish;
				}

				break;

			case MTKIT_UTREE_NODE_TYPE_TEXT:

				if ( n->text )
				{
					printf ( "%s", n->text );
				}

				break;
			}
		}
	}

	res = 0;		// Success

finish:

	return res;
}

