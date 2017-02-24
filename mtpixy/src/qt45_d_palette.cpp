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



void Mainwindow::press_palette_sort ()
{
	DialogPaletteSort dialog ( backend.file.get_image ()->get_palette ()->
				get_color_total () - 1 );


	if ( dialog.exec () == QDialog::Accepted )
	{
		operation_update ( backend.file.palette_sort (
			(unsigned char)dialog.get_start (),
			(unsigned char)dialog.get_end (),
			dialog.get_sort_type (),
			dialog.get_reverse () ),
			"Palette Sort", Mainwindow::UPDATE_ALL );
	}
}

DialogPaletteSort::DialogPaletteSort (
	int	const	coltot
	)
{
	setWindowTitle ( "Palette Sort" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	m_sbox_start = new QSpinBox;
	m_sbox_start->setRange ( 0, coltot );
	m_sbox_start->setValue ( 0 );
	m_sbox_start->selectAll ();

	m_sbox_end = new QSpinBox;
	m_sbox_end->setRange ( 0, coltot );
	m_sbox_end->setValue ( coltot );

	QGridLayout * grid = new QGridLayout;
	layv->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Start Index" ), 0, 0 );
	grid->addWidget ( m_sbox_start, 0, 1 );
	grid->addWidget ( new QLabel ( "End Index" ), 1, 0 );
	grid->addWidget ( m_sbox_end, 1, 1 );

	QGroupBox * gbox = new QGroupBox ( "Sort Type" );
	layv->addWidget ( gbox );

	QVBoxLayout * vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_rbut_hue = new QRadioButton ( "Hue" );
	vbox->addWidget ( m_rbut_hue );

	m_rbut_saturation = new QRadioButton ( "Saturation (MAX - MIN)" );
	vbox->addWidget ( m_rbut_saturation );

	m_rbut_value = new QRadioButton ( "Value (MAX)" );
	vbox->addWidget ( m_rbut_value );

	m_rbut_min = new QRadioButton ( "MIN" );
	vbox->addWidget ( m_rbut_min );

	m_rbut_brightness = new QRadioButton ( "Brightness" );
	vbox->addWidget ( m_rbut_brightness );

	m_rbut_red = new QRadioButton ( "Red" );
	vbox->addWidget ( m_rbut_red );

	m_rbut_green = new QRadioButton ( "Green" );
	vbox->addWidget ( m_rbut_green );

	m_rbut_blue = new QRadioButton ( "Blue" );
	vbox->addWidget ( m_rbut_blue );

	m_rbut_frequency = new QRadioButton ( "Frequency" );
	vbox->addWidget ( m_rbut_frequency );

	m_rbut_hue->setChecked ( true );

	m_cb_reverse = new QCheckBox ( "Reverse" );
	m_cb_reverse->setChecked ( false );
	layv->addWidget ( m_cb_reverse );


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );


	layv->addWidget ( button_box );

	connect ( button_box, SIGNAL ( accepted () ), this,
		SLOT ( accept () ) );
	connect ( button_box, SIGNAL ( rejected () ), this,
		SLOT ( reject () ) );

	move ( QCursor::pos () );
}

int DialogPaletteSort::get_start ()
{
	return m_sbox_start->value ();
}

int DialogPaletteSort::get_end ()
{
	return m_sbox_end->value ();
}

bool DialogPaletteSort::get_reverse ()
{
	return m_cb_reverse->isChecked ();
}

mtPixy::Image::PaletteSortType DialogPaletteSort::get_sort_type ()
{
	if ( m_rbut_hue->isChecked () )
	{
		return mtPixy::Image::SORT_HUE;
	}
	else if ( m_rbut_saturation->isChecked () )
	{
		return mtPixy::Image::SORT_SATURATION;
	}
	else if ( m_rbut_value->isChecked () )
	{
		return mtPixy::Image::SORT_VALUE;
	}
	else if ( m_rbut_min->isChecked () )
	{
		return mtPixy::Image::SORT_MIN;
	}
	else if ( m_rbut_brightness->isChecked () )
	{
		return mtPixy::Image::SORT_BRIGHTNESS;
	}
	else if ( m_rbut_red->isChecked () )
	{
		return mtPixy::Image::SORT_RED;
	}
	else if ( m_rbut_green->isChecked () )
	{
		return mtPixy::Image::SORT_GREEN;
	}
	else if ( m_rbut_blue->isChecked () )
	{
		return mtPixy::Image::SORT_BLUE;
	}
	else if ( m_rbut_frequency->isChecked () )
	{
		return mtPixy::Image::SORT_FREQUENCY;
	}

	return mtPixy::Image::SORT_HUE;
}

