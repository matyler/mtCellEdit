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

#include "math_double_funcs.h"



namespace FUN = mtDW::DoubleFunc;



void mtDW::DoubleGrammar::parse_nt_func (
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

int mtDW::DoubleParser::get_function_data (
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

void FUN::fn_acos ( FUN::ArgData & arg )
{
	arg[0].acos ( arg[0] );
}

void FUN::fn_asin ( FUN::ArgData & arg )
{
	arg[0].asin ( arg[0] );
}

void FUN::fn_atan ( FUN::ArgData & arg )
{
	arg[0].atan ( arg[0] );
}

void FUN::fn_atan2 ( FUN::ArgData & arg )
{
	arg[0].atan2 ( arg[0], arg[1] );
}

void FUN::fn_ceil ( FUN::ArgData & arg )
{
	arg[0].ceil ( arg[0] );
}

void FUN::fn_cos ( FUN::ArgData & arg )
{
	arg[0].cos ( arg[0] );
}

void FUN::fn_cot ( FUN::ArgData & arg )
{
	arg[0].cot ( arg[0] );
}

void FUN::fn_csc ( FUN::ArgData & arg )
{
	arg[0].csc ( arg[0] );
}

void FUN::fn_exp ( FUN::ArgData & arg )
{
	arg[0].exp ( arg[0] );
}

void FUN::fn_floor ( FUN::ArgData & arg )
{
	arg[0].floor ( arg[0] );
}

void FUN::fn_frac ( FUN::ArgData & arg )
{
	arg[0].frac ( arg[0] );
}

void FUN::fn_is_inf ( FUN::ArgData & arg )
{
	arg[0].set_number ( arg[0].is_inf () ? 1 : 0 );
}

void FUN::fn_is_nan ( FUN::ArgData & arg )
{
	arg[0].set_number ( arg[0].is_nan () ? 1 : 0 );
}

void FUN::fn_is_number ( FUN::ArgData & arg )
{
	arg[0].set_number ( arg[0].is_number () ? 1 : 0 );
}

void FUN::fn_log ( FUN::ArgData & arg )
{
	arg[0].log ( arg[0] );
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

void FUN::fn_pi ( FUN::ArgData & arg )
{
	arg.vec().emplace_back();
	arg.vec().back().pi ();
}

void FUN::fn_round ( FUN::ArgData & arg )
{
	arg[0].round ( arg[0] );
}

void FUN::fn_sec ( FUN::ArgData & arg )
{
	arg[0].sec ( arg[0] );
}

void FUN::fn_sin ( FUN::ArgData & arg )
{
	arg[0].sin ( arg[0] );
}

void FUN::fn_trunc ( FUN::ArgData & arg )
{
	arg[0].trunc ( arg[0] );
}

