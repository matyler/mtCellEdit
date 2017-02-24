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



int MainWindow::projectLoad (
	char	const * const	filename
	)
{
	int		ii;
	char	const	* ctxt;
	eleanaIndex	* new_index;


	if ( ! filename || ! filename[0] )
	{
		return 1;
	}

	new_index = new eleanaIndex;
	if ( ! new_index )
	{
		return 1;
	}

	if ( new_index->load ( filename ) )
	{
		QString		tmp = mtQEX::qstringFromC ( filename );


		QMessageBox::critical ( this, "Error",
			QString( "Unable to load index file %1.").arg ( tmp ) );

		delete new_index;

		return 1;
	}

	// Index was loaded correctly so purge all other data
	purgeIndexLoaded ();
	purgeDataLoaded ();

	index = new_index;

	// Remove the old years in the picker
	comboYear->clear ();

	if ( 0 != prefs.getInt ( PREFS_LAST_YEAR ) )
	{
		comboYear->blockSignals ( true );
	}

	for ( ii = 0; ; ii++ )
	{
		ctxt = index->getText ( ii, ELEANA_INDEX_COL_YEAR );
		if ( ! ctxt )
		{
			break;
		}

		comboYear->addItem ( mtQEX::qstringFromC ( ctxt ) );
	}

	if ( 0 != prefs.getInt ( PREFS_LAST_YEAR ) )
	{
		comboYear->blockSignals ( false );

		// Default to the previous year that was used
		comboYear->setCurrentIndex ( prefs.getInt ( PREFS_LAST_YEAR ));
	}

	resetMapZoomPos ();

	return 0;			// Success
}

void MainWindow::purgeIndexLoaded ()
{
	delete ( index );
	index = NULL;
}

void MainWindow::purgeDataLoaded ()
{
	delete ( election );
	election = NULL;
}

void MainWindow::clearSeatTable ()
{
	labelSeat->setText ( "Right click map to select a seat" );

	tableSeat->clear ();
	tableSeat->setRowCount ( 0 );
	tableSeat->setColumnCount ( 4 );
	tableSeat->setMinimumHeight ( 0 );

	QStringList columnLabels = ( QStringList () << "Candidate" << "Party"
		<< "Votes" << "%" );

	tableSeat->setHorizontalHeaderLabels ( columnLabels );
	tableSeat->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
}

void mtEleana::tableSetupDetails (
	QTableWidget	* const table
	)
{
	int colH = table->verticalHeader ()->fontMetrics ().height () + 4;


	table->verticalHeader ()->setDefaultSectionSize ( colH );
	table->verticalHeader ()->QEX_RESIZEMODE ( QHeaderView::Fixed );

	table->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
}

void MainWindow::setupSeatTable (
	int	const	row
	)
{
	int		i;
	int		candidates;
	int		votes = 0;
	CedCell		* cell;
	QTableWidgetItem * twItem;
	CedSheet	* const sheet = election->getResults ();


	clearSeatTable ();

	// Count candidates and the total votes
	for ( candidates = 0; ; candidates ++ )
	{
		cell = ced_sheet_get_cell ( sheet, row + candidates,
			FULL_COL_VOTES );

		if ( ! cell )
		{
			break;
		}

		votes += (int)cell->value;
	}


	QString	st;

	cell = ced_sheet_get_cell ( sheet, row, FULL_COL_SEAT_NAME );
	if ( cell && cell->text )
	{
		st += cell->text;
	}

	cell = ced_sheet_get_cell ( sheet, row, FULL_COL_COUNTY );
	if ( cell && cell->text )
	{
		st = st + " (" + cell->text + ")";
	}

	labelSeat->setText ( st );

	tableSeat->setRowCount ( candidates + 3 );

	int textalign = Qt::AlignRight | Qt::AlignVCenter;

	tableSeat->setItem ( candidates + 1, 1,
		new QTableWidgetItem ( "Turnout" ) );

	twItem = new QTableWidgetItem ( QString ( "%L1" ).arg ( votes ) );
	twItem->setTextAlignment ( textalign );
	tableSeat->setItem ( candidates + 1, 2, twItem );

	tableSeat->setItem ( candidates + 2, 1,
		new QTableWidgetItem ( "Electorate" ) );
	cell = ced_sheet_get_cell ( sheet, row, FULL_COL_ELECTORATE );

	if ( cell && cell->text )
	{
		twItem = new QTableWidgetItem ( QString ( "%L1" ).arg (
			cell->value ) );
		twItem->setTextAlignment ( textalign );
		tableSeat->setItem ( candidates + 2, 2, twItem );

		if ( cell->value != 0 )
		{
			st = QString ( "%1" ).arg ( 100 * ( (double)votes ) /
				cell->value, 0, 'f', 2 );
			twItem = new QTableWidgetItem ( st );

			twItem->setTextAlignment ( textalign );
			tableSeat->setItem ( candidates + 1, 3, twItem );
		}
	}

	// Populate table with candidates
	for ( i = 0; i < candidates; i++ )
	{
		cell = ced_sheet_get_cell ( sheet, row + i,
			FULL_COL_MP_NAME );

		if ( cell )
		{
			tableSeat->setItem ( i, 0, new QTableWidgetItem (
				mtQEX::qstringFromC ( cell->text ) ) );
		}

		cell = ced_sheet_get_cell ( sheet, row + i,
			FULL_COL_PARTY );

		if ( cell )
		{
			tableSeat->setItem ( i, 1, new QTableWidgetItem (
				mtQEX::qstringFromC ( cell->text ) ) );
		}

		cell = ced_sheet_get_cell ( sheet, row + i,
			FULL_COL_VOTES );

		if ( cell )
		{
			twItem = new QTableWidgetItem ( QString ( "%L1" ).arg (
				cell->value ) );

			twItem->setTextAlignment ( textalign );
			tableSeat->setItem ( i, 2, twItem );

			st = QString ( "%1" ).arg ( 100 * cell->value /
				( (double)votes ), 0, 'f', 2 );
			twItem = new QTableWidgetItem ( st );

			twItem->setTextAlignment ( textalign );
			tableSeat->setItem ( i, 3, twItem );
		}
	}

	mtEleana::tableSetupDetails ( tableSeat );
}

