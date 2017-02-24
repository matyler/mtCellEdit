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

CedParser::CedParser ()
	:
	flag		( 0 ),
	ced_errno	( 0 ),
	sp		( 0 ),
	data		( 0.0 ),
	sheet		(),
	cell		(),
	row		( 0 ),
	column		( 0 )
{
}

static int ced_strtosheetref (
	char	const	* const	input,
				// Input string to parse NUL terminated
				// & ' character = start/finish, e.g.
				// 'Sheet 12'
				// '' = ' : 'Sheet '' 12' = Sheet ' 12
	CedSheet	** const result,
				// Put result here
	char	const *	* const next,
				// Next un-parsed character is put here,
				// (NULL = don't use
	CedBook		* const book
				// The book to search for this sheet
				// reference
	)
{
	char		* nst,
			* dest;
	char	const	* ss;
	mtTreeNode	* node;


	if ( ! input || input[0] != '\'' || ! result || ! book )
	{
		return 1;
	}

	nst = strdup ( input + 1 );
	if ( ! nst )
	{
		return 1;
	}

	// Filter out occurences of '' and replace with ' + remove trailing '
	for (	dest = nst, ss = input + 1;
		ss[0] != 0;
		ss++, dest++ )
	{
		if ( ss[0] == '\'' )
		{
			if ( ss[1] == '\'' )
			{
				// ''
				ss++;
			}
			else
			{
				// Final trailing ' at end
				// Ensure next[0] points to next unparsed char
				ss++;
				break;
			}
		}

		dest[0] = ss[0];
	}

	dest[0] = 0;

	if ( next )
	{
		next[0] = ss;
	}

	node = mtkit_tree_node_find ( book->sheets, nst );
	free ( nst );

	if ( ! node )
	{
		return 1;
	}

	result[0] = (CedSheet *)node->data;

	return 0;
}

static int yylex (
	void		* const	lvalp,
	char	const	* const	input,
	CedParser	* const	state
	)
{
	char	const	* cnext;
	char		* unsafe;
	char		ch;


	if ( state->flag & CED_PARSER_FLAG_ERROR )
	{
		return ERROR;		// Stop parsing as we have an error
	}

	// Skip white space
	while ( isspace ( input[ state->sp ] ) )
	{
		state->sp ++;
	}

	switch ( input[ state->sp ] )
	{
	case '+':
	case '-': // The grammar handles these chars directly
		return ( input[ state->sp ++ ] );

	case '"':
		cnext = strchr ( input + state->sp + 1, '"' );
		if ( cnext )
		{
			char	const	** cpp = (char const **)lvalp;


			cpp[0] = input + state->sp + 1;
			state->sp = (int)(cnext + 1 - input);

			return STRING;
		}
		break;
	}

	// Process numbers
	if ( ! mtkit_strtod ( input + state->sp, (double *)lvalp, &unsafe, 0 ) )
	{
		state->sp = (int)(unsafe - input);

		return NUM;
	}

	// Sheet Reference
	if ( input[ state->sp ] == '\'' )
	{
		if ( ced_strtosheetref ( input + state->sp,
			(CedSheet **)lvalp, &cnext, state->sheet->book )
			)
		{
			state->ced_errno = CED_ERROR_BAD_SHEETREF;

			return ERROR;
		}

		state->sp = (int)(cnext - input);

		return SHEETREF;
	}

	// Possible cell reference
	if ( input[ state->sp ] == 'r' || input[ state->sp ] == 'R' )
	{
		PCellRef	* cr = (PCellRef *)lvalp;


		if ( ! ced_strtocellref ( input + state->sp, &cr->cellref,
			&cnext, 0 )
			)
		{
			cr->sheet = state->sheet;
			state->sp = (int)(cnext - input);

			return CELLREF;
		}
	}

	// Process function names
	if ( isalpha ( input[ state->sp ] ) )
	{
		size_t			len = 0;
		char		const	* start = input + state->sp;
		char			* ns,
					* s;
		CedToken	const	* token;
		CedToken	const	** tok_ptr = (CedToken const **)lvalp;


		while ( isalpha ( input[ state->sp ] ) )
		{
			len ++;
			state->sp ++;
		}

		ns = (char *)malloc ( len + 1 );
		if ( ! ns )
		{
			state->ced_errno = CED_ERROR_MEMORY_ALLOCATION;

			return ERROR;
		}

		mtkit_strnncpy ( ns, start, len + 1 );

		s = ns;
		while ( s[0] )			// Make it lower case
		{
			s[0] = (char)tolower ( s[0] );
			s++;
		}

		token = ced_token_get ( ns );
		free ( ns );

		if ( ! token )
		{
			state->ced_errno = CED_ERROR_BAD_FUNCTION_NAME;

			return ERROR;
		}

		tok_ptr[0] = token;

		if ( token->flag & CED_TOKEN_FLAG_VOLATILE )
		{
			/* Set the volatile flag for functions like rand ()
			which change upon recalculation */

			state->flag |= CED_PARSER_FLAG_VOLATILE;
		}

		return FNCT;
	}

	ch = input[ state->sp ];
	state->sp ++;

	// Function argument separators
	if ( ch == ',' )
	{
		// Must come after number parse - European decimal point
		return ARGSEP;
	}

	if ( ch == ';' )
	{
		return ARGSEP;
	}

	// Return a single char
	return ch;
}

