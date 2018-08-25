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

#include <sqlite3.h>

#include "mtdatawell.h"

#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sndfile.h>
#include <mtpixy.h>



namespace mtDW
{



class OpenDir
{
public:
	inline explicit OpenDir ( std::string const & path )
	{
		dp = opendir ( path.c_str () );
	}

	inline ~OpenDir ()
	{
		if ( dp )
		{
			closedir ( dp );
			dp = NULL;
		}
	}

/// ----------------------------------------------------------------------------

	DIR * dp;
};



class FilenameSwap
{
public:
	explicit FilenameSwap ( char const * const output );
	~FilenameSwap ();

	void swap ();

/// ----------------------------------------------------------------------------

	char	const * f1;
	char	const * f2;

	std::string	m_tmp;
	int		m_res;

private:
	char	const * const	m_prefix;
};



std::string prepare_path ( char const * path );

void get_temp_filename (
	std::string	&	filename,
	char	const * const	prefix
	);
	// filename = "prefix_01" or and other number up to _99 for unused file



}	// namespace mtDW

