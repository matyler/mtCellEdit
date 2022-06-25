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

%{
	#include "ced.h"



typedef struct
{
	CedSheet	* sheet;
	CedCellRef	cellref[2];
} PCellRange;

typedef struct
{
	CedSheet	* sheet;
	CedCellRef	cellref;
} PCellRef;

typedef struct
{
	int		tot;
	CedFuncArg	arg[CED_FUNC_ARG_MAX + 1];
} PArgSet;



static int yyparse (
	char		const	* input,
	CedParser		* state
	);
//	0 = success
//	1 = invalid input
//	2 = memory exhausted

static int yylex (
	void			* lvalp,
	char		const	* input,
	CedParser		* state
	);
	// Returns token or character

static void yyerror (
	char		const	*,
	CedParser		* state,
	char		const	*
	);

static int stat_sheet_get_cell_value (	// Get cell value from spreadsheet
	CedParser	* state,
	PCellRef	* cref,
	double		* result
	);
	// 0 = success

static int arg_add (
	PArgSet		* argset,
	CedFuncArg	* arg
	);
	// 0 = success

%}



// Bison declarations.

%define api.pure

%parse-param {	char		const	* input }
%parse-param {	CedParser		* state }

%lex-param {	char		const	* input }
%lex-param {	CedParser		* state }


%union {
	double		val;		// For returning numbers
	CedToken	* tptr;		// For returning functions/variables
	int		error;		// Error reporting
	PCellRef	cellref;	// Cell reference
	PCellRange	cellrange;	// Cell range
	CedSheet	* sheetref;
	char	const	* string;	// String pointer
	PArgSet		fargset;	// Function Argument Set
	CedFuncArg	farg;		// Function Argument
}


%token	<val>		NUM		// Double precision floating point
%token	<tptr>		FNCT		// Functions
%token	<error>		ERROR		// Error reporting
%token	<sheetref>	SHEETREF	// Sheet reference
%token	<cellref>	CELLREF		// Cell reference
%token	<string>	STRING		// String pointer
%token			ARGSEP		// Function argument separator

%type	<val>		exp		// Numerical expression
%type	<cellref>	cellref		// Cell reference
%type	<cellrange>	cellrange	// Cell range
%type	<fargset>	argset		// Function Argument Set
%type	<farg>		arg		// Function Argument

%left	'-' '+'
%left	'*' '/'

//%left NEG		// negation--unary minus BISON old
%precedence NEG		// negation--unary minus BISON 3

%right	'^'		// exponentiation
%right	'<' '>' '='	// conditional operators



%%	// The grammar follows.

input:
	// empty
	%empty	// BISON 3

	| exp
	{
		state->data = $1;
	}

	| '=' exp
	{
		if ( state->cell )
		{
			if ( isnan ( $2 ) )
			{
				state->flag |= CED_PARSER_FLAG_ERROR;
				state->ced_errno = CED_ERROR_NAN;

				YYERROR;
			}
			else if ( isinf ( $2 ) )
			{
				state->flag |= CED_PARSER_FLAG_ERROR;
				state->ced_errno = CED_ERROR_INFINITY;

				YYERROR;
			}
			else
			{
				state->cell->value = $2;
			}
		}
	}
;

cellref:
	CELLREF
	{
		$$.sheet = state->sheet;
		$$.cellref = $1.cellref;
	}

	| SHEETREF CELLREF
	{
		$$.sheet = $1;
		$$.cellref = $2.cellref;
	}
;

cellrange:
	cellref ':' CELLREF
	{
		$$.sheet = $1.sheet;
		$$.cellref[0] = $1.cellref;
		$$.cellref[1] = $3.cellref;
	}
;

