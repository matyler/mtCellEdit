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



DialogGetInt::DialogGetInt (
	int		const	min,
	int		const	max,
	int		const	val,
	char	const * const	title,
	char	const * const	subtitle,
	int		const	apply
	)
{
	setWindowTitle ( title );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	QGroupBox * gbox = new QGroupBox ( subtitle );
	layv->addWidget ( gbox );

	QVBoxLayout * vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	sbox_int = new QSpinBox;
	vbox->addWidget ( sbox_int );
	sbox_int->setRange ( min, max );
	sbox_int->setValue ( val );
	sbox_int->selectAll ();


	QDialogButtonBox::StandardButtons	but = QDialogButtonBox::Close;
	QDialogButtonBox			* button_box;


	if ( apply )
	{
		but |= QDialogButtonBox::Apply;
	}
	else
	{
		but |= QDialogButtonBox::Ok;
	}

	button_box = new QDialogButtonBox ( but );
	layv->addWidget ( button_box );

	if ( apply )
	{
		connect ( button_box->button ( QDialogButtonBox::Apply ),
			SIGNAL ( pressed () ), this, SLOT ( accept () ));
		button_box->button ( QDialogButtonBox::Apply )->setDefault (
			true );
	}
	else
	{
		connect ( button_box, SIGNAL ( accepted () ), this,
			SLOT ( accept () ));
	}

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	move ( QCursor::pos () );
}

int DialogGetInt::get_int ()
{
	return sbox_int->value ();
}

