/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include "qt4.h"



void Mainwindow::press_butt_analysis ()
{
	DialogButtAnalysis dialog ( *this );
}



#define LABEL_COL	0
#define INFO_COL	1
#define PAD_COL		2



DialogButtAnalysis::DialogButtAnalysis (
	Mainwindow	&mw
	)
	:
	QDialog		( &mw ),
	m_bucket_analyse (),
	m_bucket_all	(),
	m_bit_table	(),
	m_byte_min	(),
	m_byte_max	(),
	m_qi_8bit	(),
	m_qi_16bit	(),
	analysis	( *mw.backend.db.get_butt () ),
	mainwindow	( mw )
{
	mtKit::unique_ptr<mtPixy::Image> im_8bit;
	mtKit::unique_ptr<mtPixy::Image> im_16bit;

	if ( report_lib_error ( &mw, analysis.init ( im_8bit, im_16bit ) ) )
	{
		return;
	}

	QWidget		* w;
	QHBoxLayout	* hbox;
	QLabel		* label;

	setWindowTitle ( "Butt Analysis" );

	QVBoxLayout * vbox = new QVBoxLayout;
	setLayout ( vbox );

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	label = new QLabel ( "Bucket: " );
	hbox->addWidget ( label );

	m_bucket_analyse = new QSpinBox;
	m_bucket_analyse->setRange ( 0, mw.backend.db.get_butt ()->
		get_bucket_total () - 1 );
	hbox->addWidget ( m_bucket_analyse );

	connect ( m_bucket_analyse, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( press_spin_bucket ( int ) ) );

	m_bucket_all = new QCheckBox ( "All Buckets" );
	m_bucket_all->setCheckState ( Qt::Unchecked );
	m_bucket_all->setSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Preferred );
	hbox->addWidget ( m_bucket_all );

	connect ( m_bucket_all, SIGNAL ( stateChanged ( int ) ),
		this, SLOT ( press_bucket_all ( int ) ) );

/// ----------------------------------------------------------------------------

	w = new QWidget;
	vbox->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	label = new QLabel ( "Size: " );
	hbox->addWidget ( label );

	m_bucket_size = new QLabel ( "" );
	hbox->addWidget ( m_bucket_size );

	label = new QLabel ( "" );
	hbox->addWidget ( label );
	label->setSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Preferred );

/// ----------------------------------------------------------------------------

	QTabWidget * tabWidget = new QTabWidget;
	vbox->addWidget ( tabWidget );

	QVBoxLayout	* layv;
	QWidget		* tab;
	QSlider		* slider;
	QGridLayout	* grid;

	tab = new QWidget;
	tabWidget->addTab ( tab, "Bits" );

	layv = new QVBoxLayout;
	tab->setLayout ( layv );

	m_bit_table = new QTableWidget;
	layv->addWidget ( m_bit_table );

	m_bit_table->setSelectionMode ( QAbstractItemView::SingleSelection );
	m_bit_table->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_bit_table->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_bit_table->setShowGrid ( false );
	m_bit_table->verticalHeader ()->hide ();

	m_bit_table->setCurrentItem ( NULL );	// Stops double selection of 0,0
	m_bit_table->setRowCount ( 9 );
	m_bit_table->setColumnCount ( 2 );
	m_bit_table->setMinimumHeight ( 0 );

	char const * const bit_txt[] = { "1", "2", "3", "4",
			"5", "6", "7", "8", "Total" };

	for ( int i = 0; i < 9; i++ )
	{
		QTableWidgetItem * twItem;

		twItem = new QTableWidgetItem;
		twItem->setText ( mtQEX::qstringFromC ( bit_txt[i] ) );
		m_bit_table->setItem ( i, 0, twItem );
	}

	QStringList columnLabels;
	columnLabels
		<< "Bit"
		<< "% of 1"
		;
	m_bit_table->setHorizontalHeaderLabels ( columnLabels );

/// ----------------------------------------------------------------------------

	tab = new QWidget;
	tabWidget->addTab ( tab, "Bytes" );

	layv = new QVBoxLayout;
	tab->setLayout ( layv );

	w = new QWidget;
	layv->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );
	grid->setColumnStretch ( PAD_COL, 1 ); // Stretch column INFO_COL

	grid->addWidget ( new QLabel ( "Mean: " ), 0, LABEL_COL );
	grid->addWidget ( new QLabel ( "Min: " ), 1, LABEL_COL );
	grid->addWidget ( new QLabel ( "Max: " ), 2, LABEL_COL );

	m_byte_min = new QLabel ( "-" );
	m_byte_max = new QLabel ( "-" );

	QString mean;

	mean = QString::number ( 100.0 * analysis. get_byte_mean(), 'f', 9 ) +
		QString ( "%" );
	label = new QLabel ( mean );

	grid->addWidget ( label, 0, INFO_COL );
	grid->addWidget ( m_byte_min, 1, INFO_COL );
	grid->addWidget ( m_byte_max, 2, INFO_COL );

	label->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
	m_byte_min->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
	m_byte_max->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

	slider = new QSlider ( Qt::Horizontal );
	layv->addWidget ( slider );

	slider->setRange ( 1, 16 );
	slider->setValue ( 2 );

	connect ( slider, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( press_slider_8bit ( int ) ) );

	m_qi_8bit = new mtQEX::Image;
	m_qi_8bit->setZoom ( 2 );
	layv->addWidget ( m_qi_8bit );
	m_qi_8bit->setImage ( im_8bit.release () );

