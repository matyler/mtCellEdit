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



Global::Global ()
{
	err_table[ ERROR_BOTTLE_MISSING ]	= "No bottle selected";
	err_table[ ERROR_SODA_ENCODE ]		= "Unable to encode soda";
	err_table[ ERROR_SODA_DECODE ]		= "Unable to decode soda";
	err_table[ ERROR_BOTTLE_ENCODE ]	= "Unable to encode bottle";
	err_table[ ERROR_BOTTLE_DECODE ]	= "Unable to decode bottle";
	err_table[ ERROR_DB_OPEN ]		= "Unable to open database";
	err_table[ ERROR_FONT_ENCODE ]		= "Unable to encode font";
	err_table[ ERROR_FONT_DECODE ]		= "Unable to decode font";
	err_table[ ERROR_HG_ENCODE ]		= "Unable to encode homoglyphs";
	err_table[ ERROR_HG_DECODE ]		= "Unable to decode homoglyphs";

	jump_table[ "decbot" ]	= [this](){ return dw_decbot(); };
	jump_table[ "decfont" ]	= [this](){ return dw_decfont(); };
	jump_table[ "dechg" ]	= [this](){ return dw_dechg(); };
	jump_table[ "decsoda" ]	= [this](){ return dw_decsoda(); };
	jump_table[ "encbot" ]	= [this](){ return dw_encbot(); };
	jump_table[ "encfont" ]	= [this](){ return dw_encfont(); };
	jump_table[ "enchg" ]	= [this](){ return dw_enchg(); };
	jump_table[ "encsoda" ]	= [this](){ return dw_encsoda(); };
}

Global::~Global ()
{
}

void Global::print_help ()
{
	if ( m_function )
	{
		printf ( "dw%s - Part of %s\n"
			"For further information consult the man page "
			"dw%s(1) or the mtDataWell Handbook.\n"
			"\n"
			, m_function_name.c_str(), VERSION,
			m_function_name.c_str() );
	}
	else
	{
		printf ( "%s\n\n"
			"For further information consult the man page "
			"%s(1) or the mtDataWell Handbook.\n"
			"\n"
			, VERSION, BIN_NAME );
	}
}

void Global::print_version ()
{
	if ( m_function )
	{
		printf ( "dw%s - Part of %s\n\n", m_function_name.c_str(),
			VERSION );
	}
	else
	{
		printf ( "%s\n\n", VERSION );
	}
}

int Global::parse_com ()
{
	set_function ( s_arg );

	if ( ! m_function )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such command '%s'\n",
			s_arg );

		exit.set_value ( 1 );

		return 1;
	}

	return 0;			// Continue parsing args
}

int Global::file_func ( char const * const filename )
{
	if ( m_function )
	{
		s_arg = filename;

		// Check we have loaded a DB
		if ( ! db.get_butt () )
		{
			if ( db.open ( s_db_path ) )
			{
				exit.set_value ( 1 ); // Terminate ASAP
				return exit.value ();
			}

			s_db_path = "";
		}

		int res = m_function ();

		if ( res > 0 )
		{
			fprintf ( stderr, "%s error: %s. arg = '%s'\n",
				m_function_name.c_str(),
				get_error_text(res).c_str(),
				filename );

			exit.set_value ( 1 );	// Terminate ASAP
		}
	}

	return exit.value ();	// Keep parsing if no errors encountered
}

int Global::command_line ( int argc, char const * const argv[] )
{
	// Establish what command has been used to call this program
	s_arg = strrchr ( argv[0], MTKIT_DIR_SEP );

	if ( ! s_arg )
	{
		s_arg = argv[0];
	}
	else
	{
		s_arg ++;
	}

	set_function ( s_arg );

	// Parse & action the command line arguments
	mtKit::Arg args ( [this](char const * const filename)
		{
			return file_func ( filename );
		} );

	args.add ( "-help",	[this](){ print_help(); return 1; } );
	args.add ( "-version",	[this](){ print_version(); return 1; } );

	args.add ( "bottle",	s_bottle );
	args.add ( "com",	s_arg, [this](){ return parse_com(); } );
	args.add ( "db",	s_db_path );
	args.add ( "font",	i_font );
	args.add ( "o",		s_o );
	args.add ( "v",		i_verbose,	1 );

	args.parse ( argc, argv );

	return exit.value ();
}

std::string Global::get_error_text ( int const code ) const
{
	auto const it = err_table.find ( code );

	if ( it == err_table.end () )
	{
		return "Unknown";
	}

	return it->second;
}

FunCB Global::get_function ( std::string const & txt ) const
{
	auto const it = jump_table.find ( txt );

	if ( it == jump_table.end () )
	{
		return nullptr;
	}

	return it->second;
}

void Global::set_function ( char const * name )
{
	if ( ! name )
	{
		m_function = nullptr;
	}
	else
	{
		if ( name[0] == 'd' && name[1] == 'w' )
		{
			name += 2;
		}

		m_function = get_function ( name );
	}

	if ( m_function )
	{
		m_function_name = name;
	}
	else
	{
		m_function_name = "";
	}
}

