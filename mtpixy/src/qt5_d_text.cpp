/*
	Copyright (C) 2016-2023 Mark Tyler

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



DialogPasteText::DialogPasteText ( Mainwindow &mw )
	:
	QDialog		( &mw ),
	mainwindow	( mw )
{
	setWindowTitle ( "Paste Text" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setContentsMargins ( 5, 5, 5, 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	QGroupBox * gbox = new QGroupBox ( "Text" );
	layv->addWidget ( gbox );

	QVBoxLayout * vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_line_edit = new QLineEdit ( mainwindow.mprefs.text_entry.c_str() );
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
			mainwindow.mprefs.text_font_name.c_str(),
			mainwindow.mprefs.text_font_size
			);

	fnt.setStyleName ( mainwindow.mprefs.text_font_style.c_str() );
	fnt.setBold ( mainwindow.mprefs.text_font_bold );
	fnt.setItalic ( mainwindow.mprefs.text_font_italic );
	fnt.setUnderline ( mainwindow.mprefs.text_font_underline );
	fnt.setStrikeOut ( mainwindow.mprefs.text_font_strikethrough );

	fnt = QFontDialog::getFont ( &ok, fnt, this, "Select Font",
		QFontDialog::DontUseNativeDialog );

	if ( ! ok )
	{
		return;
	}

	mainwindow.backend.uprefs.set ( PREFS_TEXT_FONT_NAME,
		fnt.family ().toUtf8 ().data () );

	mainwindow.backend.uprefs.set ( PREFS_TEXT_FONT_STYLE,
		fnt.styleName ().toUtf8 ().data () );

	mainwindow.backend.uprefs.set ( PREFS_TEXT_FONT_SIZE, fnt.pointSize() );
	mainwindow.backend.uprefs.set ( PREFS_TEXT_FONT_BOLD, fnt.bold () ?
		1 : 0 );
	mainwindow.backend.uprefs.set ( PREFS_TEXT_FONT_ITALIC, fnt.italic () ?
		1 : 0 );
	mainwindow.backend.uprefs.set ( PREFS_TEXT_FONT_UNDERLINE,
		fnt.underline () ? 1 : 0 );
	mainwindow.backend.uprefs.set ( PREFS_TEXT_FONT_STRIKETHROUGH,
		fnt.strikeOut() ? 1 : 0 );

	update_font_table ();
	update_preview ();
}

int Mainwindow::render_text_paste ()
{
	return backend.file.clipboard_render_text (
		backend.clipboard,
		mprefs.text_entry.c_str(),
		mprefs.get_font_name_full().c_str(),
		mprefs.text_font_size,
		mprefs.text_font_bold,
		mprefs.text_font_italic,
		mprefs.text_font_underline ?
			mtPixy::Font::STYLE_UNDERLINE_SINGLE :
			mtPixy::Font::STYLE_UNDERLINE_NONE,
		mprefs.text_font_strikethrough );
}

void DialogPasteText::press_ok ()
{
	close ();

	if ( 0 == mainwindow.render_text_paste () )
	{
		mainwindow.press_edit_paste_centre ();
	}
}

void DialogPasteText::change_entry_text ( QString const & txt )
{
	mainwindow.backend.uprefs.set ( PREFS_TEXT_ENTRY, txt.toUtf8().data() );
	update_preview ();
}

void DialogPasteText::update_preview ()
{
	mtPixmap * pixmap = mtPixy::text_render_preview_pixmap (
		pixy_pixmap_get_bytes_per_pixel ( mainwindow.backend.file.
			get_pixmap () ),
		mainwindow.mprefs.text_entry.c_str(),
		mainwindow.mprefs.get_font_name_full().c_str(),
		mainwindow.mprefs.text_font_size,
		mainwindow.mprefs.text_font_bold,
		mainwindow.mprefs.text_font_italic,
		mainwindow.mprefs.text_font_underline ?
			mtPixy::Font::STYLE_UNDERLINE_SINGLE :
			mtPixy::Font::STYLE_UNDERLINE_NONE,
		mainwindow.mprefs.text_font_strikethrough );

	m_qex_image->setPixmap ( pixmap );
}

void DialogPasteText::update_font_table ()
{
	m_label_font_name->setText ( mainwindow.mprefs.
		get_font_name_full().c_str() );
	m_label_font_size->setText ( QString ( "%1" )
		.arg ( mainwindow.mprefs.text_font_size ) );

	QString		txt;

	if ( mainwindow.mprefs.text_font_bold )
	{
		txt.append ( "Bold " );
	}

	if ( mainwindow.mprefs.text_font_italic )
	{
		txt.append ( "Italic " );
	}

	if ( mainwindow.mprefs.text_font_underline )
	{
		txt.append ( "Underline " );
	}

	if ( mainwindow.mprefs.text_font_strikethrough )
	{
		txt.append ( "Strikeout" );
	}

	m_label_font_effects->setText ( txt );
}

