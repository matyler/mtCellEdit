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

#include "math_integer_funcs.h"



namespace FUN = mtDW::IntegerFunc;



void mtDW::IntegerGrammar::parse_nt_func (
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

	fd->func ( arg_data );
}

int mtDW::IntegerParser::get_function_data (
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



void FUN::fn_abs ( FUN::ArgData & arg )
{
	arg[0].abs ();
}

void FUN::fn_bit_and ( FUN::ArgData & arg )
{
	arg[0].bit_and ( arg[0], arg[1] );
}

void FUN::fn_bit_not ( FUN::ArgData & arg )
{
	arg[0].bit_not ();
}

void FUN::fn_bit_or ( FUN::ArgData & arg )
{
	arg[0].bit_or ( arg[0], arg[1] );
}

void FUN::fn_bit_xor ( FUN::ArgData & arg )
{
	arg[0].bit_xor ( arg[0], arg[1] );
}

void FUN::fn_factorial ( FUN::ArgData & arg )
{
	arg[0].factorial ( arg[0] );
}

void FUN::fn_gcd ( FUN::ArgData & arg )
{
	arg[0].gcd ( arg[0], arg[1] );
}

void FUN::fn_lcm ( FUN::ArgData & arg )
{
	arg[0].lcm ( arg[0], arg[1] );
}

void FUN::fn_max ( FUN::ArgData & arg )
{
	arg[0].max ( arg[0], arg[1] );
}

void FUN::fn_min ( FUN::ArgData & arg )
{
	arg[0].min ( arg[0], arg[1] );
}

void FUN::fn_mod ( FUN::ArgData & arg )
{
	arg[0].mod ( arg[0], arg[1] );
}

