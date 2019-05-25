/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include "rndfile.h"



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

	try
	{
		if ( backend.m_db.open ( backend.get_db_path () ) )
		{
			throw 123;
		}

		if ( ! backend.get_output_path () )
		{
			std::cerr << "No output path!\n";
			throw 123;
		}

		int const file_size_range = backend.m_file_size_max -
			backend.m_file_size_min + 1;

		mtDW::Well * const well = backend.m_db.get_well ();

		std::string path = mtKit::realpath ( backend.get_output_path());
		path += MTKIT_DIR_SEP;

		for ( int i = 0; i < backend.m_file_tot; i++ )
		{
			int const file_size = backend.m_file_size_min +
				well->get_int ( file_size_range );

			char buf[32];
			snprintf ( buf, sizeof(buf), "%06i", i );

			std::string filename = path;
			filename += buf;

			if ( ! backend.m_quiet )
			{
				std::cout << "filename=" << filename << " size="
					<< file_size << "\n";
			}

			if ( well->save_file ( file_size, filename.c_str() ) )
			{
				throw 123;
			}
		}
	}
	catch ( ... )
	{
		backend.exit.set_value ( 1 );
	}

	return backend.exit.value ();
}

Backend::Backend ()
	:
	m_file_tot	( 1000 ),
	m_file_size_min	( 1000 ),
	m_file_size_max	( 100000 ),
	m_quiet		( 0 ),
	m_db_path	( NULL ),
	m_output_path	( NULL )
{
}

Backend::~Backend ()
{
}

static int arg_file (
	char	const * const	ARG_UNUSED ( filename ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	return 0;			// Continue
}

int Backend::command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtArg	const	arg_list[] = {
		{ "db",		MTKIT_ARG_STRING, &m_db_path, 0, NULL },
		{ "path",	MTKIT_ARG_STRING, &m_output_path, 0, NULL },
		{ "tot",	MTKIT_ARG_INT, &m_file_tot, 0, NULL },
		{ "min",	MTKIT_ARG_INT, &m_file_size_min, 0, NULL },
		{ "max",	MTKIT_ARG_INT, &m_file_size_max, 0, NULL },
		{ "quiet",	MTKIT_ARG_SWITCH, &m_quiet, 1, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};

	mtkit_arg_parse ( argc, argv, arg_list, arg_file, NULL, NULL );

	return 0;			// Continue
}

