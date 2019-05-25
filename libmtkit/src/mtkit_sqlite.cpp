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

#include <sqlite3.h>

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

int mtKit::Sqlite::empty_table ( std::string const & table ) const
{
	return exec_sql ( "DELETE FROM " + table +  ";" );
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
	int count = 0;

	std::string const sql = "SELECT COUNT(*) FROM " + table;

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
	)
{
	char buf[32];

	snprintf ( buf, sizeof(buf), "_%i", suffix );

	std::string sql = "ALTER TABLE ";
	sql += table;
	sql += " RENAME TO ";
	sql += table;
	sql += buf;

	exec_sql ( sql.c_str () );
}



/// SqliteAddRecord ------------------------------------------------------------

class RecordField
{
public:
	enum
	{
		FIELD_TEXT,
		FIELD_BLOB,
		FIELD_INTEGER
	};



	RecordField ( char const * name, char const * value )
		:
		m_type		(FIELD_TEXT),
		m_name		(name),
		m_mem_value	(value),
		m_mem_len	(0),
		m_int_value	(0)
	{}

	RecordField ( char const * name, char const * value, int value_len )
		:
		m_type		(FIELD_BLOB),
		m_name		(name),
		m_mem_value	(value),
		m_mem_len	(value_len),
		m_int_value	(0)
	{}

	RecordField ( char const * name, sqlite3_int64 value )
		:
		m_type		(FIELD_INTEGER),
		m_name		(name),
		m_mem_value	(),
		m_mem_len	(0),
		m_int_value	(value)
	{}

/// ----------------------------------------------------------------------------

	int			m_type;
	char	const * const	m_name;
	char	const * const	m_mem_value;
	int			m_mem_len;
	sqlite3_int64		m_int_value;
};



namespace mtKit
{

template <typename T> class ArrayList
{
public:
	ArrayList ()
		:
		m_size		( 0 ),
		m_array_tot	( m_block ),
		m_array		( (T **)calloc ( sizeof(T*), m_array_tot ) )
	{
		if ( ! m_array )
		{
			throw 123;
		}
	}

	~ArrayList ()
	{
		for ( size_t i = 0; i < m_size; i++ )
		{
			delete m_array[ i ];
		}

		free ( m_array );
	}

	void add ( T * t )		// Throw & delete t on fail
	{
		if ( m_size == m_array_tot )
		{
			size_t const new_tot = m_array_tot + m_block;
			T ** const new_array = (T **)realloc ( m_array,
				new_tot * sizeof(T*) );

			if ( ! new_array )
			{
				delete t;
				throw 123;
			}

			m_array_tot = new_tot;
			m_array = new_array;
		}

		m_array[ m_size ] = t;
		m_size++;
	}

	inline size_t size () const { return m_size; };
	inline T ** get_array () const { return m_array; };

private:
	static size_t const	m_block = 64;
	size_t			m_size;
	size_t			m_array_tot;	// Allocated items in m_array
	T		**	m_array;

	ArrayList ( const ArrayList & );	// Disable copy constructor
	ArrayList & operator = (const ArrayList &);	// Disable = operator
};



class SqliteAddRecordOp
{
public:
	SqliteAddRecordOp ( Sqlite const & db, char const * const table )
		:
		m_db		( db ),
		m_table		( table )
	{
	}

	~SqliteAddRecordOp () {}

/// ----------------------------------------------------------------------------

	Sqlite		const &	m_db;
	char	const * const	m_table;

	ArrayList<RecordField>	m_fields;
};

}	// namespace mtKit



mtKit::SqliteAddRecord::SqliteAddRecord (
	Sqlite		const &	db,
	char	const *	const	table
	)
	:
	op ( new SqliteAddRecordOp ( db, table ) )
{
}

mtKit::SqliteAddRecord::~SqliteAddRecord ()
{
	delete ( op );
}

void mtKit::SqliteAddRecord::text (
	char	const * const	name,
	char	const * const	value
	) const
{
	op->m_fields.add ( new RecordField ( name, value ) );
}

void mtKit::SqliteAddRecord::blob (
	char	const * const	name,
	char	const * const	value,
	int		const	value_len
	) const
{
	op->m_fields.add ( new RecordField ( name, value, value_len ) );
}

void mtKit::SqliteAddRecord::integer (
	char	const * const	name,
	sqlite3_int64	const	value
	) const
{
	op->m_fields.add ( new RecordField ( name, value ) );
}