DialogImageInfo::DialogImageInfo (
	Backend		&be
	)
{
	int		urp, pnip, pt;
	int		pf [ mtPixy::Palette::COLOR_TOTAL_MAX ];
	mtPixy::Image * const im = be.file.get_image ();


	if ( ! im )
	{
		close ();
		return;
	}

	if ( im->get_information ( urp, pnip, pf, pt ) )
	{
		QMessageBox::critical ( this, "Error", "Unable analyse image.");

		return;
	}

	setWindowTitle ( "Image Information" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	QGroupBox * gbox = new QGroupBox ( "General" );
	layv->addWidget ( gbox );

	QVBoxLayout * vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	QGridLayout * grid = new QGridLayout;
	vbox->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Unique RGB Pixels" ), 0, 0 );
	grid->addWidget ( new QLabel ( QString ( "%1" ).arg ( urp ) ), 0, 1 );
	grid->addWidget ( new QLabel ( "Pixels not in palette" ), 1, 0 );
	grid->addWidget ( new QLabel ( QString ( "%1" ).arg ( pnip ) ), 1, 1 );
	grid->addWidget ( new QLabel ( "Undo memory" ), 2, 0 );
	grid->addWidget ( new QLabel ( QString ( "%1MB" ).
		arg ( be.file.get_undo_mb (), 0, 'f', 1 ) ), 2, 1 );

	gbox = new QGroupBox ( "Palette Frequency" );
	layv->addWidget ( gbox );

	vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	QTableWidget * pf_table = new QTableWidget;
	vbox->addWidget ( pf_table );

	pf_table->setColumnCount ( 3 );
	pf_table->setRowCount ( pt );
	pf_table->setSelectionMode ( QAbstractItemView::SingleSelection );
	pf_table->setSelectionBehavior ( QAbstractItemView::SelectRows );
	pf_table->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	pf_table->setShowGrid ( false );
	pf_table->verticalHeader ()->setDefaultSectionSize (
		pf_table->verticalHeader ()->fontMetrics ().height () + 4 );
	pf_table->verticalHeader ()->hide ();

	QStringList	column_labels;
	column_labels << "Index" << "Frequency" << "%";
	pf_table->setHorizontalHeaderLabels ( column_labels );


	int	const	tot_pixels = im->get_width () * im->get_height ();
	int		freq, i;
	double		perc;


	for ( i = 0; i < pt; i++ )
	{
		freq = pf[i];
		perc = 100.0 * (double)freq / (double)tot_pixels;

		pf_table->setCellWidget ( i, 0, new QLabel ( QString( "%1" ).
			arg( i ) ) );

		pf_table->setCellWidget ( i, 1, new QLabel ( QString( "%1" ).
			arg( freq ) ) );

		pf_table->setCellWidget ( i, 2, new QLabel ( QString( "%1" ).
			arg( perc, 0, 'f', 1 ) ) );
	}

	pf_table->resizeColumnsToContents ();


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	layv->addWidget ( button_box );
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	resize ( -1, (int)(1.5 * height ()) );

	move ( QCursor::pos () );
	exec ();
}

DialogImageNew::DialogImageNew (
	int	const	w,
	int	const	h,
	int	const	t,
	bool	const	clip
	)
	:
	m_result	( 0 )
{
	setWindowTitle ( "New Image" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	m_sbox_width = new QSpinBox;
	m_sbox_width->setRange ( mtPixy::Image::WIDTH_MIN,
		mtPixy::Image::WIDTH_MAX );
	m_sbox_width->setValue ( w );
	m_sbox_width->selectAll ();

	m_sbox_height = new QSpinBox;
	m_sbox_height->setRange ( mtPixy::Image::HEIGHT_MIN,
		mtPixy::Image::HEIGHT_MAX );
	m_sbox_height->setValue ( h );

	QGridLayout * grid = new QGridLayout;
	layv->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Width" ), 0, 0 );
	grid->addWidget ( m_sbox_width, 0, 1 );
	grid->addWidget ( new QLabel ( "Height" ), 1, 0 );
	grid->addWidget ( m_sbox_height, 1, 1 );

	m_rbut_rgb = new QRadioButton ( "24 bit RGB" );
	layv->addWidget ( m_rbut_rgb );

	m_rbut_indexed = new QRadioButton ( "Indexed Palette" );
	layv->addWidget ( m_rbut_indexed );

	m_rbut_clipboard = new QRadioButton ( "From Clipboard" );
	layv->addWidget ( m_rbut_clipboard );
	m_rbut_clipboard->setEnabled ( clip );

	if ( t == mtPixy::Image::RGB )
	{
		m_rbut_rgb->setChecked ( true );
	}
	else
	{
		m_rbut_indexed->setChecked ( true );
	}


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );


	layv->addWidget ( button_box );
	connect ( button_box, SIGNAL ( accepted () ), this, SLOT ( accept () ));
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	move ( QCursor::pos () );
}

int DialogImageNew::get_width ()
{
	return m_sbox_width->value ();
}

int DialogImageNew::get_height ()
{
	return m_sbox_height->value ();
}

int DialogImageNew::get_type ()
{
	if ( m_rbut_rgb->isChecked () )
	{
		return mtPixy::Image::RGB;
	}
	else if ( m_rbut_indexed->isChecked () )
	{
		return mtPixy::Image::INDEXED;
	}

	return -1;		// Clipboard
}

DialogImageIndexed::DialogImageIndexed (
	Mainwindow	&mw
	)
	:
	mainwindow	( mw )
{
	setWindowTitle ( "Convert To Indexed" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 5 );
	layv->setSpacing ( 5 );
	setLayout ( layv );

	QGroupBox * gbox = new QGroupBox ( "Dither Type" );
	layv->addWidget ( gbox );

	QVBoxLayout * vbox = new QVBoxLayout;
	gbox->setLayout ( vbox );

	m_rbut_none = new QRadioButton ( "None" );
	vbox->addWidget ( m_rbut_none );

	m_rbut_basic = new QRadioButton ( "Basic" );
	vbox->addWidget ( m_rbut_basic );

	m_rbut_floyd = new QRadioButton ( "Floyd-Steinberg" );
	vbox->addWidget ( m_rbut_floyd );

	m_rbut_none->setChecked ( true );


	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );


	layv->addWidget ( button_box );

	connect ( button_box->button ( QDialogButtonBox::Ok ),
		SIGNAL ( pressed () ), this, SLOT ( press_ok () ));
	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	move ( QCursor::pos () );
	exec ();
}

