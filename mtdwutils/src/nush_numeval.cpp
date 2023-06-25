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

#include "nush_numeval.h"



int NumEval::evaluate_float (
	char	const * const	text
	)
{
	mpfr_set_default_prec ( m_num_bits );

	return evaluate ( text, m_parser_float );
}

int NumEval::evaluate_integer (
	char	const * const	text
	)
{
	return evaluate ( text, m_parser_integer );
}

int NumEval::evaluate_rational (
	char	const * const	text
	)
{
	return evaluate ( text, m_parser_rational );
}

void NumEval::set_number_size ( int const size )
{
	int const sz = mtkit_int_bound( size, NUMBER_SIZE_MIN, NUMBER_SIZE_MAX);
	m_num_bits = (mpfr_prec_t(1)) << sz;

	// This is needed so the result is set to the new precision for
	// printing the significant digits
	mpfr_set_default_prec ( m_num_bits );
	m_parser_float.evaluate ( "0" );
}



template < typename Tparser >
int print_funcs ( Tparser & parser )
{
	std::map<std::string, std::string> map;
	char	const	* name;
	std::string	help;

	for ( int i = 0; ; i++ )
	{
		if ( parser.get_function_data ( i, &name, help ) )
		{
			break;
		}

		map[ name ] = help;
	}

	for ( auto && it : map )
	{
		std::cout << it.first << " " << it.second << "\n";
	}

	return 0;
}

template < typename Tparser >
int print_vars ( Tparser & parser )
{
	auto const & vars = parser.variables ();

	for ( auto && it : vars )
	{
		std::cout << it.first << " = " << it.second.to_string() << "\n";
	}

	return 0;
}

int NumEval::print_float_funcs () const
{
	return print_funcs ( m_parser_float );
}

int NumEval::print_float_vars () const
{
	return print_vars ( m_parser_float );
}

int NumEval::print_integer_funcs () const
{
	return print_funcs ( m_parser_integer );
}

int NumEval::print_integer_vars () const
{
	return print_vars ( m_parser_integer );
}

int NumEval::print_rational_funcs () const
{
	return print_funcs ( m_parser_rational );
}

int NumEval::print_rational_vars () const
{
	return print_vars ( m_parser_rational );
}

