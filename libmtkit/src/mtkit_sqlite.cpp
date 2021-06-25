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

#include "private.h"



int mtKit::Sqlite::open ( std::string const &filename )
{
	sqlite3 * db = NULL;

	if ( sqlite3_open ( filename.c_str (), &db ) )
	{
		std::cerr << "Unable to open " << filename << "\n";

		sqlite3_close ( db );
		db = NULL;

		return 1;
	}

	set_sqlite3 ( db );

	return 0;
}

void mtKit::Sqlite::set_sqlite3 ( sqlite3 * const db )
{
	sqlite3_close ( m_db );
	m_db = db;
}

int mtKit::Sqlite::exec_sql ( char const * const sql ) const
{
	if ( SQLITE_OK == sqlite3_exec ( m_db, sql, NULL, NULL, NULL ) )
	{
		return 0;
	}

	std::cerr << "ERROR executing SQL!! - " << sql << "\n";

	return 1;
}

int mtKit::Sqlite::add_column_field (
	std::string	const &	table_name,
	std::string	const &	field_name,
	std::string	const &	field_type
	) const
{
	std::string sql ( "ALTER TABLE " );

	sql += table_name;
	sql += " ADD ";
	sql += field_name;
	sql += " ";
	sql += field_type;

	return exec_sql ( sql );
}

int mtKit::Sqlite::empty_table ( std::string const & table ) const
{
	std::string	sql ( "DELETE FROM " );

	sql += table;

	return exec_sql ( sql );
}

static int count_rows_callback (
	void		* const	count,
	int		const	ARG_UNUSED ( argc ),
	char	**	const	argv,
	char	**	const	ARG_UNUSED ( col )
	)
{
	int * const c = (int *)count;

	c[0] = atoi ( argv[0] );

	return 0;
}

int mtKit::Sqlite::count_rows ( std::string const & table ) const
{
	int		count = 0;
	std::string	sql ( "SELECT COUNT(*) FROM " );

	sql += table;

	sqlite3_exec ( m_db, sql.c_str (), count_rows_callback, &count, NULL );

	return count;
}

int mtKit::Sqlite::get_version () const
{
	int version = 0;

	sqlite3_exec ( m_db, "PRAGMA user_version", count_rows_callback,
		&version, NULL );

	return version;
}

void mtKit::Sqlite::set_version ( int const version )
{
	char buf[64];

	snprintf ( buf, sizeof(buf), "PRAGMA user_version = %i", version );

	exec_sql ( buf );
}

void mtKit::Sqlite::archive_table (
	char	const * const	table,
	int		const	suffix
	) const
{
	char buf[32];

	snprintf ( buf, sizeof(buf), "_%i", suffix );

	std::string sql = "ALTER TABLE ";
	sql += table;
	sql += " RENAME TO ";
	sql += table;
	sql += buf;

	exec_sql ( sql );
}

void mtKit::Sqlite::vacuum () const
{
	exec_sql ( "VACUUM" );
}



/// SqliteAddRecord ------------------------------------------------------------



mtKit::SqliteAddRecord::SqliteAddRecord (
	Sqlite		const	& db,
	char	const * const	table
	)
	:
	m_stmt		( db ),
	m_db		( db ),
	m_sql		( "INSERT INTO " ),
	m_field_num	( 0 )
{
	m_sql += table;
	m_sql += " (";
}

void mtKit::SqliteAddRecord::add_field ( char const * const name )
{
	if ( m_field.size () > 0 )
	{
		m_sql += ",";
	}

	m_sql += name;

	m_field.push_back ( SqliteAddRecordField () );
}

void mtKit::SqliteAddRecord::end_field ()
{
	m_sql += ") VALUES (?";

	size_t const tot = m_field.size ();

	for ( size_t i = 1; i < tot; i++ )
	{
		m_sql += ",?";
	}

	m_sql += ")";

	if ( SQLITE_OK != m_stmt.prepare ( m_sql ) )
	{
		throw 123;
	}
}

void mtKit::SqliteAddRecord::set_text (
	char	const * const	value
	)
{
	m_field[ m_field_num ].m_type = SqliteAddRecordField::FIELD_TEXT;
	m_field[ m_field_num ].m_mem_value = value;

	m_field_num++;
}

