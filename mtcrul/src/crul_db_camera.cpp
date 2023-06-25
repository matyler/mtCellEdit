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



int Crul::DB::load_cameras ( std::map<int, Crul::Camera> * const map )
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
					DB_FIELD_X		","
					DB_FIELD_Y		","
					DB_FIELD_Z		","
					DB_FIELD_ROT_X		","
					DB_FIELD_ROT_Z		","
					DB_FIELD_PROTECTED

			" FROM " DB_TABLE_CAMERA_LIST
			;

		mtDW::SqliteGetRecord rec ( m_db, sql );
		Crul::Camera cam;

		for ( int i = 0; 0 == rec.next (); i++ )
		{
			int id = 0, prot = 0;
			std::string label;
			double x = 0.0, y = 0.0, z = 0.0, rotX = 0.0, rotZ =0.0;

			rec.get_int ( id );
			rec.get_text ( label );
			rec.get_double ( x );
			rec.get_double ( y );
			rec.get_double ( z );
			rec.get_double ( rotX );
			rec.get_double ( rotZ );
			rec.get_int ( prot );

			cam.set_id ( id );
			cam.set_label ( label );
			cam.set_position ( x, y, z );
			cam.set_angle ( rotX, 0, rotZ );
			cam.set_read_only ( prot ? true : false );

			map->insert (std::pair<int,Crul::Camera>(id,cam));
		}

		// Ensure that the defaults are always available at ID=1,2

		cam.set_id ( 1 );
		cam.set_label ( "Default A" );
		cam.set_position ( Crul::CAM_A_X_DEFAULT, Crul::CAM_A_Y_DEFAULT,
			Crul::CAM_A_Z_DEFAULT );
		cam.set_angle ( Crul::CAM_A_XROT_DEFAULT, 0,
			Crul::CAM_A_ZROT_DEFAULT );
		cam.set_read_only ( true );

		map->insert( std::pair<int, Crul::Camera>(cam.get_id(), cam) );

		cam.set_id ( 2 );
		cam.set_label ( "Default B" );
		cam.set_position ( Crul::CAM_B_X_DEFAULT, Crul::CAM_B_Y_DEFAULT,
			Crul::CAM_B_Z_DEFAULT );
		cam.set_angle ( Crul::CAM_B_XROT_DEFAULT, 0,
			Crul::CAM_B_ZROT_DEFAULT );
		cam.set_read_only ( true );

		map->insert( std::pair<int, Crul::Camera>(cam.get_id(), cam) );
	}
	catch (...)
	{
		std::cerr << "Unable to load cameras from DB\n";
		return 1;
	}

	return 0;
}

int Crul::DB::save_cameras ( std::map<int, Crul::Camera> const * const map )
{
	if ( ! map || ! m_db.get_sqlite3 () )
	{
		return 1;
	}

	try
	{
		m_db.empty_table ( DB_TABLE_CAMERA_LIST );

		mtDW::SqliteTransaction trans ( m_db );
		mtDW::SqliteAddRecord rec ( m_db, DB_TABLE_CAMERA_LIST );

		rec.add_field ( DB_FIELD_ID );
		rec.add_field ( DB_FIELD_LABEL );
		rec.add_field ( DB_FIELD_X );
		rec.add_field ( DB_FIELD_Y );
		rec.add_field ( DB_FIELD_Z );
		rec.add_field ( DB_FIELD_ROT_X );
		rec.add_field ( DB_FIELD_ROT_Z );
		rec.add_field ( DB_FIELD_PROTECTED );
		rec.end_field ();

		for ( auto && i : *map )
		{
			Crul::Camera const & cam = i.second;

			rec.set_integer ( cam.get_id() );
			rec.set_text ( cam.get_label().c_str () );
			rec.set_real ( cam.get_x() );
			rec.set_real ( cam.get_y() );
			rec.set_real ( cam.get_z() );
			rec.set_real ( cam.get_rot_x() );
			rec.set_real ( cam.get_rot_z() );
			rec.set_integer ( cam.get_read_only() ? 1 : 0 );
			rec.insert_record ();
		}
	}
	catch (...)
	{
		std::cerr << "Unable to save cameras\n";
		return 1;
	}

	return 0;
}

