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

#include "butt.h"



int mtDW::ButtOp::read_set_butt (
	std::string	const & name,
	int		const	bucket,
	int		const	pos
	)
{
	m_read_path = m_butt_root + name + MTKIT_DIR_SEP;

	std::string const filename = get_read_filename ( bucket );

	m_read_bucket = bucket;

	m_read_file.close ();

	return m_read_file.open ( filename.c_str (), (uint64_t)pos );
}

int mtDW::ButtOp::read_get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	if ( ! m_read_file.is_open () )
	{
		return 1;
	}

	uint8_t * dest = buf;
	size_t todo = buflen;

	while ( todo > 0 )
	{
		size_t const done = m_read_file.read ( dest, todo );

		if ( done < 1 )
		{
			m_read_bucket++;

			if ( m_read_bucket >= m_write_next )
			{
				return 1;
			}

			std::string const filename = get_read_filename (
				m_read_bucket );

			if ( m_read_file.open ( filename.c_str (), 0 ) )
			{
				return 1;
			}

			continue;
		}

		todo -= done;
	}

	return 0;
}

std::string mtDW::ButtOp::get_read_filename ( int const num ) const
{
	return std::string ( m_read_path + get_butt_num_text ( num ) );
}