void mtKit::SqliteAddRecord::set_blob (
	char	const * const	value,
	size_t		const	value_len
	)
{
	m_field[ m_field_num ].m_type = SqliteAddRecordField::FIELD_BLOB;
	m_field[ m_field_num ].m_mem_value = value;
	m_field[ m_field_num ].m_mem_len = value_len;

	m_field_num++;
}

void mtKit::SqliteAddRecord::set_integer (
	sqlite3_int64	const	value
	)
{
	m_field[ m_field_num ].m_type = SqliteAddRecordField::FIELD_INTEGER;
	m_field[ m_field_num ].m_int_value = value;

	m_field_num++;
}

void mtKit::SqliteAddRecord::set_real (
	double	const	value
	)
{
	m_field[ m_field_num ].m_type = SqliteAddRecordField::FIELD_REAL;
	m_field[ m_field_num ].m_real_value = value;

	m_field_num++;
}

void mtKit::SqliteAddRecord::insert_record ()
{
	m_field_num = 0;

	size_t const tot = m_field.size ();

	for ( size_t i = 0; i < tot; i++ )
	{
		SqliteAddRecordField const &rec = m_field[ i ];

		switch ( rec.m_type )
		{
		case SqliteAddRecordField::FIELD_TEXT:
			m_stmt.bind_text ( (int)(i + 1), rec.m_mem_value );
			break;

		case SqliteAddRecordField::FIELD_BLOB:
			m_stmt.bind_blob ( (int)(i + 1), rec.m_mem_value,
				(int)rec.m_mem_len );
			break;

		case SqliteAddRecordField::FIELD_INTEGER:
			m_stmt.bind_int64 ( (int)(i + 1), rec.m_int_value );
			break;

		case SqliteAddRecordField::FIELD_REAL:
			m_stmt.bind_real ( (int)(i + 1), rec.m_real_value );
			break;

		default:
			std::cerr << "Bad field type: " << rec.m_type << "\n";
			throw 123;
		}

		if ( SQLITE_OK != m_stmt.err )
		{
			std::cerr << "Sqlite error: " << m_stmt.err
				<< " " << sqlite3_errmsg ( m_db.get_sqlite3 () )
				<< "\n";
			throw 123;
		}
	}

	switch ( m_stmt.step () )
	{
	case SQLITE_OK:
	case SQLITE_DONE:
		break;
	default:
		std::cerr << "step error: " << m_sql << " err=" << m_stmt.err
			<< " " << sqlite3_errmsg ( m_db.get_sqlite3 () )
			<< "\n";
		throw 123;
	}

	m_stmt.reset ();
}



/// SqliteGetRecord ------------------------------------------------------------



mtKit::SqliteGetRecord::SqliteGetRecord (
	Sqlite		&	db,
	std::string	const &	sql
	)
	:
	stmt		( db ),
	m_field_num	( 0 )
{
	if ( SQLITE_OK != stmt.prepare ( sql ) )
	{
		throw 123;
	}
}

mtKit::SqliteGetRecord::~SqliteGetRecord ()
{
}

int mtKit::SqliteGetRecord::next ()
{
	m_field_num = 0;

	if ( SQLITE_ROW == stmt.step () )
	{
		return 0;
	}

	return 1;
}

int mtKit::SqliteGetRecord::get_type ()
{
	return sqlite3_column_type ( stmt.stmt, m_field_num );
}

/*
int mtKit::SqliteGetRecord::get_blob (
	void	const ** mem,
	int		& memlen
	)
{
	switch ( get_type () )
	{
	case SQLITE_BLOB:
		// Valid
		break;

	case SQLITE_NULL:
		m_field_num++;
		return 1;	// NULL

	default:
		m_field_num++;
		return 2;	// Mismatched type
	}

	*mem = sqlite3_column_blob ( stmt.stmt, m_field_num );
	memlen = sqlite3_column_bytes ( stmt.stmt, m_field_num );

	m_field_num++;

	return 0;
}
*/

