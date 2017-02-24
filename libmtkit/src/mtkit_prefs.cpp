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


	new_item = (mtPrefValue *)calloc ( 1, sizeof ( mtPrefValue ) );
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
	return strcmp ( (char const *)k1, (char const *)k2 );
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


	// Check this key exists and is of a compatible type
	piv = mtkit_prefs_get_value ( prefs, key, vtype );
	if ( ! piv )
	{
		return 1;
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
			return 1;
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
		return 1;
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

static int mtkit_prefs_add (
	mtPrefs		*	const	prefs,
	mtPrefTable	const * const	table,
	char		const * const	prefix
	)
{
	mtPrefTable const * item;
	mtPrefValue	* data;
	size_t		lenprefix = 0;


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
			char		* new_key;
			size_t		len, lenkey;


			lenkey = strlen ( data->key );

			len = lenkey + lenprefix + 1;
			new_key = (char *)calloc ( len, 1 );

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

mtPrefs * mtkit_prefs_new (
	mtPrefTable	const * const	table
	)
{
	mtPrefs		* prefs;


	prefs = (mtPrefs *)calloc ( sizeof ( mtPrefs ), 1 );
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

static int mtkit_prefs_load (
	mtPrefs		*	const	prefs,
	char		const * const	filename
	)
{
	int		errors = 0;
	mtUtreeNode	* root, * node;


	if ( ! filename )
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

static int mtkit_prefs_save (
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
	// prefs & key validated by mtkit_prefs_get_value

	mtPrefValue	* piv;


	if ( ! value )
	{
		return 1;
	}

	piv = mtkit_prefs_get_value ( prefs, key, VTYPE_INT );
	if ( ! piv )
	{
		return 1;		// Not in tree/table or wrong type
	}

	return mtkit_strtoi ( piv->value, value, NULL, 0 );
}

int mtkit_prefs_get_double (
	mtPrefs		* const prefs,
	char	const	* const key,
	double		* const value
	)
{
	// prefs & key validated by mtkit_prefs_get_value

	mtPrefValue	* piv;


	if ( ! value )
	{
		return 1;
	}

	piv = mtkit_prefs_get_value ( prefs, key, VTYPE_DOUBLE );
	if ( ! piv )
	{
		return 1;		// Not in tree/table or wrong type
	}

	return mtkit_strtod ( piv->value, value, NULL, 0 );
}

int mtkit_prefs_get_str (
	mtPrefs		*	const	prefs,
	char	const	*	const	key,
	char	const	**	const	value
	)
{
	// prefs & key validated by mtkit_prefs_get_value

	mtPrefValue	* piv;


	if ( ! value )
	{
		return 1;
	}

	piv = mtkit_prefs_get_value ( prefs, key, VTYPE_STR );
	if ( ! piv )
	{
		return 1;		// Not in tree/table or wrong type
	}

	value[0] = piv->value;

	return 0;
}

int mtkit_prefs_set_default (
	mtPrefs		*	const	prefs,
	char		const * const	key
	)
{
	return mtkit_prefs_set_value ( prefs, key, NULL, VTYPE_ALL, 1 );
}

int mtkit_prefs_set_int (
	mtPrefs		*	const	prefs,
	char		const * const	key,
	int			const	value
	)
{
	char		str[256];


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


	snprintf ( str, sizeof ( str ), "%.15lg", value );

	return mtkit_prefs_set_value ( prefs, key, str, VTYPE_DOUBLE, 1 );
}

int mtkit_prefs_set_str (
	mtPrefs		*	const	prefs,
	char		const * const	key,
	char		const * const	value
	)
{
	return mtkit_prefs_set_value ( prefs, key, value, VTYPE_STR, 1 );
}

char const * mtkit_prefs_type_text (
	int		const type
	)
{
	static char const * ptypes[MTKIT_PREF_TYPE_TOTAL] = {
			"?",
			"integer",
			"boolean",
			"RGB",
			"option",
			"decimal",
			"string",
			"string multi-line",
			"filename",
			"directory"
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
			txt = "FALSE";
		}
		else
		{
			txt = "TRUE";
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
	mtPrefValue	* item_src, * item_dest;
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

		item_src = (mtPrefValue *)tnode->data;

		// Get destination data
		tnode = mtkit_tree_node_find ( dest->tree, table[i].dest );

		if ( ! tnode || ! tnode->data )
		{
			goto fail;
		}

		item_dest = (mtPrefValue *)tnode->data;

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

mtKit::Prefs::Prefs ()
	:
	prefsMem	(),
	prefsFilename	()
{
	prefsMem = mtkit_prefs_new ( NULL );
}

mtKit::Prefs::~Prefs ()
{
	save ();
	mtkit_prefs_destroy ( prefsMem );

	free ( prefsFilename );
	prefsFilename = NULL;
}

int mtKit::Prefs::load (
	char	const * const	filename,
	char	const * const	bin_name
	)
{
	char		* tmp_str = NULL;


	if ( filename )
	{
		tmp_str = strdup ( filename );

		if ( ! tmp_str )
		{
			return 1;
		}
	}
	else if ( bin_name )
	{
		mtString	* str;


		str = mtkit_string_new ( mtkit_file_home () );

		if (	! str ||
			mtkit_string_append ( str, "/.config" )
			)
		{
			mtkit_string_destroy ( str );

			return 1;
		}

		mkdir ( mtkit_string_get_buf ( str ),
			S_IRWXU | S_IRWXG | S_IRWXO );

		if (	mtkit_string_append ( str, "/" ) ||
			mtkit_string_append ( str, bin_name )
			)
		{
			mtkit_string_destroy ( str );

			return 1;
		}

		mkdir ( mtkit_string_get_buf ( str ),
			S_IRWXU | S_IRWXG | S_IRWXO );

		if ( mtkit_string_append ( str, "/prefs.txt" ) )
		{
			mtkit_string_destroy ( str );

			return 1;
		}

		tmp_str = mtkit_string_destroy_get_buf ( str );
	}
	else
	{
		// No filename and no binary name - i.e. no prefs to load!
		return 0;
	}

	if ( mtkit_prefs_load ( prefsMem, tmp_str ) )
	{
		// Error loading prefs
		free ( tmp_str );
		tmp_str = NULL;

		return 1;
	}

	// Success so start using new filename
	free ( prefsFilename );
	prefsFilename = tmp_str;

	return 0;
}

int mtKit::Prefs::save ()
{
	if ( mtkit_prefs_save ( prefsMem, prefsFilename ) )
	{
		// Error saving prefs
		return 1;
	}

	return 0;
}

int mtKit::Prefs::addTable (
	mtPrefTable const * const	table
	)
{
	return mtkit_prefs_add ( prefsMem, table, NULL );
}

mtPrefs * mtKit::Prefs::getPrefsMem ()
{
	return prefsMem;
}

void mtKit::Prefs::set (
	char	const *	key,
	int		value
	)
{
	mtkit_prefs_set_int ( prefsMem, key, value );
}

void mtKit::Prefs::set (
	char	const *	key,
	double		value
	)
{
	mtkit_prefs_set_double ( prefsMem, key, value );
}

void mtKit::Prefs::set (
	char	const *	key,
	char	const * value
	)
{
	mtkit_prefs_set_str ( prefsMem, key, value );
}

int mtKit::Prefs::getInt (
	char	const	* key
	)
{
	int		res = 0;


	mtkit_prefs_get_int ( prefsMem, key, &res );

	return res;
}

double mtKit::Prefs::getDouble (
	char	const	* key
	)
{
	double		res = 0;


	mtkit_prefs_get_double ( prefsMem, key, &res );

	return res;
}

char const * mtKit::Prefs::getString (
	char	const	* key
	)
{
	static char	const	* const	nothing	= "";
	char		const	*	res	= NULL;


	mtkit_prefs_get_str ( prefsMem, key, &res );

	if ( ! res )
	{
		return nothing;
	}

	return res;
}



static mtPrefTable const default_prefs_table[] = {
{ "prefs.col1",		MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.col2",		MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.col3",		MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
{ "prefs.col4",		MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },

{ "prefs.window_x",	MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_y",	MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_w",	MTKIT_PREF_TYPE_INT, "800", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_h",	MTKIT_PREF_TYPE_INT, "600", NULL, NULL, 0, NULL, NULL },

{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }

	};



int mtKit::Prefs::initWindowPrefs ()
{
	return mtkit_prefs_add ( prefsMem, default_prefs_table, 0 );
}

int mtKit::prefsWindowMirrorPrefs (
	mtPrefs		* const	dest,
	mtPrefs		* const	src
	)
{
	return mtkit_prefs_value_mirror ( dest, src, default_prefs_table );
}

int mtKit::prefsInitWindowPrefs (
	mtPrefs		* const	prefs
	)
{
	return mtkit_prefs_add ( prefs, default_prefs_table, 0 );
}

int mtkit_prefs_set_callback (
	mtPrefs		* const	prefs,
	char	const	* const	key,
	mtPrefCB	const	callback,
	void		* const	callback_ptr
	)
{
	// prefs & key validated by mtkit_prefs_get_value

	mtPrefValue	* piv;


	piv = mtkit_prefs_get_value ( prefs, key, VTYPE_ALL );
	if ( ! piv )
	{
		return 1;		// Key not in tree
	}

	piv->callback = callback;
	piv->callback_ptr = callback_ptr;

	return 0;
}

