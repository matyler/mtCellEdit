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



int Global::dw_decbot ()
{
	mtDW::Butt * const butt = db.get_butt ();
	std::string filename ( s_o );

	filename += "/";
	filename += mtKit::basename ( s_arg );

	mtKit::string_strip_extension ( filename, NULL );

	if ( i_verbose )
	{
		std::cout << s_arg << " -> " << filename << "\n";
	}

	if ( mtDW::Tap::multi_decode ( butt, s_arg, filename.c_str () ) )
	{
		return ERROR_BOTTLE_DECODE;
	}

	return 0;
}

int Global::dw_encbot ()
{
	if ( ! s_bottle )
	{
		return ERROR_BOTTLE_MISSING;
	}

	mtDW::Well * const well = db.get_well ();
	mtDW::Butt * const butt = db.get_butt ();
	mtDW::Soda * const soda = db.get_soda ();
	mtDW::Tap const * const tap = db.get_tap ();

	std::string filename ( s_o );

	filename += "/";
	filename += mtKit::basename ( s_arg );

	char const * const pos = strrchr ( s_bottle, '.' );
	char const * const sep = strrchr ( s_bottle, MTKIT_DIR_SEP );

	if ( pos > sep )
	{
		filename += pos;
	}
	else
	{
		filename += ".bottle";
	}

	if ( i_verbose )
	{
		std::cout << s_arg << " -> " << filename << "\n";
	}

	if ( tap->encode ( well, butt, soda, s_bottle, s_arg, filename.c_str()))
	{
		return ERROR_BOTTLE_ENCODE;
	}

	return 0;
}

