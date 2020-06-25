/*
	Copyright (C) 2019-2020 Mark Tyler

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



#define APP_TITLE_TXT "Unicode Fonts"



void Mainwindow::press_apps_unicodefonts ()
{
	DialogUnicodeFonts ( *this );
}



DialogUnicodeFonts::DialogUnicodeFonts (
	Mainwindow	&mw
	)
	:
	QDialog			( &mw ),

	m_utf8_info		( NULL ),
	m_utf8_input		( NULL ),
	m_utf8_output		( NULL ),

	m_font_type		( NULL ),

	m_file_info		( NULL ),
	m_font_list		( NULL ),

	m_file_input		( NULL ),
	m_file_output		( NULL ),

	m_font_index		( mw.backend.font_index )
{
	QPushButton	* button;
	QGroupBox	* groupBox;
	QVBoxLayout	* gvb;
	QHBoxLayout	* hbox;
	QVBoxLayout	* vbox;
	QWidget		* tab;
	QWidget		* w;
	QGridLayout	* grid;

	setWindowTitle ( APP_TITLE_TXT );
	setModal ( true );

	QVBoxLayout * vbox_main = new QVBoxLayout;
	setLayout ( vbox_main );

	QTabWidget * tabWidget = new QTabWidget;
	vbox_main->addWidget ( tabWidget );

///	UTF-8 tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "UTF-8" );

	vbox = new QVBoxLayout;
	tab->setLayout ( vbox );

	w = new QWidget;
	vbox->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );

	groupBox = new QGroupBox ( "Input" );
	grid->addWidget ( groupBox, 0, 0 );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_utf8_input = new QTextEdit;
	m_utf8_input->setAcceptRichText ( false );
	gvb->addWidget ( m_utf8_input );

	groupBox = new QGroupBox ( "Font Type" );
	grid->addWidget ( groupBox, 0, 1 );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_font_type = new mtQEX::ButtonMenu;
	m_font_type->setSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Preferred );
	gvb->addWidget ( m_font_type );

	int const a = mtDW::Utf8Font::TYPE_MIN;
	int const b = mtDW::Utf8Font::TYPE_MAX;

	for ( int i = a; i <= b; i++ )
	{
		std::string txt;

		m_font_index.get_type_name ( i, txt );
		m_font_type->addItem ( txt.c_str () );
	}

	w = new QWidget;
	w->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding );
	gvb->addWidget ( w );

	groupBox = new QGroupBox ( "Output" );
	grid->addWidget ( groupBox, 1, 0 );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_utf8_output = new QTextEdit;
	gvb->addWidget ( m_utf8_output );
	m_utf8_output->setReadOnly ( true );

	groupBox = new QGroupBox ( "Info" );
	grid->addWidget ( groupBox, 1, 1 );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_utf8_info = new QTextEdit;
	gvb->addWidget ( m_utf8_info );
	m_utf8_info->setReadOnly ( true );

///	UTF-8 buttons

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( "Encode" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_utf8_encode () ) );

	button = new QPushButton ( "Decode" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_utf8_decode () ) );

	button = new QPushButton ( QIcon::fromTheme ( "edit-copy" ),
		"Copy Output" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_utf8_copy_output () ) );

	// Squashes buttons together to the left
	hbox->addWidget ( new QWidget );

///	Font List tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "Font List" );

	vbox = new QVBoxLayout;
	tab->setLayout ( vbox );

	m_font_list = new QTextEdit;
	m_font_list->setAcceptRichText ( false );
	m_font_list->setReadOnly ( true );
	vbox->addWidget ( m_font_list );

	std::string txt;
	m_font_index.get_font_list ( txt );
	m_font_list->setText ( txt.c_str () );

///	File I/O tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "File I/O" );

	vbox = new QVBoxLayout;
	tab->setLayout ( vbox );

	m_file_input = new fileCombo ( mw.prefs, "Input UTF-8 File",
		QFileDialog::ExistingFile );
	vbox->addWidget ( m_file_input );

	m_file_output = new fileCombo ( mw.prefs, "Output UTF-8 File",
		QFileDialog::AnyFile );
	vbox->addWidget ( m_file_output );

	groupBox = new QGroupBox ( "Info" );
	vbox->addWidget ( groupBox );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_file_info = new QTextEdit;
	gvb->addWidget ( m_file_info );
	m_file_info->setReadOnly ( true );

///	File I/O buttons

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme ( "document-save" ),
		"Encode" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_file_encode () ) );

	button = new QPushButton ( QIcon::fromTheme ( "document-save" ),
		"Decode" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_file_decode () ) );

	// Squashes buttons together to the left
	hbox->addWidget ( new QWidget );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	vbox_main->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	show ();

	exec ();
}



/// ----------------------------------------------------------------------------



void DialogUnicodeFonts::press_file_encode ()
{
	int const type = m_font_type->currentIndex ();
	std::string info;

	m_font_index.file_encode (
		m_file_input->get_directory().toUtf8().data(),
		type,
		m_file_output->get_directory().toUtf8().data(),
		info );

	m_file_info->setText ( info.c_str () );
}

void DialogUnicodeFonts::press_file_decode ()
{
	std::string info;

	m_font_index.file_clean (
		m_file_input->get_directory().toUtf8().data(),
		m_file_output->get_directory().toUtf8().data(),
		info );

	m_file_info->setText ( info.c_str () );
}



/// ----------------------------------------------------------------------------

void DialogUnicodeFonts::press_utf8_decode ()
{
	std::string info, output;

	m_font_index.utf8_clean ( m_utf8_input->toPlainText ().toUtf8().data(),
		info, output );

	m_utf8_info->setText ( info.c_str () );
	m_utf8_output->setText ( output.c_str () );
}

void DialogUnicodeFonts::press_utf8_encode ()
{
	int const type = m_font_type->currentIndex ();
	std::string output, info;

	m_font_index.utf8_encode ( m_utf8_input->toPlainText ().toUtf8().data(),
		type, info, output );

	m_utf8_info->setText ( info.c_str () );
	m_utf8_output->setText ( output.c_str () );
}

void DialogUnicodeFonts::press_utf8_copy_output ()
{
	QApplication::clipboard()->setText ( m_utf8_output->toPlainText () );
}

