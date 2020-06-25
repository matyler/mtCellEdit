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

#include "qt5_mw_edit.h"



void Mainwindow::set_edit_mode ( int const mode )
{
	switch ( mode )
	{
	case CloudView::MODE_CAMERA:
		if ( m_tabs_mode->currentWidget () == m_tab_ruler )
		{
			m_tabs_mode->setCurrentWidget ( m_tab_camera );
		}

		m_menu_ruler->setEnabled ( false );
		m_tabs_mode->setTabEnabled ( m_tabs_mode->indexOf(m_tab_ruler),
			false );
		break;

	case CloudView::MODE_RULER:
		m_menu_ruler->setEnabled ( true );
		m_tabs_mode->setTabEnabled ( m_tabs_mode->indexOf(m_tab_ruler),
			true );
		m_tabs_mode->setCurrentWidget ( m_tab_ruler );
		break;

	default:
		std::cerr << "set_edit_mode: invalid mode = " << mode << "\n";
		return;
	}

	m_cloud_view_a->set_mode ( mode );
	m_cloud_view_b->set_mode ( mode );
}

void Mainwindow::press_edit_mode_camera ()
{
	set_edit_mode ( CloudView::MODE_CAMERA );
}

void Mainwindow::press_edit_mode_ruler ()
{
	set_edit_mode ( CloudView::MODE_RULER );
}

void ThreadImportPTS::run ()
{
	m_error = m_fe.cloud.load_pts ( m_filename, & m_fe.crul_db );
}

void Mainwindow::press_edit_import_pts ()
{
	QString filename = QFileDialog::getOpenFileName ( this,
		"Import PTS File", NULL, NULL, NULL,
		QFileDialog::DontUseNativeDialog );

	if ( filename.isEmpty () )
	{
		return;
	}

	std::string const path ( filename.toUtf8 ().data () );

	mtQEX::BusyDialog dialog ( this, "Importing data from PTS file." );
	ThreadImportPTS work ( path, m_fe );

	work.start ();
	dialog.wait_for_thread ( work );

	switch ( work.error () )
	{
	case 0:
		break;

	case Crul::ERROR_USER_ABORT:
		QMessageBox::critical ( this, "Information",
			"Unable to import the PTS file: user abort." );
		break;

	default:
		QMessageBox::critical ( this, "Error",
			"Unable to import the PTS file." );
		break;
	}

	load_cloud_from_db ();
	update_gl_view ();
	update_info ();
}

void ThreadImportModel::run ()
{
	m_error = m_fe.model.import_file ( m_filename, & m_fe.crul_db );
}

void Mainwindow::press_edit_import_model ()
{
	QString filename = QFileDialog::getOpenFileName ( this,
		"Import Model", NULL, NULL, NULL,
		QFileDialog::DontUseNativeDialog );

	if ( filename.isEmpty () )
	{
		return;
	}

	std::string const path ( filename.toUtf8 ().data () );

	mtQEX::BusyDialog dialog ( this, "Importing data from model file." );
	ThreadImportModel work ( path, m_fe );

	work.start ();
	dialog.wait_for_thread ( work );

	switch ( work.error () )
	{
	case 0:
		break;

	case Crul::ERROR_USER_ABORT:
		QMessageBox::critical ( this, "Information",
			"Unable to import the model file: user abort." );
		break;

	default:
		QMessageBox::critical ( this, "Error",
			"Unable to import the model file." );
		break;
	}

	load_model_from_db ();
	update_gl_view ();
	update_info ();
}

void Mainwindow::press_edit_prefs ()
{
	m_cloud_view_a->store_prefs ();
	m_cloud_view_b->store_prefs ();

	mtQEX::PrefsWindow ( m_prefs.getPrefsMem (), "Preferences" );

	set_nudge ( m_prefs.getInt ( PREFS_VIEW_NUDGE_SIZE ) );
	set_point_size ( (int)m_prefs.getDouble ( PREFS_GL_POINT_SIZE ) );
	set_line_butt_size ( (int)m_prefs.getDouble ( PREFS_GL_LINE_BUTT_SIZE));
	set_line_thickness ( (int)m_prefs.getDouble ( PREFS_GL_LINE_THICKNESS));

	set_xrotation_a ( m_prefs.getDouble ( PREFS_VIEW_A PREFS_CAM_XROT ) );
	set_zrotation_a ( m_prefs.getDouble ( PREFS_VIEW_A PREFS_CAM_ZROT ) );
	set_xrotation_b ( m_prefs.getDouble ( PREFS_VIEW_B PREFS_CAM_XROT ) );
	set_zrotation_b ( m_prefs.getDouble ( PREFS_VIEW_B PREFS_CAM_ZROT ) );

	populate_gl_rulers ();

	m_cloud_view_a->restore_prefs_update ();
	m_cloud_view_b->restore_prefs_update ();

	m_fe.cloud.set_resampling_rates (
		m_prefs.getInt ( PREFS_CLOUD_RATE_LOW ),
		m_prefs.getInt ( PREFS_CLOUD_RATE_MEDIUM )
		);

	update_view_show_items ();
}

