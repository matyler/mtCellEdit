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

#include "icon.xpm"



#define DEFAULTzoomCartogram	10
#define DEFAULTzoomDiagram	2
#define DATA_DIR		DATA_INSTALL "/" DATA_NAME "/"



MainWindow	* mainwindow;



MainWindow::MainWindow ()
	:
	menuBar		(),
	comboYear	(),
	labelSeat	(),
	tableSeat	(),
	tableSummary	(),

	tabWidget	(),

	comboMapMode	(),
	sliderMapZoom	(),
	polymapScrollArea(),
	mapCanvas	(),

	cartogramWidget	(),

	diagramWidget	(),
	comboDiagramLeft(),
	comboDiagramRight(),

	editFindText	(),
	findTable	(),

	zoomMap		( 1.0 ),
	index		(),
	election	()
{
	QWidget		* widget, * tab, * scrollAreaContent;
	QVBoxLayout	* layv, * vbox;
	QSplitter	* split;
	QSplitter	* vsplit;
	QHBoxLayout	* row;
	QGroupBox	* groupBox;
	QScrollArea	* scrollArea;
	QSlider		* slider;
	QGridLayout	* grid;
	QStringList	columnLabels;


	mainwindow = this;


	// Preparation

	mtPrefTable	const	prefs_table[] = {

{ PREFS_LAST_MAP_FILE_NAME, MTKIT_PREF_TYPE_FILE, NULL, NULL, NULL, 0, NULL, NULL },
{ PREFS_LAST_MAP_FILE_FORMAT, MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },

{ PREFS_LAST_YEAR, MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },

{ PREFS_WINDOW_X, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ PREFS_WINDOW_Y, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ PREFS_WINDOW_W, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
{ PREFS_WINDOW_H, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },

{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
		};

	prefs.addTable ( prefs_table );
	prefs.initWindowPrefs ();
	prefs.load ( NULL, BIN_NAME );

	ced_init ();


	// Widgets

	setWindowTitle ( VERSION );
	setWindowIcon ( QPixmap ( icon_xpm ) );

	layv = new QVBoxLayout ( this );
	layv->setMargin ( 0 );

	split = new QSplitter ( Qt::Horizontal );
	layv->addWidget ( split );

	// Left split area

	widget = new QWidget;
	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	widget->setLayout ( layv );
	split->addWidget ( widget );

	row = new QHBoxLayout;
	row->setMargin ( 0 );
	row->setSpacing ( 0 );
	layv->addLayout ( row );

	menuBar = new QMenuBar;
	row->addWidget ( menuBar );

	scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable ( true );
	layv->addWidget ( scrollArea );

	scrollAreaContent = new QWidget;
	layv = new QVBoxLayout ( scrollAreaContent );

	groupBox = new QGroupBox ( "Settings" );
	layv->addWidget ( groupBox );

	grid = new QGridLayout;
	groupBox->setLayout ( grid );
	grid->setColumnStretch ( 0, 0 );
	grid->setColumnStretch ( 1, 1 );

	grid->addWidget ( new QLabel ( "Year" ), 1, 0 );
	grid->addWidget ( new QLabel ( "Map Mode" ), 2, 0 );
	grid->addWidget ( new QLabel ( "Party A" ), 3, 0 );
	grid->addWidget ( new QLabel ( "Party B" ), 4, 0 );

	comboYear = new QComboBox;
	grid->addWidget ( comboYear, 1, 1 );
	connect ( comboYear, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( yearChanged ( int ) ) );

	comboMapMode = new QComboBox;
	// NOTE: Must be in same order as MAP_MODE_*
	comboMapMode->addItem ( "Winning Party" );
	comboMapMode->addItem ( "Party A Placing" );
	comboMapMode->addItem ( "Party A Vote Share" );
	comboMapMode->addItem ( "Winning Margin" );
	comboMapMode->addItem ( "Turnout" );
	grid->addWidget ( comboMapMode, 2, 1 );
	connect ( comboMapMode, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( mapModeChanged ( int ) ) );

	row = new QHBoxLayout;
	row->setMargin ( 0 );
	layv->addLayout ( row );

	comboDiagramLeft = new QComboBox;
	comboDiagramLeft->setSizeAdjustPolicy (
		QComboBox::AdjustToMinimumContentsLength );
	comboDiagramLeft->addItem ( "Labour" );
	grid->addWidget ( comboDiagramLeft, 3, 1 );
	connect ( comboDiagramLeft, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( diagramLeftChanged ( int ) ) );

	comboDiagramRight = new QComboBox;
	comboDiagramRight->setSizeAdjustPolicy (
		QComboBox::AdjustToMinimumContentsLength );
	comboDiagramRight->addItem ( "Conservative" );
	grid->addWidget ( comboDiagramRight, 4, 1 );
	connect ( comboDiagramRight, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( diagramRightChanged ( int ) ) );

	vsplit = new QSplitter ( Qt::Vertical );
	layv->addWidget ( vsplit );

	groupBox = new QGroupBox ( "Results" );
	vsplit->addWidget ( groupBox );

	vbox = new QVBoxLayout;
	groupBox->setLayout ( vbox );

	tableSummary = new QTableWidget;
	vbox->addWidget ( tableSummary );

	tableSummary->setSelectionMode ( QAbstractItemView::SingleSelection );
	tableSummary->setSelectionBehavior ( QAbstractItemView::SelectRows );
	tableSummary->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	tableSummary->setShowGrid ( false );
	tableSummary->verticalHeader ()->hide ();

	groupBox = new QGroupBox ( "Seat" );
	groupBox->setSizePolicy ( QSizePolicy ( QSizePolicy::Preferred,
		QSizePolicy::Expanding ) );
	vsplit->addWidget ( groupBox );

	vbox = new QVBoxLayout;
	groupBox->setLayout ( vbox );

	labelSeat = new QLabel;
	vbox->addWidget ( labelSeat );

	tableSeat = new QTableWidget;
	vbox->addWidget ( tableSeat );

	tableSeat->setSelectionMode ( QAbstractItemView::SingleSelection );
	tableSeat->setSelectionBehavior ( QAbstractItemView::SelectRows );
	tableSeat->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	tableSeat->setShowGrid ( false );
	tableSeat->verticalHeader ()->hide ();

	scrollArea->setWidget ( scrollAreaContent );

	// Right split area

	tabWidget = new QTabWidget;
	split->addWidget ( tabWidget );

	tab = new QWidget;
	tabWidget->addTab ( tab, "Map" );

	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	tab->setLayout ( layv );

	slider = new QSlider ( Qt::Horizontal );
	layv->addWidget ( slider );

	slider->setRange ( 30, 2500 );
	slider->setValue ( (int)( 100 * zoomMap ) );
	sliderMapZoom = slider;

	connect ( slider, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( zoomMapChanged ( int ) ) );

	polymapScrollArea = new MapView;
	layv->addWidget ( polymapScrollArea );

	mapCanvas = new MapCanvas;
	polymapScrollArea->setWidget ( mapCanvas );

	// Cartogram Tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "Cartogram" );

	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	tab->setLayout ( layv );

	slider = new QSlider ( Qt::Horizontal );
	layv->addWidget ( slider );

	slider->setRange ( 1, 32 );
	slider->setValue ( DEFAULTzoomCartogram );

	connect ( slider, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( zoomCartogramChanged ( int ) ) );

	cartogramWidget = new mtQEX::Image;
	cartogramWidget->setImage ( mtPixy::image_create ( mtPixy::Image::RGB,
		CARTOGRAM_WIDTH, CARTOGRAM_HEIGHT ) );
	cartogramWidget->setZoom ( DEFAULTzoomCartogram );
	layv->addWidget ( cartogramWidget );

	// Diagram Tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "Diagram" );

	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	tab->setLayout ( layv );

	slider = new QSlider ( Qt::Horizontal );
	layv->addWidget ( slider );

	slider->setRange ( 1, 16 );
	slider->setValue ( DEFAULTzoomDiagram );

	connect ( slider, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( zoomDiagramChanged ( int ) ) );

	diagramWidget = new mtQEX::Image;
	diagramWidget->setZoom ( DEFAULTzoomDiagram );
	layv->addWidget ( diagramWidget );

	// Find Tab

	tab = new QWidget;
	tabWidget->addTab ( tab, "Find" );

	layv = new QVBoxLayout;
	layv->setMargin ( 0 );
	layv->setSpacing ( 0 );
	tab->setLayout ( layv );

	row = new QHBoxLayout;
	row->setMargin ( 0 );
	layv->addLayout ( row );

	editFindText = new QLineEdit;
	row->addWidget ( editFindText );

	connect ( editFindText, SIGNAL ( returnPressed () ), this,
		SLOT ( pressFindButton () ) );

	QPushButton * button = new QPushButton ( "Find" );
	row->addWidget ( button );

	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressFindButton () ) );

	findTable = new QTableWidget;
	layv->addWidget ( findTable );

	findTable->setSelectionMode ( QAbstractItemView::SingleSelection );
	findTable->setSelectionBehavior ( QAbstractItemView::SelectRows );
	findTable->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	findTable->setColumnCount ( 3 );
	findTable->setShowGrid ( false );
	findTable->verticalHeader ()->setDefaultSectionSize (
		findTable->verticalHeader ()->fontMetrics ().height () + 4 );
	findTable->verticalHeader ()->QEX_RESIZEMODE ( QHeaderView::Fixed );
	findTable->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
	findTable->horizontalHeader ()->setStretchLastSection ( true );

	connect ( findTable, SIGNAL( currentCellChanged ( int, int, int, int )),
		this, SLOT ( findCellChanged ( int, int, int, int ) ) );

	columnLabels = ( QStringList () << "Seat" << "Column" << "Content" );

	findTable->setHorizontalHeaderLabels ( columnLabels );
	findTable->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );

	// Final details

	createMenus ();

	setMinimumSize ( 160, 160 );

	setGeometry ( prefs.getInt ( PREFS_WINDOW_X ),
		prefs.getInt ( PREFS_WINDOW_Y ),
		prefs.getInt ( PREFS_WINDOW_W ),
		prefs.getInt ( PREFS_WINDOW_H ) );

	projectLoad ( DATA_DIR "eleana_index.tsv.zip" );

	// Make split sensible
	QList<int> ss = split->sizes ();
	ss.replace ( 0, 1 );
	ss.replace ( 1, 1 );
	split->setSizes ( ss );

	show ();

	resetMapZoomPos ();
}

MainWindow::~MainWindow ()
{
	prefs.set ( PREFS_WINDOW_X, geometry().x () );
	prefs.set ( PREFS_WINDOW_Y, geometry().y () );
	prefs.set ( PREFS_WINDOW_W, geometry().width () );
	prefs.set ( PREFS_WINDOW_H, geometry().height () );

	prefs.save ();

	purgeDataLoaded ();
	purgeIndexLoaded ();
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	be_cline ( argc, argv );


	// I don't want Qt snooping or changing my command line.
	int		dummy_argc	= 1;
	char		dummy_str[1]	= { 0 },
			* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	MainWindow	window;

	return app.exec ();
}

