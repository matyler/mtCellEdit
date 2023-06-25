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

#include "math_grammar.h"



int mtDW::FloatParser::evaluate_internal ( char const * const text )
{
	FloatGrammar grammar ( m_lexer, m_variables );

	try
	{
		// We must create a new number 0 to use the current precision
		Float tmp ( "0" );
		m_result.swap ( tmp );

		m_lexer.init ( text );
		grammar.parse ();

		// We are here, so no errors in the grammar caused a exception
		m_result = grammar.parse_result ();
	}
	catch (...)
	{
		int const error = grammar.error();

		return error ? error : mtDW::ERROR_EXCEPTION;
	}

	return 0;
}



/// ----------------------------------------------------------------------------



int mtDW::IntegerParser::evaluate_internal ( char const * const text )
{
	IntegerGrammar grammar ( m_lexer, m_variables );

	try
	{
		m_result.set_number ( (signed long int)0 );

		m_lexer.init ( text );
		grammar.parse ();

		// We are here, so no errors in the grammar caused a exception
		m_result = grammar.parse_result ();
	}
	catch (...)
	{
		int const error = grammar.error();

		return error ? error : mtDW::ERROR_EXCEPTION;
	}

	return 0;
}



/// ----------------------------------------------------------------------------



int mtDW::RationalParser::evaluate_internal ( char const * const text )
{
	RationalGrammar grammar ( m_lexer, m_variables );

	try
	{
		m_result.set_number ( (signed long int)0 );

		m_lexer.init ( text );
		grammar.parse ();

		// We are here, so no errors in the grammar caused a exception
		m_result = grammar.parse_result ();
	}
	catch (...)
	{
		int const error = grammar.error();

		return error ? error : mtDW::ERROR_EXCEPTION;
	}

	return 0;
}



/// ----------------------------------------------------------------------------



void mtDW::get_math_function_data (
	std::string		& dest,
	int		const	tot,
	char	const * const	help
	)
{
	dest = '(';

	if ( tot > 0 )
	{
		dest += " a1";

		for ( int i = 2; i <= tot; i++ )
		{
			char buf[16];
			snprintf ( buf, sizeof(buf), ", a%i", i );

			dest += buf;
		}

		dest += ' ';
	}

	dest += ") - ";
	dest += help;
}

