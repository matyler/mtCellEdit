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



#define MODEL_CACHE_FILENAME	"cache_model.bin"



void Crul::DB::clear_model ()
{
	try
	{
		clear_cache_index ( CACHE_TYPE_MODEL );
	}
	catch (...)
	{
	}
}

int Crul::DB::load_model (
	std::vector<mtGin::GL::VertexRGBnormal>	& model
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

		mtKit::ByteFileRead file;
		std::string filename ( m_dir );

		filename += MODEL_CACHE_FILENAME;

		if ( file.open ( filename.c_str (), 0 ) )
		{
			std::cerr << "DB::load_model - unable to open file "
				<< filename << "\n";
			return 1;
		}

		size_t const len = (int)sizeof(model[0]);

		model.resize ( (size_t)cache_tot );

		int vec_done = 0;
		mtGin::GL::VertexRGBnormal * dest = model.data ();

		for ( ; vec_done < cache_tot; vec_done++ )
		{
			size_t const loaded = file.read ( dest, len );

			if ( loaded != len )
			{
				std::cerr << "Model file ended prematurely\n";
				break;
			}

			dest++;
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
	std::vector<mtGin::GL::VertexRGBnormal>	const &	model
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

		mtKit::ByteFileWrite file;
		std::string filename ( m_dir );

		filename += MODEL_CACHE_FILENAME;

		if ( file.open ( filename.c_str () ) )
		{
			std::cerr << "DB::save_model - unable to open file "
				<< filename << "\n";
			return 1;
		}

		size_t const todo = model.size ();
		mtGin::GL::VertexRGBnormal const * const mem = model.data ();
		size_t const len = sizeof( mem[0] );

		for ( size_t i = 0; i < todo; i++ )
		{
			if ( file.write ( &mem[i], len ) )
			{
				std::cerr << "DB::save_model - error writing "
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

