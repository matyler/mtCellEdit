/*
	Copyright (C) 2013-2020 Mark Tyler

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



MainWindow	* mainwindow;



void MainWindow::createLeftSplit (
	QSplitter	* const	split
	)
{
	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );

	QWidget * widget = new QWidget;
	widget->setLayout ( layv );

	split->addWidget ( widget );

	QHBoxLayout * row = new QHBoxLayout;
	row->setMargin ( 0 );
	row->setSpacing ( 0 );
	layv->addLayout ( row );

	menuBar = new QMenuBar;
	row->addWidget ( menuBar );
	menuBar->setSizePolicy ( QSizePolicy ( QSizePolicy::Maximum,
		QSizePolicy::Preferred ) );

	// Widget that expands to centralise the sheet picker
	widget = new QWidget;
	row->addWidget ( widget );

	buttonSheet = new mtQEX::ButtonMenu;
	row->addWidget ( buttonSheet );
	connect ( buttonSheet, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( sheetChanged ( int ) ) );

	// Widget that expands to centralise the sheet picker
	widget = new QWidget;
	row->addWidget ( widget );

	buttonQuicksum = new mtQEX::ButtonMenu;
	row->addWidget ( buttonQuicksum );

	row = new QHBoxLayout;
	row->setMargin ( 0 );
	layv->addLayout ( row );

	editCellref = new MyLineEdit;
	editCellref->setSizePolicy (
		QSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Preferred ) );
	editCellref->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );
	row->addWidget ( editCellref );
	connect ( editCellref, SIGNAL ( pressUpdateKey () ),
		this, SLOT ( pressUpdateCellRef () ) );
	connect ( editCellref, SIGNAL ( pressStopKey () ),
		this, SLOT ( pressStopCellRef () ) );
	connect ( editCellref, SIGNAL ( pressTabKey ( int ) ),
		this, SLOT ( pressTabCellRef ( int ) ) );
	connect ( editCellref, SIGNAL ( pressFocusOut () ),
		this, SLOT ( pressFocusOutCellRef () ) );

	editCelltext = new MyLineEdit;
	row->addWidget ( editCelltext );
	connect ( editCelltext, SIGNAL ( pressUpdateKey () ),
		this, SLOT ( pressUpdateCellText () ) );
	connect ( editCelltext, SIGNAL ( pressStopKey () ),
		this, SLOT ( pressStopCellText () ) );
	connect ( editCelltext, SIGNAL ( pressTabKey ( int ) ),
		this, SLOT ( pressTabCellText ( int ) ) );
	connect ( editCelltext, SIGNAL ( pressArrowKey ( int ) ),
		this, SLOT ( pressArrowCellText ( int ) ) );

	labelQuicksum = new QLabel ( "0" );
	labelQuicksum->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
	labelQuicksum->setMinimumWidth ( 64 );
	labelQuicksum->setMargin ( 3 );
	row->addWidget ( labelQuicksum );

	viewMain = new CedView;
	layv->addWidget ( viewMain );
}

void MainWindow::pressRightSplitTab (
	int	const	index
	)
{
	if ( index == TAB_CLOSE )
	{
		tabWidget->hide ();
		viewMain->setFocus ();
	}

	if ( index == TAB_VIEW )
	{
		viewTab->show ();
	}
	else
	{
		// We need to hide the CedView when not visible otherwise it
		// pushes the split divider unexpectedly to the left when using:
		// the find tool; graphs; etc. when using frozen panes.
		viewTab->hide ();
	}
}

void MainWindow::createRightSplit (
	QSplitter	* const	split
	)
{
	tabWidget = new QTabWidget;
	split->addWidget ( tabWidget );

	// NOTE: any changes to order must be put into TAB_* in the header file

	QWidget * tabFind = new QWidget;
	tabWidget->addTab ( tabFind, "Find" );

	QVBoxLayout * layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	tabFind->setLayout ( layv );

	QHBoxLayout * row = new QHBoxLayout;
	row->setMargin ( 0 );
	layv->addLayout ( row );

	editFindText = new QLineEdit ( mtQEX::qstringFromC (
		pprfs->getString ( GUI_INIFILE_FIND_TEXT ) ) );
	row->addWidget ( editFindText );
	editFindText->setSizePolicy ( QSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Preferred ) );
	connect ( editFindText, SIGNAL ( returnPressed () ), this,
		SLOT ( press_Find () ) );

	QPushButton * button = new QPushButton ( "Find" );
	row->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this, SLOT ( press_Find () ) );

	findMenuBar = new QMenuBar;
	row->addWidget ( findMenuBar );
	findMenuBar->setSizePolicy ( QSizePolicy ( QSizePolicy::Fixed,
		QSizePolicy::Preferred ) );

	findTable = new QTableWidget;
	layv->addWidget ( findTable );

	findTable->setSelectionMode ( QAbstractItemView::SingleSelection );
	findTable->setSelectionBehavior ( QAbstractItemView::SelectRows );
	findTable->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	findTable->setColumnCount ( 4 );
	findTable->setShowGrid ( false );
	findTable->verticalHeader ()->setDefaultSectionSize (
		findTable->verticalHeader ()->fontMetrics ().height () + 4 );
	findTable->verticalHeader ()->QEX_RESIZEMODE ( QHeaderView::Fixed );
	findTable->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
	findTable->horizontalHeader ()->setStretchLastSection ( true );

	connect ( findTable, SIGNAL( currentCellChanged ( int, int, int, int )),
		this, SLOT ( findCellChanged ( int, int, int, int ) ) );

	QStringList columnLabels;

	columnLabels
		<< "Sheet"
		<< "Row"
		<< "Column"
		<< "Content"
		;
	findTable->setHorizontalHeaderLabels ( columnLabels );
	findTable->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );

	QWidget * tabGraph = new QWidget;
	tabWidget->addTab ( tabGraph, "Graph" );

	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	tabGraph->setLayout ( layv );

	graphSplit = new QSplitter ( Qt::Vertical );
	layv->addWidget ( graphSplit );


	QWidget * widget = new QWidget;
	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	widget->setLayout ( layv );
	graphSplit->addWidget ( widget );


	row = new QHBoxLayout;
	row->setMargin ( 0 );
	row->setSpacing ( 0 );
	layv->addLayout ( row );

	graphMenuBar = new QMenuBar;
	row->addWidget ( graphMenuBar );

	buttonGraph = new mtQEX::ButtonMenu;
	row->addWidget ( buttonGraph );
	connect ( buttonGraph, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( graphChanged ( int ) ) );

	graphWidget = new mtQEX::Image;
	layv->addWidget ( graphWidget );

	widget = new QWidget;
	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	widget->setLayout ( layv );
	graphSplit->addWidget ( widget );

	graphTextEdit = new QTextEdit;
	layv->addWidget ( graphTextEdit );

	widget = new QWidget;
	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	widget->setLayout ( layv );
	tabWidget->addTab ( widget, "View" );

	viewTab = new CedView;
	layv->addWidget ( viewTab );

	QWidget * tabClose = new QWidget;
	tabWidget->addTab ( tabClose, "Close" );

	connect ( tabWidget, SIGNAL ( currentChanged (int) ), this,
		SLOT ( pressRightSplitTab (int) ) );
}

void MainWindow::initGraphSplit ()
{
	// This is needed when setting the split size
	tabWidget->setCurrentIndex ( TAB_GRAPH );

	QList<int> ss = graphSplit->sizes ();
	tabWidget->hide ();
	ss.replace ( 0, ss.value ( 1 ) );
	graphSplit->setSizes ( ss );
}

MainWindow::MainWindow (
	Backend		* const be
	)
	:
	backend		( be ),
	pprfs		( &be->preferences ),
	cedFile		(),
	cedClipboard	(),
	sheetRows	( 0 ),
	sheetCols	( 0 ),
	memChanged	( 0 ),
	lastExportSheetType ( CUI_SHEET_EXPORT_TSV_QUOTED ),
	lastExportGraphType ( CUI_GRAPH_TYPE_PDF )
{
	mainwindow = this;

	// Preparation

	memset ( &crendr, 0, sizeof ( crendr ) );

	ced_init ();

	cedFile = cui_file_new ();
	cedClipboard = cui_clip_new ();

	if (	! cedFile		||
		! cedClipboard		||
		cui_file_book_new ( cedFile )
		)
	{
		QMessageBox::critical ( this, "Error",
			"Unable to initialize program." );

		exit ( 0 );
	}


	// Widgets

	setWindowTitle ( VERSION );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/256x256/apps/"
		BIN_NAME ".png" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	QVBoxLayout * layv = new QVBoxLayout ( this );
	layv->setMargin ( 0 );

	QSplitter * split = new QSplitter ( Qt::Horizontal );
	layv->addWidget ( split );

	createLeftSplit ( split );
	createRightSplit ( split );
	createMenus ();
	createQuicksum ();

	pref_change_font ( NULL, 0, pprfs );
	updateRecentFiles ();

	setMinimumSize ( 160, 160 );
	setGeometry ( pprfs->getInt ( PREFS_WINDOW_X ),
		pprfs->getInt ( PREFS_WINDOW_Y ),
		pprfs->getInt ( PREFS_WINDOW_W ),
		pprfs->getInt ( PREFS_WINDOW_H ) );


	if ( 0 == pprfs->getInt ( GUI_INIFILE_MAIN_WINDOW"_state" ) )
	{
		showMaximized ();
	}
	else
	{
		show ();
	}

	// Make the graph split area appear near the centre
	initGraphSplit ();

	viewMain->setFocus ();

	// Create and show UI before loading a file
	mtQEX::process_qt_pending ();

	// This is needed to stop visual corruption of the graph area widgets
	tabWidget->setCurrentIndex ( TAB_VIEW );

	if (	! backend->get_cline_filename () ||
		projectLoad ( backend->get_cline_filename () )
		)
	{
		projectClearAll ();
	}
}

MainWindow::~MainWindow ()
{
	delete crendr.font;
	crendr.font = NULL;

	cui_clip_free ( cedClipboard );
	cui_file_free ( cedFile );
}


void MainWindow::storeWindowGeometry ()
{
	if ( isMaximized () )
	{
		pprfs->set ( GUI_INIFILE_MAIN_WINDOW"_state", 0 );
	}
	else
	{
		pprfs->set ( GUI_INIFILE_MAIN_WINDOW"_state", 1 );

		pprfs->set ( PREFS_WINDOW_X, geometry().x () );
		pprfs->set ( PREFS_WINDOW_Y, geometry().y () );
		pprfs->set ( PREFS_WINDOW_W, geometry().width () );
		pprfs->set ( PREFS_WINDOW_H, geometry().height () );
	}
}

void MainWindow::closeEvent (
	QCloseEvent	* const	ev
	)
{
	if ( isEnabled () == false )
	{
		// Main window is currently disabled so ignore all requests
		ev->ignore ();

		return;
	}

	if ( okToLoseChanges () )
	{
		// No changes, or user happy to lose them
		ev->accept ();
		storeWindowGeometry ();
	}
	else
	{
		// Changes have occured, user not consenting to lose them
		ev->ignore ();
	}
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Backend		backend;


	// Parse command line and initialize prefs as required
	if ( backend.command_line ( argc, argv ) )
	{
		return 0;
	}

	// I don't want Qt snooping or changing my command line.
	int	dummy_argc	= 1;
	char	dummy_str[1]	= { 0 };
	char	* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	MainWindow	window ( &backend );

	return app.exec ();
}

