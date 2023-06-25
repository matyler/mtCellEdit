/*
	Copyright (C) 2020-2022 Mark Tyler

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

#include "dwutils.h"



int Global::dw_dechg ()
{
	std::string	filename ( s_o );

	filename += "/";
	filename += mtKit::basename ( s_arg );

	mtKit::string_strip_extension ( filename, "txt" );

	if ( i_verbose )
	{
		std::cout << s_arg << " -> " << filename << "\n";
	}

	std::string	info;

	if ( hg_index.file_decode ( s_arg, filename.c_str (), info ) )
	{
		std::cerr << info << "\n";
		return ERROR_HG_DECODE;
	}

	return 0;
}

int Global::dw_enchg ()
{
	if ( ! s_bottle )
	{
		return ERROR_BOTTLE_MISSING;
	}

	std::string	filename ( s_o );

	filename += "/";
	filename += mtKit::basename ( s_arg );
	filename += ".txt";

	if ( i_verbose )
	{
		std::cout << s_arg << " -> " << filename << "\n";
	}

	std::string	info;
	mtDW::Well * const well = db.get_well ();

	if ( hg_index.file_encode ( s_bottle, s_arg, filename.c_str (), well,
		info ) )
	{
		std::cerr << info << "\n";
		exit.set_value ( 1 );
		return ERROR_HG_ENCODE;
	}

	return 0;
}

