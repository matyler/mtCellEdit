/*
	Copyright (C) 2018-2022 Mark Tyler

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

#include <time.h>
#include <sys/time.h>
#include "private.h"



void mtKit::get_binary_dir ( std::string & path )
{
	path = "";

	char dest[PATH_MAX] = {0};

	if ( -1 == readlink ( "/proc/self/exe", dest, sizeof(dest) ) )
	{
		return;
	}

	char * ch = strrchr ( dest, MTKIT_DIR_SEP );
	if ( ch )
	{
		// Extract path, lose binary name
		ch[1] = 0;
		path += dest;
	}
	else
	{
		// No MTKIT_DIR_SEP separator so no path
	}
}

void mtKit::get_data_dir (
	std::string		& path,
	char	const * const	data
	)
{
	path = "";

	if ( data && data[0] == '.' )
	{
		// Relative path is being used so get binary path as a base
		get_binary_dir ( path );
	}

	path += data;
}

int mtKit::get_user_name ( std::string &name )
{
	name.clear ();

	try
	{
		struct passwd	* p;

		p = getpwuid ( getuid () );
		if ( p )
		{
			if ( p->pw_gecos && p->pw_gecos[0] )
			{
				name = p->pw_gecos;
			}
			else if ( p->pw_name && p->pw_name[0] )
			{
				name = p->pw_name;
			}
		}

		size_t const comma = name.find ( "," );

		if ( comma != std::string::npos )
		{
			name.resize ( comma );
		}

		if ( name.size () < 1 )
		{
			throw 123;
		}
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

std::string mtKit::realpath ( std::string const & path )
{
	std::string res;

	char * real = ::realpath ( path.c_str (), NULL );
	if ( real )
	{
		try
		{
			res = real;
		}
		catch ( ... )
		{
		}

		free ( real );
		real = NULL;
	}

	return res;
}

std::string mtKit::basename ( std::string const &path )
{
	size_t const found = path.find_last_of ( MTKIT_DIR_SEP );

	if ( found == std::string::npos )
	{
		return path;
	}

	return path.substr ( found + 1 );
}

int mtKit::string_strip_extension (
	std::string		&filename,
	char	const * const	extension
	)
{
	if ( ! extension )
	{
		char const * const st = filename.c_str ();
		char const * const pos = strrchr ( st, '.' );
		char const * const sep = strrchr ( st, MTKIT_DIR_SEP );

		if ( pos > sep && pos > st )
		{
			filename.resize ( (size_t)(pos - st) );
			return 1;
		}

		return 0;
	}

	std::string scan ("*.");
	scan += extension;

	int const len = (int)filename.length ();
	int const pos = mtkit_strmatch ( filename.c_str (), scan.c_str (), 0 );

	if ( pos > 0 )
	{
		if ( len > pos )
		{
			filename.resize ( (size_t)pos );
			return 1;
		}
	}

	return 0;
}

mtKit::Random::Random ()
	:
	m_seed (0)
{
	set_seed_by_time ();
}

void mtKit::Random::set_seed_by_time ()
{
	struct timeval tv;

	gettimeofday ( &tv, NULL );

	m_seed = 1000000 * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;

	get_int ();
	get_int ();
}

int mtKit::Random::get_int ()
{
	m_seed = m_seed * ((uint64_t)6364136223846793005) +
		(uint64_t)1442695040888963407;

	return (int)(m_seed >> 32);
}

int mtKit::Random::get_int ( int const modulo )
{
	// Return an integer from 0 to (modulo - 1) ensuring that it is evenly
	// distributed.

	if ( modulo < 2 )
	{
		return 0;
	}

	int const floor = ((INT_MAX - modulo) + 1) % modulo;

	int res = 0;

	do
	{
		// Lose negative int's
		res = get_int () & INT_MAX;

	} while ( res < floor );

	return (res % modulo);
}

void mtKit::Random::get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	uint8_t * const end = buf + buflen;

	for ( uint8_t * dest = buf; dest < end; dest++ )
	{
		dest[0] = (uint8_t)get_int ();
	}
}

mtKit::FileLock::FileLock ()
	:
	m_id		( -1 )
{
}

mtKit::FileLock::~FileLock ()
{
	unset ();
}

int mtKit::FileLock::set ( std::string const &filename )
{
	unset ();

	if ( mtkit_file_lock ( filename.c_str (), &m_id ) )
	{
		std::cerr << "Unable to lock file '" << filename << "'\n";
		return 1;
	}

	m_filename = filename;

	return 0;
}

void mtKit::FileLock::unset ()
{
	if ( -1 != m_id )
	{
		mtkit_file_unlock ( &m_id );
		remove ( m_filename.c_str () );
		m_filename = "";
	}
}



double mtKit::Clock::now ()
{
	struct timespec ts;

	if ( clock_gettime ( CLOCK_MONOTONIC, &ts ) == -1 )
	{
		return 0.0;
	}

	return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}



#include <iostream>
#include <fstream>



int mtkit_file_copy (
	char	const * const	filename_dest,
	char	const * const	filename_src
	)
{
	try
	{
		std::ifstream src ( filename_src, std::ios::binary );
		std::ofstream dest ( filename_dest, std::ios::binary );

		dest << src.rdbuf ();
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

