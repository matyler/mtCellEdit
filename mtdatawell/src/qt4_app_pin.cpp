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

#include "qt4.h"



#define APP_TITLE_TXT "Pin Numbers"



void Mainwindow::press_apps_pins ()
{
	DialogPins ( *this );
}



DialogPins::DialogPins (
	Mainwindow	&mw
	)
	:
	QDialog		( &mw ),
	m_total		( NULL ),
	m_digits	( NULL ),
	m_text		( NULL ),
	m_well		( mw.backend.db.get_well () )
{
	setWindowTitle ( APP_TITLE_TXT );
	setModal ( true );

	QVBoxLayout * vbox = new QVBoxLayout;
	setLayout ( vbox );

	QWidget * w;
	QHBoxLayout * hbox;
	QLabel * label;

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	label = new QLabel ( "Total" );
	hbox->addWidget ( label );

	m_total = new QSpinBox;
	hbox->addWidget ( m_total );
	m_total->setRange ( mtDW::Well::PIN_TOTAL_MIN,
		mtDW::Well::PIN_TOTAL_MAX );
	m_total->setValue ( mtDW::Well::PIN_TOTAL_DEFAULT );

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	label = new QLabel ( "Digits" );
	hbox->addWidget ( label );

	m_digits = new QSpinBox;
	hbox->addWidget ( m_digits );
	m_digits->setRange ( mtDW::Well::PIN_DIGITS_MIN,
		mtDW::Well::PIN_DIGITS_MAX );
	m_digits->setValue ( mtDW::Well::PIN_DIGITS_DEFAULT );

	m_text = new QTextEdit;
	vbox->addWidget ( m_text );
	m_text->setReadOnly ( true );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close | QDialogButtonBox::Apply );


	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));
	connect ( button_box->button ( QDialogButtonBox::Apply ),
		SIGNAL ( clicked () ), this, SLOT(press_apply () ) );

	show ();

	press_apply ();

	exec ();
}

void DialogPins::press_apply ()
{
	int const total = m_total->value ();
	int const digits = m_digits->value ();
	std::string txt;

	m_well->app_pins ( txt, total, digits );
	m_text->setPlainText ( mtQEX::qstringFromC ( txt.c_str () ) );
}