exp:
	NUM
	{
		$$ = $1;
	}

	| cellref
	{
		if ( stat_sheet_get_cell_value ( state, &$1, &$$ ) )
		{
			state->flag |= CED_PARSER_FLAG_ERROR;
			state->ced_errno = CED_ERROR_CELLREF;

			YYERROR;
		}
		else
		{
			state->flag |= CED_PARSER_FLAG_VOLATILE;
		}
	}

	| FNCT '(' argset ')'
	{
		CedFuncState	funcs = { $1, state, &$$, $3.arg };


		if ( ced_token_exe ( &funcs ) )
		{
			YYERROR;
		}
	}

	| exp '+' exp		{ $$ = $1 + $3;	}
	| exp '-' exp		{ $$ = $1 - $3;	}
	| exp '*' exp		{ $$ = $1 * $3;	}
	| exp '/' exp		{ $$ = $1 / $3;	}
	| '-' exp  %prec NEG	{ $$ = -$2; }
	| exp '^' exp		{ $$ = pow ( $1, $3 ); }
	| '(' exp ')'		{ $$ = $2; }
	| exp '<' exp		{ if ( $1 < $3 )  $$ = 1; else $$ = 0; }
	| exp '<' '=' exp	{ if ( $1 <= $4 ) $$ = 1; else $$ = 0; }
	| exp '>' exp		{ if ( $1 > $3 )  $$ = 1; else $$ = 0; }
	| exp '>' '=' exp	{ if ( $1 >= $4 ) $$ = 1; else $$ = 0; }
	| exp '=' exp		{ if ( $1 == $3 ) $$ = 1; else $$ = 0; }
	| exp '<' '>' exp	{ if ( $1 != $4 ) $$ = 1; else $$ = 0; }

;	// Finish exp


argset:
	// empty
	%empty	// BISON 3

	{
		$$.arg[ 0 ].type = 0;
	}

	| arg
	{
		$$.arg[ 0 ] = $1;
		$$.arg[ 1 ].type = 0;
		$$.tot = 1;
	}

	| argset ARGSEP arg
	{
		if ( arg_add ( &$$, &$3 ) )
		{
			state->flag |= CED_PARSER_FLAG_ERROR;
			state->ced_errno = CED_ERROR_BAD_FUNCTION_ARGUMENTS;

			YYERROR;
		}
	}

;	// Finish argset

arg:
	exp
	{
		$$.type = CED_FARG_TYPE_NUM;
		$$.u.val = $1;
	}

	| cellrange
	{
		$$.type = CED_FARG_TYPE_CELLRANGE;
		$$.u.cellref[ 0 ] = $1.cellref[ 0 ];
		$$.u.cellref[ 1 ] = $1.cellref[ 1 ];
		$$.sheet = $1.sheet;
	}

	| '#' cellref
	{
		$$.type = CED_FARG_TYPE_CELLREF;
		$$.u.cellref[ 0 ] = $2.cellref;
		$$.sheet = $2.sheet;
	}

	| STRING
	{
		$$.type = CED_FARG_TYPE_STRING;
		$$.u.str = $1;
	}
;	// Finish arg

%%



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
	if ( ! input || input[0] != '\'' || ! result || ! book )
	{
		return 1;
	}

	char * nst = strdup ( input + 1 );
	if ( ! nst )
	{
		return 1;
	}

	char		* dest;
	char	const	* ss;

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

	mtTreeNode const * const node = mtkit_tree_node_find(book->sheets, nst);

	free ( nst );
	nst = NULL;

	if ( ! node )
	{
		return 1;
	}

	result[0] = (CedSheet *)(node->data);

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
		size_t			len = 1;
		char		const	* start = input + state->sp;
		char			* ns,
					* s;
		CedToken	const	* token;
		CedToken	const	** tok_ptr = (CedToken const **)lvalp;

		state->sp ++;
		while ( isalnum ( input[ state->sp ] ) )
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


	state.flag	= 0;
	state.ced_errno	= 0;
	state.sp	= 0;
	state.data	= 0.0;
	state.sheet	= sheet;
	state.cell	= cell;
	state.row	= row;
	state.column	= column;

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

