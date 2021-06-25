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



DialogImageScale::DialogImageScale ( Mainwindow &mw )
	:
	QDialog		( &mw ),
	m_cb_smooth	(),
	m_start_w	( pixy_pixmap_get_width( mw.backend.file.get_pixmap())),
	m_start_h	( pixy_pixmap_get_height(mw.backend.file.get_pixmap())),
	m_wh_scale	( (double)m_start_h / (double)m_start_w ),
	mainwindow	( mw )
{
	setWindowTitle ( "Image Scale" );

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setContentsMargins ( 5, 5, 5, 5 );
	vbox->setSpacing ( 5 );
	setLayout ( vbox );

	QGridLayout * grid = new QGridLayout;
	vbox->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Width" ), 0, 1 );
	grid->addWidget ( new QLabel ( "Height" ), 0, 2 );
	grid->addWidget ( new QLabel ( "Original" ), 1, 0 );
	grid->addWidget ( new QLabel ( "New" ), 2, 0 );

	m_sbox_width = new QSpinBox;
	m_sbox_width->setRange ( PIXY_PIXMAP_WIDTH_MIN, PIXY_PIXMAP_WIDTH_MAX );
	m_sbox_width->setValue ( m_start_w );
	m_sbox_width->setEnabled ( false );
	grid->addWidget ( m_sbox_width, 1, 1 );

	m_sbox_height = new QSpinBox;
	m_sbox_height->setRange ( PIXY_PIXMAP_HEIGHT_MIN,
		PIXY_PIXMAP_HEIGHT_MAX );
	m_sbox_height->setValue ( m_start_h );
	m_sbox_height->setEnabled ( false );
	grid->addWidget ( m_sbox_height, 1, 2 );

	m_sbox_width = new QSpinBox;
	m_sbox_width->setRange ( PIXY_PIXMAP_WIDTH_MIN, PIXY_PIXMAP_WIDTH_MAX );
	m_sbox_width->setValue ( m_start_w );
	grid->addWidget ( m_sbox_width, 2, 1 );
	connect ( m_sbox_width, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( changed_sbox_width ( int ) ) );
	m_sbox_width->selectAll ();

	m_sbox_height = new QSpinBox;
	m_sbox_height->setRange ( PIXY_PIXMAP_HEIGHT_MIN,
		PIXY_PIXMAP_HEIGHT_MAX );
	m_sbox_height->setValue ( m_start_h );
	grid->addWidget ( m_sbox_height, 2, 2 );
	connect ( m_sbox_height, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( changed_sbox_height ( int ) ) );

	if ( pixy_pixmap_get_bytes_per_pixel ( mainwindow.backend.file.
		get_pixmap() ) == PIXY_PIXMAP_BPP_RGB
		)
	{
		m_cb_smooth = new QCheckBox ( "Smooth" );
		m_cb_smooth->setChecked ( true );
		vbox->addWidget ( m_cb_smooth );
	}

	m_cb_aspect_ratio = new QCheckBox ( "Fix Aspect Ratio" );
	m_cb_aspect_ratio->setChecked ( true );
	vbox->addWidget ( m_cb_aspect_ratio );


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );


	vbox->addWidget ( button_box );

	connect ( button_box->button ( QDialogButtonBox::Ok ),
		SIGNAL ( pressed () ), this, SLOT ( press_ok () ));
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	move ( QCursor::pos () );
	exec ();
}

void DialogImageScale::changed_sbox_width (
	int	const	i
	)
{
	if ( m_cb_aspect_ratio->isChecked () )
	{
		m_sbox_height->blockSignals ( true );
		m_sbox_height->setValue( (int)lround( (double)i * m_wh_scale ));
		m_sbox_height->blockSignals ( false );
	}
}

void DialogImageScale::changed_sbox_height (
	int	const	i
	)
{
	if ( m_cb_aspect_ratio->isChecked () )
	{
		m_sbox_width->blockSignals ( true );
		m_sbox_width->setValue( (int)lround( (double)i / m_wh_scale ) );
		m_sbox_width->blockSignals ( false );
	}
}

void DialogImageScale::press_ok ()
{
	int t = PIXY_SCALE_BLOCKY;

	close ();

	if ( m_cb_smooth && m_cb_smooth->isChecked () )
	{
		t = PIXY_SCALE_SMOOTH;
	}

	mainwindow.operation_update ( mainwindow.backend.file.scale (
		m_sbox_width->value (), m_sbox_height->value (), t ),
		"Image Scale", Mainwindow::UPDATE_ALL_IMAGE );
}

