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



DialogAudioDevicePicker::DialogAudioDevicePicker (
	MainWindow		&mw,
	mtKit::UserPrefs	& uprefs,
	MemPrefs	const	& mprefs
	)
	:
	QDialog		( &mw ),
	mainwindow	( mw ),
	m_uprefs	( uprefs )
{
	setWindowTitle ( "Set Audio Device" );

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setContentsMargins ( 5, 5, 5, 5 );
	vbox->setSpacing ( 5 );
	setLayout ( vbox );

	m_table_devices = new QTableWidget;
	vbox->addWidget ( m_table_devices );
	m_table_devices->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_table_devices->setSelectionMode ( QAbstractItemView::SingleSelection );
	m_table_devices->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_table_devices->setColumnCount ( 2 );
	m_table_devices->setShowGrid ( false );
	m_table_devices->verticalHeader ()->hide ();
	m_table_devices->horizontalHeader ()->setStretchLastSection ( true );
	m_table_devices->setHorizontalScrollMode ( QAbstractItemView::
		ScrollPerPixel );
	m_table_devices->setHorizontalHeaderLabels( {"Device", "Description"} );

	int const devtot = 1 + SDL_GetNumAudioDevices ( 0 );

	m_table_devices->setRowCount ( devtot );

	for ( int i = -1; i < devtot; i++ )
	{
		char txt[32];
		snprintf ( txt, sizeof(txt), "%i", i );
		auto twItem = new QTableWidgetItem (
			mtQEX::qstringFromC ( txt ) );
		m_table_devices->setItem ( i+1, 0, twItem );

		char const * const devtxt = (-1 == i) ? "Default" :
			SDL_GetAudioDeviceName ( i, 0 );

		twItem = new QTableWidgetItem ( mtQEX::qstringFromC (devtxt) );
		m_table_devices->setItem ( i+1, 1, twItem );
	}

	m_table_devices->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );

	int const current = 1 + mprefs.audio_output_device;

	m_table_devices->setCurrentCell ( current, 0 );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

	vbox->addWidget ( button_box );

	connect ( button_box->button ( QDialogButtonBox::Ok ),
		SIGNAL ( pressed () ), this, SLOT ( press_ok () ));
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	setMinimumSize ( 400, 64 );

	move ( QCursor::pos () );
	exec ();
}

void DialogAudioDevicePicker::press_ok ()
{
	int const device = m_table_devices->currentRow() - 1;

	m_uprefs.set ( PREFS_AUDIO_OUTPUT_DEVICE, device );

	close ();
}

