/*
	Copyright (C) 2008-2016 Mark Tyler

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



static mtUtreeNode * mtkit_utree_new_comment (
	mtUtreeNode		* parent,
	char		const	* comment
	);



static char const	valid_name_char[256] = {
	0,0,0,0,0,0,0,0, 0,1,1,0,0,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	1,0,0,0,0,0,0,0, 0,0,0,0,0,9,9,0, 9,9,9,9,9,9,9,9, 9,9,9,0,0,0,0,0,
	0,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,0,0,0,0,9,
	0,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,0,0,0,0,0,

	9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,
	9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,
	9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,
	9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9
	};



#define MTKIT_UTREE_IS_WHITESPACE(X) (valid_name_char[ (unsigned char)X ] == 1)
#define MTKIT_UTREE_IS_NAME_CHAR(X)  (valid_name_char[ (unsigned char)X ] == 9)



static char * utree_formulate_input (
	char		** const	in
	)
{
	size_t		extra = 0, len = 0, newlen;
	char		* input = in[0], * output = NULL, * s1, * s2;


	// Count the substitutions required
	for ( s1 = input; s1[0] != '"'; s1++ )
	{
		if ( s1[0] == '\\' )
		{
			s1 ++;
			extra ++;
		}

		if ( s1[0] == 0 )
		{
			return 0;	// Illegal, no trailing "
		}
	}

	len = (size_t)(s1 - input);
	newlen = len - extra;
	output = (char *)malloc ( newlen + 1 );

	if ( ! output )
	{
		return NULL;
	}

	output[newlen] = 0;		// Terminator

	if ( extra < 1 )
	{
		memcpy ( output, input, len );

		goto finish;
	}

	for ( s1 = input, s2 = output; s1[0] != '"'; )
	{
		if ( s1[0] == '\\' )
		{
			s1 ++;
		}

		*s2++ = *s1++;
	}

finish:
	in[0] = input + len + 1;	// Jump over length + trailing "

	return output;
}

static mtUtreeNode * parse_node_comment (
	mtUtreeNode	*	const	current,
	char		**	const	s
	)
{
	mtUtreeNode	* new_node = NULL;
	char		* b = s[0], * c, * text;
	size_t		len;


	c = strstr ( b, "*/" );

	if ( ! c )
	{
		return NULL;		// No endquote found so fail
	}

	if ( c == b )
	{
		return NULL;		// Empty comment is illegal
	}

	len = (size_t)(c - b + 1);
	text = (char *)malloc ( len );

	if ( ! text )
	{
		return NULL;
	}

	mtkit_strnncpy ( text, b, len );

	new_node = mtkit_utree_new_comment ( current, text );
	free ( text );

	s[0] = c + 2;

	return new_node;
}

static char * utree_formulate_input_name (
	char		** const	buf
	)
{
	size_t		len;
	char		* newval, * newtxt;


	for (	newval = buf[0];
		MTKIT_UTREE_IS_NAME_CHAR ( newval[0] );
		newval ++
		)
	{
	}

	if ( newval <= buf[0] )
	{
		return NULL;
	}

	len = (size_t)(newval - buf[0] + 1);
	newtxt = (char *)malloc ( len );

	if ( newtxt )
	{
		mtkit_strnncpy ( newtxt, buf[0], len );

		buf[0] = newval;
	}

	return newtxt;
}

static mtUtreeNode * parse_node_element (
	mtUtreeNode	*	const	node,
	char		**	const	pb
	)
{
	mtUtreeNode	* new_node = NULL;
	char		* b,
			* b2,
			* new_name = NULL;


	b = pb[0];

	if ( b[0] == '"' )
	{
		// Create name of node using "" notation
		b++;
		new_name = utree_formulate_input ( &b );

		if ( ! new_name )
		{
			return NULL;
		}

		b2 = b;
	}
	else
	{
		b2 = b;
		new_name = utree_formulate_input_name ( &b2 );

		if ( ! new_name )
		{
			return NULL;
		}
	}

	pb[0] = b2;		// Parsed successfully to here

	new_node = mtkit_utree_new_element ( node, new_name );
	free ( new_name );

	if ( ! new_node )
	{
		return NULL;
	}

	return new_node;
}

