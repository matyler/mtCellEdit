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



int mtDW::ButtOp::otp_get_int ( int & res )
{
	uint8_t		buf[4] = {0};

	if ( otp_get_data ( buf, sizeof(buf) ) )
	{
		return 1;
	}

	res = (int)(	buf[0]
			| (buf[1] << 8)
			| (buf[2] << 16)
			| (buf[3] << 24)
			);

	return 0;
}

int mtDW::ButtOp::otp_get_int (
	int	const	modulo,
	int	&	res
	)
{
	if ( modulo < 2 )
	{
		return 1;
	}

	int const floor = ((INT_MAX - modulo) + 1) % modulo;

	res = 0;

	do
	{
		// Lose negative int's
		if ( otp_get_int ( res ) )
		{
			return 1;
		}

		res = res & INT_MAX;

	} while ( res < floor );

	res = res % modulo;

	return 0;
}

int mtDW::ButtOp::otp_get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	if ( ! m_file_otp.is_open () )
	{
		if ( otp_open_bucket () )
		{
			std::cerr << "Butt unable to open bucket\n";
			return 1;
		}
	}

	uint8_t * dest = buf;
	size_t todo = buflen;

	while ( todo > 0 )
	{
		size_t const done = m_file_otp.read ( dest, todo );

		if ( done < 1 )
		{
			m_otp_bucket++;
			m_otp_position = 0;

			if (	m_otp_bucket >= m_write_next
				|| otp_open_bucket ()
				)
			{
				std::cerr << "Butt data exhausted\n";
				return 1;
			}

			continue;
		}

		m_otp_position = (int)((size_t)m_otp_position + done);
		todo -= done;
	}

	return 0;
}

int mtDW::ButtOp::otp_open_bucket ()
{
	std::string const butt_file = get_butt_filename ( m_otp_bucket );

	return m_file_otp.open ( butt_file.c_str (), 0 );
}

