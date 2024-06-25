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

#ifndef NUSH_MAIN_H_
#define NUSH_MAIN_H_

#undef BIN_NAME
#define BIN_NAME "nushutils"



// Mark Tyler's Number Shell Utilities



#include <mtdatawell_math.h>

#include "nush_numeval.h"



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



class Core;



class Core
{
public:

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

/// ----------------------------------------------------------------------------

enum
{
	MODE_DOUBLE,
	MODE_FLOAT,
	MODE_INTEGER,
	MODE_RATIONAL
};

	mtKit::Exit		exit;

	mtDW::MathState		math;

	NumEval			numeval;

private:
	int evaluate_line ( char const * text );
	int argcb_i ();
	int cli_mode ();
	int print_funcs () const;
	int print_vars () const;

/// ----------------------------------------------------------------------------

	mtKit::CliShell	m_clishell;

	int		m_verbose	= 0;
	int		m_num_size	= 0;
	int		m_num_mode	= MODE_FLOAT;

	char	const * m_arg_i	= nullptr;
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// NUSH_MAIN_H_

