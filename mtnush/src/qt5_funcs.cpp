/*
	Copyright (C) 2022-2024 Mark Tyler

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

#include "qt5.h"



template < typename Tparser >
void show_funcs (
	MainWindow	* const	parent,
	char	const * const	tname,
	Tparser			& parser
	)
{
	std::map<std::string, std::string> map;
	char	const	* name;
	std::string	result;
	std::string	help;

	for ( int i = 0; ; i++ )
	{
		if ( parser.get_function_data ( i, &name, help ) )
		{
			break;
		}

		map[ name ] = help;
	}

	for ( auto && it : map )
	{
		result += it.first;
		result += " ";
		result += it.second;
		result += "\n";
	}

	mtQEX::DialogAbout dialog ( parent, VERSION );
	std::string title ( tname );
	title += " Functions";

	dialog.add_info ( title.c_str(), result.c_str() );

	dialog.exec ();
}



void MainWindow::press_show_float_funcs ()
{
	show_funcs ( this, "Float", m_parser_float );
}

void MainWindow::press_show_double_funcs ()
{
	show_funcs ( this, "Double", m_parser_double );
}

void MainWindow::press_show_int_funcs ()
{
	show_funcs ( this, "Integer", m_parser_integer );
}

void MainWindow::press_show_rational_funcs ()
{
	show_funcs ( this, "Rational", m_parser_rational );
}