mtUtreeNode * mtkit_utree_load_mem (
	mtUtreeNode	*	const	parent,
	char		*	const	buf,
	size_t			const	size,
	char		**	const	breakpoint
	)
{
	char		* b,
			old_end,
			* newtxt,
			* newval;
	int		errs = 1,
			res;
	mtUtreeNode	* root = parent,
			* current,
			* new_node;


	if ( ! buf || size < 1 )
	{
		return NULL;
	}

	// Create empty root node if none provided
	if ( ! root )
	{
		root = mtkit_utree_new_root ();
		if ( ! root )
		{
			free ( buf );

			return NULL;
		}
	}

	// We impose NUL termination as we use C string functions
	old_end = buf[size - 1];
	buf[size - 1] = 0;

	// Parse the memory, creating strings as needed and allocating
	// nodes/attributes

	current = root;
	b = buf;
	while ( b[0] )
	{
		while ( MTKIT_UTREE_IS_WHITESPACE ( b[0] ) )
		{
			b++;
		}

		if ( b[0] == 0 )
		{
			break;
		}

		if ( b[0] == '{' )	// Element start
		{
			b++;
			while ( MTKIT_UTREE_IS_WHITESPACE ( b[0] ) )
			{
				b++;
			}

			new_node = parse_node_element ( current, &b );
			if ( ! new_node )
			{
				goto error;
			}

			current = new_node;
		}
		else if ( b[0] == '}' )		// Element finish
		{
			if ( current == root )
			{
				goto error;	// Cannot go above root
			}

			current = current->parent;
			b++;
		}
		else if ( b[0] == '/' && b[1] == '*' )	// Comment
		{
			b = b + 2;

			new_node = parse_node_comment ( current, &b );
			if ( ! new_node )
			{
				goto error;
			}
		}
		else if ( b[0] == '"' || MTKIT_UTREE_IS_NAME_CHAR ( b[0] )  )
		{
			// Parse as a text chunk or beginning of an attribute
			if ( b[0] == '"' )
			{
				b++;

				newtxt = utree_formulate_input ( &b );
				if ( ! newtxt )
				{
					goto error;
				}
			}
			else
			{
				newtxt = utree_formulate_input_name ( &b );
				if ( ! newtxt )
				{
					goto error;
				}
			}

			if ( b[0] == '=' )
			{
				// Parse as attribute
				b++;
				if ( b[0] != '"' )
				{
					goto error;
				}

				b++;
				newval = utree_formulate_input ( &b );
				if ( ! newval )
				{
					free ( newtxt );

					goto error;
				}

				res = mtkit_utree_set_attribute_str ( current,
					newtxt, newval );
				free ( newtxt );
				free ( newval );

				if ( res )
				{
					goto error;
				}
			}
			else
			{
				// Store as text element
				new_node = mtkit_utree_new_text ( current,
					newtxt );
				free ( newtxt );

				if ( ! new_node )
				{
					goto error;
				}
			}
		}
		else
		{
			goto error;	// Character not recognised so stop
		}
	}

	errs = 0;			// Success

error:
	if ( breakpoint )
	{
		if ( errs )
		{
			breakpoint[0] = b;	// Parsing error
		}
		else
		{
			breakpoint[0] = NULL;	// No error
		}
	}

	buf[size - 1] = old_end;

	return root;
}

mtUtreeNode * mtkit_utree_load_file (
	mtUtreeNode		* const parent,
	char		const	* const filename,
	int			* const errors,
	int			* const filetype
	)
{
	char		* buf,
			* err = NULL;
	int		size;
	mtUtreeNode	* root;


	// Load the file into memory and place a NUL at the end
	buf = mtkit_file_load ( filename, &size,
		MTKIT_FILE_ZERO | MTKIT_FILE_GUNZIP, filetype );

	if ( ! buf )
	{
		return NULL;
	}

	root = mtkit_utree_load_mem ( parent, buf, (size_t)size, &err );

	if ( errors )
	{
		if ( err )
		{
			errors[0] = 1;
		}
		else
		{
			errors[0] = 0;
		}
	}

	// Release the file memory
	free ( buf );

	return root;
}

static int utree_output_indents (
	mtFile		* const mtfp,
	int		const	i
	)
{
	if ( i > 0 )
	{
		char		* mem;
		int		res = 0;


		mem = (char *)malloc ( (size_t)i );
		if ( ! mem )
		{
			return 1;
		}

		memset ( mem, '\t', (size_t)i );
		res = mtkit_file_write ( mtfp, mem, i );
		free ( mem );

		if ( res )
		{
			return 1;
		}
	}

	return 0;
}

