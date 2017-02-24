/*
	Copyright (C) 2016 Mark Tyler

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



Backend		backend;



int operation_update (
	int	const	res
	)
{
	if ( 0 == res )
	{
		backend.file().reset_tool_mode ();
	}

	return res;
}

int main (
	int		const	argc,
	char	const * const	argv[]
	)
{
	if ( 1 == backend.command_line ( argc, argv ) )
	{
		return 0;
	}

	backend.main_loop ();

	return backend.m_exit_value;
}

