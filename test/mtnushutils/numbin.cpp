/*
	Copyright (C) 2022-2023 Mark Tyler

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

#include "numbin.h"



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

	return backend.exit.value ();
}

int Backend::command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtKit::Arg args ( [this]( char const * const filename )
		{ return this->evaluate ( filename ); } );

	char const * tmp;

	args.add ( "i", tmp, [this, &tmp]() { return import_file ( tmp ); } );
	args.add ( "o", tmp, [this, &tmp]() { return export_file ( tmp ); } );
	args.add ( "mem", tmp, [this, &tmp]() { return mem_file ( tmp ); } );
	args.add ( "cmp", tmp, [this, &tmp]() { return cmp_file ( tmp ); } );
	args.add ( "himp", tmp, [this, &tmp]() { return himp_file ( tmp ); } );
	args.add ( "p", tmp, [this, &tmp]() { return print_file ( tmp ); } );

	args.parse ( argc, argv );

	return 0;			// Continue
}

int Backend::evaluate ( char const * const text )
{
	int const e = m_parser_int.evaluate ( text ? text : "" );

	if ( e )
	{
		int const error_point = m_parser_int.error_pos ();

		std::string error ( text ? text : "" );

		error += '\n';

		for ( int i = 0; i < error_point; i++ )
		{
			error += " ";
		}

		error += "^\n";
		error += mtDW::get_error_text ( e );
		error += "\n";

		std::cerr << error << "\n";

		return 1;
	}

	if ( m_imem.import_number ( m_parser_int.result() ) )
	{
		std::cerr << "error importing number: "
			<< m_parser_int.result().to_string()
			<< "\n";
		return 1;
	}

	return 0;
}

int Backend::import_file ( char const * const filename )
{
	return m_imem.import_file ( filename );
}

int Backend::export_file ( char const * const filename ) const
{
	return m_imem.export_file ( filename );
}

int Backend::mem_file ( char const * const filename )
{
	unsigned char mem[256];

	for ( size_t i = 0; i < sizeof(mem); i++ )
	{
		mem[i] = (unsigned char)i;
	}

	if (	m_imem.import_memory ( mem, sizeof(mem), 0 ) ||
		m_imem.export_file ( filename )
		)
	{
		// import_memory & export_file report errors
		return 1;
	}

	return 0;
}

int Backend::cmp_file ( char const * const filename )
{
	if ( export_file ( filename ) )
	{
		// export_file reports errors
		return 1;
	}

	if ( import_file ( filename ) )
	{
		// import_file reports errors
		return 1;
	}

	mtDW::Integer new_num;

	m_imem.export_number ( new_num );

	std::cout << new_num.to_string()
		<< (m_parser_int.result() == new_num ? "\n\t==\n" : "\n\t!=\n")
		<< m_parser_int.result().to_string()
		<< "\n";

	return 0;
}

int Backend::himp_file ( char const * const filename )
{
	int size = 0;
	char * const ptr = mtkit_file_load ( filename, &size, MTKIT_FILE_NONE,
		nullptr );

	if ( ! ptr )
	{
		std::cerr << "Unable to load file: '" << filename << "'\n";
		return 1;
	}

	mtKit::CMemPtr<char> mem ( ptr, (size_t)size );

	if ( m_imem.import_memory_with_header ( (unsigned char const *)
		mem.ptr(), mem.buflen() ) )
	{
		// import_memory_with_header reports errors
		return 1;
	}

	return 0;
}

int Backend::print_file ( char const * const filename )
{
	if ( import_file ( filename ) )
	{
		return 1;
	}

	mtDW::Integer num;

	m_imem.export_number ( num );

	std::cout << num.to_string() << "\n";

	return 0;
}

