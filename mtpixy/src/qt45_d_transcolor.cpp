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



DialogTransColor::DialogTransColor (
	Mainwindow		&mw,
	mtPixy::Image	* const im
	)
	:
	m_palette_live	( im->get_palette () ),
	m_rgb		( im->get_type () == mtPixy::Image::RGB ? 1 : 0 ),
	mainwindow	( mw )
{
	m_opal.copy ( m_palette_live );

	setWindowTitle ( "Transform Colour" );

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setMargin ( 5 );
	vbox->setSpacing ( 5 );
	setLayout ( vbox );

	QGridLayout * grid = new QGridLayout;
	vbox->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Gamma" ), 0, 0 );
	grid->addWidget ( new QLabel ( "Brightness" ), 1, 0 );
	grid->addWidget ( new QLabel ( "Contrast" ), 2, 0 );
	grid->addWidget ( new QLabel ( "Saturation" ), 3, 0 );
	grid->addWidget ( new QLabel ( "Hue" ), 4, 0 );
	grid->addWidget ( new QLabel ( "Posterize" ), 5, 0 );

	m_label_gamma = new QLabel;
	m_label_brightness = new QLabel;
	m_label_contrast = new QLabel;
	m_label_saturation = new QLabel;
	m_label_hue = new QLabel ( "-1529" );	// Needed below - label_hue
	m_label_posterize = new QLabel;

	grid->addWidget ( m_label_gamma, 0, 2 );
	grid->addWidget ( m_label_brightness, 1, 2 );
	grid->addWidget ( m_label_contrast, 2, 2 );
	grid->addWidget ( m_label_saturation, 3, 2 );
	grid->addWidget ( m_label_hue, 4, 2 );
	grid->addWidget ( m_label_posterize, 5, 2 );

	m_sl_gamma = new QSlider ( Qt::Horizontal );
	m_sl_brightness = new QSlider ( Qt::Horizontal );
	m_sl_contrast = new QSlider ( Qt::Horizontal );
	m_sl_saturation = new QSlider ( Qt::Horizontal );
	m_sl_hue = new QSlider ( Qt::Horizontal );
	m_sl_posterize = new QSlider ( Qt::Horizontal );

	m_sl_gamma->setRange ( -100, 100 );
	m_sl_brightness->setRange ( -100, 100 );
	m_sl_contrast->setRange ( -100, 100 );
	m_sl_saturation->setRange ( -100, 100 );
	m_sl_hue->setRange ( -1529, 1529 );
	m_sl_posterize->setRange ( 1, 8 );

	grid->addWidget ( m_sl_gamma, 0, 1 );
	grid->addWidget ( m_sl_brightness, 1, 1 );
	grid->addWidget ( m_sl_contrast, 2, 1 );
	grid->addWidget ( m_sl_saturation, 3, 1 );
	grid->addWidget ( m_sl_hue, 4, 1 );
	grid->addWidget ( m_sl_posterize, 5, 1 );


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
		QDialogButtonBox::Reset );


	vbox->addWidget ( button_box );

	connect ( this, SIGNAL ( finished ( int ) ), this,
		SLOT ( press_finished ( int ) ));

	connect ( button_box, SIGNAL ( accepted () ), this, SLOT ( accept () ));
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));
	connect ( button_box->button ( QDialogButtonBox::Reset ),
		SIGNAL ( clicked () ), this, SLOT ( slider_reset () ));

	move ( QCursor::pos () );
	setModal ( true );
	show ();

	// Enlarge width to twice minimal
	resize ( 2 * width(), height() );

	// Fix label column width to avoid wobbling when sliding
	m_label_hue->setMinimumWidth ( m_label_hue->width () );
	slider_reset ();

	connect ( m_sl_gamma, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( slider_changed ( int ) ));
	connect ( m_sl_brightness, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( slider_changed ( int ) ));
	connect ( m_sl_contrast, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( slider_changed ( int ) ));
	connect ( m_sl_saturation, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( slider_changed ( int ) ));
	connect ( m_sl_hue, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( slider_changed ( int ) ));
	connect ( m_sl_posterize, SIGNAL ( valueChanged ( int ) ), this,
		SLOT ( slider_changed ( int ) ));

	exec ();
}

void DialogTransColor::slider_reset ()
{
	m_sl_gamma->blockSignals ( true );
	m_sl_brightness->blockSignals ( true );
	m_sl_contrast->blockSignals ( true );
	m_sl_saturation->blockSignals ( true );
	m_sl_hue->blockSignals ( true );
	m_sl_posterize->blockSignals ( true );

	m_sl_gamma->setValue ( 0 );
	m_sl_brightness->setValue ( 0 );
	m_sl_contrast->setValue ( 0 );
	m_sl_saturation->setValue ( 0 );
	m_sl_hue->setValue ( 0 );
	m_sl_posterize->setValue ( 8 );

	m_sl_gamma->blockSignals ( false );
	m_sl_brightness->blockSignals ( false );
	m_sl_contrast->blockSignals ( false );
	m_sl_saturation->blockSignals ( false );
	m_sl_hue->blockSignals ( false );
	m_sl_posterize->blockSignals ( false );

	slider_changed ( 0 );
}

void DialogTransColor::slider_changed ( int ARG_UNUSED ( i ) )
{
	int	const	g = m_sl_gamma->value ();
	int	const	b = m_sl_brightness->value ();
	int	const	c = m_sl_contrast->value ();
	int	const	s = m_sl_saturation->value ();
	int	const	h = m_sl_hue->value ();
	int	const	p = m_sl_posterize->value ();


	m_label_gamma->setText ( QString ( "%1" ).arg ( g ) );
	m_label_brightness->setText ( QString ( "%1" ).arg ( b ) );
	m_label_contrast->setText ( QString ( "%1" ).arg ( c ) );
	m_label_saturation->setText ( QString ( "%1" ).arg ( s ) );
	m_label_hue->setText ( QString ( "%1" ).arg ( h ) );
	m_label_posterize->setText ( QString ( "%1" ).arg ( p ) );

	m_palette_live->copy ( &m_opal );
	m_palette_live->transform_color ( g, b, c, s, h, p );

	mainwindow.update_ui ( Mainwindow::UPDATE_CANVAS
				| Mainwindow::UPDATE_PALETTE
				| Mainwindow::UPDATE_BRUSH
				| Mainwindow::UPDATE_TOOLBAR );

	if ( m_rgb )
	{
		mainwindow.set_canvas_rgb_transform ( g, b, c, s, h, p );
	}
}

void DialogTransColor::press_finished (
	int	const	r
	)
{
	if ( m_rgb )
	{
		mainwindow.unset_canvas_rgb_transform ();
	}

	m_palette_live->copy ( &m_opal );

	if ( r == QDialog::Rejected )
	{
		mainwindow.update_ui ( Mainwindow::UPDATE_ALL );
	}
	else
	{
		int	const	g = m_sl_gamma->value ();
		int	const	b = m_sl_brightness->value ();
		int	const	c = m_sl_contrast->value ();
		int	const	s = m_sl_saturation->value ();
		int	const	h = m_sl_hue->value ();
		int	const	p = m_sl_posterize->value ();


		mainwindow.operation_update ( mainwindow.backend.file.
			effect_transform_color ( g, b, c, s, h, p ),
			"Transform Colour", Mainwindow::UPDATE_ALL );
	}
}