/// ----------------------------------------------------------------------------

	tab = new QWidget;
	tabWidget->addTab ( tab, "16 Bit" );

	layv = new QVBoxLayout;
	tab->setLayout ( layv );

	slider = new QSlider ( Qt::Horizontal );
	layv->addWidget ( slider );

	slider->setRange ( 1, 16 );
	slider->setValue ( 2 );

	connect ( slider, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( press_slider_16bit ( int ) ) );

	m_qi_16bit = new mtQEX::Image;
	m_qi_16bit->setZoom ( 2 );
	layv->addWidget ( m_qi_16bit );
	m_qi_16bit->setImage ( im_16bit.release () );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	show ();

	do_analysis ();
	repopulate ();

	exec ();
}

void DialogButtAnalysis::press_spin_bucket ( int const ARG_UNUSED(i) )
{
	do_analysis ();
}

void DialogButtAnalysis::press_bucket_all ( int const ARG_UNUSED(i) )
{
	do_analysis ();
}

void DialogButtAnalysis::press_slider_8bit ( int const i )
{
	m_qi_8bit->setZoom ( i );
}

void DialogButtAnalysis::press_slider_16bit ( int const i )
{
	m_qi_16bit->setZoom ( i );
}

void DialogButtAnalysis::repopulate ()
{
	int64_t const bucket_size = analysis.get_bucket_size ();
	double const bucket_size_mb = ((double)bucket_size) / 1024.0 / 1024.0;

	m_bucket_size->setText ( QString::number ( bucket_size ) +
		QString (" (") + QString::number ( bucket_size_mb, 'f', 1 ) +
		QString (" MB)") );

	double const byte_mean = analysis.get_byte_mean ();
	double const * byte_list = analysis.get_byte_list ();
	double blist[9];

	analysis.get_bit_percents ( blist[8], blist );

	for ( int i = 0; i < 9; i++ )
	{
		QTableWidgetItem * const twItem = new QTableWidgetItem;
		twItem->setText ( QString::number(100.0 * blist[i], 'f', 9) +
			QString ("%") );
		twItem->setTextAlignment ( Qt::AlignRight | Qt::AlignVCenter );
		m_bit_table->setItem ( i, 1, twItem );
	}

	m_bit_table->resizeColumnsToContents ();

	double byte_min = 1.0;
	double byte_max = -1.0;

	for ( int i = 0; i < 256; i++ )
	{
		double const v = 100.0 * (byte_list[i] - byte_mean);
		byte_min = MIN ( byte_min, v );
		byte_max = MAX ( byte_max, v );
	}

	m_byte_min->setText( QString::number( byte_min, 'f', 9) + QString("%"));
	m_byte_max->setText( QString::number( byte_max, 'f', 9) + QString("%"));

	m_qi_8bit->update ();
	m_qi_16bit->update ();
}



class ButtAnalThread : public QThread
{
public:
	ButtAnalThread (
		mtDW::OTPanalysis	&analysis,
		int		const	bucket,
		bool		const	all_buckets,
		mtPixy::Image	* const	image_8bit,
		mtPixy::Image	* const	image_16bit
		)
		:
		m_analysis	( analysis ),
		m_error		( 0 ),
		m_bucket	( bucket ),
		m_all_buckets	( all_buckets ),
		m_image_8bit	( image_8bit ),
		m_image_16bit	( image_16bit )
	{
	}

	void run ()
	{
		if ( m_all_buckets )
		{
			m_error = m_analysis.analyse_all_buckets ( m_image_8bit,
				m_image_16bit );
		}
		else
		{
			m_error = m_analysis.analyse_bucket ( m_image_8bit,
				m_image_16bit, m_bucket );
		}
	}

	inline int error () { return m_error; }

private:
	mtDW::OTPanalysis	&m_analysis;
	int			m_error;
	int		const	m_bucket;
	bool		const	m_all_buckets;
	mtPixy::Image * const	m_image_8bit;
	mtPixy::Image * const	m_image_16bit;
};



void DialogButtAnalysis::do_analysis ()
{
	mtQEX::BusyDialog busy ( this );
	ButtAnalThread work ( analysis, m_bucket_analyse->value (),
		m_bucket_all->checkState (),
		m_qi_8bit->getImage (), m_qi_16bit->getImage () );

	work.start ();

	busy.wait_for_thread ( work );

	report_lib_error ( this, work.error () );

	repopulate ();
}

