/*
	Copyright (C) 2023-2024 Mark Tyler

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



void Mainwindow::press_palette_equalize ()
{
	DialogEqualize dialog ( *this, DialogEqualize::TYPE_EQUALIZE_PALETTE );

	if ( dialog.exec () == QDialog::Accepted )
	{
		unsigned char res[6];

		dialog.get_results ( res );
		operation_update ( backend.file.effect_equalize_palette (res),
			dialog.get_type_text().c_str(),
			Mainwindow::UPDATE_ALL
			);
	}
}

void Mainwindow::press_effects_equalize ()
{
	DialogEqualize dialog ( *this, DialogEqualize::TYPE_EQUALIZE_IMAGE );

	if ( dialog.exec () == QDialog::Accepted )
	{
		unsigned char res[6];

		dialog.get_results ( res );
		operation_update ( backend.file.effect_equalize_image (res),
			dialog.get_type_text().c_str(),
			Mainwindow::UPDATE_ALL
			);
	}
}

DialogEqualize::DialogEqualize (
	Mainwindow	&mw,
	int	const	type
	)
	:
	QDialog		( &mw ),
	mainwindow	( mw )
{
	unsigned char rgb_min_max[6];
	int r;

	if ( TYPE_EQUALIZE_PALETTE == type )
	{
		m_type_text = "Palette Equalize";
		r = mw.backend.file.effect_equalize_palette_info( rgb_min_max );
	}
	else
	{
		m_type_text = "Image Equalize";
		r = mw.backend.file.effect_equalize_image_info ( rgb_min_max );
	}

	if ( r )
	{
		return;
	}

	setWindowTitle ( m_type_text.c_str() );

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setContentsMargins ( 5, 5, 5, 5 );
	vbox->setSpacing ( 5 );
	setLayout ( vbox );

	QGridLayout * grid = new QGridLayout;
	vbox->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Red" ), 0, 1 );
	grid->addWidget ( new QLabel ( "Green" ), 0, 2 );
	grid->addWidget ( new QLabel ( "Blue" ), 0, 3 );
	grid->addWidget ( new QLabel ( "Original Min" ), 1, 0 );
	grid->addWidget ( new QLabel ( "Original Max" ), 2, 0 );
	grid->addWidget ( new QLabel ( "Final Min" ), 3, 0 );
	grid->addWidget ( new QLabel ( "Final Max" ), 4, 0 );

	for ( int i = 0; i < 6; i++ )
	{
		char txt[16];
		snprintf ( txt, sizeof(txt), "%i", (int)rgb_min_max[i] );
		grid->addWidget ( new QLabel ( txt ), 1 + (i%2), 1 + (i / 2) );
	}

	for ( int i = 0; i < 6; i++ )
	{
		m_sbox[i] = new QSpinBox;
		m_sbox[i]->setRange ( 0, 255 );
		m_sbox[i]->setValue ( 255 * (i%2) );
		grid->addWidget ( m_sbox[i], 3 + (i%2), 1 + (i / 2) );
	}

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( accepted () ), this, SLOT ( accept () ));
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	move ( QCursor::pos () );
}

void DialogEqualize::get_results ( unsigned char rgb_min_max[6] )
{
	for ( int i = 0; i < 6; i++ )
	{
		rgb_min_max[i] = (unsigned char)m_sbox[i]->value();
	}
}

