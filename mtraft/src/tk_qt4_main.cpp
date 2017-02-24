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

#include "tk_qt4.h"
#include "icon_xpm.xpm"



MainWindow::MainWindow (
	char	const *	scan_directory
	)
	:
	progressBar	(),
	tabWidget	(),
	buttonCopy	(),
	table		()
{
	QWidget		* widget;
	QHBoxLayout	* topRow;
	QPushButton	* buttonQuit;
	QShortcut	* shortcut;


	setWindowTitle ( VERSION );
	setWindowIcon ( QPixmap ( icon_xpm ) );

	widget = new QWidget;
	setCentralWidget ( widget );

	shortcut = new QShortcut ( QKeySequence ( Qt::CTRL + Qt::Key_PageUp ),
		this );
	connect ( shortcut, SIGNAL ( activated () ), this,
		SLOT ( pressTabNext () ) );

	shortcut = new QShortcut ( QKeySequence ( Qt::CTRL +
		Qt::Key_PageDown ), this );
	connect ( shortcut, SIGNAL ( activated () ), this,
		SLOT ( pressTabPrevious () ) );

	buttonQuit = new QPushButton ( "Quit" );
	buttonQuit->setShortcut ( Qt::CTRL + Qt::Key_Q );
	connect ( buttonQuit, SIGNAL ( clicked () ), this,
		SLOT ( pressButtonQuit () ) );

	progressBar = new QProgressBar;
	progressBar->setMinimum ( 0 );

	buttonCopy = new QPushButton ( "Copy To Clipboard" );
	buttonCopy->setShortcut ( Qt::CTRL + Qt::Key_C );
	connect ( buttonCopy, SIGNAL ( clicked () ), this,
		SLOT ( pressButtonCopy () ) );

	buttonCopy->setEnabled ( false );

	topRow = new QHBoxLayout ();
	topRow->addWidget ( buttonQuit );
	topRow->addWidget ( progressBar );
	topRow->addWidget ( buttonCopy );

	tabWidget = new QTabWidget;

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setMargin ( 5 );
	vbox->addLayout ( topRow );
	vbox->addWidget ( tabWidget );

	widget->setLayout ( vbox );


	mtPrefTable	const	prefs_table[] =
	{
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

	doAnalysis ( scan_directory );
}

MainWindow::~MainWindow ()
{
	prefs.set ( PREFS_WINDOW_X, geometry().x () );
	prefs.set ( PREFS_WINDOW_Y, geometry().y () );
	prefs.set ( PREFS_WINDOW_W, geometry().width () );
	prefs.set ( PREFS_WINDOW_H, geometry().height () );

	for ( int i = 0; i < MAX_TABS; i++ )
	{
		if ( table[i] == NULL )
		{
			continue;
		}

		delete table[i];
		table[i] = NULL;
	}
}

static int raftScan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	int		const	ARG_UNUSED ( row ), // Row calculated in sheet
	void		* const	user_data
	)
{
	MainWindow	* const	main = (MainWindow *)user_data;


	while ( QCoreApplication::hasPendingEvents () )
	{
		QCoreApplication::processEvents ();
	}

	if ( busyState::WORKING != main->busy.getStatus () )
	{
		return 1;		// User wants to stop
	}

	return 0;			// Keep scanning
}

void MainWindow::doAnalysis (
	char	const * const	path
	)
{
	char		* new_path = NULL;
	int		tabTot;
	int		tabCurrent;
	CedSheet	* sheet;


	tabTot = tabWidget->count ();
	tabCurrent = tabWidget->currentIndex ();

	if (	tabTot < 0		||
		tabTot >= MAX_TABS	||
		table[ tabTot ] != NULL
		)
	{
		// Should never happen, so silently fail

		return;
	}

	new_path = raft_path_check ( path );
	if ( ! new_path )
	{
		return;
	}

	tabWidget->setEnabled ( false );
	busy.setWorking ();
	progressBar->setMaximum ( 0 );

	if ( raft_scan_sheet ( new_path, &sheet, raftScan, this ) )
	{
		// Fail
	}
	else
	{
		QWidget * widget = new QWidget;
		tabWidget->addTab ( widget, QString ("%1").arg ( tabTot + 1 ) );
		tabWidget->setCurrentWidget ( widget );

		QVBoxLayout * vbox = new QVBoxLayout;
		vbox->setMargin ( 0 );
		widget->setLayout ( vbox );

		table[ tabTot ] = new tableAnalysis ( sheet, new_path, this,
			vbox );

		buttonCopy->setEnabled ( true );
	}

	tabWidget->setEnabled ( true );
	busy.setIdle ();
	progressBar->setMaximum ( 1 );

	if ( tabCurrent >= 0 && tabCurrent < MAX_TABS && table[ tabCurrent ] )
	{
		/*
		This hack is required because:

		After successfully creating a new tab, stop focus on the old tab
		moving to the QLineEdit widget.

		Keep the table selected when there is a failure resulting from
		trying to scan the <.> or <TOTAL> rows.
		*/

		table[ tabCurrent ]->setFocus ();
	}

	if ( table[ tabTot ] )
	{
		table[ tabTot ]->setFocus ();
	}

	free ( new_path );
}

void MainWindow::pressButtonQuit ()
{
	if ( busy.getStatus () != busyState::IDLE )
	{
		// We are analysing so stop
		busy.setStopped ();

		return;
	}

	close ();
}

void MainWindow::pressButtonCopy ()
{
	int		tabNum;


	tabNum = tabWidget->currentIndex ();

	if (	tabNum >= 0		&&
		tabNum < MAX_TABS	&&
		table[ tabNum ] != NULL
		)
	{
		table[ tabNum ]->copyToClipboard ();
	}
}

void MainWindow::pressTabNext ()
{
	tabWidget->setCurrentIndex ( tabWidget->currentIndex () - 1 );
}

void MainWindow::pressTabPrevious ()
{
	tabWidget->setCurrentIndex ( tabWidget->currentIndex () + 1 );
}

void MainWindow::closeEvent (
	QCloseEvent	* const	ev
	)
{
	if ( busy.getStatus () != busyState::IDLE )
	{
		// Program is busy so we can't stop yet, but signal a stop
		busy.setStopped ();
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
	char	const * const	scan_directory = raft_cline ( argc, argv );


	// I don't want Qt snooping or changing my command line.
	int		dummy_argc	= 1;
	char		dummy_str[1]	= { 0 },
			* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	MainWindow	window ( scan_directory );


	return app.exec ();
}

