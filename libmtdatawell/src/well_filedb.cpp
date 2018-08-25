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

#include "well.h"



#define DB_TABLE_FILES		"Files"

#define DB_FIELD_ID		"id"
#define DB_FIELD_FILENAME	"filename"



mtDW::FileDB::FileDB ()
	:
	m_file_id ( 1 )
{
}

mtDW::FileDB::~FileDB ()
{
}

int mtDW::FileDB::open ( std::string const & filename )
{
	if (	mtKit::Sqlite::open ( filename )
		|| exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_FILES
		" ("
		" " DB_FIELD_ID		" INTEGER PRIMARY KEY"	","
		" " DB_FIELD_FILENAME	" BLOB"
		" );" )
		)
	{
		return 1;
	}

	return 0;
}

int mtDW::FileDB::add_table_filename (
	std::string	const & filename,
	char	const * const	table
	) const
{
	try
	{
		mtKit::SqliteAddRecord rec ( *this, table );

		rec.blob ( DB_FIELD_FILENAME, filename.c_str (),
			(int)filename.size () );

		rec.insert ();
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

int mtDW::FileDB::add_todo_filename ( std::string const & filename ) const
{
	return add_table_filename ( filename, DB_TABLE_FILES );
}

std::string const mtDW::FileDB::get_todo_filename_internal ()
{
	std::string res;

	try
	{
		static const std::string sql =
			"SELECT " DB_FIELD_FILENAME ", " DB_FIELD_ID
			" FROM " DB_TABLE_FILES
			" WHERE " DB_FIELD_ID " >= ?1"
			" LIMIT 1;";

		mtKit::SqliteGetRecord record ( *this, sql );

		record.stmt.bind_int64 ( 1, (sqlite3_int64)m_file_id );

		if ( 0 == record.next () )
		{
			record.blob_string ( 0, res );
			m_file_id = (uint32_t)record.integer ( 1 );
		}
	}
	catch ( ... )
	{
	}

	return res;
}

void mtDW::FileDB::remove_todo_filename ()
{
	static const std::string sql = "DELETE FROM " DB_TABLE_FILES
		" WHERE ID = ";

	char buf[16];
	snprintf ( buf, sizeof(buf), "%" PRIu32 ";", m_file_id );

	exec_sql ( sql + buf );
}

std::string const mtDW::FileDB::get_todo_filename ()
{
	std::string const res = get_todo_filename_internal ();

	if ( res.size () < 1 && m_file_id > 1 )
	{
		// We have no more rows ahead, so go back to the beginning

		m_file_id = 1;

		return get_todo_filename_internal ();
	}

	return res;
}

int mtDW::FileDB::count_files () const
{
	return count_rows ( DB_TABLE_FILES );
}

void mtDW::FileDB::remove_all_files () const
{
	empty_table ( DB_TABLE_FILES );
	exec_sql ( "VACUUM" );
}

