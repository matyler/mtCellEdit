/*
	Copyright (C) 2013-2015 Mark Tyler

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



tableAnalysis :: tableAnalysis (
	CedSheet	* const	sheetData,
	char	const	* const	pathAnalysed,
	MainWindow	* const	mainwin,
	QVBoxLayout	* const	layout
	)
	:
	sortDirection	( Qt::DescendingOrder ),
	sortColumn	( RAFT_COL_BYTES ),
	sheetRowTotal	( 0 ),
	path		( NULL ),
	sheet		( sheetData ),
	mainWindow	( mainwin )
{
	int		rowtot,
			i,
			width;
	QTableWidgetItem * twItem;
	QStringList	columnLabels;
	QLineEdit	* pathEdit;


	if ( pathAnalysed )
	{
		path = strdup ( pathAnalysed );
	}

	if ( ! path )
	{
		fprintf ( stderr, "Error (tableAnalysis): path is NULL\n" );
	}

	ced_sheet_get_geometry ( sheet, &rowtot, NULL );
	sheetRowTotal = rowtot;

	setSelectionMode ( QAbstractItemView::SingleSelection );
	setSelectionBehavior ( QAbstractItemView::SelectRows );
	setEditTriggers ( QAbstractItemView::NoEditTriggers );
	setColumnCount ( RAFT_COL_TOTAL - 1 );

	columnLabels
		<< tr ( "Name" )
		<< tr ( "Files" )
		<< tr ( "%" )
		<< tr ( "Bytes" )
		<< tr ( "MB" )
		<< tr ( "%" )
		<< tr ( "Subdirs" )
		<< tr ( "Other" )
		;
	setHorizontalHeaderLabels ( columnLabels );

	twItem = horizontalHeaderItem ( 0 );
	if ( twItem )
	{
		twItem->setTextAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
	}

	for ( i = 1; i < (RAFT_COL_TOTAL - 1); i++ )
	{
		twItem = horizontalHeaderItem ( i );
		if ( twItem )
		{
			twItem->setTextAlignment ( Qt::AlignRight |
				Qt::AlignVCenter );
		}
	}

	horizontalHeader ()->setSortIndicatorShown ( true );
	sortTable ( sortColumn, sortDirection );

	verticalHeader ()->hide ();
	verticalHeader ()->setDefaultSectionSize (
		verticalHeader ()->fontMetrics ().height () + 4 );

	setShowGrid ( false );

	connect ( horizontalHeader (), SIGNAL ( sectionClicked ( int ) ),
		this, SLOT ( tableHeaderClick ( int ) ) );

	connect ( this, SIGNAL ( cellActivated ( int, int ) ),
		this, SLOT ( tableCellActivated ( int, int ) ) );

	horizontalHeader ()->resizeSections ( QHeaderView::ResizeToContents );

	// Add some padding to columns
	for ( i = 0; i < (RAFT_COL_TOTAL - 1); i++ )
	{
		width = horizontalHeader ()->sectionSize ( i );
		horizontalHeader ()->resizeSection ( i, width + 8 );
	}

	pathEdit = new QLineEdit ( mtQEX :: qstringFromC ( pathAnalysed ) );
	pathEdit->setReadOnly ( true );

	layout->addWidget ( pathEdit );
	layout->addWidget ( this );
}

tableAnalysis :: ~tableAnalysis ()
{
	free ( path );
	path = NULL;

	ced_sheet_destroy ( sheet );
	sheet = NULL;
}

void tableAnalysis :: copyToClipboard ()
{
	char		* txt;


	txt = raft_get_clipboard ( sheet );
	if ( txt )
	{
		QApplication :: clipboard ()->setText ( txt );

		free ( txt );
	}
}

void tableAnalysis :: sortTable (
	int		column,
	Qt::SortOrder	direction
	)
{
	int		cols[] = { 1, 0 },
			r,
			c;
	char		* csp;
	CedCell		* cell;
	QTableWidgetItem * twItem;


	if ( column < 1 )
	{
		column = -column;

		if ( column == sortColumn )
		{
			// Toggle the current sort direction
			if ( sortDirection == Qt::DescendingOrder )
			{
				direction = Qt::AscendingOrder;
			}
			else
			{
				direction = Qt::DescendingOrder;
			}
		}
		else
		{
			direction = Qt::DescendingOrder;
		}
	}

	horizontalHeader ()->setSortIndicator ( (column - 1), direction );

	sortColumn = column;
	sortDirection = direction;

	r = 0;
	if ( sortDirection == Qt::DescendingOrder )
	{
		r = CED_SORT_MODE_DESCENDING;
	}

	cols[0] = sortColumn;
	ced_sheet_sort_rows ( sheet, 1, 0, cols, r, NULL );

	setCurrentItem ( NULL );	// Stops double selection of 0,0
	clearContents ();
	setRowCount ( sheetRowTotal );

	for ( r = 1; r <= sheetRowTotal; r++ )
	{
		for ( c = 1; c < RAFT_COL_TOTAL; c++ )
		{
			cell = ced_sheet_get_cell ( sheet, r, c );
			if ( ! cell )
			{
				// Should never happen
				continue;
			}

			csp = ced_cell_create_output ( cell, NULL );
			if ( ! csp )
			{
				continue;
			}

			twItem = new QTableWidgetItem;
			twItem->setText ( mtQEX :: qstringFromC ( csp ) );
			free ( csp );

			if ( c > 1 )
			{
				twItem->setTextAlignment ( Qt::AlignRight |
					Qt::AlignVCenter );
			}

			setItem ( r - 1, c - 1, twItem );
		}
	}

	setCurrentCell ( 0, 0 );
}

void tableAnalysis :: tableHeaderClick (
	int	const	index
	)
{
	sortTable ( -(index + 1), Qt::DescendingOrder );
}

void tableAnalysis :: tableCellActivated (
	int	const	row,
	int	const	ARG_UNUSED ( column )
	)
{
	char		* new_path;


	new_path = raft_path_merge ( path, sheet, row + 1 );
	if ( new_path )
	{
		mainWindow->doAnalysis ( new_path );
		free ( new_path );
	}
}

