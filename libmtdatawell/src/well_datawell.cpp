/*
	Copyright (C) 2018-2020 Mark Tyler

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



char const * const mtDW::Well::PASSWORD_OTHER_DEFAULT = "!?£#$@&%*";



mtDW::Well::Well ( char const * const path )
	:
	m_op		( new Well::Op ( path ) )
{
}

mtDW::Well::~Well ()
{
}

void mtDW::Well::get_shifts ( int shifts[8] ) const
{
	m_op->m_bitshift.get_shifts ( shifts );
}

void mtDW::Well::set_shifts () const
{
	m_op->m_bitshift.set_shifts ( m_op->m_random );
}

uint64_t mtDW::Well::get_seed () const
{
	return m_op->m_random.get_seed ();
}

void mtDW::Well::set_seed ( uint64_t seed ) const
{
	m_op->m_random.set_seed ( seed );
}

void mtDW::Well::set_seed_by_time () const
{
	m_op->m_random.set_seed_by_time ();
}

int mtDW::Well::get_files_done () const
{
	return m_op->count_files_done ();
}

int mtDW::Well::get_files_todo () const
{
	return m_op->count_files_todo ();
}

void mtDW::Well::add_path ( char const * const path ) const
{
	try
	{
		FileScan ( m_op->m_file_db, path );
	}
	catch ( ... )
	{
	}
}

void mtDW::Well::empty () const
{
	m_op->m_file_db.remove_all_files ();
}

int mtDW::Well::save_file (
	int		const	bytes,
	char	const * const	filename
	) const
{
	return m_op->save_file ( bytes, filename );
}

int mtDW::Well::get_int () const
{
	return m_op->get_int ();
}

int mtDW::Well::get_int ( int modulo ) const
{
	return m_op->get_int ( modulo );
}

void mtDW::Well::get_data ( uint8_t * buf, size_t buflen ) const
{
	m_op->get_data ( buf, buflen );
}

void mtDW::Well::save_state () const
{
	return m_op->save_state ();
}

void mtDW::Well::shuffle ( std::vector<int> &items ) const
{
	size_t const size = items.size ();
	if ( size < 2 )
	{
		return;
	}

	for ( size_t i = 0; i < size; i++ )
	{
		size_t	const	other = (size_t)get_int ( (int)size );
		int	const	tmp = items[ other ];

		items[ other ] = items[ i ];
		items[ i ] = tmp;
	}
}

