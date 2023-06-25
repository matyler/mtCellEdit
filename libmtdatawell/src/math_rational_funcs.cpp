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

#include "math_rational_funcs.h"



namespace FUN = mtDW::RationalFunc;



void mtDW::RationalGrammar::parse_nt_func (
	std::string	const	& name,
	char	const * const	var_token_pos
	)
{
	FUN::func_def_t const * fd = FUN::Function::lookup_name ( name.c_str(),
		name.size() );

	if ( ! fd )
	{
		m_lexer.set_token_position ( var_token_pos );
		m_error = mtDW::ERROR_FUNCTION_UNKNOWN;
		throw 123;
	}

	parse_nt_argset ( fd->arg_tot );

	FUN::ArgData arg_data ( m_nt_num, fd->arg_tot );

	m_error = fd->func ( arg_data );
	if ( m_error )
	{
		m_lexer.set_token_position ( var_token_pos );
		throw 123;
	}
}

int mtDW::RationalParser::get_function_data (
	int		const	index,
	char	const ** const	name,
	std::string		& help
	) const
{
	if ( index < 0 )
	{
		return TOTAL_KEYWORDS;
	}

	if ( index >= TOTAL_KEYWORDS )
	{
		return 1;	// No such function
	}

	FUN::func_def_t const * const fd = &FUN::function_name_list[ index ];

	*name = fd->name;

	get_math_function_data ( help, (int)fd->arg_tot, fd->help );

	return 0;		// Function exists
}



/// ----------------------------------------------------------------------------



int FUN::fn_abs ( FUN::ArgData & arg )
{
	arg[0].abs ();

	return 0;
}

int FUN::fn_denom ( FUN::ArgData & arg )
{
	arg[0].denom ();

	return 0;
}

int FUN::fn_inv ( FUN::ArgData & arg )
{
	try
	{
		arg[0].inv ();
	}
	catch (...)
	{
		return mtDW::ERROR_DIVIDE_BY_ZERO;
	}

	return 0;
}

int FUN::fn_max ( FUN::ArgData & arg )
{
	arg[0].max ( arg[0], arg[1] );

	return 0;
}

int FUN::fn_min ( FUN::ArgData & arg )
{
	arg[0].min ( arg[0], arg[1] );

	return 0;
}

int FUN::fn_numer ( FUN::ArgData & arg )
{
	arg[0].numer ();

	return 0;
}



/// ----------------------------------------------------------------------------

static void throw_bad_number (
	char	const * const	input,
	char	const * const	error
	)
{
	std::cerr << "Bad rational number (" << error << ") : " << input
		<< "\n";

	throw 123;
}

void mtDW::Rational::parse_rational_number ( char const * const text )
{
/* Parse hybrid rational/scientific notation, or any useful variant such as:

	-- mpq parser --
	{Integer}
	{Integer}/{Integer}

	-- scientific parser --
	{Integer}.{Integer}
	{Integer}.{Integer}E{Integer}
	{Integer}.{Integer}e{Integer}
	etc...
*/

	if ( 0 == mpq_set_str ( m_num, text, 10 ) )
	{
		if ( 0 == mpz_sgn ( mpq_denref ( m_num ) ) )
		{
			throw_bad_number ( text, "divide by zero" );
		}

		mpq_canonicalize ( m_num );

		// "text" is an integer or fraction.
		return;
	}

	// "text" must be a scientific notation (or is an error).

	// Step 1 - establish existence of . or e/E in the string, and length
	char const * tx_point = nullptr;
	char const * tx_exp = nullptr;
	char const * tx_end = text;

	for ( ; *tx_end != 0; tx_end++ )
	{
		switch ( *tx_end )
		{
		case '.':
			if ( tx_point )
			{
				// Only one point is allowed
				throw_bad_number ( text, "multiple ." );
			}

			if ( tx_exp )
			{
				// Must be before tx_exp
				throw_bad_number ( text, ". after exp" );
			}

			tx_point = tx_end;
			continue;

		case 'e':
		case 'E':
			if ( tx_exp )
			{
				// Only one exponent is allowed
				throw_bad_number ( text, "multiple exp" );
			}
			tx_exp = tx_end;
			continue;
		}
	}

	if ( (tx_end - text) > mtDW::Number::EVAL_TEXT_MAX_SIZE )
	{
		throw_bad_number ( text, "input too long" );
	}

	if ( ! tx_exp )
	{
		tx_exp = tx_end;
	}

	if ( ! tx_point )
	{
		tx_point = tx_exp;
	}

	mtDW::Integer i_numerator, i_exp;

	// {i1} . {i2} e {i3}
	auto	const	i1_len = (size_t)(tx_point - text);
	auto		i2_len = (size_t)(tx_exp - tx_point);
	auto	const	i3_len = (size_t)(tx_end - tx_exp);

	signed long int num_exp = 0;

	// Parse s1/s2, s3 if they exist
	std::string str_num;

	if ( i1_len > 0 )
	{
		str_num.append ( text, i1_len );
	}

	// Strip off trailing zeros if any exist to right of decimal point
	while ( i2_len > 1 && tx_point[ i2_len ] == '0' )
	{
		i2_len--;
	}

	// Append together both sides of the dp
	if ( i2_len > 1 )
	{
		str_num.append ( tx_point + 1, i2_len - 1 );
	}

	// Parse as number
	if ( i_numerator.set_number ( str_num ) )
	{
		throw_bad_number ( text, "bad numerator string" );
	}

	if ( i3_len > 0 )
	{
		std::string const s ( tx_exp + 1, i3_len - 1 );

		if ( i_exp.set_number ( s ) )
		{
			throw_bad_number ( text, "parsing exp" );
		}

		if ( ! mpz_fits_slong_p ( i_exp.get_num() ) )
		{
			throw_bad_number ( text, "exp too large 1" );
		}

		num_exp = mpz_get_si ( i_exp.get_num() );

		if (	num_exp > mtDW::Number::RATIONAL_EXP_MAX
			|| num_exp < -mtDW::Number::RATIONAL_EXP_MAX
			)
		{
			throw_bad_number ( text, "exp too large 2" );
		}
	}

	mtDW::Integer const ten ( "10" );

	// Shift exponent by the decimal places used
	if ( i2_len > 1 )
	{
		num_exp -= (signed long int)(i2_len - 1);
	}

	if ( 0 == num_exp )
	{
		this->set_number ( i_numerator );
	}
	else if ( num_exp > 0 )
	{
		mtDW::Integer d;

		mpz_pow_ui ( d.get_num(), ten.get_num(),
			(long unsigned int)num_exp );

		i_numerator *= d;

		this->set_number ( i_numerator );
	}
	else	// num_exp < 0
	{
		mtDW::Integer d;

		mpz_pow_ui ( d.get_num(), ten.get_num(),
			(long unsigned int)(-num_exp) );

		this->set_number ( i_numerator, d );
	}
}

int mtDW::Rational::set_number ( char const * const text )
{
	if ( ! text )
	{
		return 1;
	}

	try
	{
		parse_rational_number ( text );
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}
