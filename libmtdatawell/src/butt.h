/*
	Copyright (C) 2018-2024 Mark Tyler

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

#ifndef BUTT_H_
#define BUTT_H_



#include "core.h"



namespace mtDW
{

class OTP;
class OTPread;
class OTPactive;



class OTP
{
public:
	explicit OTP ( Butt::Op & op );

	std::string const & get_path () const { return m_path; }
	std::string const & get_name () const { return m_name; }
	int get_bucket () const		{ return m_bucket; }
	int get_position () const	{ return m_position; }

	void set_path ( std::string const & name );

	int open_bucket ( int bucket, int pos );
		// Returns error code.  Interpret via get_error_text()

	int read ( uint8_t * buf, size_t buflen );
		// Returns error code.  Interpret via get_error_text()

	int64_t get_bucket_size () const;

protected:
	std::string get_bucket_filename ( int num ) const;

/// ----------------------------------------------------------------------------

	Butt::Op		& m_op;

	mtKit::ByteFileRead	m_file;

	std::string		m_path;		// <m_butt_root> / <m_name> /
	std::string		m_name;

	int			m_bucket;
	int			m_position;
};



struct OTPprefs
{
	std::string		comment;
	int			write_next	= 0;
	int			bucket		= 0;
	int			position	= 0;
	int			status		= 0;


	// Below must be destroyed first before the above items
	mtKit::UserPrefs	uprefs;
};



class OTPactive : public OTP
{
public:
	enum
	{
		STATUS_READ_ONLY = 1
	};


	explicit OTPactive ( Butt::Op & op );
	~OTPactive ();

	int add_otp ( std::string const & name );	// Dir must NOT exist
		// Returns error code.  Interpret via get_error_text()

	int set_otp ( std::string const & name );	// Dir must exist
		// Returns error code.  Interpret via get_error_text()

	int import_otp ( std::string const & path );
		// Returns error code.  Interpret via get_error_text()

	int delete_otp ( std::string const & name ) const;
		// Returns error code.  Interpret via get_error_text()

	int set_comment ( std::string const & comment );
		// Returns error code.  Interpret via get_error_text()

	char const * get_comment () const;

	int add_buckets ( Well const * well, int const tot );
		// Returns error code.  Interpret via get_error_text()

	int empty_buckets ();
		// Returns error code.  Interpret via get_error_text()

	bool is_read_only() const	{ return (m_status & STATUS_READ_ONLY);}
	void set_read_only ();
	void set_read_write ();

	int get_write_next () const	{ return m_write_next; }

	void get_otp_list ( std::vector<OTPinfo> &list ) const;

	void save_state ();			// Save state to disk

///	Encoding (changes internal OTP bookkeeping)

	int get_data ( uint8_t * buf, size_t buflen );
		// Returns error code.  Interpret via get_error_text()

private:
	static OTPprefs * create_otp_prefs ();
	void new_otp_prefs ();
	void store_otp_state ();
	void restore_otp_state ();

	int init_otp_path ( std::string const & name, int exists );
		// Returns error code.  Interpret via get_error_text()

	int check_read_only () const;
		// Returns error code.  Interpret via get_error_text()

/// ----------------------------------------------------------------------------

	// Butt prefs
	std::unique_ptr<OTPprefs> m_prefs_butt;
	int			m_write_next;
	int			m_status;
};



struct ButtPrefs
{
	std::string		butt_name;

	// Below must be destroyed first before the above items
	mtKit::UserPrefs	uprefs;
};



class Butt::Op
{
public:
	explicit Op ( char const * path );
	~Op ();

	void save_state ();			// Save state to disk

/// ----------------------------------------------------------------------------

	OTPactive		m_active_otp;
	OTP			m_read_otp;

	std::string	const	m_butt_root;	// <DB path> / butt /

private:
	static void create_otp_name ( mtKit::Random &random, std::string & str);

/// ----------------------------------------------------------------------------

	mtKit::FileLock		m_lock;
	ButtPrefs		m_prefs;
};



class OTPanalysis::Op
{
public:
	explicit Op ( Butt &b );
	~Op ();

	void clear_tables ();
	int analyse_bucket ( int bucket );
	int analyse_all_buckets ();
	int analyse_finish (
		mtPixmap const * image_8bit,
		mtPixmap const * image_16bit
		);

/// ----------------------------------------------------------------------------

	Butt		&butt;

	int64_t		m_bucket_size;

	double		m_bit_1;		// % 0.0 - 1.0
	double		m_bit_list[8];		// % 0.0 - 1.0

	double	const	m_byte_mean;
	double		m_byte_list[256];	// % 0.0 - 1.0

private:
	int64_t		m_bit_count[8];
	int64_t		m_1byte_count[256];
	int64_t		m_2byte_count[256][256];

	bool		m_old_byte;
	uint8_t		m_old_byte_value;
};



}	// namespace mtDW



#endif		// BUTT_H_

