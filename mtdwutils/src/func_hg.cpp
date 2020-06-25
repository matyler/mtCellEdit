/*
	Copyright (C) 2020 Mark Tyler

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

#include "private.h"



int dwut_dechg ()
{
	std::string	filename ( global.s_o );

	filename += "/";
	filename += mtKit::basename ( global.s_arg );

	mtKit::string_strip_extension ( filename, "txt" );

	if ( global.i_verbose )
	{
		std::cout << global.s_arg << " -> " << filename << "\n";
	}

	std::string	info;

	if ( global.hg_index.file_decode ( global.s_arg, filename.c_str (),
		info ) )
	{
		std::cerr << info << "\n";
		return ERROR_HG_DECODE;
	}

	return 0;
}

int dwut_enchg ()
{
	if ( ! global.s_bottle )
	{
		return ERROR_BOTTLE_MISSING;
	}

	std::string	filename ( global.s_o );

	filename += "/";
	filename += mtKit::basename ( global.s_arg );
	filename += ".txt";

	if ( global.i_verbose )
	{
		std::cout << global.s_arg << " -> " << filename << "\n";
	}

	std::string	info;
	mtDW::Well * const well = global.db.get_well ();

	if ( global.hg_index.file_encode ( global.s_bottle, global.s_arg,
		filename.c_str (), well, info )
		)
	{
		std::cerr << info << "\n";
		global.exit.set_value ( 1 );
		return ERROR_HG_ENCODE;
	}

	return 0;
}

