/*
	Copyright (C) 2018 Mark Tyler

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

#include "core.h"



std::string mtDW::prepare_path ( char const * const path )
{
	std::string real_path;

	if ( path )
	{
		real_path += path;

		mkdir ( real_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );

		real_path += MTKIT_DIR_SEP;
	}
	else
	{
		real_path += mtkit_file_home ();
		real_path += MTKIT_DIR_SEP;
		real_path += ".config";

		mkdir ( real_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );

		real_path += MTKIT_DIR_SEP;
		real_path += APP_NAME;

		mkdir ( real_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );

		real_path += MTKIT_DIR_SEP;
	}

	return real_path;
}

void mtDW::get_temp_filename (
	std::string	&	filename,
	char	const * const	prefix
	)
{
	char buf[16];

	for ( int i = 1; i < 100; i++ )
	{
		snprintf ( buf, sizeof(buf), "_%02i", i );

		filename = prefix;
		filename += buf;

		if ( ! mtkit_file_readable ( filename.c_str () ) )
		{
			break;
		}
	}
}

mtDW::FilenameSwap::FilenameSwap ( char const * const output )
	:
	m_res		( 0 ),
	m_prefix	( output )
{
	get_temp_filename ( m_tmp, output );

	// Reserve this file on the filesystem so further calls
	// don't use this same filename.
	mtkit_file_save ( m_tmp.c_str (), m_tmp.c_str (), 0, 0 );

	f1 = output;
	f2 = m_tmp.c_str ();
}

mtDW::FilenameSwap::~FilenameSwap ()
{
	if ( 0 == m_res )
	{
		// On success remove/rename as required

		if ( f1 == m_prefix )
		{
			rename ( f2, f1 );
		}
		else
		{
			remove ( f1 );
		}

		return;
	}

	// On failure remove both files
	remove ( f1 );
	remove ( f2 );
}

void mtDW::FilenameSwap::swap ()
{
	char const * const f0 = f1;
	f1 = f2;
	f2 = f0;
}

