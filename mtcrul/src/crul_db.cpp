/*
	Copyright (C) 2020 Mark Tyler

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

#include "crul_db.h"



Crul::DB::DB ()
{
}

Crul::DB::~DB ()
{
}

int Crul::DB::open ( std::string const & filename )
{
	if ( m_db.open ( filename ) )
	{
		return 1;
	}

	m_filename = filename;

	int const version = m_db.get_version ();

	if ( DB_SCHEMA_VERSION == version )
	{
		// We are using an up to date schema, nothing to set up.

		return 0;
	}

	if ( DB_SCHEMA_VERSION < version )
	{
		// Archive tables created by FUTURE versions of this library
	}
	else if ( version > 0 )		// SCHEMA_VERSION > version
	{
		// NOTE: migration between old table versions goes here
	}
	// else version < 1 so assume nothing exists (i.e. first time run)

/// NOTE: when changing the following schema, bump SCHEMA_VERSION and deal with
/// any migration of data as required.

///	CACHE	----------------------------------------------------------------

	m_db.exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_CACHE_PTS " ( "
		DB_FIELD_ID " INTEGER, "
		DB_FIELD_ITEM " INTEGER, "
		"PRIMARY KEY( " DB_FIELD_ID ", " DB_FIELD_ITEM " )"
		" );" );
	m_db.add_column_field ( DB_TABLE_CACHE_PTS, DB_FIELD_MEM,
		"BLOB" );

	m_db.exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_CACHE_PTS_MOD
		" ( "
		DB_FIELD_ID " INTEGER, "
		DB_FIELD_ITEM " INTEGER, "
		"PRIMARY KEY( " DB_FIELD_ID ", " DB_FIELD_ITEM " )"
		" );" );
	m_db.add_column_field ( DB_TABLE_CACHE_PTS_MOD, DB_FIELD_MEM,
		"BLOB" );

	m_db.exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_CACHE_PTS_IDX
		" ( " DB_FIELD_ID " INTEGER PRIMARY KEY );" );
	m_db.add_column_field ( DB_TABLE_CACHE_PTS_IDX, DB_FIELD_NAME,
		"TEXT" );
	m_db.add_column_field ( DB_TABLE_CACHE_PTS_IDX, DB_FIELD_TOTAL,
		"INTEGER" );

	m_db.exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_RULER_LIST
		" ( " DB_FIELD_ID " INTEGER PRIMARY KEY );" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_LABEL,
		"TEXT" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_X1,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_Y1,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_Z1,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_X2,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_Y2,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_Z2,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_RGB,
		"INTEGER" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_VISIBLE,
		"INTEGER" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_PLANE,
		"INTEGER" );
	m_db.add_column_field ( DB_TABLE_RULER_LIST, DB_FIELD_PROTECTED,
		"INTEGER" );

	m_db.exec_sql ( "CREATE TABLE IF NOT EXISTS " DB_TABLE_CAMERA_LIST
		" ( " DB_FIELD_ID " INTEGER PRIMARY KEY );" );
	m_db.add_column_field ( DB_TABLE_CAMERA_LIST, DB_FIELD_LABEL,
		"TEXT" );
	m_db.add_column_field ( DB_TABLE_CAMERA_LIST, DB_FIELD_X,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_CAMERA_LIST, DB_FIELD_Y,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_CAMERA_LIST, DB_FIELD_Z,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_CAMERA_LIST, DB_FIELD_ROT_X,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_CAMERA_LIST, DB_FIELD_ROT_Z,
		"FLOAT" );
	m_db.add_column_field ( DB_TABLE_CAMERA_LIST, DB_FIELD_PROTECTED,
		"INTEGER" );


	m_db.set_version ( DB_SCHEMA_VERSION );

	return 0;
}

void Crul::DB::clear_cache_index ( int const id )
{
	mtKit::SqliteStmt stmt ( m_db );

	char const * sql =
		"DELETE FROM "	DB_TABLE_CACHE_PTS_IDX
		" WHERE "	DB_FIELD_ID	" = ?1";

	stmt.prepare ( sql );
	stmt.bind_int64 ( 1, id );
	stmt.step ();
	stmt.reset ();
}

