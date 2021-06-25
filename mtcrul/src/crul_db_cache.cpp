/*
	Copyright (C) 2020-2021 Mark Tyler

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



void Crul::DB::clear_cache ()
{
	clear_cache_index ( CACHE_TYPE_LOW );
	clear_cache_index ( CACHE_TYPE_MEDIUM );
	clear_cache_index ( CACHE_TYPE_HIGH );
}

char const * Crul::DB::get_cache_name ( int const type )
{
	switch ( type )
	{
	case CACHE_TYPE_HIGH:	return DB_NAME_HIGH;
	case CACHE_TYPE_MEDIUM:	return DB_NAME_MEDIUM;
	case CACHE_TYPE_LOW:	return DB_NAME_LOW;
	}

	return NULL;
}

int Crul::DB::load_cache (
	int		const	type,
	std::vector<mtGin::GL::VertexRGB> * const cloud
	)
{
	char const * const name = get_cache_name ( type );

	if ( ! name )
	{
		std::cerr << "DB::load_cache - no such type =" << type << "\n";
		return 1;
	}

	try
	{
		char const * sql =
			"SELECT "	DB_FIELD_TOTAL
			" FROM "	DB_TABLE_CACHE_PTS_IDX
			" WHERE "	DB_FIELD_ID	" = ?1";

		mtKit::SqliteGetRecord idx ( m_db, sql );
		idx.stmt.bind_int64 ( 1, type );

		if ( 0 != idx.next () )
		{
			std::cerr << "Unable to get cache index for type = "
				<< name << "\n";
			return 1;
		}

		int cache_tot;

		if ( idx.get_int ( cache_tot ) )
		{
			std::cerr << "Unable to get cache total for type = "
				<< name << "\n";
			return 1;
		}

		if ( cache_tot < 1 )
		{
			return 0;
		}

		mtKit::ByteFileRead file;
		std::string filename ( m_dir );

		filename += "cache_";
		filename += name;
		filename += ".bin";

		if ( file.open ( filename.c_str (), 0 ) )
		{
			std::cerr << "DB::load_cache - unable to open file "
				<< filename << "\n";
			return 1;
		}

		size_t const len = sizeof((*cloud)[0]);

		cloud->resize ( (size_t)cache_tot );

		int vec_done = 0;
		mtGin::GL::VertexRGB * dest = cloud->data ();

		for ( ; vec_done < cache_tot; vec_done++ )
		{
			size_t const loaded = file.read ( dest, len );

			if ( loaded != len )
			{
				std::cerr << "Cache file ended prematurely\n";
				break;
			}

			dest++;
		}

		if ( vec_done != cache_tot )
		{
			std::cerr << "Mismatch: cache_tot=" << cache_tot
				<< " vec_done=" << vec_done
				<< "\n";

			cloud->resize ( (size_t)vec_done );
		}
	}
	catch (...)
	{
		return ERROR_EXCEPTION;
	}

	return 0;
}

int Crul::DB::save_cache (
	int					const	type,
	std::vector<mtGin::GL::VertexRGB> const * const	cloud
	)
{
	char const * const name = get_cache_name ( type );

	if ( ! name )
	{
		std::cerr << "DB::save_cache - no such type =" << type << "\n";
		return 1;
	}

	if ( cloud->size () < 1 )
	{
		return 0;
	}

	try
	{
		mtKit::SqliteTransaction trans ( m_db );

		mtKit::SqliteAddRecord rec_idx ( m_db, DB_TABLE_CACHE_PTS_IDX );

		rec_idx.add_field ( DB_FIELD_ID );
		rec_idx.add_field ( DB_FIELD_NAME );
		rec_idx.add_field ( DB_FIELD_TOTAL );
		rec_idx.end_field ();

		rec_idx.set_integer ( type );
		rec_idx.set_text ( name );
		rec_idx.set_integer ( (sqlite3_int64)cloud->size () );
		rec_idx.insert_record ();

		mtKit::ByteFileWrite file;
		std::string filename ( m_dir );

		filename += "cache_";
		filename += name;
		filename += ".bin";

		if ( file.open ( filename.c_str () ) )
		{
			std::cerr << "DB::save_cache - unable to open file "
				<< filename << "\n";
			return 1;
		}

		size_t const todo = cloud->size ();
		mtGin::GL::VertexRGB const * const mem = cloud->data ();
		size_t const len = sizeof( mem[0] );

		for ( size_t i = 0; i < todo; i++ )
		{
			if ( file.write ( &mem[i], len ) )
			{
				std::cerr << "DB::save_cache - error writing "
					"to file " << filename << "\n";
				return 1;
			}
		}
	}
	catch (...)
	{
		return ERROR_EXCEPTION;
	}

	return 0;
}

