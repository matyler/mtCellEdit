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

#include "well.h"



mtDW::Well::Well ( char const * const path )
	:
	op		( new WellOp ( path ) )
{
}

mtDW::Well::~Well ()
{
	delete op;
}

void mtDW::Well::get_shifts ( int shifts[8] ) const
{
	op->m_bitshift.get_shifts ( shifts );
}

uint64_t mtDW::Well::get_seed () const
{
	return op->m_random.get_seed ();
}

void mtDW::Well::set_seed ( uint64_t seed ) const
{
	op->m_random.set_seed ( seed );
}

void mtDW::Well::set_seed_by_time () const
{
	op->m_random.set_seed_by_time ();
}

std::string const & mtDW::Well::get_path () const
{
	return op->get_path ();
}

int mtDW::Well::get_files_done () const
{
	return op->count_files_done ();
}

int mtDW::Well::get_files_todo () const
{
	return op->count_files_todo ();
}

int mtDW::Well::add_path ( char const * const path ) const
{
	try
	{
		FileScan ( op->m_file_db, path );
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

void mtDW::Well::empty () const
{
	op->m_file_db.remove_all_files ();
}

int mtDW::Well::save_file (
	int		const	bytes,
	char	const * const	filename
	) const
{
	return op->save_file ( bytes, filename );
}