void DialogImageIndexed::press_ok ()
{
	close ();

	mtPixy::Image::DitherType	dt = mtPixy::Image::DITHER_NONE;

	if ( m_rbut_none->isChecked () )
	{
		dt = mtPixy::Image::DITHER_NONE;
	}
	else if ( m_rbut_basic->isChecked () )
	{
		dt = mtPixy::Image::DITHER_BASIC;
	}
	else if ( m_rbut_floyd->isChecked () )
	{
		dt = mtPixy::Image::DITHER_FLOYD;
	}

	mainwindow.operation_update (
		mainwindow.backend.file.convert_to_indexed ( dt ),
		"Convert To Indexed", Mainwindow::UPDATE_ALL_IMAGE );
}

PanView::PanView (
	QDialog		* const	dia,
	Mainwindow		&mw,
	QScrollArea	* const	sa,
	int		const	ui,
	mtPixy::Image	* const	im
	)
	:
	m_dialog	( dia ),
	m_scroll_area	( sa ),
	mainwindow	( mw )
{
	m_iw = im->get_width ();
	m_ih = im->get_height ();

	if ( m_iw >= m_ih )
	{
		m_pw = 128 * ui;
		m_ph = (m_pw * m_ih) / m_iw;
	}
	else
	{
		m_ph = 128 * ui;
		m_pw = (m_ph * m_iw) / m_ih;
	}


	QPixmap		* pmap;
	mtPixy::Image	* ti;


	if ( im->get_canvas_bpp () == 3 )
	{
		ti = im->scale ( m_pw, m_ph, mtPixy::Image::SMOOTH );
	}
	else
	{
		mtPixy::Image	* const	ati = im->convert_to_rgb ();


		ti = ati->scale ( m_pw, m_ph, mtPixy::Image::SMOOTH );
		delete ati;
	}

	pmap = mtQEX::qpixmap_from_pixyimage ( ti );
	delete ti;

	if ( pmap )
	{
		setPixmap ( *pmap );
		delete pmap;
	}
}

void PanView::mousePressEvent (
	QMouseEvent	* const	ev
	)
{
	if ( ev->buttons () & Qt::RightButton )
	{
		m_dialog->close ();
	}
	else if ( ev->buttons () & Qt::LeftButton )
	{
		set_scroll_position_h(m_scroll_area, ((double)ev->x ()) / m_pw);
		set_scroll_position_v(m_scroll_area, ((double)ev->y ()) / m_ph);
	}
}

void PanView::mouseMoveEvent (
	QMouseEvent	* const	ev
	)
{
	mousePressEvent ( ev );
}

void DialogPan::keyPressEvent (
	QKeyEvent	* const	ev
	)
{
	int const dx = MAX ( 1, m_scroll_area->horizontalScrollBar ()->
				pageStep () / 4 );
	int const dy = MAX ( 1, m_scroll_area->verticalScrollBar ()->
				pageStep () / 4 );


	switch ( ev->key () )
	{
	case Qt::Key_Up:
		m_scroll_area->verticalScrollBar ()->setValue (
			m_scroll_area->verticalScrollBar ()->value () - dy );
		return;

	case Qt::Key_Down:
		m_scroll_area->verticalScrollBar ()->setValue (
			m_scroll_area->verticalScrollBar ()->value () + dy );
		return;

	case Qt::Key_Left:
		m_scroll_area->horizontalScrollBar ()->setValue (
			m_scroll_area->horizontalScrollBar()->value() - dx );
		return;

	case Qt::Key_Right:
		m_scroll_area->horizontalScrollBar ()->setValue (
			m_scroll_area->horizontalScrollBar()->value() + dx );
		return;

	default:
		break;
	}

	if ( mainwindow.handle_zoom_keys ( ev ) )
	{
		return;
	}

	close ();
}

DialogPan::DialogPan (
	Mainwindow		&mw,
	int		const	ui,
	mtPixy::Image	* const	im,
	QScrollArea	* const	sa
	)
	:
	m_scroll_area	( sa ),
	mainwindow	( mw )
{
	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setMargin ( 0 );
	vbox->setSpacing ( 0 );
	setLayout ( vbox );


	PanView * label = new PanView ( this, mw, sa, ui, im );
	label->setAlignment ( Qt::AlignTop | Qt::AlignLeft );
	vbox->addWidget ( label );

	move ( QCursor::pos () );

	exec ();
}