void mtKit::SqliteAddRecord::insert () const
{
	size_t const tot = op->m_fields.size ();

	if ( tot < 1 )
	{
		return;
	}

	RecordField const * const * const field = op->m_fields.get_array ();

	std::string sql = "INSERT INTO ";
	sql += op->m_table;
	sql += " ( ";
	sql += field[0]->m_name;

	std::string values = " ) VALUES ( ?";

	for ( size_t i = 1; i < tot; i++ )
	{
		sql += ", ";
		sql += field[i]->m_name;

		values += ", ?";
	}

	sql += values;
	sql += " )";

	SqliteStmt ss ( op->m_db, sql );

	if ( SQLITE_OK != ss.err )
	{
		throw 123;
	}

	for ( size_t i = 0; i < tot; i++ )
	{
		int		const	ii = (int)(i + 1);
		char	const * const	mem = field[i]->m_mem_value;
		int		const	len = field[i]->m_mem_len;
		sqlite3_int64	const	i64 = field[i]->m_int_value;

		switch ( field[i]->m_type )
		{
		case RecordField::FIELD_TEXT:
			ss.bind_text ( ii, mem );
			break;

		case RecordField::FIELD_BLOB:
			ss.bind_blob ( ii, mem, len );
			break;

		case RecordField::FIELD_INTEGER:
			ss.bind_int64 ( ii, i64 );
			break;

		default:
			throw 123;
		}

		if ( SQLITE_OK != ss.err )
		{
			throw 123;
		}
	}

	if ( ss.step () )
	{
		throw 123;
	}
}



/// SqliteGetRecord ------------------------------------------------------------



mtKit::SqliteGetRecord::SqliteGetRecord (
	Sqlite		&	db,
	std::string	const &	sql
	)
	:
	stmt		( db, sql )
{
	if ( SQLITE_OK != stmt.err )
	{
		throw 123;
	}
}

mtKit::SqliteGetRecord::~SqliteGetRecord ()
{
}

int mtKit::SqliteGetRecord::next ()
{
	if ( SQLITE_ROW == stmt.step () )
	{
		return 0;
	}

	return 1;
}

int mtKit::SqliteGetRecord::get_type ( int const arg )
{
	return sqlite3_column_type ( stmt.stmt, arg );
}

int mtKit::SqliteGetRecord::get_blob (
	int	const	arg,
	std::string	& res
	)
{
	switch ( get_type ( arg ) )
	{
	case SQLITE_BLOB:
	case SQLITE_TEXT:
		// Valid
		break;

	case SQLITE_NULL:
		return 1;	// NULL

	default:
		return 2;	// Mismatched type
	}

	int const size = sqlite3_column_bytes ( stmt.stmt, arg );
	void const * const data = sqlite3_column_blob ( stmt.stmt, arg );

	mtKit::string_from_data ( res, data, (size_t)size );

	return 0;
}

int mtKit::SqliteGetRecord::get_text (
	int	const	arg,
	std::string	& res
	)
{
	switch ( get_type ( arg ) )
	{
	case SQLITE_TEXT:
		// Valid
		break;

	case SQLITE_NULL:
		return 1;	// NULL

	default:
		return 2;	// Mismatched type
	}

	int const size = sqlite3_column_bytes ( stmt.stmt, arg );
	void const * const data = sqlite3_column_blob ( stmt.stmt, arg );

	mtKit::string_from_data ( res, data, (size_t)size );

	return 0;
}

int mtKit::SqliteGetRecord::get_int (
	int	const	arg,
	int		& res
	)
{
	switch ( get_type ( arg ) )
	{
	case SQLITE_INTEGER:
		// Valid
		break;

	case SQLITE_NULL:
		return 1;	// NULL

	default:
		return 2;	// Mismatched type
	}

	res = (int)sqlite3_column_int64 ( stmt.stmt, arg );

	return 0;
}

int mtKit::SqliteGetRecord::get_int64 (
	int	const	arg,
	int64_t		& res
	)
{
	switch ( get_type ( arg ) )
	{
	case SQLITE_INTEGER:
		// Valid
		break;

	case SQLITE_NULL:
		return 1;	// NULL

	default:
		return 2;	// Mismatched type
	}

	res = (int64_t)sqlite3_column_int64 ( stmt.stmt, arg );

	return 0;
}

int mtKit::SqliteGetRecord::get_double (
	int	const	arg,
	double		& res
	)
{
	switch ( get_type ( arg ) )
	{
	case SQLITE_FLOAT:
		// Valid
		break;

	case SQLITE_NULL:
		return 1;	// NULL

	default:
		return 2;	// Mismatched type
	}

	res = sqlite3_column_double ( stmt.stmt, arg );

	return 0;
}

