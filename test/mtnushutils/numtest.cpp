/*
	Copyright (C) 2023 Mark Tyler

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

#include "numtest.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Backend backend;

	if ( backend.command_line ( argc, argv ) )
	{
		return backend.exit.value ();
	}

	backend.test_rational ();

	return backend.exit.value ();
}

int Backend::command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtKit::Arg args ( [this]( char const * const filename )
		{
			std::cout << "filename=" << filename << "\n";
			return 0;
		}
		);

	args.parse ( argc, argv );

	return 0;			// Continue
}

void Backend::test_rational ()
{
	puts ( "Rational tests" );

	char const * text[] = {
		// Valid
		"0"
		, "0000"
		, "5"
		, "-5"
		, "1/5"
		, "1 / 5"
		, "1/-5"
		, "-1/-5"
		, "0/5"
		, "10/5"
		, "1.1"
		, "001.10000"
		, "000.00001"
		, ".00001"
		, "1e1"
		, "-1.25e-5"
		, "-1.25e0"
		, "-1.25e5"

		// Invalid
		, "1/0"
		, "1/1/1"
		, "1e1e1"
		, "1e"
		, "-1-1"
		, "1.1.1"
		, "1e1.1"

		, nullptr
		};

	for ( int i = 0; text[i]; i++ )
	{
		std::string res;

		try
		{
			mtDW::Rational a ( text[i] );

			res = a.to_string ();
		}
		catch (...)
		{
			res = "Exception";
		}

		std::cout << text[i] << "\t->\t" << res << "\n";
	}
}

