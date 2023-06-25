/*
	Copyright (C) 2021-2023 Mark Tyler

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

#include "bigman.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mpfr_set_default_prec ( Mandelbrot::NUMBER_PRECISION_BITS );

	Core	core;

	if ( core.command_line ( argc, argv ) )
	{
		return core.exit.value();
	}

	Mainwindow	window ( core );

	return core.exit.value();
}