int mtKit::SqliteGetRecord::get_blob_text ( std::string & res )
{
	switch ( get_type () )
	{
	case SQLITE_BLOB:
	case SQLITE_TEXT:
		// Valid
		break;

	case SQLITE_NULL:
		m_field_num++;
		return 1;	// NULL

	default:
		m_field_num++;
		return 2;	// Mismatched type
	}

	void const * const data = sqlite3_column_blob ( stmt.stmt, m_field_num);
	int const size = sqlite3_column_bytes ( stmt.stmt, m_field_num );

	res = std::string ( (char const *)data, (size_t)size );

	m_field_num++;

	return 0;
}

int mtKit::SqliteGetRecord::get_text (
	std::string	& res
	)
{
	switch ( get_type () )
	{
	case SQLITE_TEXT:
		// Valid
		break;

	case SQLITE_NULL:
		m_field_num++;
		return 1;	// NULL

	default:
		m_field_num++;
		return 2;	// Mismatched type
	}

	int const size = sqlite3_column_bytes ( stmt.stmt, m_field_num );
	void const * const data = sqlite3_column_blob ( stmt.stmt, m_field_num);

	res = std::string ( (char const *)data, (size_t)size );

	m_field_num++;

	return 0;
}

int mtKit::SqliteGetRecord::get_int ( int & res )
{
	switch ( get_type () )
	{
	case SQLITE_INTEGER:
		// Valid
		break;

	case SQLITE_NULL:
		m_field_num++;
		return 1;	// NULL

	default:
		m_field_num++;
		return 2;	// Mismatched type
	}

	res = (int)sqlite3_column_int64 ( stmt.stmt, m_field_num );

	m_field_num++;

	return 0;
}

int mtKit::SqliteGetRecord::get_int64 ( int64_t & res )
{
	switch ( get_type () )
	{
	case SQLITE_INTEGER:
		// Valid
		break;

	case SQLITE_NULL:
		m_field_num++;
		return 1;	// NULL

	default:
		m_field_num++;
		return 2;	// Mismatched type
	}

	res = (int64_t)sqlite3_column_int64 ( stmt.stmt, m_field_num );

	m_field_num++;

	return 0;
}

int mtKit::SqliteGetRecord::get_double ( double & res )
{
	switch ( get_type () )
	{
	case SQLITE_FLOAT:
		// Valid
		break;

	case SQLITE_NULL:
		m_field_num++;
		return 1;	// NULL

	default:
		m_field_num++;
		return 2;	// Mismatched type
	}

	res = sqlite3_column_double ( stmt.stmt, m_field_num );

	m_field_num++;

	return 0;
}



/// SqliteStmt -----------------------------------------------------------------



mtKit::SqliteStmt::SqliteStmt ( Sqlite const & db )
	:
	m_db	( db ),
	stmt	( NULL ),
	err	( 0 )
{
}

mtKit::SqliteStmt::~SqliteStmt ()
{
	sqlite3_finalize ( stmt );
	stmt = NULL;
}

int mtKit::SqliteStmt::prepare ( std::string const & sql )
{
	return (err = sqlite3_prepare_v2 ( m_db.get_sqlite3 (), sql.c_str(),
		-1, &stmt, 0 ) );
}

int mtKit::SqliteStmt::step ()
{
	return (err = sqlite3_step ( stmt ));
}

int mtKit::SqliteStmt::reset ()
{
	return (err = sqlite3_reset ( stmt ));
}

int mtKit::SqliteStmt::bind_text (
	int		const	item,
	char	const * const	text
	)
{
	return (err = sqlite3_bind_text ( stmt, item, text, -1, SQLITE_STATIC));
}

int mtKit::SqliteStmt::bind_blob (
	int		const	item,
	void	const * const	mem,
	int		const	mem_len
	)
{
	return (err = sqlite3_bind_blob ( stmt, item, mem, mem_len,
		SQLITE_STATIC ));
}

int mtKit::SqliteStmt::bind_int64 (
	int		const	item,
	sqlite3_int64	const	num
	)
{
	return (err = sqlite3_bind_int64 ( stmt, item, num ));
}

int mtKit::SqliteStmt::bind_real (
	int	const	item,
	double	const	num
	)
{
	return (err = sqlite3_bind_double ( stmt, item, num ));
}

