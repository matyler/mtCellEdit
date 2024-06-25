/*
	Copyright (C) 2024 Mark Tyler

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

#include "wasp_qt.h"



void MainWindow::update_recent_files ()
{
	int const maxlen = mtkit_int_bound ( m_mprefs.file_recent_maxlen,
		PREFS_RECENT_MAXLEN_MIN, PREFS_RECENT_MAXLEN_MAX
		);

	int c = 0;

	for ( size_t i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		char buf[ 2048 ];

		if ( 1 == mtkit_snip_filename (
			m_recent_files.filename ( i + 1 ).c_str(),
			buf, sizeof ( buf ), maxlen )
			)
		{
			// Hide if empty
			act_file_recent[ i ]->setVisible ( false );

			continue;
		}

		act_file_recent[ i ]->setText (
			mtQEX::qstringFromC ( buf ) );

		act_file_recent[ i ]->setVisible ( true );

		c++;		// Count items displayed
	}

	if ( c > 0 )
	{
		act_file_recent_separator->setVisible ( true );
	}
	else
	{
		// Hide separator if not needed
		act_file_recent_separator->setVisible ( false );
	}
}

void MainWindow::press_file_new ()
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	project_new ();
}

void MainWindow::press_file_open ()
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	QString filename = QFileDialog::getOpenFileName ( this,
		"Load Image File", mtQEX::qstringFromC (
			m_project.get_filename().c_str() ),
		NULL, NULL, QFileDialog::DontUseNativeDialog );


	if ( ! filename.isEmpty () )
	{
		project_load ( filename.toUtf8 ().data () );
	}
}

void MainWindow::press_file_save ()
{
	std::string const & filename = m_project.get_filename ();

	if ( filename.size() < 1 )
	{
		press_file_save_as ();
		return;
	}

	project_save ( filename.c_str() );
}

void MainWindow::press_file_save_as ()
{
	mtQEX::SaveFileDialog dialog ( this, "Save Wasp File", QStringList (),
		0, m_project.get_filename().c_str() );

	dialog.setOption ( QFileDialog::DontConfirmOverwrite );

	if ( m_project.get_filename().size() < 1 )
	{
		std::string const last_dir ( m_recent_files.directory () );

		dialog.setDirectory ( mtQEX::qstringFromC ( last_dir.c_str()) );
	}

	// Loop until successful save or user cancel
	while ( dialog.exec () )
	{
		QString const chosen = mtQEX::get_filename ( dialog );

		if ( chosen.isEmpty () )
		{
			continue;
		}

		std::string const f1 = chosen.toStdString();
		std::string const f2 = mtKit::string_set_extension (
			f1.c_str(), "wasp" );
		char const * const filename = f2.c_str();

		if ( mtQEX::message_file_overwrite ( this, filename ) )
		{
			continue;
		}

		if ( 0 == project_save ( filename ) )
		{
			break;
		}
	}
}

void MainWindow::press_file_export_wave ()
{
	mtQEX::SaveFileDialog dialog ( this, "Export Wave File", QStringList (),
		0, m_project.get_filename().c_str() );

	dialog.setOption ( QFileDialog::DontConfirmOverwrite );

	if ( m_project.get_filename().size() < 1 )
	{
		std::string const last_dir ( m_recent_files.directory () );

		dialog.setDirectory ( mtQEX::qstringFromC ( last_dir.c_str()) );
	}

	// Loop until successful save or user cancel
	while ( dialog.exec () )
	{
		QString filename = mtQEX::get_filename ( dialog );

		if ( filename.isEmpty () )
		{
			continue;
		}

		if ( mtQEX::message_file_overwrite ( this, filename ) )
		{
			continue;
		}

		if ( 0 == project_export_wave( filename.toUtf8().data() ) )
		{
			break;
		}
	}
}

void MainWindow::press_file_recent ( size_t const i )
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	project_load ( m_recent_files.filename (i).c_str() );
}

int MainWindow::project_new ()
{
	if ( m_project.new_file () )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to create new file." ) );

		return 1;	// Problem loading image file
	}

	update_inputs ();
	update_titlebar ();
	update_recent_files ();
	update_statusbar ();

	return 0;
}

int MainWindow::project_load ( char const * const filename )
{
	if ( m_project.load_file ( filename ) )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to load file:\n%1" ).arg (
			mtQEX::qstringFromC ( filename ) ) );

		return 1;	// Problem loading image file
	}

	m_recent_files.set ( filename );

	update_inputs ();
	update_titlebar ();
	update_recent_files ();
	update_statusbar ();

	return 0;
}

int MainWindow::project_save ( char const * const filename )
{
	if ( m_project.save_file ( filename ) )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to save file:\n%1" ).arg (
			mtQEX::qstringFromC ( filename ) ) );

		return 1;	// Problem loading image file
	}

	m_recent_files.set ( filename );

	update_titlebar ();
	update_recent_files ();
	update_statusbar ();

	return 0;
}

int MainWindow::project_export_wave ( char const * const filename )
{
	if ( m_project.export_wave_file ( filename ) )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to export wave file:\n%1" ).arg (
			mtQEX::qstringFromC ( filename ) ) );

		return 1;	// Problem loading image file
	}

	return 0;
}

void MainWindow::press_file_quit ()
{
	close ();
}

int MainWindow::ok_to_lose_changes ()
{
	if ( m_project.has_file_changed () )
	{
		int const res = QMessageBox::warning ( this, "Warning",
			"This file has been modified. Do you really want to "
			"lose these changes?",
			QMessageBox::Cancel | QMessageBox::Discard,
			QMessageBox::Cancel );

		if ( res == QMessageBox::Cancel )
		{
			return 0;	// Not OK to lose changes
		}
	}

	return 1;			// OK to lose changes
}

