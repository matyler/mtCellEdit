/*
	Copyright (C) 2022-2023 Mark Tyler

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

#ifndef MATH_GRAMMAR_H_
#define MATH_GRAMMAR_H_

#include "mtdatawell_math.h"



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

/*
input:
	<EMPTY>			# Returns 0 value
	| exp			# Returns exp value
	| exp [; exp]... [;]	# Returns rightmost exp value

exp:
	NUM
	| func
	| var			# Must exist, else error
	| var = exp		# Always evaluate the RHS first: a = b = c = 1
	| var += exp		# Must exist, else error. Eval RHS 1st
	| var -= exp		# Must exist, else error. Eval RHS 1st
	| var *= exp		# Must exist, else error. Eval RHS 1st
	| var /= exp		# Must exist, else error. Eval RHS 1st
	| var ^= exp		# Must exist, else error. Eval RHS 1st

NOTE: precedence here is determined by looking ahead by one token, such as:
2 + 2 * 3
* has a higher precedence than +, so the RHS is evaluated first via recursion.

	Comparisons return 1=True, 0=False (-1, 0, 1 for <=>)

	| exp < exp		# precedence = -1, associativity = left
	| exp <= exp		# precedence = -1, associativity = left
	| exp > exp		# precedence = -1, associativity = left
	| exp >= exp		# precedence = -1, associativity = left
	| exp == exp		# precedence = -1, associativity = left
	| exp != exp		# precedence = -1, associativity = left
	| exp <=> exp		# precedence = -1, associativity = left

	| exp + exp		# precedence = 1, associativity = left
	| exp - exp		# precedence = 1, associativity = left
	| exp * exp		# precedence = 2, associativity = left
	| exp / exp		# precedence = 2, associativity = left
	| exp ^ exp		# precedence = 3, associativity = right

	| - exp			# -2^2 = (-2)^2; 0-2^2 = 0 - (2^2)
	| ( exp )
	| -( exp )

var:
	STRING

func:
	STRING ()
	| STRING ( argset )

argset:
	exp [, exp]...
*/



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API



