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



static int		sort_table [ MAX_SORT ][ SORT_TABLE_TOTAL ],
			sort_axis,
			sort_axis_total,
			sort_row,
			sort_col,
			sort_rowtot,
			sort_coltot,
			sort_origin
			;


SortDialog::SortDialog (
	int		const	axis,
	QWidget		* const	par
	)
	:
	QDialog		( par )
{
	CedSheet	* const	sheet = mainwindow->projectGetSheet ();
	int		i, new_axis_tot, new_sort_origin;
	QStringList	tableTitles;
	QPushButton	* button;


	if ( ! sheet || axis < 0 || axis > 1 )
	{
		return;
	}

	sort_row = MIN ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	sort_col = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	sort_rowtot = MAX ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 ) -
		sort_row + 1;
	sort_coltot = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 ) -
		sort_col + 1;

	if ( axis == 1 )		// Sort columns
	{
		if ( sort_coltot < 2 )
		{
			return;		// Nothing to do
		}

		new_axis_tot = MIN ( sort_rowtot, MAX_SORT );
		setWindowTitle ( "Sort Columns" );
		tableTitles << "Row";
		new_sort_origin = sort_row;
	}
	else				// Sort rows
	{
		if ( sort_rowtot < 2 )
		{
			return;		// Nothing to do
		}

		new_axis_tot = MIN ( sort_coltot, MAX_SORT );
		setWindowTitle ( "Sort Rows" );
		tableTitles << "Column";
		new_sort_origin = sort_col;
	}

	if (	sort_axis_total	!= new_axis_tot	||
		sort_axis != axis		||
		sort_origin != new_sort_origin
		)
	{
		/*
		Only clear out old data if user changes anything.
		Otherwise its a re-sort of the same area with the same settings.
		*/

		sort_axis = axis;
		sort_axis_total = new_axis_tot;
		sort_origin = new_sort_origin;

		for ( i = 0; i < MAX_SORT; i++ )
		{
			sort_table[i][SORT_TABLE_ROWCOL] = i + sort_origin;
			sort_table[i][SORT_TABLE_DIRECTION] = 0;
			sort_table[i][SORT_TABLE_CASE_SENSITIVE] = 0;
		}
	}


	QVBoxLayout * layv = new QVBoxLayout;
	setLayout ( layv );

	QHBoxLayout * topRow = new QHBoxLayout;
	layv->addLayout ( topRow );

	tableTitles << "Direction" << "Case Sensitive";

	tableWidget = new QTableWidget ( sort_axis_total, 3 );
	topRow->addWidget ( tableWidget );

	tableWidget->setSelectionMode ( QAbstractItemView::SingleSelection );
	tableWidget->setSelectionBehavior ( QAbstractItemView::SelectRows );
	tableWidget->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	tableWidget->setHorizontalHeaderLabels ( tableTitles );
	tableWidget->horizontalHeader ()->QEX_RESIZEMODE (
		QHeaderView::ResizeToContents );
	tableWidget->verticalHeader ()->setDefaultSectionSize (
		tableWidget->verticalHeader ()->fontMetrics ().height () + 4 );
	tableWidget->verticalHeader ()->hide ();
	tableWidget->setShowGrid ( false );

	connect ( tableWidget, SIGNAL(currentCellChanged( int, int, int, int )),
		this, SLOT ( tableRowClick ( int, int, int, int ) ) );

	for ( i = 0; i < sort_axis_total; i++ )
	{
		setRowValues ( i );
	}

	QVBoxLayout * vlay = new QVBoxLayout;
	topRow->addLayout ( vlay );

	spinBoxWidget = new QSpinBox;
	vlay->addWidget ( spinBoxWidget );
	spinBoxWidget->setMinimum ( 1 );
	spinBoxWidget->setMaximum ( CED_MAX_COLUMN );
	connect ( spinBoxWidget, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( spinChanged ( int ) ) );

	checkDirection = new QCheckBox ( "Direction" );
	vlay->addWidget ( checkDirection );
	connect ( checkDirection, SIGNAL ( stateChanged ( int ) ),
		this, SLOT ( directionChanged ( int ) ) );

	checkCase = new QCheckBox ( "Case Sensitive" );
	vlay->addWidget ( checkCase );
	connect ( checkCase, SIGNAL ( stateChanged ( int ) ),
		this, SLOT ( caseChanged ( int ) ) );

	button = new QPushButton ( "Move Up" );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressMoveUp () ) );
	vlay->addWidget ( button );

	button = new QPushButton ( "Move Down" );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressMoveDown () ) );
	vlay->addWidget ( button );

	QHBoxLayout * hRow = new QHBoxLayout;
	layv->addLayout ( hRow );

	button = new QPushButton ( "&Close" );
	connect ( button, SIGNAL ( clicked () ), this, SLOT ( pressClose () ) );
	hRow->addWidget ( button );

	button = new QPushButton ( "&Sort" );
	button->setDefault ( true );
	connect ( button, SIGNAL ( clicked () ), this, SLOT ( pressSort () ) );
	hRow->addWidget ( button );

	tableWidget->setCurrentCell ( 0, 0 );

	// Adjust so that no resizing is needed to see the whole table.
	tableWidget->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	tableWidget->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

	i = tableWidget->verticalHeader ()->width () +
		tableWidget->columnWidth ( 0 ) +
		tableWidget->columnWidth ( 1 ) +
		tableWidget->columnWidth ( 2 ) + 4;
	tableWidget->setMinimumWidth ( i );

	i = tableWidget->horizontalHeader()->height () +
		tableWidget->rowHeight ( 0 ) * tableWidget->rowCount () + 4;
	tableWidget->setMinimumHeight ( i );

	exec ();
}

