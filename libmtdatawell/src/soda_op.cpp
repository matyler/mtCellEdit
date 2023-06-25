/*
	Copyright (C) 2018-2022 Mark Tyler

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

#include "soda.h"



#define SODA_PATH		"soda"

#define DB_FILENAME		"log.sqlite"
#define SCHEMA_VERSION		3202

#define DB_TABLE_LOG_ENCODE	"Log_Encode"

#define DB_FIELD_KEY		"Key"		// Allocated by SQLite
#define DB_FIELD_DATETIME	"UTC_Time"	// YYYY-MM-DD HH:MM:SS UTC/GMT
#define DB_FIELD_FILENAME	"Filename"
#define DB_FIELD_FILESIZE	"Filesize"
#define DB_FIELD_MODE		"Mode"		// 0=xor 1=raw, etc
#define DB_FIELD_OTP_BUCKET	"OTP_Bucket"	// Only used for encoding=xor
#define DB_FIELD_BUCKET_POS	"Bucket_Pos"	// Only used for encoding=xor
#define DB_FIELD_OTP_NAME	"OTP_Name"	// Only used for encoding=xor

#define MODE_BIT_ENCODE_RAW	0x00000001	// Set = encode raw (no XOR)



mtDW::Soda::Op::Op ( char const * const path )
	:
	m_mode		( 0 )
{
	std::string const root = mtDW::prepare_path(path) + SODA_PATH +
		MTKIT_DIR_SEP;

	mtkit_mkdir ( root.c_str () );

	if (	m_lock.set ( root + "soda.lock" )	||
		m_db.open ( root + DB_FILENAME )
		)
	{
		throw 123;
	}

	int const version = m_db.get_version ();

	if ( SCHEMA_VERSION == version )
	{
		// We are using an up to date schema, nothing to set up.
		return;
	}

	if ( SCHEMA_VERSION < version )
	{
		// Archive tables created by FUTURE versions of this library
		m_db.archive_table ( DB_TABLE_LOG_ENCODE, version );
	}
	else if ( version > 0 )		// SCHEMA_VERSION > version
	{
		// Archive stale tables from an OLD version of this library
		m_db.archive_table ( DB_TABLE_LOG_ENCODE, version );
	}
	// else version < 1 so assume nothing exists (i.e. first time run)

/// NOTE: after changes to the following schema, you must bump SCHEMA_VERSION.

	m_db.exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_LOG_ENCODE
		" ("
		" " DB_FIELD_KEY " INTEGER PRIMARY KEY"		","
		" " DB_FIELD_DATETIME		" DATE"		","
		" " DB_FIELD_FILENAME		" BLOB"		","
		" " DB_FIELD_FILESIZE		" INTEGER"	","
		" " DB_FIELD_MODE		" INTEGER"	","
		" " DB_FIELD_OTP_NAME		" TEXT"		","
		" " DB_FIELD_OTP_BUCKET		" INTEGER"	","
		" " DB_FIELD_BUCKET_POS		" INTEGER"
		" );" );

	m_db.set_version ( SCHEMA_VERSION );
}

int mtDW::Soda::Op::encode_raw () const
{
	if ( m_mode & MODE_BIT_ENCODE_RAW )
	{
		return 1;
	}

	return 0;
}

void mtDW::Soda::Op::db_add_encode (
	char	const * const	filename,
	uint64_t	const	filesize,
	int		const	mode,
	std::string	const & otp_name,
	int		const	otp_bucket,
	int		const	bucket_position
	) const
{
	try
	{
		char			buf_date[64] = { '?' };
		time_t			t;
		struct tm		* tmp;
		char	const * const	fmt = "%Y-%m-%d %H:%M:%S";

		t = time ( NULL );
		tmp = gmtime ( &t );

		if ( tmp )
		{
			strftime ( buf_date, sizeof(buf_date), fmt, tmp );
		}

		mtDW::SqliteAddRecord rec ( m_db, DB_TABLE_LOG_ENCODE );

		rec.add_field ( DB_FIELD_DATETIME );
		rec.add_field ( DB_FIELD_FILENAME );
		rec.add_field ( DB_FIELD_FILESIZE );
		rec.add_field ( DB_FIELD_MODE );
		rec.add_field ( DB_FIELD_OTP_BUCKET );
		rec.add_field ( DB_FIELD_BUCKET_POS );
		rec.add_field ( DB_FIELD_OTP_NAME );

		rec.end_field ();

		rec.set_text ( buf_date );
		rec.set_blob ( filename, strlen ( filename ) );
		rec.set_integer ( (sqlite3_int64)filesize );
		rec.set_integer ( mode );
		rec.set_integer ( otp_bucket );
		rec.set_integer ( bucket_position );
		rec.set_text ( otp_name.c_str () );

		rec.insert_record ();
	}
	catch ( ... )
	{
	}
}

