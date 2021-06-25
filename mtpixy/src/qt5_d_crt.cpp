/*
	Copyright (C) 2020 Mark Tyler

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



void Mainwindow::press_effects_crt ()
{
	DialogCRT dialog ( this );

	if ( dialog.exec () == QDialog::Accepted )
	{
		operation_update ( backend.file.effect_crt (
			dialog.get_scale () ),
			"CRT Effect", Mainwindow::UPDATE_ALL );
	}
}

DialogCRT::DialogCRT (
	Mainwindow	* const	parent
	)
	:
	QDialog		( parent )
{
	setWindowTitle ( "CRT Effect" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setContentsMargins ( 5, 5, 5, 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	m_sbox_scale = new QSpinBox;
	m_sbox_scale->setRange ( PIXY_EFFECT_CRT_SCALE_MIN,
		PIXY_EFFECT_CRT_SCALE_MAX );
	m_sbox_scale->setValue ( 6 );
	m_sbox_scale->selectAll ();

	QGridLayout * grid = new QGridLayout;
	layv->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Scale" ), 0, 0 );
	grid->addWidget ( m_sbox_scale, 0, 1 );


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );


	layv->addWidget ( button_box );

	connect ( button_box, SIGNAL ( accepted () ), this,
		SLOT ( accept () ) );
	connect ( button_box, SIGNAL ( rejected () ), this,
		SLOT ( reject () ) );

	move ( QCursor::pos () );
}

int DialogCRT::get_scale ()
{
	return m_sbox_scale->value ();
}

