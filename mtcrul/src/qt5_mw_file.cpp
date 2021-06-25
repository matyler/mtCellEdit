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

#include "qt5_mw.h"



void Mainwindow::press_file_quit ()
{
	close ();
}

void Mainwindow::init_db_success ( int const fail )
{
	if ( fail )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to open the database from the filesystem." );
	}
	else
	{
		// Enable menu items that require a DB
		act_edit_import_pts->setEnabled ( true );
		act_edit_import_model->setEnabled ( true );

		update_recent_db_menu ();
		update_extents_clear ();

		m_fe.cloud.clear ();
		load_cloud_from_db ();
		load_model_from_db ();

		populate_camera_list ();
		populate_ruler_list ();
	}

	update_info ();
	press_view_select_a ();
}

void Frontend::save_db_state ()
{
	crul_db.save_rulers ( & ruler_map );
	crul_db.save_cameras ( & camera_map );
}

void Frontend::load_db_state ()
{
	crul_db.load_rulers ( & ruler_map );
	crul_db.load_cameras ( & camera_map );
}

int Mainwindow::database_load ( std::string const & path )
{
	int error = 0;
	mtQEX::BusyDialog	dialog	( this, "Opening Database.",
		[this, &error, &path]()
		{
			m_fe.save_db_state ();

			error = m_fe.crul_db.open ( path );

			if ( 0 == error )
			{
				m_fe.load_db_state ();
			}
		});
	dialog.wait_for_thread ();

	if ( 0 == error )
	{
		recent_crul_db.set ( path );
	}

	init_db_success ( error );

	return error;
}

void Mainwindow::press_file_open_db ()
{
	QString const filename = QFileDialog::getExistingDirectory (
		this, "Open Database", m_fe.crul_db.get_dir().c_str(),
		QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly );


	if ( ! filename.isEmpty () )
	{
		database_load ( filename.toUtf8 ().data () );
	}
}

void Mainwindow::press_file_recent ( size_t const i )
{
	std::string const filename ( m_mprefs.recent_crul_db.filename (i) );

	if ( filename.size () > 0 )
	{
		database_load ( filename );
	}
}

void Mainwindow::update_recent_db_menu ()
{
	char buf[ PATH_MAX ];

	for ( size_t i = 0; i < PREFS_CRUL_RECENT_DB_TOTAL; i++ )
	{
		if ( 1 == mtkit_snip_filename (
			m_mprefs.recent_crul_db.filename ( i + 1 ).c_str(),
			buf, sizeof ( buf ), 80 )
			)
		{
			// Hide if empty
			act_db_recent[ i ]->setVisible ( false );

			continue;
		}

		act_db_recent[ i ]->setText (
			mtQEX::qstringFromC ( buf ) );

		act_db_recent[ i ]->setVisible ( true );
	}
}

void Mainwindow::load_model_from_db ()
{
	mtQEX::BusyDialog	dialog ( this, "Loading Model.",
		[this]()
		{
			m_fe.model.load_db_pts ( & m_fe.crul_db );
		});
	dialog.wait_for_thread ();

	// Must be done in GUI thread
	m_fe.model_gl.populate (
		m_fe.model.get_pts().data(),
		m_fe.model.get_pts().size()
		);
	update_gl_view ();
}

void Mainwindow::load_cloud_from_db ( int type )
{
	if ( type == Crul::DB::CACHE_TYPE_ERROR )
	{
		type = get_selected_resolution ();
	}

	mtQEX::BusyDialog	dialog ( this, "Loading Points.",
		[this, type]()
		{
			m_fe.cloud.set_resolution ( type, & m_fe.crul_db );
		});

	dialog.wait_for_thread ();

	// Must be done in GUI thread
	m_fe.cloud_gl.populate ( m_fe.cloud.data(), m_fe.cloud.size() );
	populate_gl_rulers ();
	update_gl_view ();
}

