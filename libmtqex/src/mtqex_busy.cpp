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

#include "private.h"



mtQEX::BusyDialog::BusyDialog (
	QWidget		* const	parent,
	char	const * const	message
	)
	:
	QDialog		( parent ),
	m_progress	(),
	m_button_box	(),
	m_default	( true )
{
	setModal ( true );
	setWindowTitle ( "Please Wait" );
	setMinimumWidth ( 300 );

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setMargin ( 5 );
	setLayout ( vbox );

	if ( message )
	{
		vbox->addWidget ( new QLabel ( message ) );
	}

	m_progress = new QProgressBar;

	// Sadly the GTK2 Qt theme doesn't have a nice animated "busy indicator"
	// for min/max=0 so we must create our own one and use as the default
	// option.
	m_progress->setMinimum ( 0 );
	m_progress->setMaximum ( 100 );
	m_progress->setValue ( 0 );

	m_busy.set_minmax ( 0, 100 );
	m_busy.set_value ( 0 );
	m_busy.clear_range_changed ();

	m_progress->setTextVisible ( false );
	vbox->addWidget ( m_progress );

	setSizeGripEnabled ( false );

	m_button_box = new QDialogButtonBox ( QDialogButtonBox::Abort );
	m_button_box->setCenterButtons ( true );
	vbox->addWidget ( m_button_box );
	connect ( m_button_box->button ( QDialogButtonBox::Abort ),
		SIGNAL ( clicked () ), this, SLOT ( press_abort () ) );
	m_button_box->hide ();

	show ();
}

mtQEX::BusyDialog::~BusyDialog ()
{
}

void mtQEX::BusyDialog::show_abort () const
{
	m_button_box->show ();
}

void mtQEX::BusyDialog::wait_for_thread ( QThread &thread )
{
	do
	{
		if ( m_default )
		{
			int const num = (m_progress->value () + 1) %
				(m_progress->maximum () + 1);

			m_busy.set_value ( num );
		}

		process_qt_pending ();

		usleep ( 20000 );	// 50th sec

		if ( m_busy.range_changed () )
		{
			m_busy.clear_range_changed ();

			m_default = false;

			m_progress->setMinimum ( m_busy.get_min () );
			m_progress->setMaximum ( m_busy.get_max () );

			m_progress->setTextVisible ( true );
		}

		if ( m_busy.get_value () != m_progress->value () )
		{
			m_progress->setValue ( m_busy.get_value () );
		}

	} while ( thread.isRunning () );
}

void mtQEX::BusyDialog::accept ()
{
}

void mtQEX::BusyDialog::reject ()
{
}

void mtQEX::BusyDialog::closeEvent ( QCloseEvent * ev )
{
	ev->ignore ();
}

void mtQEX::BusyDialog::press_abort ()
{
	m_busy.set_aborted ();
}

