/*
	Copyright (C) 2024 Mark Tyler

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

#include "wasp_qt.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	CommandLine	cline;

	if ( cline.parse ( argc, argv ) || cline.exit.aborted () )
	{
		return cline.exit.value();
	}

	try
	{
		// I don't want Qt snooping or changing my command line.
		int	dummy_argc	= 1;
		char	dummy_str[1]	= { 0 };
		char	* dummy_argv	= dummy_str;

		QApplication	app ( dummy_argc, &dummy_argv );
		MainWindow	window ( cline );

		cline.exit.set_value ( app.exec () );
	}
	catch (...)
	{
		std::cerr << BIN_NAME " Error: top level exception\n";

		cline.exit.set_value ( 1 );
	}

	return cline.exit.value();
}

