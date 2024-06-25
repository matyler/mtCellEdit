/*
	Copyright (C) 2022-2024 Mark Tyler

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

#ifndef NUSH_NUMEVAL_H_
#define NUSH_NUMEVAL_H_



#include <map>

#include <mtdatawell_math.h>


// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API

class NumEval;



class NumEval
{
public:
	void set_number_size ( int size );
	enum
	{
		// Bits used for the number: 2^n where MIN <= n <= MAX
		NUMBER_SIZE_MIN		= 10,
		NUMBER_SIZE_MAX		= 32
	};

	int evaluate_double ( char const * text );
	int evaluate_float ( char const * text );
	int evaluate_integer ( char const * text );
	int evaluate_rational ( char const * text );
		// ERROR_*
		// ERROR_NONE => result set

	std::string const & error () const { return m_error; }
	int error_pos () const { return m_error_pos; }

	mpfr_prec_t num_bits () const { return m_num_bits; }

	mtDW::Double const & result_double () const
		{ return m_parser_double.result(); }

	mtDW::Float const & result_float () const
		{ return m_parser_float.result(); }

	mtDW::Integer const & result_integer () const
		{ return m_parser_integer.result(); }

	mtDW::Rational const & result_rational () const
		{ return m_parser_rational.result(); }

	int print_double_funcs () const;
	int print_double_vars () const;

	int print_float_funcs () const;
	int print_float_vars () const;

	int print_integer_funcs () const;
	int print_integer_vars () const;

	int print_rational_funcs () const;
	int print_rational_vars () const;

private:

template < typename Tparser >
int evaluate (
	char	const * const	text,
	Tparser			& parser
	)
{
	int const e = parser.evaluate ( text ? text : "" );

	if ( e )
	{
		m_error_pos = parser.error_pos ();

		m_error = text ? text : "";
		m_error += '\n';

		for ( int i = 0; i < m_error_pos; i++ )
		{
			m_error += " ";
		}

		m_error += "^\n";
		m_error += mtDW::get_error_text ( e );
		m_error += "\n";
	}

	return e;
}

/// ----------------------------------------------------------------------------

	std::string	m_error;		// Error message
	int		m_error_pos = 0;	// Position in "text" of error

	mpfr_prec_t	m_num_bits = 1 << NUMBER_SIZE_MIN;

	mtDW::DoubleParser	m_parser_double;
	mtDW::FloatParser	m_parser_float;
	mtDW::IntegerParser	m_parser_integer;
	mtDW::RationalParser	m_parser_rational;
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// NUSH_NUMEVAL_H_

