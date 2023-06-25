/*
	Copyright (C) 2022 Mark Tyler

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

#include <string.h>
#include <math.h>
#include <limits.h>

#include <mtdatawell_math.h>



class Backend
{
public:
	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program, returning exit.value()

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

private:

	int import_file ( char const * filename );
	int export_file ( char const * filename ) const;
	int mem_file ( char const * filename );
	int cmp_file ( char const * filename );
	int himp_file ( char const * filename );
	int print_file ( char const * filename );

	int evaluate ( char const * text );

/// ----------------------------------------------------------------------------

	mtDW::MathState		m_math;
	mtDW::IntegerMemory	m_imem;
	mtDW::IntegerParser	m_parser_int;
};