static int utree_formulate_output (
	char		*	input,
	mtFile		* const	mtfp
	)
{
	int		n;


	if ( ! input )
	{
		return 1;
	}

	if ( mtkit_file_write ( mtfp, "\"", 1 ) )
	{
		return 1;
	}

	while ( 1 )
	{
		for (	n = 0;
				input[n] != '\\' &&
				input[n] != '"' &&
				input[n] != 0;
			n++
			)
		{
		}

		if ( n )
		{
			if ( mtkit_file_write ( mtfp, input, n ) )
			{
				return 1;
			}

			input += n;
		}

		if ( input[0] == 0 )
		{
			break;
		}

		if ( mtkit_file_write ( mtfp, "\\", 1 ) )
		{
			return 1;
		}

		if ( mtkit_file_write ( mtfp, input, 1 ) )
		{
			return 1;
		}

		input ++;
	}

	if ( mtkit_file_write ( mtfp, "\"", 1 ) )
	{
		return 1;
	}

	return 0;
}

static int attr_out_cb (
	mtTreeNode	* const	node,
	void		* const	user_data
	)
{
	mtFile		* mtfp = (mtFile *)user_data;


	if (	mtkit_file_write ( mtfp, " ", 1 ) ||
		utree_formulate_output ( (char *)node->key, mtfp ) ||
		mtkit_file_write ( mtfp, "=", 1 ) ||
		utree_formulate_output ( (char *)node->data, mtfp )
		)
	{
		return 1;
	}

	return 0;
}

static int utree_recurse_save (
	mtFile		* const	mtfp,
	int		const	indents,
	mtUtreeNode	* const	node,
	int		const	output
	)
{
	if ( node->type == MTKIT_UTREE_NODE_TYPE_COMMENT )
	{
		if ( (output & MTKIT_UTREE_OUTPUT_INDENTS) )
		{
			if ( utree_output_indents ( mtfp, indents ) )
			{
				return 1;
			}
		}

		if (	mtkit_file_write ( mtfp, "/*", 2 ) ||
			mtkit_file_write_string ( mtfp, node->text ) ||
			mtkit_file_write ( mtfp, "*/", 2 )
			)
		{
			return 1;
		}

		if ( output & MTKIT_UTREE_OUTPUT_NEWLINES )
		{
			if ( mtkit_file_write_string ( mtfp, "\n" ) )
			{
				return 1;
			}
		}

		return 0;		// Comments have no children
	}

	if ( node->type == MTKIT_UTREE_NODE_TYPE_TEXT )
	{
		if ( (output & MTKIT_UTREE_OUTPUT_INDENTS) )
		{
			if (	(output & MTKIT_UTREE_OUTPUT_TEXT_NEWLINES) ||
				! node->previous ||
				node->previous->type !=
					MTKIT_UTREE_NODE_TYPE_TEXT
				)
			{
				if ( utree_output_indents ( mtfp, indents ) )
				{
					return 1;
				}
			}
		}

		if ( utree_formulate_output ( node->text, mtfp ) )
		{
			return 1;
		}

		if ( (output & MTKIT_UTREE_OUTPUT_NEWLINES) )
		{
			if (	! (output & MTKIT_UTREE_OUTPUT_TEXT_NEWLINES) &&
				node->next &&
				node->next->type == MTKIT_UTREE_NODE_TYPE_TEXT
				)
			{
				if ( mtkit_file_write ( mtfp, " ", 1 ) )
				{
					return 1;
				}
			}
			else
			{
				if ( mtkit_file_write ( mtfp, "\n", 1 ) )
				{
					return 1;
				}
			}
		}

		return 0;		// Text has no children
	}

	if ( node->type == MTKIT_UTREE_NODE_TYPE_ELEMENT )
	{
		if ( (output & MTKIT_UTREE_OUTPUT_INDENTS) )
		{
			if ( utree_output_indents ( mtfp, indents ) )
			{
				return 1;
			}
		}

		if (	mtkit_file_write ( mtfp, "{ ", 2 ) ||
			utree_formulate_output ( node->text, mtfp )
			)
		{
			return 1;
		}

		if ( mtkit_tree_scan ( node->attribute_tree, attr_out_cb, mtfp,
			0 )
			)
		{
			return 1;
		}

		if ( ! node->child )
		{
			// No children so use concise closure notation

			if ( mtkit_file_write ( mtfp, " }", 2 ) )
			{
				return 1;
			}

			if ( output & MTKIT_UTREE_OUTPUT_NEWLINES )
			{
				if ( mtkit_file_write ( mtfp, "\n", 1 ) )
				{
					return 1;
				}
			}

			return 0;
		}

		if ( output & MTKIT_UTREE_OUTPUT_NEWLINES )
		{
			if ( mtkit_file_write ( mtfp, "\n", 1 ) )
			{
				return 1;
			}
		}
		else
		{
			if ( mtkit_file_write ( mtfp, " ", 1 ) )
			{
				return 1;
			}
		}
	}

	if ( node->child )
	{
		mtUtreeNode	* next_node;


		// Recurse each child in turn
		next_node = node->child;
		while ( next_node )
		{
			if ( utree_recurse_save ( mtfp, indents + 1, next_node,
				output )
				)
			{
				return 1;
			}

			next_node = next_node->next;
		}
	}

	if ( node->type == MTKIT_UTREE_NODE_TYPE_ELEMENT )
	{
		// Output closing name tag
		if ( (output & MTKIT_UTREE_OUTPUT_INDENTS) )
		{
			if ( utree_output_indents ( mtfp, indents ) )
			{
				return 1;
			}
		}

		if ( mtkit_file_write ( mtfp, "}", 1 ) )
		{
			return 1;
		}

		if ( output & MTKIT_UTREE_OUTPUT_NEWLINES )
		{
			if ( mtkit_file_write ( mtfp, "\n", 1 ) )
			{
				return 1;
			}
		}

		return 0;
	}

	return 0;
}

