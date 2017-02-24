/*
	Copyright (C) 2016 Mark Tyler

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

#include "qt45.h"



DialogPasteText::DialogPasteText (
	Mainwindow	&mw
	)
	:
	mainwindow	( mw )
{
	setWindowTitle ( "Paste Text" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	QGroupBox * gbox = new QGroupBox ( "Text" );
	layv->addWidget ( gbox );

	QVBoxLayout * vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_line_edit = new QLineEdit ( mainwindow.prefs.getString (
		PREFS_TEXT_ENTRY ) );
	vbox->addWidget ( m_line_edit );
	connect ( m_line_edit, SIGNAL ( textChanged ( QString const & ) ), this,
		SLOT ( change_entry_text ( QString const & ) ));

	gbox = new QGroupBox ( "Preview" );
	layv->addWidget ( gbox );

	vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_qex_image = new mtQEX::Image;
	vbox->addWidget ( m_qex_image );

	gbox = new QGroupBox ( "Font" );
	layv->addWidget ( gbox );

	vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	QGridLayout * grid = new QGridLayout;
	vbox->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Name" ), 0, 0 );
	grid->addWidget ( new QLabel ( "Size" ), 1, 0 );
	grid->addWidget ( new QLabel ( "Effects" ), 2, 0 );

	m_label_font_name = new QLabel;
	m_label_font_size = new QLabel;
	m_label_font_effects = new QLabel;
	grid->addWidget ( m_label_font_name, 0, 1 );
	grid->addWidget ( m_label_font_size, 1, 1 );
	grid->addWidget ( m_label_font_effects, 2, 1 );

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

	QPushButton * but_sel_font = new QPushButton ( "Select Font" );
	button_box->addButton ( but_sel_font, QDialogButtonBox::ActionRole );

	layv->addWidget ( button_box );

	connect ( button_box->button ( QDialogButtonBox::Ok ),
		SIGNAL ( pressed () ), this, SLOT ( press_ok () ));
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));
	connect ( but_sel_font, SIGNAL ( pressed () ), this,
		SLOT ( press_select_font () ));

	update_font_table ();
	update_preview ();

	move ( QCursor::pos () );
	exec ();
}

void DialogPasteText::press_select_font ()
{
	bool		ok;
	QFont		fnt (
			mainwindow.prefs.getString ( PREFS_TEXT_FONT_NAME ),
			mainwindow.prefs.getInt ( PREFS_TEXT_FONT_SIZE )
			);

	fnt.setBold ( mainwindow.prefs.getInt ( PREFS_TEXT_FONT_BOLD ) );
	fnt.setItalic ( mainwindow.prefs.getInt ( PREFS_TEXT_FONT_ITALIC ) );
	fnt.setUnderline ( mainwindow.prefs.getInt (
		PREFS_TEXT_FONT_UNDERLINE ) );
	fnt.setStrikeOut ( mainwindow.prefs.getInt (
		PREFS_TEXT_FONT_STRIKETHROUGH ) );

	fnt = QFontDialog::getFont ( &ok, fnt, this, "Select Font",
		QFontDialog::DontUseNativeDialog );

	if ( ! ok )
	{
		return;
	}

	mainwindow.prefs.set ( PREFS_TEXT_FONT_NAME,
		fnt.family ().toUtf8 ().data () );
	mainwindow.prefs.set ( PREFS_TEXT_FONT_SIZE, fnt.pointSize () );
	mainwindow.prefs.set ( PREFS_TEXT_FONT_BOLD, fnt.bold () ? 1 : 0 );
	mainwindow.prefs.set ( PREFS_TEXT_FONT_ITALIC, fnt.italic () ? 1 : 0 );
	mainwindow.prefs.set ( PREFS_TEXT_FONT_UNDERLINE, fnt.underline () ?
		1 : 0 );
	mainwindow.prefs.set ( PREFS_TEXT_FONT_STRIKETHROUGH, fnt.strikeOut () ?
		1 : 0 );

	update_font_table ();
	update_preview ();
}

int Mainwindow::render_text_paste ()
{
	return backend.file.clipboard_render_text (
		backend.clipboard,
		prefs.getString ( PREFS_TEXT_ENTRY ),
		prefs.getString ( PREFS_TEXT_FONT_NAME ),
		prefs.getInt ( PREFS_TEXT_FONT_SIZE ),
		prefs.getInt ( PREFS_TEXT_FONT_BOLD ),
		prefs.getInt ( PREFS_TEXT_FONT_ITALIC ),
		prefs.getInt ( PREFS_TEXT_FONT_UNDERLINE ) ?
			mtPixy::Font::STYLE_UNDERLINE_SINGLE :
			mtPixy::Font::STYLE_UNDERLINE_NONE,
		prefs.getInt ( PREFS_TEXT_FONT_STRIKETHROUGH ) );
}

void DialogPasteText::press_ok ()
{
	close ();

	if ( 0 == mainwindow.render_text_paste () )
	{
		mainwindow.press_edit_paste_centre ();
	}
}

void DialogPasteText::change_entry_text (
	QString const &txt
	)
{
	mainwindow.prefs.set ( PREFS_TEXT_ENTRY, txt.toUtf8 ().data () );
	update_preview ();
}

void DialogPasteText::update_preview ()
{
	mtPixy::Image * im = mtPixy::text_render_preview (
		mainwindow.backend.file.get_image ()->get_type (),
		mainwindow.prefs.getString ( PREFS_TEXT_ENTRY ),
		mainwindow.prefs.getString ( PREFS_TEXT_FONT_NAME ),
		mainwindow.prefs.getInt ( PREFS_TEXT_FONT_SIZE ),
		mainwindow.prefs.getInt ( PREFS_TEXT_FONT_BOLD ),
		mainwindow.prefs.getInt ( PREFS_TEXT_FONT_ITALIC ),
		mainwindow.prefs.getInt ( PREFS_TEXT_FONT_UNDERLINE ) ?
			mtPixy::Font::STYLE_UNDERLINE_SINGLE :
			mtPixy::Font::STYLE_UNDERLINE_NONE,
		mainwindow.prefs.getInt ( PREFS_TEXT_FONT_STRIKETHROUGH ) );

	m_qex_image->setImage ( im );
	m_qex_image->update ();
}

void DialogPasteText::update_font_table ()
{
	m_label_font_name->setText ( mainwindow.prefs.getString (
		PREFS_TEXT_FONT_NAME ) );

	m_label_font_size->setText ( QString ( "%1" )
		.arg ( mainwindow.prefs.getInt ( PREFS_TEXT_FONT_SIZE ) ) );

	QString		txt;

	if ( mainwindow.prefs.getInt ( PREFS_TEXT_FONT_BOLD ) )
	{
		txt.append ( "Bold " );
	}

	if ( mainwindow.prefs.getInt ( PREFS_TEXT_FONT_ITALIC ) )
	{
		txt.append ( "Italic " );
	}

	if ( mainwindow.prefs.getInt ( PREFS_TEXT_FONT_UNDERLINE ) )
	{
		txt.append ( "Underline " );
	}

	if ( mainwindow.prefs.getInt ( PREFS_TEXT_FONT_STRIKETHROUGH ) )
	{
		txt.append ( "Strikeout" );
	}

	m_label_font_effects->setText ( txt );
}

