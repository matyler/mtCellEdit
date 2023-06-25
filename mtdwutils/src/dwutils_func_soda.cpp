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



int Global::dw_decsoda ()
{
	mtDW::Butt * const butt = db.get_butt ();
	std::string filename ( s_o );

	filename += "/";
	filename += mtKit::basename ( s_arg );

	mtKit::string_strip_extension ( filename, "soda" );

	if ( i_verbose )
	{
		std::cout << s_arg << " -> " << filename << "\n";
	}

	if ( db.get_soda ()->multi_decode ( butt, s_arg, filename.c_str () ) )
	{
		return ERROR_SODA_DECODE;
	}

	return 0;
}

int Global::dw_encsoda ()
{
	mtDW::Butt * const butt = db.get_butt ();
	std::string filename ( s_o );

	filename += "/";
	filename += mtKit::basename ( s_arg );
	filename += ".soda";

	if ( i_verbose )
	{
		std::cout << s_arg << " -> " << filename << "\n";
	}

	if ( db.get_soda()->encode ( butt, s_arg, filename.c_str() ) )
	{
		return ERROR_SODA_ENCODE;
	}

	return 0;
}