static int mtkit_utree_save_file_real (
	mtFile		* const	mtfp,
	mtUtreeNode	* const	node,
	int		const	output
	)
{
	int		start = 0;


	if ( ! node )
	{
		return 1;
	}

	if ( node->type == MTKIT_UTREE_NODE_TYPE_ROOT )
	{
		start = -1;
	}

	if ( utree_recurse_save ( mtfp, start, node, output ) )
	{
		return 1;
	}

	return 0;
}

mtFile * mtkit_utree_save_file_mem (
	mtUtreeNode	* const	node,
	int		const	output
	)
{
	mtFile		* mtfp;


	mtfp = mtkit_file_open_mem ();
	if ( ! mtfp )
	{
		return NULL;
	}

	if ( mtkit_utree_save_file_real ( mtfp, node, output ) )
	{
		mtkit_file_close ( mtfp );
		mtfp = NULL;
	}

	return mtfp;
}

int mtkit_utree_save_file (
	mtUtreeNode		* const	node,
	char		const	* const	filename,
	int			const	output,
	int			const	filetype
	)
{
	int		res;
	mtFile		* mtfp;


	if ( ! filename )
	{
		return 1;
	}

	mtfp = mtkit_file_open_disk ( filename );
	if ( ! mtfp )
	{
		return 1;
	}

	res = mtkit_utree_save_file_real ( mtfp, node, output );
	mtkit_file_close ( mtfp );

	if ( res )
	{
		return 1;		// Error
	}

	if ( filetype & MTKIT_FILE_GUNZIP )
	{
		char		* buf;
		int		size,
				i;


		// Re-save this uncompressed file to a .gz file

		buf = mtkit_file_load ( filename, &size, 0, NULL );
		if ( ! buf )
		{
			return 1;	// Error
		}

		i = mtkit_file_save ( filename, buf, size, MTKIT_FILE_GUNZIP );
		free ( buf );

		if ( i )
		{
			return 1;	// Error
		}
	}

	return 0;
}

static int attr_cmp (
	void	const * const	k1,
	void	const * const	k2
	)
{
	return strcmp ( (char const *)k1, (char const *)k2 );
}

static void attr_del (
	mtTreeNode	* const	node
	)
{
	free ( node->key );
	free ( node->data );
}

