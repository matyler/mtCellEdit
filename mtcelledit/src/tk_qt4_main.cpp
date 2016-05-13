/*
	Copyright (C) 2013-2016 Mark Tyler

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
#include "icon.xpm"



MainWindow	* mainwindow;



MainWindow :: MainWindow (
	char	const	* const	prefs_filename,
	char	const	* const arg_filename
	)
	:
	cedFile		(),
	cedClipboard	(),
	sheetRows	( 0 ),
	sheetCols	( 0 ),
	memChanged	( 0 ),
	lastExportSheetType ( CUI_SHEET_EXPORT_TSV_QUOTED ),
	lastExportGraphType ( CUI_GRAPH_TYPE_PDF )
{
	QWidget		* widget,
			* tabFind,
			* tabGraph
			;
	QVBoxLayout	* layout;
	QHBoxLayout	* row;
	QSplitter	* split,
			* graphSplit;


	mainwindow = this;

	// Preparation

	memset ( &render, 0, sizeof ( render ) );

	ced_init ();

	prefs_init ( prefs_filename );

	mtQEX :: prefsInitPrefs ( prefs_file () );

	prefs_load ();

	cedFile = cui_file_new ();
	cedClipboard = cui_clip_new ();

	if (	! cedFile		||
		! cedClipboard		||
		cui_file_book_new ( cedFile )
		)
	{
		QMessageBox :: critical ( this, tr ( "Error" ),
			tr ( "Unable to initialize program." ) );

		exit ( 0 );
	}


	// Widgets

	setWindowTitle ( VERSION );
	this->setWindowIcon ( QPixmap ( icon_xpm ) );

	layout = new QVBoxLayout ( this );
	layout->setMargin ( 0 );

	split = new QSplitter ( Qt::Horizontal );
	layout->addWidget ( split );

	// Left split area

	widget = new QWidget;
	layout = new QVBoxLayout;
	layout->setMargin ( 0 );
	layout->setSpacing ( 0 );
	widget->setLayout ( layout );
	split->addWidget ( widget );

	row = new QHBoxLayout;
	row->setMargin ( 0 );
	row->setSpacing ( 0 );
	layout->addLayout ( row );

	menuBar = new QMenuBar;
	row->addWidget ( menuBar );
	menuBar->setSizePolicy ( QSizePolicy ( QSizePolicy::Maximum,
		QSizePolicy::Preferred ) );

	// Widget that expands to centralise the sheet picker
	widget = new QWidget;
	row->addWidget ( widget );

	buttonSheet = new qexButtonMenu;
	row->addWidget ( buttonSheet );
	connect ( buttonSheet, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( sheetChanged ( int ) ) );

	// Widget that expands to centralise the sheet picker
	widget = new QWidget;
	row->addWidget ( widget );

	buttonQuicksum = new qexButtonMenu;
	row->addWidget ( buttonQuicksum );

	row = new QHBoxLayout;
	row->setMargin ( 0 );
	layout->addLayout ( row );

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
	layout->addWidget ( viewMain );

	// Right split area

	tabWidget = new QTabWidget;
	split->addWidget ( tabWidget );

	tabFind = new QWidget;
	tabWidget->addTab ( tabFind, tr ( "Find" ) );

	layout = new QVBoxLayout;
	layout->setMargin ( 0 );
	layout->setSpacing ( 0 );
	tabFind->setLayout ( layout );

	row = new QHBoxLayout;
	row->setMargin ( 0 );
	layout->addLayout ( row );

	editFindText = new QLineEdit ( mtQEX::qstringFromC (
		prefs_get_string ( GUI_INIFILE_FIND_TEXT ) ) );
	row->addWidget ( editFindText );
	editFindText->setSizePolicy ( QSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Preferred ) );
	connect ( editFindText, SIGNAL ( returnPressed () ), this,
		SLOT ( pressFind () ) );

	QPushButton * button = new QPushButton ( tr ( "Find" ) );
	row->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this, SLOT ( pressFind () ) );

	findMenuBar = new QMenuBar;
	row->addWidget ( findMenuBar );
	findMenuBar->setSizePolicy ( QSizePolicy ( QSizePolicy::Fixed,
		QSizePolicy::Preferred ) );

	findTable = new QTableWidget;
	layout->addWidget ( findTable );

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
		<< tr ( "Sheet" )
		<< tr ( "Row" )
		<< tr ( "Column" )
		<< tr ( "Content" )
		;
	findTable->setHorizontalHeaderLabels ( columnLabels );
	findTable->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );

	tabGraph = new QWidget;
	tabWidget->addTab ( tabGraph, tr ( "Graph" ) );

	layout = new QVBoxLayout;
	layout->setMargin ( 0 );
	layout->setSpacing ( 0 );
	tabGraph->setLayout ( layout );

	graphSplit = new QSplitter ( Qt::Vertical );
	layout->addWidget ( graphSplit );


	widget = new QWidget;
	layout = new QVBoxLayout;
	layout->setMargin ( 0 );
	layout->setSpacing ( 0 );
	widget->setLayout ( layout );
	graphSplit->addWidget ( widget );


	row = new QHBoxLayout;
	row->setMargin ( 0 );
	row->setSpacing ( 0 );
	layout->addLayout ( row );

	graphMenuBar = new QMenuBar;
	row->addWidget ( graphMenuBar );

	buttonGraph = new qexButtonMenu;
	row->addWidget ( buttonGraph );
	connect ( buttonGraph, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( graphChanged ( int ) ) );

	graphWidget = new qexImage;
	layout->addWidget ( graphWidget );

	widget = new QWidget;
	layout = new QVBoxLayout;
	layout->setMargin ( 0 );
	layout->setSpacing ( 0 );
	widget->setLayout ( layout );
	graphSplit->addWidget ( widget );

	graphTextEdit = new QTextEdit;
	layout->addWidget ( graphTextEdit );

	viewTab = new CedView;
	tabWidget->addTab ( viewTab, tr ( "View" ) );

	createMenus ();
	createQuicksum ();

	pref_change_font ( NULL, 0, this );
	updateRecentFiles ();

	setMinimumSize ( 160, 160 );
	setGeometry ( prefs_get_int ( PREFS_WINDOW_X ),
		prefs_get_int ( PREFS_WINDOW_Y ),
		prefs_get_int ( PREFS_WINDOW_W ),
		prefs_get_int ( PREFS_WINDOW_H ) );


	if ( 0 == prefs_get_int ( GUI_INIFILE_MAIN_WINDOW"_state" ) )
	{
		showMaximized ();
	}
	else
	{
		show ();
	}

	// This is needed when setting the split size
	tabWidget->setCurrentIndex ( TAB_GRAPH );

	// Make the graph split area appear near the centre
	QList<int> ss = graphSplit->sizes ();
	tabWidget->hide ();
	ss.replace ( 0, ss.value ( 1 ) );
	graphSplit->setSizes ( ss );

	viewMain->setFocus ();

	// This is needed to stop visual corruption of the graph area widgets
	tabWidget->setCurrentIndex ( TAB_VIEW );

	if (	! arg_filename ||
		projectLoad ( arg_filename )
		)
	{
		projectClearAll ();
	}
}

MainWindow :: ~MainWindow ()
{
	if ( isMaximized () )
	{
		prefs_set_int ( GUI_INIFILE_MAIN_WINDOW"_state", 0 );
	}
	else
	{
		prefs_set_int ( GUI_INIFILE_MAIN_WINDOW"_state", 1 );

		prefs_set_int ( PREFS_WINDOW_X, geometry().x () );
		prefs_set_int ( PREFS_WINDOW_Y, geometry().y () );
		prefs_set_int ( PREFS_WINDOW_W, geometry().width () );
		prefs_set_int ( PREFS_WINDOW_H, geometry().height () );
	}

	prefs_save ();
	prefs_close ();

	cui_font_destroy ( render.font );

	cui_clip_free ( cedClipboard );
	cui_file_free ( cedFile );
}

void MainWindow :: closeEvent (
	QCloseEvent	* const	event
	)
{
	if ( isEnabled () == false )
	{
		// Main window is currently disabled so ignore all requests
		event->ignore ();

		return;
	}

	if ( okToLoseChanges () )
	{
		// No changes, or user happy to lose them
		event->accept ();
	}
	else
	{
		// Changes have occured, user not consenting to lose them
		event->ignore ();
	}
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	char	const *	prefs_filename = NULL;
	char	const *	input_filename = NULL;


	be_cline ( argc, argv, &prefs_filename, &input_filename );

	// I don't want Qt snooping or changing my command line.
	int		dummy_argc	= 1;
	char		dummy_str[1]	= { 0 },
			* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	MainWindow	window ( prefs_filename, input_filename );


	return app.exec ();
}

