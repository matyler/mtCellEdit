/*
	Copyright (C) 2008-2024 Mark Tyler

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

#ifndef MTDATAWELL_SQLITE_H_
#define MTDATAWELL_SQLITE_H_

#include <sqlite3.h>

#include <mtdatawell.h>



#ifdef __cplusplus
extern "C" {
#endif


// C API


#ifdef __cplusplus
}

// C++ API


namespace mtDW
{

class Sqlite;
class SqliteAddRecord;
class SqliteAddRecordField;
class SqliteGetRecord;
class SqliteStmt;
class SqliteTransaction;



class Sqlite
{
public:
	Sqlite () : m_db () {}
	~Sqlite () { set_sqlite3 ( NULL ); }

	int open ( std::string const & filename );
	void set_sqlite3 ( sqlite3 * db );
	sqlite3 * get_sqlite3 () const { return m_db; };

	int exec_sql ( std::string const & sql ) const
		{ return exec_sql ( sql.c_str () ); };
	int exec_sql ( char const * const sql ) const;

	// RULE: table/field names must be alphanumeric only (e.g. no spaces)
	int add_column_field (
		std::string const & table_name,
		std::string const & field_name,
		std::string const & field_type	// TEXT, REAL, INTEGER, BLOB
		) const;

	int empty_table ( std::string const & table ) const;
	int count_rows ( std::string const & table ) const;

	int get_version () const;
	void set_version ( int version );

	void archive_table ( char const * table, int suffix ) const;

	void vacuum () const;		// Reduce size of disk file

protected:
	sqlite3		* m_db;
};



class SqliteStmt
{
public:
	explicit SqliteStmt ( Sqlite const & db );
	~SqliteStmt ();

	int prepare ( std::string const & sql );
	int step ();
	int reset ();
	int bind_text ( int item, char const * text );
	int bind_blob ( int item, void const * mem, int mem_len );
	int bind_int64 ( int item, sqlite3_int64 num );
	int bind_real ( int item, double num );

/// ----------------------------------------------------------------------------

	Sqlite	const & m_db;
	sqlite3_stmt	* stmt;
	int		err;
};



class SqliteAddRecordField
{
public:
	enum
	{
		FIELD_TEXT,
		FIELD_BLOB,
		FIELD_INTEGER,
		FIELD_REAL
	};

	SqliteAddRecordField ()
		:
		m_type		( FIELD_INTEGER ),
		m_mem_value	( NULL ),
		m_mem_len	( 0 ),
		m_int_value	( 0 ),
		m_real_value	( 0.0 )
	{}

/// ----------------------------------------------------------------------------

	int		m_type;
	char	const *	m_mem_value;
	size_t		m_mem_len;
	sqlite3_int64	m_int_value;
	double		m_real_value;
};



class SqliteAddRecord		// All items throw on fail
{
public:
	// NOTE: all text/memory values must be valid when insert is called.

	SqliteAddRecord ( Sqlite const & db, char const * table );

	void add_field ( char const * name );
	void end_field ();

	void set_text ( char const * value );
	void set_blob ( char const * value, size_t value_len );
	void set_integer ( sqlite3_int64 value );
	void set_real ( double value );

	void insert_record ();

private:
	SqliteStmt		m_stmt;
	Sqlite		const	& m_db;
	std::string		m_sql;
	std::vector<SqliteAddRecordField> m_field;
	size_t			m_field_num;
};



class SqliteTransaction
{
public:
	explicit SqliteTransaction ( Sqlite const & db )
		:
		m_db		( db )
	{
		m_db.exec_sql ( "BEGIN TRANSACTION" );
	}

	~SqliteTransaction ()
	{
		m_db.exec_sql ( "COMMIT" );
	}

protected:
	Sqlite	const	& m_db;
};



class SqliteGetRecord
{
public:
	SqliteGetRecord ( Sqlite & db, std::string const & sql ); // throws
	~SqliteGetRecord ();

	int next ();
		// 0 = Record found, 1 = None found

	int get_field_num () const { return m_field_num; }

	int get_type ();
		// = SQLITE_< INTEGER | FLOAT | TEXT | BLOB | NULL >

	/* The following functions all return:
		0 = Success
		1 = NULL
		2 = Other type mismatch
	*/
//	int get_blob ( void const ** mem, int & memlen );
	int get_blob_text ( std::string & res ); // blob/text -> String
	int get_text ( std::string & res );
	int get_int ( int & res );
	int get_int64 ( int64_t & res );
	int get_double ( double & res );

/// ----------------------------------------------------------------------------

	SqliteStmt	stmt;

private:
	int		m_field_num;
};



}		// namespace mtDW



#endif		// __cplusplus



#endif		// MTDATAWELL_SQLITE_H_

