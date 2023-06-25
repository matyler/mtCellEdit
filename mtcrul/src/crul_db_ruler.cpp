/*
	Copyright (C) 2020-2022 Mark Tyler

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



int Crul::DB::load_rulers ( std::map<int, Crul::Ruler> * const map )
{
	if ( ! map )
	{
		return 1;
	}

	try
	{
		map->clear ();

		char const * const sql =
			"SELECT "	DB_FIELD_ID		","
					DB_FIELD_LABEL		","
					DB_FIELD_X1		","
					DB_FIELD_Y1		","
					DB_FIELD_Z1		","
					DB_FIELD_X2		","
					DB_FIELD_Y2		","
					DB_FIELD_Z2		","
					DB_FIELD_RGB		","
					DB_FIELD_VISIBLE	","
					DB_FIELD_PLANE		","
					DB_FIELD_PROTECTED

			" FROM " DB_TABLE_RULER_LIST
			;

		mtDW::SqliteGetRecord rec ( m_db, sql );
		Crul::Ruler ruler;

		for ( int i = 0; 0 == rec.next (); i++ )
		{
			std::string label;
			int id = 0, vis = 0, rgb = 0, plane = 0, prot = 0;
			double x1=0.0, y1=0.0, z1=0.0, x2=0.0, y2=0.0, z2=0.0;

			rec.get_int ( id );
			rec.get_text ( label );
			rec.get_double ( x1 );
			rec.get_double ( y1 );
			rec.get_double ( z1 );
			rec.get_double ( x2 );
			rec.get_double ( y2 );
			rec.get_double ( z2 );
			rec.get_int ( rgb );
			rec.get_int ( vis );
			rec.get_int ( plane );
			rec.get_int ( prot );

			ruler.set_id ( id );
			ruler.set_label ( label );
			ruler.set_line ( x1, y1, z1, x2, y2, z2 );
			ruler.set_line_rgb ( rgb );
			ruler.set_plane ( plane );
			ruler.set_visible ( vis ? true : false );
			ruler.set_read_only ( prot ? true : false );

			map->insert (std::pair<int,Crul::Ruler>(id, ruler));
		}
	}
	catch (...)
	{
		std::cerr << "Unable to load rulers from DB\n";
		return 1;
	}

	return 0;
}

int Crul::DB::save_rulers ( std::map<int, Crul::Ruler> const * const map )
{
	if ( ! map || ! m_db.get_sqlite3 () )
	{
		return 1;
	}

	try
	{
		m_db.empty_table ( DB_TABLE_RULER_LIST );

		mtDW::SqliteTransaction trans ( m_db );
		mtDW::SqliteAddRecord rec ( m_db, DB_TABLE_RULER_LIST );

		rec.add_field ( DB_FIELD_ID );
		rec.add_field ( DB_FIELD_LABEL );
		rec.add_field ( DB_FIELD_X1 );
		rec.add_field ( DB_FIELD_Y1 );
		rec.add_field ( DB_FIELD_Z1 );
		rec.add_field ( DB_FIELD_X2 );
		rec.add_field ( DB_FIELD_Y2 );
		rec.add_field ( DB_FIELD_Z2 );
		rec.add_field ( DB_FIELD_RGB );
		rec.add_field ( DB_FIELD_VISIBLE );
		rec.add_field ( DB_FIELD_PLANE );
		rec.add_field ( DB_FIELD_PROTECTED );

		rec.end_field ();

		for ( auto && i : *map )
		{
			Crul::Ruler const & ruler = i.second;
			Crul::Line const & line = ruler.get_line ();
			int const rgb = line.get_rgb_int ();

			rec.set_integer ( ruler.get_id () );
			rec.set_text ( ruler.get_label ().c_str () );
			rec.set_real ( line.x1 );
			rec.set_real ( line.y1 );
			rec.set_real ( line.z1 );
			rec.set_real ( line.x2 );
			rec.set_real ( line.y2 );
			rec.set_real ( line.z2 );
			rec.set_integer ( rgb );
			rec.set_integer ( ruler.get_visible () ? 1 : 0 );
			rec.set_integer ( ruler.get_plane () );
			rec.set_integer ( ruler.get_read_only () ? 1 : 0 );

			rec.insert_record ();
		}
	}
	catch (...)
	{
		std::cerr << "Unable to save rulers\n";
		return 1;
	}

	return 0;
}

