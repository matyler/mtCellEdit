/*
	Copyright (C) 2018-2020 Mark Tyler

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

#include "qt5.h"



void Mainwindow::press_apps_binfile ()
{
	DialogBinFile ( *this );
}

DialogBinFile::DialogBinFile (
	Mainwindow	&mw
	)
	:
	QDialog		( &mw ),
	m_size		( NULL ),
	m_well		( mw.backend.db.get_well () )
{
	setWindowTitle ( "Binary File" );
	setModal ( true );

	QVBoxLayout * vbox = new QVBoxLayout;
	setLayout ( vbox );

	QWidget * w = new QWidget;
	vbox->addWidget ( w );

	QHBoxLayout * hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	QLabel * label = new QLabel ( "File size (bytes)" );
	hbox->addWidget ( label );

	m_size = new QSpinBox;
	hbox->addWidget ( m_size );
	m_size->setRange ( 1, 256*1024*1024 );	// 256MB
	m_size->setValue ( 16*1024*1024 );	// 16MB

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close | QDialogButtonBox::Save );

	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));
	connect ( button_box->button ( QDialogButtonBox::Save ),
		SIGNAL ( clicked () ), this, SLOT ( press_save () ) );

	exec ();
}

void DialogBinFile::press_save ()
{
	QString const filename = QFileDialog::getSaveFileName ( this,
		"Save Binary File", NULL, NULL, NULL,
		QFileDialog::DontUseNativeDialog );

	if ( filename.isEmpty () )
	{
		return;
	}

	int error = 0;
	mtQEX::BusyDialog busy ( this, nullptr, [this, &error, &filename]()
		{
			error = m_well->save_file ( m_size->value(),
				filename.toUtf8 ().data () );
		});

	busy.wait_for_thread ();

	if ( report_lib_error ( this, error ) )
	{
		return;
	}

	close ();
}

