/*
	Copyright (C) 2018-2020 Mark Tyler

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

#include "mkdata.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtKit::Exit	exit;
	mtKit::Random	random;
	char	const *	path = nullptr;

	mtKit::Arg args ( [&path]( char const * const filename )
		{
			path = filename;
			return 0;	// Continue
		} );

	args.parse ( argc, argv );

	random.set_seed_by_time ();

	if ( nullptr != path )
	{
		try
		{
			{ CreatePNG png ( path, random ); }
			{ CreateFLAC flac ( path ); }
		}
		catch ( ... )
		{
			exit.set_value ( 1 );
		}
	}

	return exit.value ();
}