static mtUtreeNode * mtkit_utree_new_node_real (
	int			const	type,
	mtUtreeNode		* const	parent,
	char		const	* const	text
	)
{
	mtUtreeNode	* node;


	if ( ! parent && type != MTKIT_UTREE_NODE_TYPE_ROOT )
	{
		return NULL;		// Illegal combination
	}

	if (	parent &&
		parent->type != MTKIT_UTREE_NODE_TYPE_ROOT &&
		parent->type != MTKIT_UTREE_NODE_TYPE_ELEMENT
		)
	{
		return NULL;		// Only root/elements can have offspring
	}

	node = (mtUtreeNode *)calloc ( sizeof ( mtUtreeNode ), 1 );

	if ( ! node )
	{
		return NULL;
	}

	node->type = type;

	node->attribute_tree = mtkit_tree_new ( attr_cmp, attr_del );
	if ( ! node->attribute_tree )
	{
		free ( node );

		return NULL;
	}

	if ( parent )
	{
		if ( ! text )
		{
			node->text = strdup ( "" );
		}
		else
		{
			node->text = strdup ( text );
		}

		if ( ! node->text )
		{
			mtkit_tree_destroy ( node->attribute_tree );
			free ( node );

			return NULL;
		}

		node->parent = parent;
		node->previous = parent->child_last;

		// Attach to parent & sibling
		if ( node->previous )
		{
			parent->child_last->next = node;
		}
		else
		{
			parent->child = node;
		}

		parent->child_last = node;
	}

	return node;
}

mtUtreeNode * mtkit_utree_new_root ( void )
{
	return mtkit_utree_new_node_real ( MTKIT_UTREE_NODE_TYPE_ROOT, NULL,
		NULL );
}

mtUtreeNode * mtkit_utree_new_element (
	mtUtreeNode		* const	parent,
	char		const	* const	name
	)
{
	return mtkit_utree_new_node_real ( MTKIT_UTREE_NODE_TYPE_ELEMENT,
		parent, name );
}

mtUtreeNode * mtkit_utree_new_text (
	mtUtreeNode		* const	parent,
	char		const	* const	text
	)
{
	if ( ! text )
	{
		return NULL;
	}

	return mtkit_utree_new_node_real ( MTKIT_UTREE_NODE_TYPE_TEXT, parent,
		text );
}

static mtUtreeNode * mtkit_utree_new_comment (
	mtUtreeNode		* const	parent,
	char		const	* const	comment
	)
{
	if ( comment )
	{
		mtUtreeNode	* node;
		char		* s = strdup ( comment ),
				* f;


		if ( ! s )
		{
			return NULL;
		}

		while ( ( f = strstr ( s, "*/" ) ) )
		{
			f[0] = ' ';	// Stamp out closures
		}

		node = mtkit_utree_new_node_real (
			MTKIT_UTREE_NODE_TYPE_COMMENT, parent, s );

		free ( s );

		return node;
	}

	return NULL;
}

mtUtreeNode * mtkit_utree_get_node (
	mtUtreeNode		* const	parent,
	char		const	* const	text,
	int			const	type
	)
{
	mtUtreeNode	* node;


	if ( ! parent )
	{
		return NULL;
	}

	for ( node = parent->child; node; node = node->next )
	{
		if ( 0 == type || type == node->type )
		{
			if ( ! text || 0 == strcmp ( text, node->text ) )
			{
				return node;
			}
		}
	}

	return NULL;
}

mtUtreeNode * mtkit_utree_get_node_next (
	mtUtreeNode		* const	start,
	char		const	* const	text,
	int			const	type
	)
{
	mtUtreeNode	* node;


	if ( ! start )
	{
		return NULL;
	}

	for ( node = start->next; node; node = node->next )
	{
		if ( 0 == type || type == node->type )
		{
			if ( ! text || 0 == strcmp ( text, node->text ) )
			{
				return node;
			}
		}
	}

	return NULL;
}

static int mtkit_utree_destroy_node_recurse (
	mtUtreeNode	* const	node
	)
{
	// Destroy all children first
	while ( node->child )
	{
		if ( mtkit_utree_destroy_node_recurse ( node->child ) )
		{
			return 1;	// Problem so bail out
		}
	}

	// Re-link siblings & parent
	if ( node->next )
	{
		node->next->previous = node->previous;
	}
	else
	{
		if ( node->parent )
		{
			node->parent->child_last = node->previous;
		}
	}

	if ( node->previous )
	{
		node->previous->next = node->next;
	}
	else
	{
		if ( node->parent )
		{
			node->parent->child = node->next;
		}
	}

	// Free memory
	mtkit_tree_destroy ( node->attribute_tree );
	free ( node->text );
	free ( node );

	return 0;
}

