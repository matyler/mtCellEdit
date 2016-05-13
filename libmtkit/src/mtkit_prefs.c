/*
	Copyright (C) 2009-2015 Mark Tyler

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



struct mtPrefs
{
	mtTree		* tree;		// key = name (char *)
					// data = (mtPrefValue *)
	int		block_callbacks; // If set don't use callbacks
};



typedef enum		// Internal representation
{
	VTYPE_INT	= 1 << 0,
	VTYPE_DOUBLE	= 1 << 1,
	VTYPE_STR	= 1 << 2,

	VTYPE_ALL	= VTYPE_INT | VTYPE_DOUBLE | VTYPE_STR
} VType;



static void pvalue_destroy (
	mtPrefValue	* const	item
	)
{
	free ( item->value );

	free ( item->key );
	free ( item->def );
	free ( item->description );
	free ( item->opt );
	free ( item );
}



#define CHECK_DUP_STR( XXX ) if ( item->XXX ) \
	{ \
		new_item->XXX = strdup ( item->XXX ); \
		if ( ! new_item->XXX ) \
		{ \
			goto error; \
		} \
	}


static mtPrefValue * pvalue_duplicate (
	mtPrefTable	const * const	item
	)
{
	mtPrefValue	* new_item;


	new_item = calloc ( 1, sizeof ( mtPrefValue ) );
	if ( ! new_item )
	{
		return NULL;
	}

	new_item->type = item->type;
	new_item->callback = item->callback;
	new_item->callback_data = item->callback_data;
	new_item->callback_ptr = item->callback_ptr;

	CHECK_DUP_STR ( key )
	CHECK_DUP_STR ( def )
	CHECK_DUP_STR ( description )
	CHECK_DUP_STR ( opt )

	return new_item;

error:
	pvalue_destroy ( new_item );

	return NULL;
}

static int mtkit_prefs_key_compare (
	void	const * const	k1,
	void	const * const	k2
	)
{
	return strcmp ( k1, k2 );
}

static void mtkit_prefs_node_destroy (
	mtTreeNode	* const node
	)
{
	mtPrefValue	* item = (mtPrefValue *) node->data;


	pvalue_destroy ( item );
}

static mtPrefValue * mtkit_prefs_get_value (
	mtPrefs		*	const	prefs,
	char		const * const	key,
	VType			const	vtype
	)
{
	mtPrefValue	* piv;
	mtTreeNode	* node;


	if ( ! prefs || ! key )
	{
		return NULL;
	}

	node = mtkit_tree_node_find ( prefs->tree, key );
	if ( ! node )
	{
		return NULL;
	}

	piv = (mtPrefValue *)node->data;

	switch ( vtype )
	{
	case VTYPE_INT:
		if (	piv->type == MTKIT_PREF_TYPE_INT	||
			piv->type == MTKIT_PREF_TYPE_BOOL	||
			piv->type == MTKIT_PREF_TYPE_RGB	||
			piv->type == MTKIT_PREF_TYPE_OPTION
			)
		{
			break;
		}

		return NULL;		// Incompatible types

	case VTYPE_DOUBLE:
		if ( piv->type == MTKIT_PREF_TYPE_DOUBLE )
		{
			break;
		}

		return NULL;		// Incompatible types

	case VTYPE_STR:
		if (	piv->type == MTKIT_PREF_TYPE_STR	||
			piv->type == MTKIT_PREF_TYPE_STR_MULTI	||
			piv->type == MTKIT_PREF_TYPE_FILE	||
			piv->type == MTKIT_PREF_TYPE_DIR
			)
		{
			break;
		}

		return NULL;		// Incompatible types

	case VTYPE_ALL:
		break;

	default:
		return NULL;		// Invalid type passed
	}

	return piv;
}

static int mtkit_prefs_set_value (
	mtPrefs		*	const	prefs,
	char		const *	const	key,
	char		const *		value,
	VType			const	vtype,
	int			const	cb
	)
{
	mtPrefValue	* piv;
	char		* v;


	if ( ! prefs || ! key )
	{
		return 1;
	}

	// Check this key exists and is of a compatible type
	piv = mtkit_prefs_get_value ( prefs, key, vtype );
	if ( ! piv )
	{
		return 2;
	}

	if ( ! value )
	{
		value = piv->def;
	}

	if ( ! value )
	{
		value = "";
	}

	if (	vtype == VTYPE_STR &&
		piv->opt
		)
	{
		int		max_chars;


		if ( mtkit_strtoi ( piv->opt, &max_chars, NULL, 0 )
			|| max_chars < 1 )
		{
			return 2;
		}

		v = strdup ( value );

		if (	v &&
			strlen ( v ) > (unsigned)max_chars
			)
		{
			v[max_chars] = 0;
		}
	}
	else
	{
		v = strdup ( value );
	}

	if ( ! v )
	{
		return 3;
	}

	free ( piv->value );		// Lose old value
	piv->value = v;

	if (	cb				&&
		! prefs->block_callbacks	&&
		piv->callback
		)
	{
		piv->callback ( piv, piv->callback_data, piv->callback_ptr );
	}

	return 0;
}

mtPrefs * mtkit_prefs_new (
	mtPrefTable	const * const	table
	)
{
	mtPrefs		* prefs;


	prefs = calloc ( sizeof ( mtPrefs ), 1 );
	if ( ! prefs )
	{
		return NULL;
	}

	prefs->tree = mtkit_tree_new ( mtkit_prefs_key_compare,
		mtkit_prefs_node_destroy );

	if ( ! prefs->tree )
	{
		goto error;
	}

	if ( table && mtkit_prefs_add ( prefs, table, NULL ) )
	{
		goto error;
	}

	return prefs;

error:
	mtkit_prefs_destroy ( prefs );

	return NULL;
}

int mtkit_prefs_add (
	mtPrefs		*	const	prefs,
	mtPrefTable	const * const	table,
	char		const * const	prefix
	)
{
	const mtPrefTable * item;
	mtPrefValue	* data;
	size_t		lenprefix = 0;
	char		* new_key;


	if ( ! prefs || ! table )
	{
		return 1;
	}

	if ( prefix && prefix[0] )
	{
		lenprefix = strlen ( prefix );
	}

	// Populate the tree from the table
	for ( item = table; item->key; item ++ )
	{
		data = pvalue_duplicate ( item );
		if ( ! data )
		{
			return 1;
		}

		if ( ! item->def )
		{
			data->value = strdup ( "" );
		}
		else
		{
			data->value = strdup ( item->def );
		}

		if ( ! data->value )
		{
			pvalue_destroy ( data );

			return 1;
		}

		if ( lenprefix > 0 )
		{
			size_t		len,
					lenkey;


			lenkey = strlen ( data->key );

			len = lenkey + lenprefix + 1;
			new_key = calloc ( len, 1 );

			if ( ! new_key )
			{
				pvalue_destroy ( data );

				return 1;
			}

			mtkit_strnncpy ( new_key, prefix, len );
			mtkit_strnncat ( new_key, data->key, len );
			free ( data->key );
			data->key = new_key;
		}

		if ( ! mtkit_tree_node_add ( prefs->tree, data->key, data ) )
		{
			pvalue_destroy ( data );

			return 1;
		}
	}

	return 0;
}

int mtkit_prefs_load (
	mtPrefs		*	const	prefs,
	char		const * const	filename
	)
{
	int		errors = 0;
	mtUtreeNode	* root,
			* node;


	if ( ! prefs || ! filename )
	{
		return 1;
	}

	root = mtkit_utree_load_file ( NULL, filename, &errors, 0 );
	if ( ! root )
	{
		return errors;
	}

	for ( node = root->child; node; node = node->next )
	{
		if (	node->type == MTKIT_UTREE_NODE_TYPE_ELEMENT	&&
			node->child					&&
			node->child->type == MTKIT_UTREE_NODE_TYPE_TEXT
			)
		{
			mtkit_prefs_set_value ( prefs, node->text,
				node->child->text, VTYPE_ALL, 0 );
		}
	}

	mtkit_utree_destroy_node ( root );

	return 0;
}

static int mtkit_prefs_save_recurse (
	mtUtreeNode	* const root,
	mtTreeNode	* const node
	)
{
	mtPrefValue	* data;


	if ( ! node )
	{
		return 0;
	}

	if ( mtkit_prefs_save_recurse ( root, node->left ) )
	{
		return 1;
	}

	data = (mtPrefValue *)node->data;

	if (	( data->value[0] != 0 && ! data->def ) ||
		( data->def && strcmp ( data->value, data->def ) )
		)
	{
		// Only save something if it deviates from the default

		mtUtreeNode	* key,
				* val;


		key = mtkit_utree_new_element ( root, (char *)node->key );
		if ( ! key )
		{
			return 1;
		}

		val = mtkit_utree_new_text ( key, data->value );
		if ( ! val )
		{
			return 1;
		}
	}

	if ( mtkit_prefs_save_recurse ( root, node->right ) )
	{
		return 1;
	}

	return 0;
}

int mtkit_prefs_save (
	mtPrefs		*	const	prefs,
	char		const * const	filename
	)
{
	int		error = 0;
	mtUtreeNode	* root;


	if ( ! prefs || ! prefs->tree || ! filename )
	{
		return 0;
	}

	root = mtkit_utree_new_root ();
	if ( ! root )
	{
		return 0;
	}

	// Populate the utree
	error = mtkit_prefs_save_recurse ( root, prefs->tree->root );

	if ( ! error )
	{
		error = mtkit_utree_save_file ( root, filename,
			MTKIT_UTREE_OUTPUT_DEFAULT, 0 );
	}

	mtkit_utree_destroy_node ( root );

	return error;
}

int mtkit_prefs_destroy (
	mtPrefs		* const	prefs
	)
{
	if ( ! prefs )
	{
		return 1;
	}

	mtkit_tree_destroy ( prefs->tree );

	free ( prefs );

	return 0;
}

int mtkit_prefs_block_callback (
	mtPrefs		* const prefs
	)
{
	if ( ! prefs )
	{
		return 1;
	}

	prefs->block_callbacks = 1;

	return 0;
}

int mtkit_prefs_unblock_callback (
	mtPrefs		* const prefs
	)
{
	if ( ! prefs )
	{
		return 1;
	}

	prefs->block_callbacks = 0;

	return 0;
}

int mtkit_prefs_get_int (
	mtPrefs		* const	prefs,
	char	const	* const	key,
	int		* const	value
	)
{
	mtPrefValue	* piv;


	if ( ! prefs || ! key || ! value )
	{
		return 1;
	}

	piv = mtkit_prefs_get_value ( prefs, key, VTYPE_INT );
	if ( ! piv )
	{
		return 2;		// Not in tree/table or wrong type
	}

	return mtkit_strtoi ( piv->value, value, NULL, 0 );
}

int mtkit_prefs_get_double (
	mtPrefs		* const prefs,
	char	const	* const key,
	double		* const value
	)
{
	mtPrefValue	* piv;


	if ( ! prefs || ! key || ! value )
	{
		return 1;
	}

	piv = mtkit_prefs_get_value ( prefs, key, VTYPE_DOUBLE );
	if ( ! piv )
	{
		return 2;		// Not in tree/table or wrong type
	}

	return mtkit_strtod ( piv->value, value, NULL, 0 );
}

int mtkit_prefs_get_str (
	mtPrefs		*	const	prefs,
	char	const	*	const	key,
	char	const	**	const	value
	)
{
	mtPrefValue	* piv;


	if ( ! prefs || ! key || ! value )
	{
		return 1;
	}

	piv = mtkit_prefs_get_value ( prefs, key, VTYPE_STR );
	if ( ! piv )
	{
		return 2;		// Not in tree/table or wrong type
	}

	value[0] = piv->value;

	return 0;
}

int mtkit_prefs_set_default (
	mtPrefs		*	const	prefs,
	char		const * const	key
	)
{
	if ( ! prefs || ! key )
	{
		return 1;
	}

	return mtkit_prefs_set_value ( prefs, key, NULL, VTYPE_ALL, 1 );
}

int mtkit_prefs_set_int (
	mtPrefs		*	const	prefs,
	char		const * const	key,
	int			const	value
	)
{
	char		str[256];


	if ( ! prefs || ! key )
	{
		return 1;
	}

	snprintf ( str, sizeof ( str ), "%i", value );

	return mtkit_prefs_set_value ( prefs, key, str, VTYPE_INT, 1 );
}

int mtkit_prefs_set_double (
	mtPrefs		*	const	prefs,
	char		const * const	key,
	double			const	value
	)
{
	char		str[256];


	if ( ! prefs || ! key )
	{
		return 1;
	}

	snprintf ( str, sizeof ( str ), "%.15lg", value );

	return mtkit_prefs_set_value ( prefs, key, str, VTYPE_DOUBLE, 1 );
}

int mtkit_prefs_set_str (
	mtPrefs		*	const	prefs,
	char		const * const	key,
	char		const * const	value
	)
{
	if ( ! prefs || ! key )
	{
		return 1;
	}

	return mtkit_prefs_set_value ( prefs, key, value, VTYPE_STR, 1 );
}

char * mtkit_prefs_type_text (
	int		const type
	)
{
	static char * ptypes[MTKIT_PREF_TYPE_TOTAL] = {
			"?",
			_("integer"),
			_("boolean"),
			_("RGB"),
			_("option"),
			_("decimal"),
			_("string"),
			_("string multi-line"),
			_("filename"),
			_("directory")
			};


	if (	type <= MTKIT_PREF_TYPE_NONE ||
		type >= MTKIT_PREF_TYPE_TOTAL
		)
	{
		return NULL;
	}

	return ptypes[type];
}

void mtkit_prefs_get_str_val (
	mtPrefValue	*	const	piv,
	char		const * const	value,
	char		*	const	buf,
	size_t			const	buf_size
	)
{
	char	const	* txt = NULL;
	int		num;


	buf[0] = 0;		// Default to empty string for sanity
	if ( ! value )
	{
		return;
	}

	switch ( piv->type )
	{
	case MTKIT_PREF_TYPE_BOOL:
		if ( value[0] == '0' )
		{
			txt = _("FALSE");
		}
		else
		{
			txt = _("TRUE");
		}

		break;

	case MTKIT_PREF_TYPE_RGB:
		if ( mtkit_strtoi ( value, &num, NULL, 0 ) )
		{
			txt = "?";
		}
		else
		{
			snprintf ( buf, buf_size, "( %i , %i , %i )",
				( (num >> 16) & 0xFF ),
				( (num >> 8) & 0xFF ),
				( num & 0xFF )
				);
		}
		break;

	case MTKIT_PREF_TYPE_OPTION:
		if ( mtkit_strtoi ( value, &num, NULL, 0 ) )
		{
			txt = "?";
		}
		else
		{
			char		* txt_option;


			txt_option = mtkit_strtok ( piv->opt, "\t", num );

			if ( txt_option )
			{
				snprintf ( buf, buf_size, "( %i ) = %s", num,
					txt_option );
				free ( txt_option );
			}
			else
			{
				snprintf ( buf, buf_size, "( %i ) = ?", num );
			}
		}
		break;

	default:
		txt = value;
		break;
	}

	// txt is checked in function: txt == NULL => do nothing
	mtkit_strnncpy ( buf, txt, buf_size );
}



#define BULK_LOOP_GET( TABLE, FUNCTION ) \
	if ( TABLE ) \
	{ \
		for ( ; TABLE->name; TABLE ++ ) \
		{ \
			if ( TABLE->var ) \
			{ \
				FUNCTION ( prefs, TABLE->name, TABLE->var ); \
			} \
		}\
	}



int mtkit_prefs_bulk_get (
	mtPrefs			* const	prefs,
	mtBulkInt	const	*	table_i,
	mtBulkDouble	const	*	table_d,
	mtBulkStr	const	*	table_s
	)
{
	BULK_LOOP_GET ( table_i, mtkit_prefs_get_int )
	BULK_LOOP_GET ( table_d, mtkit_prefs_get_double )

	if ( table_s )
	{
		char	const	* ns;


		for ( ; table_s->name; table_s ++ )
		{
			if ( table_s->var )
			{
				if ( ! mtkit_prefs_get_str ( prefs,
					table_s->name, &ns ) )
				{
					if ( mtkit_strfreedup ( table_s->var,
						ns ) )
					{
						return 1;
					}
				}
			}
		}
	}

	return 0;
}



#define BULK_LOOP_SET( TABLE, FUNCTION ) \
	if ( TABLE ) \
	{ \
		for ( ; TABLE->name; TABLE ++ ) \
		{ \
			if ( TABLE->var ) \
			{ \
				if ( FUNCTION ( prefs, TABLE->name, \
					TABLE->var[0] ) ) \
				{ \
					return 1; \
				} \
			} \
		} \
	}



int mtkit_prefs_bulk_set (
	mtPrefs			* const	prefs,
	mtBulkInt	const	*	table_i,
	mtBulkDouble	const	*	table_d,
	mtBulkStr	const	*	table_s
	)
{
	BULK_LOOP_SET ( table_i, mtkit_prefs_set_int )
	BULK_LOOP_SET ( table_d, mtkit_prefs_set_double )
	BULK_LOOP_SET ( table_s, mtkit_prefs_set_str )

	return 0;
}

int mtkit_prefs_value_mirror (
	mtPrefs			* const dest,
	mtPrefs			* const src,
	mtPrefTable	const	* const table
	)
{
	int		i,
			ival;
	char	const	* sval;
	double		dval;


	if ( ! dest || ! src || ! table )
	{
		goto fail;
	}

	for ( i = 0; table[i].key; i++ )
	{
		switch ( table[i].type )
		{
		case MTKIT_PREF_TYPE_INT:
		case MTKIT_PREF_TYPE_BOOL:
		case MTKIT_PREF_TYPE_RGB:
		case MTKIT_PREF_TYPE_OPTION:
			if ( mtkit_prefs_get_int ( src, table[i].key, &ival ) ||
				mtkit_prefs_set_int ( dest, table[i].key, ival )
				)
			{
				goto fail;
			}

			break;

		case MTKIT_PREF_TYPE_DOUBLE:
			if ( mtkit_prefs_get_double ( src, table[i].key, &dval )
				||
				mtkit_prefs_set_double ( dest, table[i].key,
					dval )
				)
			{
				goto fail;
			}

			break;

		case MTKIT_PREF_TYPE_STR:
		case MTKIT_PREF_TYPE_STR_MULTI:
		case MTKIT_PREF_TYPE_FILE:
		case MTKIT_PREF_TYPE_DIR:
			if ( mtkit_prefs_get_str ( src, table[i].key, &sval ) ||
				mtkit_prefs_set_str ( dest, table[i].key, sval )
				)
			{
				goto fail;
			}

			break;

		default:
			goto fail;
		}
	}

	return 0;

fail:
	return 1;
}

int mtkit_prefs_value_copy (
	mtPrefs		*	const dest,
	mtPrefs		*	const src,
	mtPrefTrans	const *	const table
	)
{
	int		i;
	mtPrefValue	* item_src,
			* item_dest;
	mtTreeNode	* tnode;


	if ( ! dest || ! src || ! table )
	{
		goto fail;
	}

	for ( i = 0; table[i].src && table[i].dest; i++ )
	{
		// Get source data
		tnode = mtkit_tree_node_find ( src->tree, table[i].src );

		if ( ! tnode || ! tnode->data )
		{
			goto fail;
		}

		item_src = tnode->data;

		// Get destination data
		tnode = mtkit_tree_node_find ( dest->tree, table[i].dest );

		if ( ! tnode || ! tnode->data )
		{
			goto fail;
		}

		item_dest = tnode->data;

		// Type validation
		if ( item_src->type != item_dest->type )
		{
			goto fail;
		}

		// Duplicate value
		if ( mtkit_strfreedup ( &item_dest->value, item_src->value ) )
		{
			goto fail;
		}
	}

	return 0;

fail:
	return 1;
}

mtTree * mtkit_prefs_get_tree (
	mtPrefs		* const	prefs
	)
{
	if ( ! prefs )
	{
		return NULL;
	}

	return prefs->tree;
}
