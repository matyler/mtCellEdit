/*
	Copyright (C) 2016-2022 Mark Tyler

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

#include "cli.h"



int main (
	int		const	argc,
	char	const * const	argv[]
	)
{
	Backend		backend;

	if ( 0 == backend.command_line ( argc, argv ) )
	{
		backend.main_loop ();
	}

	return backend.exit.value ();
}

