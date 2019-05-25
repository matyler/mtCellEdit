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

#include "well.h"



#define SCHEMA_VERSION		3202

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
	if ( m_db.open ( filename ) )
	{
		return 1;
	}

	int const version = m_db.get_version ();

	if ( SCHEMA_VERSION == version )
	{
		// We are using an up to date schema, nothing to set up.
		return 0;
	}

	if ( SCHEMA_VERSION < version )
	{
		// Archive tables created by FUTURE versions of this library
		m_db.archive_table ( DB_TABLE_FILES, version );
	}
	else if ( version > 0 )		// SCHEMA_VERSION > version
	{
		// NOTE: migration between old table versions goes here
	}
	// else version < 1 so assume nothing exists (i.e. first time run)

/// NOTE: when changing the following schema, bump SCHEMA_VERSION and deal with
/// any migration of data as required.

	m_db.exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_FILES
		" ("
		" " DB_FIELD_ID		" INTEGER PRIMARY KEY"	","
		" " DB_FIELD_FILENAME	" BLOB"
		" );" );

	m_db.set_version ( SCHEMA_VERSION );

	return 0;
}

int mtDW::FileDB::add_table_filename (
	std::string	const & filename,
	char	const * const	table
	) const
{
	try
	{
		mtKit::SqliteAddRecord rec ( m_db, table );

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

		mtKit::SqliteGetRecord record ( m_db, sql );

		record.stmt.bind_int64 ( 1, (sqlite3_int64)m_file_id );

		if ( 0 == record.next () )
		{
			int64_t id64;

			if ( record.get_int64 ( 1, id64 ) )
			{
				std::cerr << "Field 1 isn't an integer\n";
				throw 123;
			}

			m_file_id = (uint32_t)id64;

			// Quietly ignore error as string remains empty
			record.get_blob ( 0, res );
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

	m_db.exec_sql ( sql + buf );
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
	return m_db.count_rows ( DB_TABLE_FILES );
}

void mtDW::FileDB::remove_all_files ()
{
	m_db.empty_table ( DB_TABLE_FILES );
	m_db.exec_sql ( "VACUUM" );
	m_file_id = 1;
}

