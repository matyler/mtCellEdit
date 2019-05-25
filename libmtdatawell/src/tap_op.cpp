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

#include "tap.h"



mtDW::Tap::Op::Op ()
{
}

mtDW::Tap::Op::~Op ()
{
}



/// TapFileOp ------------------------------------------------------------------



mtDW::TapFile::Op::Op ()
	:
	m_capacity (0)
{
}

mtDW::TapFile::Op::~Op ()
{
}

void mtDW::TapFile::Op::delete_soda_filename ()
{
	if ( m_soda_file.length () > 0 )
	{
		// Delete temp file if it exists
		remove ( m_soda_file.c_str () );
		m_soda_file = "";
	}
}

void mtDW::TapFile::Op::set_soda_filename ( std::string const & filename )
{
	delete_soda_filename ();
	m_soda_file = filename;
}

