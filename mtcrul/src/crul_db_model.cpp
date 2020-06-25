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



void Crul::DB::clear_model ()
{
	try
	{
		m_db.empty_table ( DB_TABLE_CACHE_PTS_MOD );

		clear_cache_index ( CACHE_TYPE_MODEL );
	}
	catch (...)
	{
	}
}

int Crul::DB::load_model (
	std::vector<VertexGL>	& model
	)
{
	try
	{
		char const * sql =
			"SELECT "	DB_FIELD_TOTAL
			" FROM "	DB_TABLE_CACHE_PTS_IDX
			" WHERE "	DB_FIELD_ID	" = ?1";

		mtKit::SqliteGetRecord idx ( m_db, sql );
		idx.stmt.bind_int64 ( 1, CACHE_TYPE_MODEL );

		if ( 0 != idx.next () )
		{
			std::cerr << "Unable to get cache index for model.\n";
			return 1;
		}

		int cache_tot;

		if ( idx.get_int ( cache_tot ) )
		{
			std::cerr << "Unable to get cache total for model.\n";
			return 1;
		}

		if ( cache_tot < 1 )
		{
			return 0;
		}

		sql =	"SELECT "	DB_FIELD_MEM
			" FROM "	DB_TABLE_CACHE_PTS_MOD
			" WHERE "	DB_FIELD_ID	" = ?1"
			" ORDER BY "	DB_FIELD_ITEM;

		mtKit::SqliteGetRecord cache ( m_db, sql );
		cache.stmt.bind_int64 ( 1, CACHE_TYPE_MODEL );

		int const vec_len = (int)sizeof(model[0]);

		model.resize ( (size_t)cache_tot );

		int vec_todo = cache_tot;
		int vec_done = 0;
		VertexGL * vec_dest = model.data ();

		for ( int i = 0; cache.next () == 0; i++ )
		{
			void const * mem;
			int memlen;

			if ( cache.get_blob ( &mem, memlen ) || memlen < 1 )
			{
				std::cerr << "Unable to get model blob:"
					<< " item = " << i
					<< "\n";
				break;
			}

			int const vec_items = memlen / vec_len;

			if ( vec_items > vec_todo )
			{
				std::cerr << "Too much data in DB cache\n";
				break;
			}

			if ( vec_items < 1 )
			{
				std::cerr << "Too little data in DB cache\n";
				break;
			}

			memcpy ( vec_dest, mem, (size_t)vec_items * vec_len );

			vec_todo -= vec_items;
			vec_done += vec_items;
			vec_dest += vec_items;
		}

		if ( vec_done != cache_tot )
		{
			std::cerr << "Mismatch: cache_tot=" << cache_tot
				<< " vec_done=" << vec_done
				<< "\n";

			model.resize ( (size_t)vec_done );
		}
	}
	catch (...)
	{
		return ERROR_EXCEPTION;
	}

	return 0;
}

int Crul::DB::save_model (
	std::vector<VertexGL>	const &	model
	)
{
	if ( model.size () < 1 )
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

		rec_idx.set_integer ( CACHE_TYPE_MODEL );
		rec_idx.set_text ( DB_NAME_MODEL );
		rec_idx.set_integer ( (sqlite3_int64)model.size () );
		rec_idx.insert_record ();

		mtKit::SqliteAddRecord rec_data ( m_db, DB_TABLE_CACHE_PTS_MOD);

		rec_data.add_field ( DB_FIELD_ID );
		rec_data.add_field ( DB_FIELD_ITEM );
		rec_data.add_field ( DB_FIELD_MEM );
		rec_data.end_field ();

		size_t todo = model.size ();
		size_t offset = 0;
		VertexGL const * const mem = model.data ();

		for ( int i = 0; todo > 0; i++ )
		{
			size_t const items = MIN ( todo, DB_ITEM_SIZE );
			size_t const len = items * sizeof(mem[0]);

			rec_data.set_integer ( CACHE_TYPE_MODEL );
			rec_data.set_integer ( i );
			rec_data.set_blob ( (char const *)&mem[offset], len );
			rec_data.insert_record ();

			todo -= items;
			offset += items;
		}
	}
	catch (...)
	{
		return ERROR_EXCEPTION;
	}

	return 0;
}

