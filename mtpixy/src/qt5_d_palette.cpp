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



void Mainwindow::press_palette_sort ()
{
	DialogPaletteSort dialog ( this,
		pixy_pixmap_get_palette_size ( backend.file.get_pixmap() ) - 1);

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
	Mainwindow	* const	parent,
	int		const	coltot
	)
	:
	QDialog		( parent )
{
	setWindowTitle ( "Palette Sort" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setContentsMargins ( 5, 5, 5, 5 );
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

int DialogPaletteSort::get_sort_type ()
{
	if ( m_rbut_hue->isChecked () )
	{
		return PIXY_PALETTE_SORT_HUE;
	}
	else if ( m_rbut_saturation->isChecked () )
	{
		return PIXY_PALETTE_SORT_SATURATION;
	}
	else if ( m_rbut_value->isChecked () )
	{
		return PIXY_PALETTE_SORT_VALUE;
	}
	else if ( m_rbut_min->isChecked () )
	{
		return PIXY_PALETTE_SORT_MIN;
	}
	else if ( m_rbut_brightness->isChecked () )
	{
		return PIXY_PALETTE_SORT_BRIGHTNESS;
	}
	else if ( m_rbut_red->isChecked () )
	{
		return PIXY_PALETTE_SORT_RED;
	}
	else if ( m_rbut_green->isChecked () )
	{
		return PIXY_PALETTE_SORT_GREEN;
	}
	else if ( m_rbut_blue->isChecked () )
	{
		return PIXY_PALETTE_SORT_BLUE;
	}
	else if ( m_rbut_frequency->isChecked () )
	{
		return PIXY_PALETTE_SORT_FREQUENCY;
	}

	return PIXY_PALETTE_SORT_HUE;
}

void Mainwindow::press_palette_new ()
{
	int	const	type = mprefs.file_new_palette_type;
	int	const	num = mprefs.file_new_palette_num;
	DialogPaletteNew dialog ( this, type, num );


	if ( dialog.exec () == QDialog::Accepted )
	{
		// OK pressed
		backend.uprefs.set ( PREFS_FILE_NEW_PALETTE_TYPE,
			dialog.get_type () );
		backend.uprefs.set ( PREFS_FILE_NEW_PALETTE_NUM,
			dialog.get_num () );
		press_palette_load_default ();
	}
}

DialogPaletteNew::DialogPaletteNew (
	Mainwindow	* const	parent,
	int		const	type,
	int		const	num
	)
	:
	QDialog		( parent )
{
	setWindowTitle ( "New Palette" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setContentsMargins ( 5, 5, 5, 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	m_sbox_num = new QSpinBox;
	m_sbox_num->setRange ( 2, 6 );
	m_sbox_num->setValue ( num );
	m_sbox_num->selectAll ();

	QGridLayout * grid = new QGridLayout;
	layv->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Palette Size" ), 0, 0 );
	grid->addWidget ( m_sbox_num, 0, 1 );

	std::string &pal_file =parent->backend.mprefs.file_new_palette_file;
	m_file_edit = new QLineEdit ( mtQEX::qstringFromC ( pal_file.c_str() ));
	m_file_edit->setReadOnly ( true );
	auto button = new QPushButton ( "Select" );
	grid->addWidget ( new QLabel ( "File" ), 1, 0 );
	grid->addWidget ( m_file_edit, 1, 1 );
	grid->addWidget ( button, 1, 2 );
	connect ( button, &QPushButton::pressed, [parent, &pal_file, this]()
		{
			QString const filename = QFileDialog::getOpenFileName (
				parent, "Select Palette File",
				m_file_edit->text(),
				"*.gpl", Q_NULLPTR,
				QFileDialog::DontUseNativeDialog
				);

			if ( ! filename.isEmpty () )
			{
				m_file_edit->setText ( filename );
				pal_file = filename.toUtf8 ().data ();
			}
		});

	QGroupBox * gbox = new QGroupBox ( "Palette Type" );
	layv->addWidget ( gbox );

	QVBoxLayout * vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_rbut_uniform = new QRadioButton ( "Uniform" );
	vbox->addWidget ( m_rbut_uniform );

	m_rbut_balanced = new QRadioButton ( "Balanced" );
	vbox->addWidget ( m_rbut_balanced );

	m_rbut_file = new QRadioButton ( "File" );
	vbox->addWidget ( m_rbut_file );

	switch ( type )
	{
	case 0:	m_rbut_uniform->setChecked ( true );	break;
	case 1:	m_rbut_balanced->setChecked ( true );	break;
	case 2:	m_rbut_file->setChecked ( true );	break;
	}


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );


	layv->addWidget ( button_box );

	connect ( button_box, SIGNAL ( accepted () ), this,
		SLOT ( accept () ) );
	connect ( button_box, SIGNAL ( rejected () ), this,
		SLOT ( reject () ) );

	move ( QCursor::pos () );
}

int DialogPaletteNew::get_num ()
{
	return m_sbox_num->value ();
}

int DialogPaletteNew::get_type ()
{
	if ( m_rbut_uniform->isChecked () )
	{
		return 0;
	}
	else if ( m_rbut_balanced->isChecked () )
	{
		return 1;
	}
	else if ( m_rbut_file->isChecked () )
	{
		return 2;
	}

	return 0;
}

