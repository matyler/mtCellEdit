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



// Global prefs
#define PREFS_BUTT_NAME		"name"

// Local prefs (per butt)
#define PREFS_WRITE_NEXT	"write_next"
#define PREFS_OTP_BUCKET	"otp_bucket"
#define PREFS_OTP_POSITION	"otp_position"

#define PREFS_FILENAME		"butt.prefs"
#define BUTT_PATH		"butt"
#define BUTT_FILENAME_PRINTF	"%06i"

#define BUCKET_SIZE		16777216	// 16MB
#define BUCKET_MAX		999999		// ~15TB



mtDW::ButtOp::ButtOp (
	mtKit::Random	&	random,
	char	const * const	path
	)
	:
	m_write_next	( 0 ),
	m_otp_bucket	( 0 ),
	m_otp_position	( 0 ),
	m_path		( mtDW::prepare_path ( path ) ),
	m_butt_root	( m_path + BUTT_PATH + MTKIT_DIR_SEP ),
	m_random	( random ),
	m_read_bucket	( 0 )
{
	mkdir ( m_butt_root.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );

	// Global Prefs files --------------------------------------------------

	static mtPrefTable const prefs_table[] = {
	{ PREFS_BUTT_NAME, MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	m_prefs.addTable ( prefs_table );

	std::string const filename = m_path + PREFS_FILENAME;

	m_prefs.load ( filename.c_str (), NULL );

	std::string butt_name = m_prefs.getString ( PREFS_BUTT_NAME );

	if ( butt_name.size () < 1 )
	{
		// First time use, so create a new randomly named butt
		create_butt_name ( butt_name );
		add_name ( butt_name );
	}
	else
	{
		set_name ( butt_name );
	}
}

mtDW::ButtOp::~ButtOp ()
{
	store_butt_prefs ();

	// Local/Global butt prefs always saved when mtKit::Prefs destroyed
}

void mtDW::ButtOp::store_butt_prefs ()
{
	m_prefs.set ( PREFS_BUTT_NAME, m_name.c_str () );

	mtKit::Prefs * const prefs = m_prefs_local.get ();

	if ( prefs )
	{
		prefs->set ( PREFS_WRITE_NEXT, m_write_next );
		prefs->set ( PREFS_OTP_BUCKET, m_otp_bucket );
		prefs->set ( PREFS_OTP_POSITION, m_otp_position );
	}
}

void mtDW::ButtOp::new_local_prefs ()
{
	store_butt_prefs ();

	mtKit::Prefs * prefs = new mtKit::Prefs;

	static mtPrefTable const local[] = {
	{ PREFS_WRITE_NEXT,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_OTP_BUCKET,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_OTP_POSITION,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	prefs->addTable ( local );

	m_prefs_local.reset ( prefs );

	m_write_next = 0;
	m_otp_bucket = 0;
	m_otp_position = 0;
}

void mtDW::ButtOp::create_butt_name ( std::string & str )
{
	str = "";

	for ( int i = 0; i < 5; i++ )
	{
		str += (char)('a' + m_random.get_int ( 26 ));
	}

	str += '_';

	for ( int i = 0; i < 5; i++ )
	{
		str += (char)('0' + m_random.get_int ( 10 ));
	}
}

int mtDW::ButtOp::add_name ( std::string const & name )
{
	if ( init_butt_path ( name, 0 ) )
	{
		std::cerr << "Butt already exists.\n";
		set_name ( name );
		return 1;
	}

	return 0;
}

int mtDW::ButtOp::set_name ( std::string const & name )
{
	if ( init_butt_path ( name, 1 ) )
	{
		std::cerr << "Butt does not exist.\n";
		return 1;
	}

	return 0;
}

int mtDW::ButtOp::init_butt_path (
	std::string	const & name,
	int		const	exists
	)
{
	if ( mtDW::Butt::validate_butt_name ( name ) )
	{
		return 1;
	}

	std::string const path = m_butt_root + name + MTKIT_DIR_SEP;

	if ( exists )
	{
		// Caller wants this to exist
		if ( ! mtkit_file_directory_exists ( path.c_str () ) )
		{
			return 1;
		}
	}
	else
	{
		// Caller doesn't want this to exist
		if ( mtkit_file_directory_exists ( path.c_str () ) )
		{
			return 1;
		}
	}

	if ( name == m_name )
	{
		// Nothing to change
		return 0;
	}

	m_name = name;
	m_butt_path = path;

	if ( ! exists )
	{
		mkdir ( m_butt_path.c_str (), S_IRWXU | S_IRWXG | S_IRWXO );
	}

	new_local_prefs ();

	std::string const fn = m_butt_path + PREFS_FILENAME;

	mtKit::Prefs * const prefs = m_prefs_local.get ();

	prefs->load ( fn.c_str (), NULL );

	m_write_next =		prefs->getInt ( PREFS_WRITE_NEXT );
	m_otp_bucket =		prefs->getInt ( PREFS_OTP_BUCKET );
	m_otp_position =	prefs->getInt ( PREFS_OTP_POSITION );

	m_file_otp.close ();

	std::string const butt_file = get_butt_filename ( m_otp_bucket );

	if ( m_file_otp.open ( butt_file.c_str (), (uint64_t)m_otp_position ) )
	{
		if ( 0 == m_otp_bucket && 0 == m_otp_position )
		{
			// Quietly ignore failure to open file (empty butt)
		}
		else
		{
			std::cerr << "Unable to open butt file "
				<< m_otp_bucket << " at " << m_otp_position
				<< "\n";
		}
	}

	return 0;
}

int mtDW::ButtOp::add_buckets (
	Well	const * const	well,
	int		const	tot
	)
{
	int const end = MIN( m_write_next + MIN( BUCKET_MAX, tot ), BUCKET_MAX);

	for ( ; m_write_next < end; m_write_next++ )
	{
		std::string const filename = get_butt_filename ( m_write_next );

		if ( well->save_file ( BUCKET_SIZE, filename.c_str () ) )
		{
			return 1;
		}
	}

	return 0;
}

std::string mtDW::ButtOp::get_butt_filename ( int const num ) const
{
	return std::string ( m_butt_path + get_butt_num_text ( num ) );
}

std::string mtDW::ButtOp::get_butt_num_text ( int const num )
{
	char txt[16];

	snprintf ( txt, sizeof(txt), BUTT_FILENAME_PRINTF, num );

	return std::string ( txt );
}

