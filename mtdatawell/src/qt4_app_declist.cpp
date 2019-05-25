/*
	Copyright (C) 2018-2019 Mark Tyler

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



#define APP_TITLE_TXT "Decimal List"



void Mainwindow::press_apps_declist ()
{
	DialogDecList dialog ( *this );
}



DialogDecList::DialogDecList (
	Mainwindow	&mw
	)
	:
	QDialog		( &mw ),
	m_total		( NULL ),
	m_min		( NULL ),
	m_max		( NULL ),
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
	m_total->setRange ( mtDW::Well::DECLIST_TOTAL_MIN,
		mtDW::Well::DECLIST_TOTAL_MAX );
	m_total->setValue ( mtDW::Well::DECLIST_TOTAL_DEFAULT );

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	label = new QLabel ( "Minimum" );
	hbox->addWidget ( label );

	m_min = new QDoubleSpinBox;
	hbox->addWidget ( m_min );
	m_min->setRange ( mtDW::Well::DECLIST_MIN_LO,
		mtDW::Well::DECLIST_MIN_HI );
	m_min->setValue ( mtDW::Well::DECLIST_MIN_DEFAULT );

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	label = new QLabel ( "Maximum" );
	hbox->addWidget ( label );

	m_max = new QDoubleSpinBox;
	hbox->addWidget ( m_max );
	m_max->setRange ( mtDW::Well::DECLIST_MAX_LO,
		mtDW::Well::DECLIST_MAX_HI );
	m_max->setValue ( mtDW::Well::DECLIST_MAX_DEFAULT );

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

void DialogDecList::press_apply ()
{
	int const total = m_total->value ();
	double const min = m_min->value ();
	double const max = m_max->value ();
	std::string txt;

	m_well->app_declist ( txt, total, min, max );

	m_text->setPlainText ( mtQEX::qstringFromC ( txt.c_str () ) );
}