int mtkit_utree_destroy_node (
	mtUtreeNode	* const	node
	)
{
	if ( ! node )
	{
		return 1;
	}

	return mtkit_utree_destroy_node_recurse ( node );
}

static int mtkit_utree_set_attribute_real (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	char		const	* const	value
	)
{
	char		* key = NULL,
			* data = NULL;


	if ( ! node || ! name || ! value )
	{
		return 1;
	}

	if ( node->type != MTKIT_UTREE_NODE_TYPE_ELEMENT )
	{
		return 1;		// Only elements have attributes
	}

	key = strdup ( name );
	data = strdup ( value );

	if ( ! key || ! data )
	{
		goto error;
	}

	if ( ! mtkit_tree_node_add ( node->attribute_tree, key, data ) )
	{
		goto error;
	}

	return 0;			// Success

error:
	free ( key );
	free ( data );

	return 1;			// Fail
}

static int get_attr_txt (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	char		const *	* const	txt
	)
{
	if ( ! node )
	{
		return 1;
	}

	mtTreeNode * const tn = mtkit_tree_node_find ( node->attribute_tree,
					name );

	if ( ! tn )
	{
		return 1;
	}

	txt[0] = (char const *)tn->data;

	return 0;
}

int mtkit_utree_get_attribute_int (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	int			* const	value
	)
{
	char	const	* txt;


	if ( get_attr_txt ( node, name, &txt ) )
	{
		return -1;
	}

	return mtkit_strtoi ( txt, value, NULL, 1 );
}

int mtkit_utree_get_attribute_double (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	double			* const	value
	)
{
	char	const	* txt;


	if ( get_attr_txt ( node, name, &txt ) )
	{
		return -1;
	}

	return mtkit_strtod ( txt, value, NULL, 1 );
}

int mtkit_utree_get_attribute_str (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	char		const *	* const	value
	)
{
	if ( get_attr_txt ( node, name, value ) )
	{
		return 1;
	}

	return 0;
}

static int mtkit_utree_set_attribute_int (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	int			const	value
	)
{
	char		txt[128];


	snprintf ( txt, sizeof ( txt ), "%i", value );

	return mtkit_utree_set_attribute_real ( node, name, txt );
}

static int mtkit_utree_set_attribute_double (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	double			const	value
	)
{
	char		txt[128];


	snprintf ( txt, sizeof ( txt ), "%.15g", value );

	return mtkit_utree_set_attribute_real ( node, name, txt );
}

int mtkit_utree_set_attribute_str (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	char		const	* const	value
	)
{
	return mtkit_utree_set_attribute_real ( node, name, value );
}



#define BULK_LOOP_GET( TABLE, FUNCTION ) \
	if ( TABLE ) \
	{ \
		for ( ; TABLE->name; TABLE ++ ) \
		{ \
			if ( TABLE->var ) \
			{ \
				FUNCTION ( node, TABLE->name, TABLE->var );\
			} \
		} \
	} \



int mtkit_utree_bulk_get (
	mtUtreeNode	* const	node,
	mtBulkInt	*	table_i,
	mtBulkDouble	*	table_d,
	mtBulkStr	*	table_s
	)
{
	char	const	* n;


	BULK_LOOP_GET ( table_i, mtkit_utree_get_attribute_int )
	BULK_LOOP_GET ( table_d, mtkit_utree_get_attribute_double )

	if ( table_s )
	{
		for ( ; table_s->name; table_s ++ )
		{
			if ( table_s->var )
			{
				if ( ! mtkit_utree_get_attribute_str ( node,
					table_s->name, &n ) )
				{
					if ( mtkit_strfreedup( table_s->var, n))
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
				if ( FUNCTION ( node, TABLE->name, \
					TABLE->var[0] ) ) \
				{ \
					return 1; \
				} \
			} \
		} \
	} \



int mtkit_utree_bulk_set (
	mtUtreeNode	* const	node,
	mtBulkInt	*	table_i,
	mtBulkDouble	*	table_d,
	mtBulkStr	*	table_s
	)
{
	BULK_LOOP_SET ( table_i, mtkit_utree_set_attribute_int )
	BULK_LOOP_SET ( table_d, mtkit_utree_set_attribute_double )
	BULK_LOOP_SET ( table_s, mtkit_utree_set_attribute_str )

	return 0;
}

