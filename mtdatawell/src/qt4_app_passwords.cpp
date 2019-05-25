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



#define APP_TITLE_TXT "Passwords"



void Mainwindow::press_apps_passwords ()
{
	DialogPasswords dialog ( *this );
}



DialogPasswords::DialogPasswords (
	Mainwindow	&mw
	)
	:
	QDialog		( &mw ),
	m_total		( NULL ),
	m_char_tot	( NULL ),
	m_lowercase	( NULL ),
	m_uppercase	( NULL ),
	m_numbers	( NULL ),
	m_other		( NULL ),
	m_other_text	( NULL ),
	m_text		( NULL ),
	m_well		( mw.backend.db.get_well () )
{
	setWindowTitle ( "Passwords" );
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
	m_total->setRange ( mtDW::Well::PASSWORD_TOTAL_MIN,
		mtDW::Well::PASSWORD_TOTAL_MAX );
	m_total->setValue ( mtDW::Well::PASSWORD_TOTAL_DEFAULT );

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	label = new QLabel ( "Characters" );
	hbox->addWidget ( label );

	m_char_tot = new QSpinBox;
	hbox->addWidget ( m_char_tot );
	m_char_tot->setRange ( mtDW::Well::PASSWORD_CHAR_MIN,
		mtDW::Well::PASSWORD_CHAR_MAX );
	m_char_tot->setValue ( mtDW::Well::PASSWORD_CHAR_DEFAULT );

	m_lowercase = new QCheckBox ( "Lowercase" );
	vbox->addWidget ( m_lowercase );
	m_lowercase->setCheckState ( Qt::Checked );

	m_uppercase = new QCheckBox ( "Uppercase" );
	vbox->addWidget ( m_uppercase );
	m_uppercase->setCheckState ( Qt::Unchecked );

	m_numbers = new QCheckBox ( "Numbers" );
	vbox->addWidget ( m_numbers );
	m_numbers->setCheckState ( Qt::Checked );

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	m_other = new QCheckBox ( "Other" );
	hbox->addWidget ( m_other );
	m_other->setCheckState ( Qt::Unchecked );

	m_other_text = new QLineEdit ( mtQEX::qstringFromC (
		mtDW::Well::PASSWORD_OTHER_DEFAULT ) );
	m_other_text->setMaxLength ( mtDW::Well::PASSWORD_OTHER_MAX );
	hbox->addWidget ( m_other_text );

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

void DialogPasswords::press_apply ()
{
	int const total = m_total->value ();
	int const char_tot = m_char_tot->value ();
	bool const lowercase = m_lowercase->checkState ();
	bool const uppercase = m_uppercase->checkState ();
	bool const numbers = m_numbers->checkState ();
	bool const other = m_other->checkState ();
	std::string other_txt;

	if ( other )
	{
		other_txt = m_other_text->text ().toUtf8 ().data ();
	}

	std::string txt;

	m_well->app_passwords ( mtDW::AppPassword ( lowercase, uppercase,
		numbers, other_txt ), char_tot, txt, total );

	m_text->setPlainText ( mtQEX::qstringFromC ( txt.c_str () ) );
}

