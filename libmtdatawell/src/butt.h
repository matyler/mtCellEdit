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

#include "core.h"



namespace mtDW
{



class ButtOp
{
public:
	ButtOp ( mtKit::Random & random, char const * path );
	~ButtOp ();

	int add_name ( std::string const & name );	// Dir must NOT exist
	int set_name ( std::string const & name );	// Dir must exist

	int add_buckets ( Well const * well, int const tot );

	inline int get_write_next () const	{ return m_write_next; }
	inline int get_otp_bucket () const	{ return m_otp_bucket; }
	inline int get_otp_position () const	{ return m_otp_position; }
	inline std::string const & get_path () const { return m_path; }
	inline std::string const & get_butt_root () const { return m_butt_root;}
	inline std::string const & get_name () const { return m_name; }

	// Encoding (changes internal OTP bookkeeping)

	int otp_get_int ( int & res );			// res=INT_MIN..INT_MAX
	int otp_get_int ( int modulo, int & res );	// res=0..(modulo - 1)
	int otp_get_data ( uint8_t * buf, size_t buflen );

	// Decoding

	int read_set_butt ( std::string const & name, int bucket, int pos );
	int read_get_data ( uint8_t * buf, size_t buflen );

private:
	void new_local_prefs ();
	void store_butt_prefs ();
	void create_butt_name ( std::string & str );

	int otp_open_bucket ();

	static std::string get_butt_num_text ( int num );
	std::string get_butt_filename ( int num ) const;
	std::string get_read_filename ( int num ) const;

	int init_butt_path ( std::string const & name, int exists );

/// ----------------------------------------------------------------------------

	int			m_write_next;
	int			m_otp_bucket;
	int			m_otp_position;

	std::string	const	m_path;
	std::string	const	m_butt_root;	// <m_path> / butt /
	std::string		m_butt_path;	// <m_butt_root> / <m_name> /
	std::string		m_name;

	mtKit::Random	&	m_random;
	mtKit::Prefs		m_prefs;

	mtKit::unique_ptr<mtKit::Prefs> m_prefs_local;

	mtKit::ByteFileRead	m_file_otp;

/// READ -----------------------------------------------------------------------

	mtKit::ByteFileRead	m_read_file;
	int			m_read_bucket;
	std::string		m_read_path;
};



}	// namespace mtDW