int SortDialog::getTableRow ()
{
	int	const	i = tableWidget->currentRow ();


	if ( i < 0 || i >= sort_axis_total )
	{
		return 0;
	}

	return i;
}

void SortDialog::tableRowClick (
	int	const	currentRow,
	int	const	ARG_UNUSED ( currentColumn ),
	int	const	previousRow,
	int	const	ARG_UNUSED ( previousColumn )
	)
{
	if (	currentRow == previousRow	||
		currentRow < 0			||
		currentRow >= sort_axis_total
		)
	{
		return;
	}

	spinBoxWidget->setValue (
		sort_table [ currentRow ][ SORT_TABLE_ROWCOL ] );

	checkDirection->setChecked (
		sort_table [ currentRow ][ SORT_TABLE_DIRECTION ] ?
			true : false );

	checkCase->setChecked (
		sort_table [ currentRow ][ SORT_TABLE_CASE_SENSITIVE ] ?
			true : false );

	setRowValues ( currentRow );
}

void SortDialog::spinChanged (
	int	const	i
	)
{
	int	const	r = getTableRow ();


	sort_table [ r ][ SORT_TABLE_ROWCOL ] = i;

	setRowValues ( r );
}

void SortDialog::directionChanged (
	int	const	ARG_UNUSED ( i )
	)
{
	int	const	r = getTableRow ();


	sort_table [ r ][ SORT_TABLE_DIRECTION ] =
		checkDirection->isChecked () ? 1 : 0;

	setRowValues ( r );
}

void SortDialog::caseChanged (
	int	const	ARG_UNUSED ( i )
	)
{
	int	const	r = getTableRow ();


	sort_table [ r ][ SORT_TABLE_CASE_SENSITIVE ] =
		checkCase->isChecked () ? 1 : 0;

	setRowValues ( r );
}

void SortDialog::setRowValues (
	int	const	row
	)
{
	QTableWidgetItem * twItem;


	if ( row < 0 || row >= MAX_SORT )
	{
		return;
	}

	twItem = new QTableWidgetItem;
	twItem->setText ( QString ( "%1" )
		.arg ( sort_table [ row ][ SORT_TABLE_ROWCOL ] ) );
	tableWidget->setItem ( row, SORT_TABLE_ROWCOL, twItem );

	twItem = new QTableWidgetItem;
	if ( sort_table [ row ][ SORT_TABLE_CASE_SENSITIVE ] )
	{
		twItem->setText ( "Yes" );
	}
	else
	{
		twItem->setText ( "No" );
	}
	tableWidget->setItem ( row, SORT_TABLE_CASE_SENSITIVE, twItem );

	twItem = new QTableWidgetItem;
	if ( sort_table [ row ][ SORT_TABLE_DIRECTION ] )
	{
		twItem->setText ( "9 -> 1" );
	}
	else
	{
		twItem->setText ( "1 -> 9" );
	}
	tableWidget->setItem ( row, SORT_TABLE_DIRECTION, twItem );
}

void SortDialog::pressClose ()
{
	close ();
}

void SortDialog::swapRows (
	int	const	delta
	)
{
	int	const	a = getTableRow ();
	int	const	b = a + delta;
	int		t;
	int		i;


	if ( b < 0 || b > (sort_axis_total - 1) )
	{
		return;
	}

	for ( i = 0; i < SORT_TABLE_TOTAL; i++ )
	{
		t = sort_table [ a ] [ i ];

		sort_table [ a ] [ i ] = sort_table [ b ] [ i ];
		sort_table [ b ] [ i ] = t;
	}

	setRowValues ( a );
	setRowValues ( b );

	tableWidget->setCurrentCell ( b, 0 );
}

void SortDialog::pressMoveDown ()
{
	swapRows ( 1 );
}

void SortDialog::pressMoveUp ()
{
	swapRows ( -1 );
}

void SortDialog::pressSort ()
{
	int		i,
			res,
			mode_list[MAX_SORT] = { 0 },
			rowscols[MAX_SORT] = { 0 };


	for ( i = 0; i < sort_axis_total; i++ )
	{
		rowscols [ i ] = sort_table [ i ][ SORT_TABLE_ROWCOL ];

		if ( sort_table [ i ][ SORT_TABLE_DIRECTION ] )
		{
			mode_list [ i ] |= CED_SORT_MODE_DESCENDING;
		}

		if ( sort_table [ i ][ SORT_TABLE_CASE_SENSITIVE ] )
		{
			mode_list [ i ] |= CED_SORT_MODE_CASE;
		}
	}

	if ( sort_axis == SORT_COLUMNS )
	{
		res = cui_sheet_sort_columns (
			mainwindow->projectGetCedFile ()->cubook,
			mainwindow->projectGetSheet (),
			sort_col, sort_coltot, rowscols, 0, mode_list );
	}
	else
	{
		res = cui_sheet_sort_rows (
			mainwindow->projectGetCedFile ()->cubook,
			mainwindow->projectGetSheet (),
			sort_row, sort_rowtot, rowscols, 0, mode_list );
	}

	mainwindow->projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	mainwindow->updateChangesChores ( 1, 0 );

	close ();
}

void MainWindow::pressColumnSort ()
{
	SortDialog	dialog ( SORT_COLUMNS, this );
}

void MainWindow::pressRowSort ()
{
	SortDialog	dialog ( SORT_ROWS, this );
}

