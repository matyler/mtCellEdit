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



#define APP_TITLE_TXT "Homoglyphs"



void Mainwindow::press_apps_homoglyphs ()
{
	DialogHomoglyphs ( *this );
}



DialogHomoglyphs::DialogHomoglyphs (
	Mainwindow	&mw
	)
	:
	QDialog			( &mw ),
	m_text_dec_info		( NULL ),
	m_text_enc_info		( NULL ),

	m_utf8_covert		( NULL ),
	m_utf8_info		( NULL ),
	m_utf8_input		( NULL ),
	m_utf8_output		( NULL ),

	m_file_enc_overt	( NULL ),
	m_file_enc_covert	( NULL ),
	m_file_enc_output	( NULL ),

	m_file_dec_input	( NULL ),
	m_file_dec_output	( NULL ),

	m_well			( mw.backend.db.get_well () ),
	m_hg_index		( mw.backend.hg_index )
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

	groupBox = new QGroupBox ( "Covert" );
	grid->addWidget ( groupBox, 0, 1 );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_utf8_covert = new QTextEdit;
	m_utf8_covert->setAcceptRichText ( false );
	gvb->addWidget ( m_utf8_covert );

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

	button = new QPushButton ( QIcon::fromTheme ( "document-properties" ),
		"Analyse" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_utf8_analyse () ) );

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
		"Copy Covert" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_utf8_copy_covert () ) );

	button = new QPushButton ( QIcon::fromTheme ( "edit-copy" ),
		"Copy Output" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_utf8_copy_output () ) );

	// Squashes buttons together to the left
	hbox->addWidget ( new QWidget );

///	ENCODE tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "Encode File" );

	vbox = new QVBoxLayout;
	tab->setLayout ( vbox );

	m_file_enc_overt = new fileCombo ( mw.prefs, "Input UTF-8 File",
		QFileDialog::ExistingFile );
	vbox->addWidget ( m_file_enc_overt );

	m_file_enc_covert = new fileCombo ( mw.prefs, "Input Covert File",
		QFileDialog::ExistingFile );
	vbox->addWidget ( m_file_enc_covert );

	m_file_enc_output = new fileCombo ( mw.prefs, "Output UTF-8 File",
		QFileDialog::AnyFile );
	vbox->addWidget ( m_file_enc_output );

	groupBox = new QGroupBox ( "Info" );
	vbox->addWidget ( groupBox );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_text_enc_info = new QTextEdit;
	gvb->addWidget ( m_text_enc_info );
	m_text_enc_info->setReadOnly ( true );

///	ENCODE buttons

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme ( "document-properties" ),
		"Analyse" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_enc_analyse () ) );

	button = new QPushButton ( QIcon::fromTheme ( "document-save" ),
		"Encode" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_enc_encode () ) );

	// Squashes buttons together to the left
	hbox->addWidget ( new QWidget );

///	DECODE tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "Decode File" );

	vbox = new QVBoxLayout;
	tab->setLayout ( vbox );

	m_file_dec_input = new fileCombo ( mw.prefs, "Input UTF-8 File",
		QFileDialog::ExistingFile );
	vbox->addWidget ( m_file_dec_input );

	m_file_dec_output = new fileCombo ( mw.prefs, "Ouput File",
		QFileDialog::AnyFile );
	vbox->addWidget ( m_file_dec_output );

	groupBox = new QGroupBox ( "Info" );
	vbox->addWidget ( groupBox );

	gvb = new QVBoxLayout;
	groupBox->setLayout ( gvb );

	m_text_dec_info = new QTextEdit;
	gvb->addWidget ( m_text_dec_info );
	m_text_dec_info->setReadOnly ( true );

///	DECODE buttons

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme ( "edit-clear" ), "Clean" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_dec_clean () ) );

	button = new QPushButton ( QIcon::fromTheme ( "document-save" ),
		"Decode" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_dec_decode () ) );

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



void DialogHomoglyphs::press_enc_analyse ()
{
	std::string err_txt;

	m_hg_index.file_analyse(
		m_file_enc_overt->get_directory().toUtf8().data(),
		err_txt );

	m_text_enc_info->setText ( err_txt.c_str () );
}

void DialogHomoglyphs::press_enc_encode ()
{
	std::string err_txt;

	m_hg_index.file_encode ( m_file_enc_overt->get_directory().toUtf8().data(),
		m_file_enc_covert->get_directory().toUtf8().data(),
		m_file_enc_output->get_directory().toUtf8().data(),
		m_well, err_txt );

	m_text_enc_info->setText ( err_txt.c_str () );
}



/// ----------------------------------------------------------------------------



void DialogHomoglyphs::press_dec_clean ()
{
	std::string err_txt;

	m_hg_index.file_clean (
		m_file_dec_input->get_directory().toUtf8().data(),
		m_file_dec_output->get_directory().toUtf8().data(),
		err_txt );

	m_text_enc_info->setText ( err_txt.c_str () );
}

void DialogHomoglyphs::press_dec_decode ()
{
	std::string err_txt;

	m_hg_index.file_decode (
		m_file_dec_input->get_directory().toUtf8().data(),
		m_file_dec_output->get_directory().toUtf8().data(),
		err_txt );

	m_text_dec_info->setText ( err_txt.c_str () );
}



/// ----------------------------------------------------------------------------



void DialogHomoglyphs::press_utf8_analyse ()
{
	std::string info;

	m_hg_index.utf8_analyse (
		m_utf8_input->toPlainText().toUtf8().data(),
		m_utf8_covert->toPlainText().toUtf8().data(),
		info );

	m_utf8_info->setText ( info.c_str () );
}

void DialogHomoglyphs::press_utf8_decode ()
{
	std::string covert, info, output;

	m_hg_index.utf8_decode ( m_utf8_input->toPlainText ().toUtf8().data(),
		covert, info, output );

	m_utf8_info->setText ( info.c_str () );
	m_utf8_covert->setText ( covert.c_str () );
	m_utf8_output->setText ( output.c_str () );
}

void DialogHomoglyphs::press_utf8_encode ()
{
	std::string output, info;

	m_hg_index.utf8_encode ( m_utf8_input->toPlainText ().toUtf8().data(),
		m_utf8_covert->toPlainText ().toUtf8().data(),
		info, output, m_well );

	m_utf8_info->setText ( info.c_str () );
	m_utf8_output->setText ( output.c_str () );
}

void DialogHomoglyphs::press_utf8_copy_covert ()
{
	QApplication::clipboard()->setText ( m_utf8_covert->toPlainText () );
}

void DialogHomoglyphs::press_utf8_copy_output ()
{
	QApplication::clipboard()->setText ( m_utf8_output->toPlainText () );
}