CedParser ced_sheet_parse_text (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column,
	char	const	* const	text,
	CedCell		* const	cell
	)
{
	CedParser	state;


	state.sheet = sheet;
	state.cell = cell;
	state.row = row;
	state.column = column;

	if ( yyparse ( text, &state ) )
	{
		state.flag |= CED_PARSER_FLAG_ERROR;
	}

	if ( cell )
	{
		// Update cell type according to errors/live references etc.

		if ( state.flag & CED_PARSER_FLAG_ERROR )
		{
			cell->type = CED_CELL_TYPE_ERROR;
			cell->value = state.ced_errno + 1000 * state.sp;
		}
		else if ( state.flag & CED_PARSER_FLAG_VOLATILE )
		{
			cell->type = CED_CELL_TYPE_FORMULA_EVAL;
		}
	}

	return state;
}

static int stat_sheet_get_cell_value (
	CedParser	* const	state,
	PCellRef	* const	cref,
	double		* const	result
	)
{
	int		r,
			c;
	CedCell		* cell;


	r = state->row    * cref->cellref.row_m + cref->cellref.row_d;
	c = state->column * cref->cellref.col_m + cref->cellref.col_d;

	if ( ! cref->sheet || r < 1 || c < 1 )
	{
		return 1;		// Invalid cell reference
	}

	cell = ced_sheet_get_cell ( cref->sheet, r, c );

	if ( cell && cell->type != CED_CELL_TYPE_NONE )
	{
		if ( cell->type != CED_CELL_TYPE_ERROR )
		{
			result[0] = cell->value;
		}
		else
		{
			return 1;	// Error in cell
		}
	}
	else
	{
		result[0] = 0;
	}

	return 0;	// Success
}

static int arg_add (
	PArgSet		* const	argset,
	CedFuncArg	* const	arg
	)
{
	if ( argset->tot >= CED_FUNC_ARG_MAX )
	{
		return 1;
	}

	argset->arg[ argset->tot ] = arg[0];
	argset->tot ++;
	argset->arg[ argset->tot ].type = 0;

	return 0;
}


// Called by yyparse on error
static void yyerror (
	char	const	* const	ARG_UNUSED ( input ),
	CedParser	* const	state,
	char	const	* const	ARG_UNUSED ( s )
	)
{
	state->flag |= CED_PARSER_FLAG_ERROR;
}

