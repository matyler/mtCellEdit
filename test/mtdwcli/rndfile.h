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

#include <string.h>
#include <math.h>
#include <limits.h>

#include <mtkit.h>
#include <mtdatawell.h>



class Backend
{
public:
	Backend ();
	~Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program, returning exit.value()

	inline char const * get_db_path () const { return m_db_path; }
	inline char const * get_output_path () const { return m_output_path; }

/// ----------------------------------------------------------------------------

	mtKit::Exit	exit;

	mtDW::Database	m_db;

	int		m_file_tot;
	int		m_file_size_min;
	int		m_file_size_max;
	int		m_quiet;
private:

/// ----------------------------------------------------------------------------

	char	const *	m_db_path;
	char	const *	m_output_path;
};

