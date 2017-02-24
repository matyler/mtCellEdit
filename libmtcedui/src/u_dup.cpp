/*
	Copyright (C) 2010-2016 Mark Tyler

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



#define FIRST_DUPLICATE_ID	2



typedef int (* NameCB) (
	CuiBook		* cubook,
	char	const	* old_name,
	char	const	* new_name,
	void		* user_data
	);



#define DUP_STR_EXTRA		16



static int duplicate_name (
	CuiBook		* const	cubook,
	char	const	* const	name,
	NameCB		const	callback,
	void		* const	user_data
	)
{
	size_t		size;
	char		* new_name = NULL, * cp;
	int		next_id,
			res = 0;


	if ( ! name || ! callback )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	// Create space for a large number + brackets/space
	size = strlen ( name );

	// Check overflow
	if ( size > (SIZE_MAX - DUP_STR_EXTRA) )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	new_name = (char *)calloc ( size + DUP_STR_EXTRA, 1 );

	if ( ! new_name )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	mtkit_strnncpy ( new_name, name, size + DUP_STR_EXTRA );

	next_id = FIRST_DUPLICATE_ID;

	// Find out if this name is already a duplicate - if so use its number
	if ( size > 3 && new_name[size - 1] == ')' )
	{
		// Lose the final bracket so we can test for a valid integer
		new_name[size - 1] = 0;

		// Chop off the last ' (%i)'
		cp = strrchr ( new_name, '(' );
		if (	cp > new_name &&
			cp[-1] == ' ' &&
			! mtkit_strtoi ( cp + 1, &next_id, NULL, 1 ) &&
			next_id >= FIRST_DUPLICATE_ID
			)
		{
			cp[-1] = 0;
			size = (size_t)( cp - 1 - new_name );
		}
		else
		{
			// Stop negative index being valid
			if ( next_id < FIRST_DUPLICATE_ID )
			{
				next_id = FIRST_DUPLICATE_ID;
			}

			// Reinstate the bracket as its needed in the original
			// name.
			new_name[size - 1] = ')';
		}
	}

	// Keep looping until we get a valid number we can use
	for ( ; next_id < CUI_SHEET_MAX_NAME; next_id ++ )
	{
		snprintf ( new_name + size, DUP_STR_EXTRA, " (%i)", next_id );
		res = callback ( cubook, name, new_name, user_data );

		if ( res )
		{
			// Keep looping until we find a valid new name or get
			// error.

			if ( res == 1 )
			{
				res = 0;
			}

			break;
		}
	}

	if ( ! res && next_id == CUI_SHEET_MAX_NAME )
	{
		// No spare names found
		res = CUI_ERROR_NO_CHANGES;
	}

	free ( new_name );

	return res;
}

static int dup_sheet_cb (
	CuiBook		* const	cubook,
	char	const	* const	ARG_UNUSED ( old_name ),
	char	const	* const	new_name,
	void		* const	user_data
	)
{
	if ( ! ced_book_get_sheet ( cubook->book, new_name ) )
	{
		int	res;


		res = cui_book_add_sheet ( cubook, (CedSheet *)user_data,
			new_name );

		if ( res < 0 )
		{
			return res;	// Stop - error occurred
		}

		return 1;		// Use this new_name
	}

	return 0;			// Keep looking - this name is in use
}

int cui_book_duplicate_sheet (
	CuiBook		*	const	cubook,
	CedSheet	*	const	sheet,
	CedSheet	**	const	new_sh
	)
{
	int		res;
	CedSheet	* newsheet = NULL;


	if (	! cubook		||
		! sheet			||
		! sheet->book_tnode	||
		! sheet->book_tnode->key
		)
	{
		return CUI_ERROR_NO_CHANGES;
	}

	newsheet = ced_sheet_duplicate ( sheet );
	if ( ! newsheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	res = duplicate_name ( cubook, (char *)sheet->book_tnode->key,
		dup_sheet_cb, newsheet );

	if ( res )
	{
		return res;
	}

	if ( new_sh )
	{
		new_sh[0] = newsheet;
	}

	return 0;
}

static int check_graph_name_available (
	CuiBook		* const	cubook,
	char	const	* const	old_name,
	char	const	* const	new_name,
	void		* const	user_data
	)
{
	CedBook		* book = cubook->book;


	if ( ! cui_graph_get ( book, new_name ) )
	{
		CedBookFile	* old, * nb;
		char		* newmem = NULL;
		char		** nameret = (char **)user_data;


		old = cui_graph_get ( book, old_name );
		if ( ! old )
		{
			goto error;
		}

		if ( old->size > 0 )
		{
			newmem = (char *)malloc ( (size_t)old->size );
			if ( ! newmem )
			{
				goto error;
			}

			memcpy ( newmem, old->mem, (size_t)old->size );
		}

		nb = cui_graph_new ( book, newmem, old->size, new_name );
		if ( ! nb )
		{
			goto error;
		}

		if ( nameret )
		{
			// Note: we can't return 'newname' because it is free'd
			// by duplicate_name ()
			nameret[0] = strdup ( new_name );
		}

		return 1;		// Success: Use this name

error:
		free ( newmem );
		return -1;		// Bail out with error
	}

	return 0;		// Do not use, keep looking for next number
}

int cui_graph_duplicate (
	CuiBook		* const	cubook,
	char	const	* const	graph_name,
	char	*	* const	new_name
	)
{
	if (	! cubook ||
		! graph_name ||
		duplicate_name ( cubook, graph_name,
			check_graph_name_available, new_name )
		)
	{
		return 1;
	}

	return 0;
}
