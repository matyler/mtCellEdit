/*
	Copyright (C) 2013-2017 Mark Tyler

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
#include "icon_xpm.xpm"



MainWindow::MainWindow ()
	:
	m_progress	(),
	m_tab_widget	(),
	m_button_copy	(),
	m_table		()
{
	setWindowTitle ( VERSION );
	setWindowIcon ( QPixmap ( icon_xpm ) );

	QWidget * widget = new QWidget;
	setCentralWidget ( widget );

	QShortcut * shortcut = new QShortcut ( QKeySequence (
		Qt::CTRL + Qt::Key_PageUp ), this );
	connect ( shortcut, SIGNAL ( activated () ), this,
		SLOT ( press_tab_next () ) );

	shortcut = new QShortcut ( QKeySequence ( Qt::CTRL +
		Qt::Key_PageDown ), this );
	connect ( shortcut, SIGNAL ( activated () ), this,
		SLOT ( press_tab_previous () ) );

	QPushButton * button_quit = new QPushButton ( "Quit" );
	button_quit->setShortcut ( Qt::CTRL + Qt::Key_Q );
	connect ( button_quit, SIGNAL ( clicked () ), this,
		SLOT ( press_button_quit () ) );

	m_progress = new QProgressBar;
	m_progress->setMinimum ( 0 );

	m_button_copy = new QPushButton ( "Copy To Clipboard" );
	m_button_copy->setShortcut ( Qt::CTRL + Qt::Key_C );
	connect ( m_button_copy, SIGNAL ( clicked () ), this,
		SLOT ( press_button_copy () ) );
	m_button_copy->setEnabled ( false );

	QHBoxLayout * top_row = new QHBoxLayout ();
	top_row->addWidget ( button_quit );
	top_row->addWidget ( m_progress );
	top_row->addWidget ( m_button_copy );

	m_tab_widget = new QTabWidget;

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setMargin ( 5 );
	vbox->addLayout ( top_row );
	vbox->addWidget ( m_tab_widget );

	widget->setLayout ( vbox );

	mtPrefTable const prefs_table[] = {
	{ PREFS_WINDOW_X, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_Y, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_W, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_H, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	prefs.addTable ( prefs_table );
	prefs.load ( NULL, BIN_NAME );

	setMinimumSize ( 160, 160 );

	setGeometry ( prefs.getInt ( PREFS_WINDOW_X ),
		prefs.getInt ( PREFS_WINDOW_Y ),
		prefs.getInt ( PREFS_WINDOW_W ),
		prefs.getInt ( PREFS_WINDOW_H ) );

	show ();
}

MainWindow::~MainWindow ()
{
	prefs.set ( PREFS_WINDOW_X, geometry().x () );
	prefs.set ( PREFS_WINDOW_Y, geometry().y () );
	prefs.set ( PREFS_WINDOW_W, geometry().width () );
	prefs.set ( PREFS_WINDOW_H, geometry().height () );

	for ( int i = 0; i < MAX_TABS; i++ )
	{
		if ( m_table[i] == NULL )
		{
			continue;
		}

		delete m_table[i];
		m_table[i] = NULL;
	}
}

static int raftScan (
	void	* const	user_data
	)
{
	while ( QCoreApplication::hasPendingEvents () )
	{
		QCoreApplication::processEvents ();
	}

	MainWindow * const main = static_cast<MainWindow *>(user_data);

	if ( BusyState::WORKING != main->busy.get_status () )
	{
		return 1;		// User wants to stop
	}

	return 0;			// Keep scanning
}

void MainWindow::analyse (
	char	const * const	path
	)
{
	int const tab_tot = m_tab_widget->count ();
	int const tab_current = m_tab_widget->currentIndex ();

	if (	tab_tot < 0		||
		tab_tot >= MAX_TABS	||
		m_table[ tab_tot ] != NULL
		)
	{
		// Should never happen, so silently fail

		return;
	}

	char * new_path = raft_path_check ( path );
	if ( ! new_path )
	{
		return;
	}

	m_tab_widget->setEnabled ( false );
	busy.set_working ();
	m_progress->setMaximum ( 0 );

	CedSheet * sheet;

	if ( raft_scan_sheet ( new_path, &sheet, raftScan, this ) )
	{
		// Fail
	}
	else
	{
		QWidget * widget = new QWidget;
		m_tab_widget->addTab( widget, QString ("%1").arg( tab_tot + 1));
		m_tab_widget->setCurrentWidget ( widget );

		QVBoxLayout * vbox = new QVBoxLayout;
		vbox->setMargin ( 0 );
		widget->setLayout ( vbox );

		m_table[ tab_tot ] = new TableAnalysis ( sheet, new_path, *this,
			vbox );

		m_button_copy->setEnabled ( true );
	}

	m_tab_widget->setEnabled ( true );
	busy.set_idle ();
	m_progress->setMaximum ( 1 );

	if (	tab_current >= 0	&&
		tab_current < MAX_TABS	&&
		m_table[ tab_current ]
		)
	{
		/*
		This hack is required because:

		After successfully creating a new tab, stop focus on the old tab
		moving to the QLineEdit widget.

		Keep the table selected when there is a failure resulting from
		trying to scan the <.> or <TOTAL> rows.
		*/

		m_table[ tab_current ]->setFocus ();
	}

	if ( m_table[ tab_tot ] )
	{
		m_table[ tab_tot ]->setFocus ();
	}

	free ( new_path );
	new_path = NULL;
}

void MainWindow::press_button_quit ()
{
	if ( busy.get_status () != BusyState::IDLE )
	{
		// We are analysing so stop
		busy.set_stopped ();

		return;
	}

	close ();
}

void MainWindow::press_button_copy ()
{
	int const tab_num = m_tab_widget->currentIndex ();

	if (	tab_num >= 0		&&
		tab_num < MAX_TABS	&&
		m_table[ tab_num ] != NULL
		)
	{
		m_table[ tab_num ]->copy_to_clipboard ();
	}
}

void MainWindow::press_tab_next ()
{
	m_tab_widget->setCurrentIndex ( m_tab_widget->currentIndex () - 1 );
}

void MainWindow::press_tab_previous ()
{
	m_tab_widget->setCurrentIndex ( m_tab_widget->currentIndex () + 1 );
}

void MainWindow::closeEvent (
	QCloseEvent	* const	ev
	)
{
	if ( busy.get_status () != BusyState::IDLE )
	{
		// Program is busy so we can't stop yet, but signal a stop
		busy.set_stopped ();
		ev->ignore ();
	}
	else
	{
		// Program is idle so accept closure
		ev->accept ();
	}
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	char const * path = NULL;

	if ( raft_cline ( argc, argv, &path ) )
	{
		return 0;
	}


	// I don't want Qt snooping or changing my command line.
	int		dummy_argc	= 1;
	char		dummy_str[1]	= { 0 },
			* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	MainWindow	window;

	window.analyse ( path );

	return app.exec ();
}