namespace mtDW
{



class IntegerGrammar;
class FloatGrammar;
class RationalGrammar;

template < typename Tnum > class FuncArgData;
template < typename Tnum > class Grammar;



void get_math_function_data (
	std::string	& dest,
	int		tot,
	char	const	* help
	);



template < typename Tnum > class FuncArgData
{
public:
	FuncArgData ( std::vector< Tnum > & a, size_t const argtot )
		:
		m_args		( a ),
		m_argtot	( argtot ),
		m_arg_ptr	( a.size() > 0 ?
					& a[ a.size() - argtot ]
					: nullptr
					)
	{
	}

	~FuncArgData ()
	{
		if ( m_argtot > 1 && m_args.size() >= m_argtot )
		{
			for ( auto i = m_argtot - 1; i > 0; --i )
			{
				m_args.pop_back ();
			}
		}
	}

	Tnum & operator [] ( size_t i )	{ return m_arg_ptr[i]; }
	std::vector< Tnum > & vec()	{ return m_args; }

/* NOTE:
	A function takes a number of args from the end of the vector.
	On completing a function, the result is at the vector with all of the
	other excess args being 'popped' off.
*/

private:
	std::vector< Tnum >	& m_args;
	size_t		const	m_argtot;
	Tnum		* const m_arg_ptr;
};



template < typename Tnum > class Grammar
{
public:
	enum
	{
		// Precedence - higher is done first, from left to right
		GR_PREC_LOWEST		= -999,
		GR_ASSIGN_PREC		= -10,
		GR_CMP_PREC		= -1,
		GR_OP_ADD_PREC		= 1,
		GR_OP_SUB_PREC		= 1,
		GR_OP_MULT_PREC		= 2,
		GR_OP_DIV_PREC		= 2,
		GR_OP_POWER_PREC	= 3
	};



	Grammar (
		Lexer		& lexer,
		std::map<std::string, Tnum> & variables
		)
		:
		m_lexer		( lexer ),
		m_vars		( variables )
	{
	}

	virtual ~Grammar () {}

	// Parsing throws on fail (possibly setting m_error)
	void parse ()
	{
		m_nt_num.clear ();

		parse_nt_input ();
	}

	Tnum & parse_result ()
	{
		if ( m_nt_num.size() < 1 )
		{
			throw 123;
		}

		return m_nt_num.back();
	}

	int error () const { return m_error; }

protected:
/*	Recursive descent parser: parse_nt_* throw on error (setting m_error)

	void parse_nt_input ()
	void parse_nt_exp ()
	void parse_nt_exp_line ()
	void parse_nt_op_exp ()
	void parse_nt_num_var_func ()
	virtual void parse_nt_func (
		std::string	const	& name,
		char		const	* var_token_pos
		) = 0
	void parse_nt_argset ( size_t const argtot )
	void parse_nt_var (
		std::string	const	& name,
		char	const * const	var_token_pos
		)
*/

	/// EXPECT: empty, <exp>, or <exp>;<exp>
	void parse_nt_input ()
	{
		if ( m_lexer.scan_token() == Lexer::TK_END )
		{
			m_nt_num.emplace_back ( "0" );
			return;
		}

		m_lexer.rewind_token ();

		while (1)
		{
			parse_nt_exp_line ();

			switch ( m_lexer.scan_token() )
			{
			case Lexer::TK_END:
				return;

			case Lexer::TK_EXP_SEP:
				if ( Lexer::TK_END == m_lexer.scan_token() )
				{
					// ; is the last token of the input
					return;
				}
				else
				{
					m_lexer.rewind_token ();
				}

				// More to parse

				m_nt_num.pop_back();	// ; = lose left result

				// Next item must be the start of a new exp_line
				continue;
			}

			m_lexer.rewind_token ();
		}
	}

	/// EXPECT: expression line, with resulting number added to m_nt_num
	void parse_nt_exp_line ()
	{
		parse_nt_exp ();

		while (1)
		{
			switch ( m_lexer.scan_token() )
			{
			case Lexer::TK_END:
				return;

			case Lexer::TK_EXP_SEP:
			case Lexer::TK_ARG_SEP:
			case Lexer::TK_PAREN_CLOSE:
				// Caller deals with this
				m_lexer.rewind_token ();
				return;

			default:
				// <op> <exp> is only remaining option
				m_lexer.rewind_token ();
				parse_nt_op_exp ();
				break;
			}
		}
	}

	/// EXPECT: single expression, with resulting number added to m_nt_num
	void parse_nt_exp ()
	{
		int paren;
		int negate;
		int token = m_lexer.scan_token ();

		switch ( token )
		{
		case Lexer::TK_OP_SUB:
			{
				char const * pos = m_lexer.token_position ();

				if ( m_lexer.scan_token() == Lexer::TK_PAREN_OPEN )
				{
					paren = 1;
					negate = 1;
				}
				else
				{
					paren = 0;
					negate = 0;
					m_lexer.set_token_position ( pos );
				}
			}
			break;

		case Lexer::TK_PAREN_OPEN:
			paren = 1;
			negate = 0;
			break;

		default:
			paren = 0;
			negate = 0;
			m_lexer.rewind_token ();
			break;
		}

		// If we started with '(', we must finish with ')'
		if ( paren )
		{
			parse_nt_exp_line ();

			token = m_lexer.scan_token();

			switch ( token )
			{
			case Lexer::TK_PAREN_CLOSE:
				if ( negate )
				{
					m_nt_num.back().negate();
				}
				return;

			default:
				m_lexer.rewind_token ();
				m_error = mtDW::ERROR_MISSING_PAREN_CLOSE;
				throw 123;
			}
		}
		else
		{
			// No parentheses, so just a single item
			parse_nt_num_var_func ();
		}
	}

	/// EXPECT: some operation, then an expression with result added to
	/// m_nt_num.  On entry we must have a number on m_nt_num to use.
	void parse_nt_op_exp ()
	{
		/// Read in op1 (exp1 <op1> exp2 [<op2>])

		int const op1 = m_lexer.scan_token();
		int prec1 = GR_PREC_LOWEST;
		int associativity = 0;

		switch ( op1 )
		{
		case Lexer::TK_OP_ADD:
			prec1 = GR_OP_ADD_PREC;
			break;

		case Lexer::TK_OP_SUB:
			prec1 = GR_OP_SUB_PREC;
			break;

		case Lexer::TK_OP_MULT:
			prec1 = GR_OP_MULT_PREC;
			break;

		case Lexer::TK_OP_DIV:
			prec1 = GR_OP_DIV_PREC;
			break;

		case Lexer::TK_OP_POWER:
			prec1 = GR_OP_POWER_PREC;
			associativity = 1;
			break;

		case Lexer::TK_CMP_LT:
		case Lexer::TK_CMP_LTE:
		case Lexer::TK_CMP_GT:
		case Lexer::TK_CMP_GTE:
		case Lexer::TK_CMP_EQ:
		case Lexer::TK_CMP_NEQ:
		case Lexer::TK_CMP_LEG:
			prec1 = GR_CMP_PREC;
			break;

		case Lexer::TK_BAD_SYMBOL:
			m_lexer.rewind_token ();
			m_error = mtDW::ERROR_BAD_SYMBOL;
			throw 123;

		default:
			m_lexer.rewind_token ();
			m_error = mtDW::ERROR_BAD_SYNTAX;
			throw 123;
		}

		char const * const pos_n2 = m_lexer.token_position ();

		/// Read in exp2 (exp1 <op1> exp2 [<op2>])
		parse_nt_exp ();

/*
2 + 2 + xxx		Prec A = Prec B : Rewind, do 2+2, pop, and continue within this function (no nesting)
2 + 2 ^ xxx		Prec A < Prec B : Rewind, nest to new exp
2 ^ 2 + xxx		Prec A > Prec B : Rewind, do 2^2, pop, and continue within this function (no nesting)

^ or = then we nest to the right due to right associativity.  + - * / then we do left to right in sequence if equal (left associativity).
a = b = 3			Prec A = Prec B : Rewind, nest to new exp

-2 ^ (xxx)		Get number, negate it, nest to new exp at (

*/

eval_tk2:
		/// Read in op2 (exp1 <op1> exp2 [<op2>]) to resolve precedence

		int const op2 = m_lexer.scan_token();
		int prec2 = GR_PREC_LOWEST;

		switch ( op2 )
		{
		case Lexer::TK_OP_ADD:
			prec2 = GR_OP_ADD_PREC;
			break;

		case Lexer::TK_OP_SUB:
			prec2 = GR_OP_SUB_PREC;
			break;

		case Lexer::TK_OP_MULT:
			prec2 = GR_OP_MULT_PREC;
			break;

		case Lexer::TK_OP_DIV:
			prec2 = GR_OP_DIV_PREC;
			break;

		case Lexer::TK_OP_POWER:
			prec2 = GR_OP_POWER_PREC;
			break;

		case Lexer::TK_CMP_LT:
		case Lexer::TK_CMP_LTE:
		case Lexer::TK_CMP_GT:
		case Lexer::TK_CMP_GTE:
		case Lexer::TK_CMP_EQ:
		case Lexer::TK_CMP_NEQ:
		case Lexer::TK_CMP_LEG:
			prec2 = GR_CMP_PREC;
			break;

		case Lexer::TK_END:
		case Lexer::TK_ARG_SEP:
		case Lexer::TK_EXP_SEP:
		case Lexer::TK_PAREN_CLOSE:
			// Caller deals with these so do op1 and exit
			break;
	
		case Lexer::TK_BAD_SYMBOL:
			m_lexer.rewind_token ();
			m_error = mtDW::ERROR_BAD_SYMBOL;
			throw 123;
	
		default:
			// Unexpected symbol, e.g. (
			m_lexer.rewind_token ();
			m_error = mtDW::ERROR_BAD_SYNTAX;
			throw 123;
		}

		m_lexer.rewind_token();

		// Do RHS operation first if precedence or associativity
		// requires it.
		if (	(prec2 > prec1)
			|| ((prec1 == prec2) && associativity)
			)
		{
			parse_nt_op_exp ();

			// Keep looping until the precedence allows us to
			// continue as normal.
			goto eval_tk2;
		}

		// Do the operation with top 2 numbers on stack
		Tnum & n1 = m_nt_num[ m_nt_num.size() - 2 ];
		Tnum & n2 = m_nt_num.back();
	
		switch ( op1 )
		{
		case Lexer::TK_OP_ADD:	n1 += n2;		break;
		case Lexer::TK_OP_SUB:	n1 -= n2;		break;
		case Lexer::TK_OP_MULT:	n1 *= n2;		break;

		case Lexer::TK_OP_DIV:
			try
			{
				n1 /= n2;
			}
			catch (...)
			{
				m_error = mtDW::ERROR_DIVIDE_BY_ZERO;
				m_lexer.set_token_position ( pos_n2 );
				throw;
			}
			break;

		case Lexer::TK_OP_POWER:
			try
			{
				n1.pow ( n1, n2 );
			}
			catch (...)
			{
				m_error = mtDW::ERROR_BAD_POWER;
				m_lexer.set_token_position ( pos_n2 + 1 );
				throw;
			}
			break;

		case Lexer::TK_CMP_LT:
			if ( n1 < n2 )
			{
				n1.set_number("1");
			}
			else
			{
				n1.set_number("0");
			}
			break;

		case Lexer::TK_CMP_LTE:
			if ( n1 <= n2 )
			{
				n1.set_number("1");
			}
			else
			{
				n1.set_number("0");
			}
			break;

		case Lexer::TK_CMP_GT:
			if ( n1 > n2 )
			{
				n1.set_number("1");
			}
			else
			{
				n1.set_number("0");
			}
			break;

		case Lexer::TK_CMP_GTE:
			if ( n1 >= n2 )
			{
				n1.set_number("1");
			}
			else
			{
				n1.set_number("0");
			}
			break;

		case Lexer::TK_CMP_EQ:
			if ( n1 == n2 )
			{
				n1.set_number("1");
			}
			else
			{
				n1.set_number("0");
			}
			break;

		case Lexer::TK_CMP_NEQ:
			if ( n1 != n2 )
			{
				n1.set_number("1");
			}
			else
			{
				n1.set_number("0");
			}
			break;

		case Lexer::TK_CMP_LEG:
			{
				int const i = n1.cmp ( n2 );
				if ( i < 0 )
				{
					n1.set_number("-1");
				}
				else if ( i > 0 )
				{
					n1.set_number("1");
				}
				else
				{
					n1.set_number("0");
				}
			}
			break;
		}

		// Remove end item
		m_nt_num.pop_back ();
	}

	/// EXPECT: some num/var/func, with resulting number added to m_nt_num
	void parse_nt_num_var_func ()
	{
		int negate = 0;
		int token = m_lexer.scan_token();

		if ( token == Lexer::TK_OP_SUB )
		{
			negate = 1;
			token = m_lexer.scan_token();
		}

		switch ( token )
		{
		case Lexer::TK_NUM:
			m_nt_num.emplace_back ( m_lexer.string() );
			break;

		case Lexer::TK_STRING:
			{
				std::string const text ( m_lexer.string() );

				char const * const var_token_pos =
					m_lexer.token_position ();

				token = m_lexer.scan_token();
				switch ( token )
				{
				case Lexer::TK_PAREN_OPEN:
					// Read the string as a function so read args
					parse_nt_func ( text, var_token_pos );
					break;

				default:
					m_lexer.rewind_token();
					// Read the string as a variable
					parse_nt_var ( text, var_token_pos );
					break;
				}
			}
			break;

		case Lexer::TK_BAD_SYMBOL:
			m_lexer.rewind_token ();
			m_error = mtDW::ERROR_BAD_SYMBOL;
			throw 123;

		case Lexer::TK_BAD_NUMBER:
			m_lexer.rewind_token ();
			m_error = mtDW::ERROR_BAD_NUMBER;
			throw 123;

		default:
			// Anything else must be an error
			m_lexer.rewind_token();
			m_error = mtDW::ERROR_BAD_SYNTAX;
			throw 123;
		}

		if ( negate )
		{
			m_nt_num.back().negate();
		}
	}

	/// EXPECT: args separated by "," putting result onto m_nt_num
	virtual void parse_nt_func (
		std::string	const	& name,
		char		const	* var_token_pos
		) = 0;

	/// EXPECT: [exp]["," exp]...")"
	void parse_nt_argset ( size_t const argtot )
	{
		size_t argc = 0;
		int token = m_lexer.scan_token();

		switch ( token )
		{
		case Lexer::TK_END:
			m_error = mtDW::ERROR_BAD_SYNTAX;
			throw 123;

		case Lexer::TK_PAREN_CLOSE:
			// Zero args
			goto finish;

		default:
			m_lexer.rewind_token ();

			if ( 0 == argtot )
			{
				m_error = mtDW::ERROR_FUNCTION_ARGS_TOO_MANY;
				throw 123;
			}

			while ( 1 )
			{
				parse_nt_exp_line();

				argc++;

				token = m_lexer.scan_token();
				switch ( token )
				{
				case Lexer::TK_PAREN_CLOSE:
					// No more arguments coming
					goto finish;

				case Lexer::TK_ARG_SEP:
					// Another arg should be coming
					if ( argc < argtot )
					{
						break;
					}

					m_lexer.rewind_token ();
					m_error = mtDW::ERROR_FUNCTION_ARGS_TOO_MANY;
					throw 123;

				default:
					m_lexer.rewind_token ();
					m_error = mtDW::ERROR_BAD_SYNTAX;
					throw 123;
				}
			}
		}

finish:
		if ( argc < argtot )
		{
			m_lexer.rewind_token ();
			m_error = mtDW::ERROR_FUNCTION_ARGS_TOO_FEW;
			throw 123;
		}

		// Success
	}

	/// EXPECT: [?=exp] - put variable value onto m_nt_num, else exp on RHS
	void parse_nt_var (
		std::string	const	& name,
		char	const * const	var_token_pos
		)
	{
		int const token = m_lexer.scan_token();

		switch ( token )
		{
		case Lexer::TK_ASSIGN:
			// Parse exp's to the right, get the value
			parse_nt_exp_line ();

			m_vars[ name ] = m_nt_num.back();

			return;

		case Lexer::TK_ASSIGN_ADD:
		case Lexer::TK_ASSIGN_SUB:
		case Lexer::TK_ASSIGN_MULT:
		case Lexer::TK_ASSIGN_DIV:
		case Lexer::TK_ASSIGN_POWER:
			// Find variable and use the exp value
			break;

		default:
			// Use variable as a number, so let caller deal with this token
			m_lexer.rewind_token();
			break;
		}

		auto const it = m_vars.find ( m_lexer.string() );

		if ( it == m_vars.end() )
		{
			m_lexer.set_token_position ( var_token_pos );
			m_error = mtDW::ERROR_VARIABLE_UNDEFINED;

			throw 123;
		}

		switch ( token )
		{
		case Lexer::TK_ASSIGN_ADD:
			// Parse exp's to the right, get the value
			parse_nt_exp_line ();

			it->second += m_nt_num.back();
			m_nt_num.back() = it->second;
			break;

		case Lexer::TK_ASSIGN_SUB:
			// Parse exp's to the right, get the value
			parse_nt_exp_line ();

			it->second -= m_nt_num.back();
			m_nt_num.back() = it->second;
			break;

		case Lexer::TK_ASSIGN_MULT:
			// Parse exp's to the right, get the value
			parse_nt_exp_line ();

			it->second *= m_nt_num.back();
			m_nt_num.back() = it->second;
			break;

		case Lexer::TK_ASSIGN_DIV:
			// Parse exp's to the right, get the value
			parse_nt_exp_line ();

			it->second /= m_nt_num.back();
			m_nt_num.back() = it->second;
			break;

		case Lexer::TK_ASSIGN_POWER:
			// Parse exp's to the right, get the value
			parse_nt_exp_line ();

			try
			{
				it->second.pow ( it->second, m_nt_num.back() );
			}
			catch (...)
			{
				m_error = mtDW::ERROR_BAD_POWER;
				m_lexer.set_token_position ( var_token_pos + 1);
				throw;
			}

			m_nt_num.back() = it->second;
			break;

		default:
			// Use variable as a number
			m_nt_num.push_back ( it->second );
			break;
		}
	}

/// ----------------------------------------------------------------------------

	// Always cleared when calling parse();
	std::vector< Tnum >	m_nt_num;

	// Internal state
	int			m_error = 0;

	// From outside
	Lexer			& m_lexer;
	std::map<std::string, Tnum> & m_vars;
};



class IntegerGrammar : public Grammar< Integer >
{
public:
	IntegerGrammar (
		IntegerLexer	& lexer,
		std::map<std::string, Integer> & variables
		)
		:
		Grammar ( lexer, variables )
	{
	}

private:
	void parse_nt_func (
		std::string	const	& name,
		char		const	* var_token_pos
		) override;
};



class FloatGrammar : public Grammar< Float >
{
public:
	FloatGrammar (
		FloatLexer	& lexer,
		std::map<std::string, Float> & variables
		)
		:
		Grammar ( lexer, variables )
	{
	}

private:
	void parse_nt_func (
		std::string	const	& name,
		char		const	* var_token_pos
		) override;
};



class RationalGrammar : public Grammar< Rational >
{
public:
	RationalGrammar (
		RationalLexer	& lexer,
		std::map<std::string, Rational> & variables
		)
		:
		Grammar ( lexer, variables )
	{
	}

private:
	void parse_nt_func (
		std::string	const	& name,
		char		const	* var_token_pos
		) override;
};



}	// namespace mtDW



#endif		// C++ API



#endif		// MATH_GRAMMAR_H_

