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

#include "butt.h"



mtDW::Butt::Butt (
	char	const * const	path
	)
	:
	m_op		( new Butt::Op ( path ) )
{
}

mtDW::Butt::~Butt ()
{
}

int mtDW::Butt::add_buckets (
	Well	* const	well,
	int	const	tot
	) const
{
	return m_op->m_active_otp.add_buckets ( well, tot );
}

int mtDW::Butt::empty_buckets () const
{
	return m_op->m_active_otp.empty_buckets ();
}

bool mtDW::Butt::is_read_only () const
{
	return m_op->m_active_otp.is_read_only ();
}

void mtDW::Butt::set_read_only () const
{
	m_op->m_active_otp.set_read_only ();
}

void mtDW::Butt::set_read_write () const
{
	m_op->m_active_otp.set_read_write ();
}

int mtDW::Butt::get_bucket_total () const
{
	return m_op->m_active_otp.get_write_next ();
}

int mtDW::Butt::get_bucket_used () const
{
	return m_op->m_active_otp.get_bucket ();
}

int mtDW::Butt::get_bucket_position () const
{
	return m_op->m_active_otp.get_position ();
}

std::string const & mtDW::Butt::get_otp_name () const
{
	return m_op->m_active_otp.get_name ();
}

int mtDW::Butt::add_otp ( std::string const & name ) const
{
	return m_op->m_active_otp.add_otp ( name );
}

int mtDW::Butt::set_otp ( std::string const & name ) const
{
	return m_op->m_active_otp.set_otp ( name );
}

int mtDW::Butt::import_otp ( std::string const & path ) const
{
	return m_op->m_active_otp.import_otp ( path );
}

int mtDW::Butt::delete_otp ( std::string const & name ) const
{
	return m_op->m_active_otp.delete_otp ( name );
}

void mtDW::Butt::get_new_name (
	Well	const * const	well,
	std::string		&result
	) const
{
	AppPassword body ( true, true, true, "" );

	static int const BODY_LEN = 5;

	// Bound attempts to avoid an infinite loop.
	for ( int i = 0; i < 5; i++ )
	{
		std::string tmp;

		result.clear ();

		body.get_password ( well, BODY_LEN, tmp );
		result += tmp;

		for ( int j = 0; j < 2; j++ )
		{
			result += '_';

			body.get_password ( well, BODY_LEN, tmp );
			result += tmp;
		}

		std::string const path ( m_op->m_butt_root + result );

		if ( ! mtkit_file_directory_exists ( path.c_str () ) )
		{
			// This named butt doesn't exist yet so its valid
			break;
		}
	}
}

int mtDW::Butt::set_comment ( std::string const & comment ) const
{
	return m_op->m_active_otp.set_comment ( comment );
}

char const * mtDW::Butt::get_comment () const
{
	return m_op->m_active_otp.get_comment ();
}

void mtDW::Butt::get_otp_list ( std::vector<OTPinfo> &list ) const
{
	m_op->m_active_otp.get_otp_list ( list );
}

int mtDW::Butt::validate_otp_name ( std::string const & name )
{
	if ( name.size () < OTP_NAME_LEN_MIN )
	{
		return report_error ( ERROR_OTP_NAME_TOO_SMALL );
	}

	if ( name.size () > OTP_NAME_LEN_MAX )
	{
		return report_error ( ERROR_OTP_NAME_TOO_LARGE );
	}

	char const * const str = name.c_str ();

	for ( size_t i = 0; i < name.size (); i++ )
	{
		if (	! (str[i] >= '0' && str[i] <= '9')
			&& ! (str[i] >= 'a' && str[i] <= 'z')
			&& ! (str[i] >= 'A' && str[i] <= 'Z')
			&& str[i] != '_'
			&& str[i] != '.'
			&& str[i] != '-'
			)
		{
			return report_error ( ERROR_OTP_NAME_ILLEGAL );
		}
	}

	return 0;
}

int mtDW::Butt::get_otp_data ( uint8_t * buf, size_t buflen ) const
{
	return m_op->m_active_otp.get_data ( buf, buflen );
}

int mtDW::Butt::read_set_otp (
	std::string	const & name,
	int		const	bucket,
	int		const	pos
	) const
{
	m_op->m_read_otp.set_path ( name );

	return m_op->m_read_otp.open_bucket ( bucket, pos );
}

int64_t mtDW::Butt::read_get_bucket_size () const
{
	return m_op->m_read_otp.get_bucket_size ();
}

int mtDW::Butt::read_get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	) const
{
	return m_op->m_read_otp.read ( buf, buflen );
}

void mtDW::Butt::save_state () const
{
	m_op->m_active_otp.save_state ();
}

