/*
	Copyright (C) 2016-2020 Mark Tyler

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



DialogClickImage::DialogClickImage (
	Mainwindow	* const	parent,
	mtPixmap const * const	pm,
	int			&px,
	int			&py
	)
	:
	QDialog		( parent )
{
	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setContentsMargins ( 0, 0, 0, 0 );
	vbox->setSpacing ( 0 );
	setLayout ( vbox );

	QLabel * label = new LabelClick ( this, px, py );
	label->setAlignment ( Qt::AlignTop | Qt::AlignLeft );
	vbox->addWidget ( label );

	std::unique_ptr<QPixmap> const pixmap (
		mtQEX::qpixmap_from_pixypixmap (pm) );
	if ( pixmap )
	{
		label->setPixmap ( *pixmap.get() );
	}

	move ( QCursor::pos () );
}

void LabelClick::mouseReleaseEvent (
	QMouseEvent	* const	ev
	)
{
	if ( ev->button () & Qt::LeftButton )
	{
		m_dialog->accept ();
		m_x = ev->x ();
		m_y = ev->y ();
	}
	else if ( ev->button () & Qt::RightButton )
	{
		m_dialog->reject ();
	}
}

LabelClick::LabelClick (
	QDialog * const	dia,
	int		&px,
	int		&py
	)
	:
	m_dialog( dia ),
	m_x	( px ),
	m_y	( py )
{
}

DialogBrushSettings::DialogBrushSettings (
	Mainwindow	* const parent,
	int		&s,
	int		&f
	)
	:
	QDialog		( parent ),
	m_spacing	( s ),
	m_flow		( f )
{
	QVBoxLayout	* vbox;
	QGroupBox	* gbox;


	setWindowTitle ( "Brush Settings" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setContentsMargins ( 5, 5, 5, 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	gbox = new QGroupBox ( "Spacing" );
	layv->addWidget ( gbox );

	vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_sbox_spacing = new QSpinBox;
	vbox->addWidget ( m_sbox_spacing );
	m_sbox_spacing->setRange ( mtPixy::Brush::SPACING_MIN,
		mtPixy::Brush::SPACING_MAX );
	m_sbox_spacing->setValue ( m_spacing );
	m_sbox_spacing->selectAll ();

	gbox = new QGroupBox ( "Flow" );
	layv->addWidget ( gbox );

	vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_sbox_flow = new QSpinBox;
	vbox->addWidget ( m_sbox_flow );
	m_sbox_flow->setRange( mtPixy::Brush::FLOW_MIN, mtPixy::Brush::FLOW_MAX );
	m_sbox_flow->setValue ( m_flow );


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close | QDialogButtonBox::Ok );


	layv->addWidget ( button_box );

	connect ( button_box->button ( QDialogButtonBox::Ok ),
		SIGNAL ( pressed () ), this, SLOT ( press_ok () ));
	connect( button_box, SIGNAL ( rejected () ), this, SLOT( reject () ));

	move ( QCursor::pos () );
}

void DialogBrushSettings::press_ok ()
{
	m_spacing = m_sbox_spacing->value ();
	m_flow = m_sbox_flow->value ();

	accept ();
}

