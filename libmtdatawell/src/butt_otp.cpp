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

#include "butt.h"



mtDW::OTP::OTP ( Butt::Op & op )
	:
	m_op		(op),
	m_bucket	(0),
	m_position	(0)
{
}



int mtDW::OTP::read (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	if ( ! buf )
	{
		return report_error ( ERROR_BUTT_OTP_READ_BUFFER );
	}

	if ( ! m_file.is_open () )
	{
		RETURN_ON_ERROR ( open_bucket ( m_bucket, m_position ) )
	}

	uint8_t * dest = buf;
	size_t todo = buflen;

	while ( todo > 0 )
	{
		size_t const done = m_file.read ( dest, todo );

		if ( done < 1 )
		{
			RETURN_ON_ERROR ( open_bucket ( m_bucket + 1, 0 ) )

			continue;
		}

		m_position = (int)((size_t)m_position + done);
		todo -= done;
	}

	return 0;
}

void mtDW::OTP::set_path ( std::string const & name )
{
	m_file.close ();
	m_path = m_op.m_butt_root + name + MTKIT_DIR_SEP;
	m_name = name;
}

int mtDW::OTP::open_bucket (
	int	const	bucket,
	int	const	pos
	)
{
	std::string const filename = get_bucket_filename ( bucket );

	if ( m_file.open ( filename.c_str (), (uint64_t)pos ) )
	{
		std::cerr << "Bucket " << bucket << " at " << pos << "\n";
		return report_error ( ERROR_BUTT_OTP_OPEN_BUCKET );
	}

	m_bucket = bucket;
	m_position = pos;

	return 0;
}

std::string mtDW::OTP::get_bucket_filename ( int const num ) const
{
	char txt[16];

	snprintf ( txt, sizeof(txt), "%06i", num );

	return std::string ( m_path + txt );
}

int64_t mtDW::OTP::get_bucket_size () const
{
	std::string const filename = get_bucket_filename ( m_bucket );

	return mtkit_file_size ( filename.c_str () );
}

