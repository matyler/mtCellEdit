/*
	Copyright (C) 2018-2021 Mark Tyler

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

#include <algorithm>		// std::sort

#include "butt.h"



#define OTP_PREFS_COMMENT	"comment"
#define OTP_PREFS_WRITE_NEXT	"write_next"
#define OTP_PREFS_BUCKET	"bucket"
#define OTP_PREFS_POSITION	"position"
#define OTP_PREFS_STATUS	"status"

#define OTP_PREFS_FILENAME	"otp.prefs"

#define BUCKET_SIZE		16777216	// 16MB per file
#define BUCKET_MAX		999999		// ~15TB per butt



mtDW::OTPactive::OTPactive ( Butt::Op & op )
	:
	OTP		(op),
	m_write_next	(0),
	m_status	(0)
{
}

mtDW::OTPactive::~OTPactive ()
{
	save_state ();
}

int mtDW::OTPactive::get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	RETURN_ON_ERROR ( check_read_only () )

	return OTP::read ( buf, buflen );
}

void mtDW::OTPactive::store_otp_state ()
{
	OTPprefs * const prefs = m_prefs_butt.get ();

	if ( prefs )
	{
		prefs->write_next	= m_write_next;
		prefs->bucket		= m_bucket;
		prefs->position		= m_position;
		prefs->status		= m_status;
	}
}

void mtDW::OTPactive::restore_otp_state ()
{
	OTPprefs * const prefs = m_prefs_butt.get ();

	if ( prefs )
	{
		m_write_next	= prefs->write_next;
		m_bucket	= prefs->bucket;
		m_position	= prefs->position;
		m_status	= prefs->status;
	}
}

mtDW::OTPprefs * mtDW::OTPactive::create_otp_prefs ()
{
	auto * prefs = new OTPprefs;

	prefs->uprefs.add_string ( OTP_PREFS_COMMENT, prefs->comment, "" );
	prefs->uprefs.add_int ( OTP_PREFS_WRITE_NEXT, prefs->write_next, 0 );
	prefs->uprefs.add_int ( OTP_PREFS_BUCKET, prefs->bucket, 0 );
	prefs->uprefs.add_int ( OTP_PREFS_POSITION, prefs->position, 0 );
	prefs->uprefs.add_int ( OTP_PREFS_STATUS, prefs->status, 0 );

	return prefs;
}

void mtDW::OTPactive::new_otp_prefs ()
{
	if ( m_prefs_butt.get () )
	{
		// We have old prefs so save them
		store_otp_state ();
	}

	m_prefs_butt.reset ( create_otp_prefs () );
}

int mtDW::OTPactive::add_otp ( std::string const & name )
{
	RETURN_ON_ERROR ( init_otp_path ( name, 0 ) )

	time_t		now;
	struct tm	* now_tm;
	char		buf[128];

	now = time ( NULL );
	now_tm = localtime ( &now );

	snprintf ( buf, sizeof(buf), "%02i-%02i-%02i %02i:%02i:%02i",
		now_tm->tm_year + 1900,
		now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour,
		now_tm->tm_min, now_tm->tm_sec );

	std::string	comment ( buf );
	std::string	username;

	if ( 0 == mtKit::get_user_name ( username ) )
	{
		comment += " ";
		comment += username;
	}

	set_comment ( comment );

	return 0;
}

int mtDW::OTPactive::set_otp ( std::string const & name )
{
	RETURN_ON_ERROR ( init_otp_path ( name, 1 ) )

	return 0;
}

int mtDW::OTPactive::import_otp ( std::string const & path )
{
	std::string const full_path = mtKit::realpath ( path );

	if ( ! mtkit_file_directory_exists ( full_path.c_str () ) )
	{
		return report_error ( ERROR_IMPORT_OTP_BAD_DIR );
	}

	char const * name = strrchr ( full_path.c_str (), MTKIT_DIR_SEP );

	if ( ! name )
	{
		name = full_path.c_str ();
	}
	else
	{
		name++;
	}

	RETURN_ON_ERROR ( mtDW::Butt::validate_otp_name ( name ) )

	std::string const dest_path = m_op.m_butt_root + name;

	if ( mtkit_file_directory_exists ( dest_path.c_str () ) )
	{
		return report_error ( ERROR_IMPORT_OTP_EXISTS );
	}

	mtkit_mkdir ( dest_path.c_str() );

	// Open pathname given
	DIR * dp = opendir ( full_path.c_str () );
	if ( ! dp )
	{
		return report_error ( ERROR_IMPORT_OTP_OPEN_DIR );
	}

	struct dirent * ep;

	while ( ( ep = readdir ( dp ) ) )
	{
		std::string const src = full_path + MTKIT_DIR_SEP + ep->d_name;
		struct stat buf;

		if ( lstat ( src.c_str (), &buf ) )
		{
			continue;
		}

		if (	S_ISDIR ( buf.st_mode )		||
			S_ISLNK ( buf.st_mode )		||
			! S_ISREG ( buf.st_mode )
			)
		{
			continue;
		}

		std::string const dest = dest_path + MTKIT_DIR_SEP + ep->d_name;

		if ( mtkit_file_copy ( dest.c_str (), src.c_str () ) )
		{
			continue;
		}
	}

	closedir ( dp );

	RETURN_ON_ERROR ( set_otp ( name ) )
		// Unable to open new directory (possibly already exists) or
		// is an invalid name.

	set_read_only ();

	return 0;
}

int mtDW::OTPactive::delete_otp ( std::string const & name ) const
{
	if ( m_name == name )
	{
		return report_error ( ERROR_BUTT_OTP_DELETE_ACTIVE );
	}

	std::string const full_path = m_op.m_butt_root + name + MTKIT_DIR_SEP;

	std::unique_ptr<OTPprefs> prefs ( create_otp_prefs () );
	std::string const prefs_filename = full_path + OTP_PREFS_FILENAME;
	prefs->uprefs.load ( prefs_filename.c_str (), NULL );

	if ( STATUS_READ_ONLY & prefs->status )
	{
		return report_error ( ERROR_DISK_OTP_READ_ONLY );
	}

	return mtDW::remove_dir ( full_path );
}

int mtDW::OTPactive::set_comment ( std::string const & comment )
{
	RETURN_ON_ERROR ( check_read_only () )

	OTPprefs * const prefs = m_prefs_butt.get ();

	if ( prefs )
	{
		prefs->comment = comment;
		prefs->uprefs.save ();
	}

	return 0;
}

char const * mtDW::OTPactive::get_comment () const
{
	OTPprefs const * const prefs = m_prefs_butt.get ();

	if ( prefs )
	{
		return prefs->comment.c_str();
	}

	return "";
}

int mtDW::OTPactive::init_otp_path (
	std::string	const & name,
	int		const	exists
	)
{
	RETURN_ON_ERROR ( mtDW::Butt::validate_otp_name ( name ) )

	std::string const path = m_op.m_butt_root + name + MTKIT_DIR_SEP;

	if ( exists )
	{
		// Caller wants this to exist
		if ( ! mtkit_file_directory_exists ( path.c_str () ) )
		{
			return report_error ( ERROR_BUTT_OTP_MISSING );
		}
	}
	else
	{
		// Caller doesn't want this to exist
		if ( mtkit_file_directory_exists ( path.c_str () ) )
		{
			return report_error ( ERROR_BUTT_OTP_EXISTS );
		}
	}

	if ( name == m_name )
	{
		// Nothing to change
		return 0;
	}

	if ( ! exists )
	{
		mtkit_mkdir ( path.c_str () );
	}

	// Store new path, name, etc
	set_path ( name );

	// Get accounting info
	new_otp_prefs ();
	std::string const fn = m_path + OTP_PREFS_FILENAME;
	m_prefs_butt->uprefs.load ( fn.c_str (), NULL );
	restore_otp_state ();

	// Tell Butt to save its state as we have a new active OTP
	m_op.save_state ();

	return 0;
}

int mtDW::OTPactive::add_buckets (
	Well	const * const	well,
	int		const	tot
	)
{
	RETURN_ON_ERROR ( check_read_only () )

	if ( ! well )
	{
		return report_error ( ERROR_BUTT_OTP_NO_WELL );
	}

	int const end = MIN( m_write_next + MIN( BUCKET_MAX, tot ),
		BUCKET_MAX );

	for ( ; m_write_next < end; m_write_next++ )
	{
		std::string const filename = get_bucket_filename (m_write_next);

		RETURN_ON_ERROR( well->save_file(BUCKET_SIZE, filename.c_str()))
	}

	save_state ();

	return 0;
}

int mtDW::OTPactive::empty_buckets ()
{
	RETURN_ON_ERROR ( check_read_only () )

	m_file.close ();

	for ( int i = 0; i < m_write_next; i++ )
	{
		std::string const filename = get_bucket_filename ( i );

		remove ( filename.c_str () );
	}

	m_write_next = 0;
	m_bucket = 0;
	m_position = 0;

	save_state ();

	return 0;
}

int mtDW::OTPactive::check_read_only () const
{
	if ( is_read_only () )
	{
		return report_error ( ERROR_BUTT_OTP_READ_ONLY );
	}

	return 0;
}

void mtDW::OTPactive::save_state ()
{
	OTPprefs * const prefs = m_prefs_butt.get ();

	if ( prefs )
	{
		store_otp_state ();
		prefs->uprefs.save ();
	}
}

static bool cmp_buttinfo (
	mtDW::OTPinfo	const	&l,
	mtDW::OTPinfo	const	&r
	)
{
	return l.m_name < r.m_name;
}

void mtDW::OTPactive::get_otp_list ( std::vector<OTPinfo> &list ) const
{
	std::string const &path = m_op.m_butt_root;

	mtDW::OpenDir dir ( path );

	if ( ! dir.dp )
	{
		return;
	}

	struct dirent * ep;

	while ( (ep = readdir(dir.dp)) )
	{
		if (	! strcmp ( ep->d_name, "." ) ||
			! strcmp ( ep->d_name, ".." )
			)
		{
			// Quietly ignore "." and ".." directories
			continue;
		}

		std::string const tmp = path + ep->d_name;
		struct stat buf;

		if ( lstat ( tmp.c_str (), &buf ) )	// Get file details
		{
			// Unable to access
			continue;
		}

		if ( S_ISDIR ( buf.st_mode ) )
		{
			std::string filename ( path );

			filename += ep->d_name;
			filename += MTKIT_DIR_SEP;
			filename += OTP_PREFS_FILENAME;

			std::unique_ptr<OTPprefs> prefs
				( create_otp_prefs () );

			prefs->uprefs.load ( filename.c_str (), NULL );

			list.push_back ( OTPinfo ( ep->d_name,
				prefs->comment.c_str(),
				prefs->status,
				prefs->write_next
				) );
		}
	}

	std::sort ( list.begin (), list.end (), cmp_buttinfo );
}

void mtDW::OTPactive::set_read_only ()
{
	m_status |= STATUS_READ_ONLY;
	save_state ();
}

void mtDW::OTPactive::set_read_write ()
{
	m_status &= ~STATUS_READ_ONLY;
	save_state ();
}

