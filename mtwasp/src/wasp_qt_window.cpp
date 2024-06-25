/*
	Copyright (C) 2024 Mark Tyler

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

#include "wasp_qt.h"



MainWindow::MainWindow ( CommandLine & cline )
	:
	m_uprefs	( cline.uprefs ),
	m_mprefs	( cline.mprefs ),
	m_recent_files	( cline.recent_files ),
	m_gin		( SDL_INIT_AUDIO ),
	m_volume_view	( new WaveVolumeView ( *this, m_project ) )
{
	QWidget		* w;
	QVBoxLayout	* vbox, * vb;
	QHBoxLayout	* hbox, * hb;
	QGroupBox	* gbox;
	QPushButton	* button;
	QGridLayout	* grid;
	QSplitter	* split;

	setWindowTitle ( VERSION );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/scalable/apps/"
		BIN_NAME ".svg" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	w = new QWidget;
	setCentralWidget ( w );

	create_menu ();

	vbox = new QVBoxLayout;
	w->setLayout ( vbox );

	vbox->setContentsMargins ( 5, 5, 5, 5 );

/// ----------------------------------------------------------------------------

	hb = new QHBoxLayout;
	vbox->addLayout ( hb );

	gbox = new QGroupBox ( "Control" );
	hb->addWidget ( gbox );

	vb = new QVBoxLayout;
	gbox->setLayout ( vb );

	hbox = new QHBoxLayout;
	vb->addLayout ( hbox );

	button = new QPushButton ( "&Play" );
	hbox->addWidget ( button );
	connect( button, SIGNAL( clicked() ), this, SLOT(press_audio_play()));

	button = new QPushButton ( "&Stop" );
	hbox->addWidget ( button );
	connect( button, SIGNAL( clicked() ), this, SLOT(press_audio_stop()));

	hbox = new QHBoxLayout;
	vb->addLayout ( hbox );

	grid = new QGridLayout;
	hbox->addLayout ( grid );
	grid->addWidget ( new QLabel ( "Duration (secs)" ), 0, 0 );
	grid->addWidget ( new QLabel ( "Octave" ), 1, 0 );

	m_spin_wave_duration = new QDoubleSpinBox;
	m_spin_wave_duration->setDecimals ( 3 );
	m_spin_wave_duration->setRange (
		mtWasp::Project::WAVE_DURATION_MIN(),
		mtWasp::Project::WAVE_DURATION_MAX()
		);
	m_spin_wave_duration->setValue ( m_project.get_wave_duration_seconds());
	grid->addWidget ( m_spin_wave_duration, 0, 1 );
	connect ( m_spin_wave_duration, SIGNAL ( valueChanged (double) ), this,
		SLOT ( changed_spin_wave_duration ( double ) ) );

	m_spin_wave_octave = new QSpinBox;
	m_spin_wave_octave->setRange (
		mtWasp::Project::WAVE_OCTAVE_MIN,
		mtWasp::Project::WAVE_OCTAVE_MAX
		);
	m_spin_wave_octave->setValue ( m_project.get_wave_octave () );
	grid->addWidget ( m_spin_wave_octave, 1, 1 );
	connect ( m_spin_wave_octave, SIGNAL ( valueChanged (int) ), this,
		SLOT ( changed_spin_wave_octave ( int ) ) );

	// Add a padding widget to keep the grid to an optimal size
	w = new QWidget;
	hbox->addWidget ( w );
	w->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Preferred );

	// Add a padding widget to keep the widgets above to an optimal size
	w = new QWidget;
	vb->addWidget ( w );

/// ----------------------------------------------------------------------------

	gbox = new QGroupBox ( "Volume Envelope" );
	hb->addWidget ( gbox );

	hbox = new QHBoxLayout;
	gbox->setLayout ( hbox );

	vb = new QVBoxLayout;
	hbox->addLayout ( vb );

	grid = new QGridLayout;
	vb->addLayout ( grid );

	grid->addWidget ( new QLabel ( "P1" ), 0, 1, Qt::AlignHCenter );
	grid->addWidget ( new QLabel ( "P2" ), 0, 2, Qt::AlignHCenter );
	grid->addWidget ( new QLabel ( "X" ), 1, 0 );
	grid->addWidget ( new QLabel ( "Y" ), 2, 0 );

	grid->addWidget ( m_volume_view->get_spin_p1x(), 1, 1 );
	grid->addWidget ( m_volume_view->get_spin_p1y(), 2, 1 );
	grid->addWidget ( m_volume_view->get_spin_p2x(), 1, 2 );
	grid->addWidget ( m_volume_view->get_spin_p2y(), 2, 2 );

	// Add a padding widget to keep the widgets above to an optimal size
	w = new QWidget;
	vb->addWidget ( w );

	m_volume_view->setMinimumSize ( 128, 64 );
	m_volume_view->setSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Preferred );
	hbox->addWidget ( m_volume_view );

/// ----------------------------------------------------------------------------

	gbox = new QGroupBox ( "Waveform" );
	vbox->addWidget ( gbox );
	gbox->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Expanding );

	vb = new QVBoxLayout;
	gbox->setLayout ( vb );

	split = new QSplitter;
	vb->addWidget ( split );

	m_text_wave_function = new QTextEdit;
	split->addWidget ( m_text_wave_function );
	m_text_wave_function->setText ( m_project.get_wave_function().c_str() );
	connect ( m_text_wave_function, SIGNAL ( textChanged () ), this,
		SLOT ( changed_text_wave_function () ) );

	m_wave_function_view = new WaveFunctionView ( m_project );
	m_wave_function_view->setSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Expanding );
	split->addWidget ( m_wave_function_view );

/// ----------------------------------------------------------------------------

//	setMinimumSize ( 160, 160 );
	setGeometry ( m_mprefs.window_x, m_mprefs.window_y,
		m_mprefs.window_w, m_mprefs.window_h );

	if ( 1 == m_mprefs.window_maximized )
	{
		showMaximized ();
	}
	else
	{
		show ();
	}

	create_statusbar ();
	create_prefs_callbacks ();

	update_statusbar ();
	update_recent_files ();

	std::vector< char const * > const & fnames = cline.get_cline_files ();
	if ( fnames.size() > 0
		&& 0 == project_load ( fnames[0] )
		)
	{
		// Load a file from the command line if it's been set
	}
	else
	{
		update_titlebar ();
	}
}

MainWindow::~MainWindow ()
{
}

void MainWindow::store_window_geometry ()
{
	if ( isMaximized () )
	{
		m_uprefs.set ( PREFS_WINDOW_MAXIMIZED, 1 );
	}
	else
	{
		m_uprefs.set ( PREFS_WINDOW_MAXIMIZED, 0 );

		m_uprefs.set ( PREFS_WINDOW_X, geometry().x () );
		m_uprefs.set ( PREFS_WINDOW_Y, geometry().y () );
		m_uprefs.set ( PREFS_WINDOW_W, geometry().width () );
		m_uprefs.set ( PREFS_WINDOW_H, geometry().height () );
	}
}

void MainWindow::update_titlebar ()
{
	std::string const txt ( m_project.get_titlebar_text () );

	setWindowTitle ( mtQEX::qstringFromC ( txt.c_str() ) );
}

void MainWindow::update_inputs ()
{
	m_spin_wave_duration->blockSignals ( true );
	m_spin_wave_duration->setValue ( m_project.get_wave_duration_seconds());
	m_spin_wave_duration->blockSignals ( false );

	m_spin_wave_octave->blockSignals ( true );
	m_spin_wave_octave->setValue ( m_project.get_wave_octave () );
	m_spin_wave_octave->blockSignals ( false );

	m_text_wave_function->blockSignals ( true );
	m_text_wave_function->setText ( m_project.get_wave_function ().c_str());
	m_text_wave_function->blockSignals ( false );

	m_volume_view->load_values_from_project ();

	m_wave_function_view->update ();
}

void MainWindow::closeEvent ( QCloseEvent * const ev )
{
	if ( isEnabled () == false )
	{
		// Main window is currently disabled so ignore all requests
		ev->ignore ();

		return;
	}

	if ( ok_to_lose_changes () )
	{
		// No changes, or user happy to lose them
		ev->accept ();
		store_window_geometry ();
	}
	else
	{
		// Changes have occured, user not consenting to lose them
		ev->ignore ();
	}
}

void MainWindow::create_prefs_callbacks ()
{
	m_uprefs.set_callback ( PREFS_FILE_RECENT_MAXLEN, [this]()
		{
			update_recent_files ();
		} );
	m_uprefs.set_callback ( PREFS_AUDIO_OUTPUT_DEVICE, [this]()
		{
			update_statusbar ();
			m_project.set_audio_device (
				m_mprefs.audio_output_device );
		} );
}

void MainWindow::press_audio_play ()
{
	if ( m_project.play_wave_audio () )
	{
		QMessageBox::critical ( this, "Error", "Unable to play audio.");
	}
}

void MainWindow::press_audio_stop ()
{
	if ( m_project.stop_wave_audio () )
	{
		QMessageBox::critical ( this, "Error", "Unable to stop audio.");
	}
}

void MainWindow::press_audio_set_audio_device ()
{
	DialogAudioDevicePicker ( *this, m_uprefs, m_mprefs );
}

void MainWindow::changed_spin_wave_duration ( double const seconds )
{
	m_project.set_wave_duration_seconds ( seconds );
	update_titlebar ();
}

void MainWindow::changed_spin_wave_octave ( int const octave )
{
	m_project.set_wave_octave ( octave );
	update_titlebar ();
	update_statusbar ();
}

void MainWindow::changed_text_wave_function ()
{
	m_project.set_wave_function ( m_text_wave_function->toPlainText ()
		.toUtf8 ().data () );

	m_wave_function_view->update ();
	update_titlebar ();
}

