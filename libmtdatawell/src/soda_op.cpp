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

#include "soda.h"



#define DB_FILENAME		"soda.sqlite"

#define DB_TABLE_LOG_ENCODE	"Log_Encode"

#define DB_FIELD_KEY		"Key"		// Allocated by SQLite
#define DB_FIELD_DATETIME	"UTC_Time"	// YYYY-MM-DD HH:MM:SS UTC/GMT
#define DB_FIELD_FILENAME	"Filename"
#define DB_FIELD_FILESIZE	"Filesize"
#define DB_FIELD_MODE		"Mode"		// 0=xor 1=raw, etc
#define DB_FIELD_BUTT_BUCKET	"Butt_Bucket"	// Only used for encoding=xor
#define DB_FIELD_BUCKET_POS	"Bucket_Pos"	// Only used for encoding=xor
#define DB_FIELD_BUTT_NAME	"Butt_Name"	// Only used for encoding=xor

#define MODE_BIT_ENCODE_RAW	0x00000001	// Set = encode raw (no XOR)



mtDW::SodaOp::SodaOp ( char const * const path )
	:
	m_mode		( 0 )
{
	std::string const real_path = mtDW::prepare_path ( path );

	if (	m_db.open ( real_path + DB_FILENAME )
		||
		m_db.exec_sql( "CREATE TABLE IF NOT EXISTS " DB_TABLE_LOG_ENCODE
		" ("
		" " DB_FIELD_KEY " INTEGER PRIMARY KEY"		","
		" " DB_FIELD_DATETIME		" DATE"		","
		" " DB_FIELD_FILENAME		" BLOB"		","
		" " DB_FIELD_FILESIZE		" INTEGER"	","
		" " DB_FIELD_MODE		" INTEGER"	","
		" " DB_FIELD_BUTT_NAME		" TEXT"		","
		" " DB_FIELD_BUTT_BUCKET	" INTEGER"	","
		" " DB_FIELD_BUCKET_POS		" INTEGER"
		" );" )
		)
	{
		throw 123;
	}
}

mtDW::SodaOp::~SodaOp ()
{
}

int mtDW::SodaOp::encode_raw () const
{
	if ( m_mode & MODE_BIT_ENCODE_RAW )
	{
		return 1;
	}

	return 0;
}

void mtDW::SodaOp::db_add_encode (
	char	const * const	filename,
	uint64_t	const	filesize,
	int		const	mode,
	std::string	const & bucket_name,
	int		const	butt_bucket,
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

		mtKit::SqliteAddRecord rec ( m_db, DB_TABLE_LOG_ENCODE );

		rec.text ( DB_FIELD_DATETIME, buf_date );
		rec.blob ( DB_FIELD_FILENAME, filename, (int)strlen(filename) );
		rec.integer ( DB_FIELD_FILESIZE, (sqlite3_int64)filesize );
		rec.integer ( DB_FIELD_MODE, mode );
		rec.integer ( DB_FIELD_BUTT_BUCKET, butt_bucket );
		rec.integer ( DB_FIELD_BUCKET_POS, bucket_position );
		rec.text ( DB_FIELD_BUTT_NAME, bucket_name.c_str () );

		rec.insert ();
	}
	catch ( ... )
	{
	}
}

