/*
	Copyright (C) 2008-2017 Mark Tyler

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
	#include "private.h"



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

%pure-parser

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

%left NEG		// negation--unary minus
//%precedence NEG		// negation--unary minus BISON 3

%right	'^'		// exponentiation
%right	'<' '>' '='	// conditional operators



%%	// The grammar follows.

input:
	// empty
//	%empty	// BISON 3

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
//	%empty	// BISON 3

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



#include "ced_parser_def.cpp"

