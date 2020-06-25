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



int dwut_decbot ()
{
	mtDW::Butt * const butt = global.db.get_butt ();
	std::string filename ( global.s_o );

	filename += "/";
	filename += mtKit::basename ( global.s_arg );

	mtKit::string_strip_extension ( filename, NULL );

	if ( global.i_verbose )
	{
		std::cout << global.s_arg << " -> " << filename << "\n";
	}

	if ( mtDW::Tap::multi_decode ( butt, global.s_arg, filename.c_str () ) )
	{
		return ERROR_BOTTLE_DECODE;
	}

	return 0;
}

int dwut_encbot ()
{
	if ( ! global.s_bottle )
	{
		return ERROR_BOTTLE_MISSING;
	}

	mtDW::Well * const well = global.db.get_well ();
	mtDW::Butt * const butt = global.db.get_butt ();
	mtDW::Soda * const soda = global.db.get_soda ();
	mtDW::Tap const * const tap = global.db.get_tap ();

	std::string filename ( global.s_o );

	filename += "/";
	filename += mtKit::basename ( global.s_arg );

	char const * const pos = strrchr ( global.s_bottle, '.' );
	char const * const sep = strrchr ( global.s_bottle, MTKIT_DIR_SEP );

	if ( pos > sep )
	{
		filename += pos;
	}
	else
	{
		filename += ".bottle";
	}

	if ( global.i_verbose )
	{
		std::cout << global.s_arg << " -> " << filename << "\n";
	}

	if ( tap->encode ( well, butt, soda, global.s_bottle, global.s_arg,
		filename.c_str () )
		)
	{
		return ERROR_BOTTLE_ENCODE;
	}

	return 0;
}

