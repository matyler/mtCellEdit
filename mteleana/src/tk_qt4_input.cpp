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



void MainWindow::zoomCartogramChanged (
	int	const	zoom
	)
{
	cartogramWidget->setZoom ( zoom );
}

void MainWindow::zoomDiagramChanged (
	int	const	zoom
	)
{
	diagramWidget->setZoom ( zoom );
}

void MainWindow::zoomMapChanged (
	int	const	zoom
	)
{
	int		nv_h, nv_v, w, h;
	double		cx, cy, aw, ah, xx, yy;


	aw = polymapScrollArea->viewport ()->width ();
	ah = polymapScrollArea->viewport ()->height ();

	xx = polymapScrollArea->horizontalScrollBar ()->value ();
	yy = polymapScrollArea->verticalScrollBar ()->value ();

	cx = xx - aw + aw / 2;
	cy = yy - ah + ah / 2;
	cx /= (mapCanvas->width () - 2 * aw);
	cy /= (mapCanvas->height () - 2 * ah);

	zoomMap = ( (double)zoom ) / 100;

	if ( cx > 1 ) cx = 1;
	if ( cx < 0 ) cx = 0;
	if ( cy > 1 ) cy = 1;
	if ( cy < 0 ) cy = 0;

	w = (int)lrint ( zoomMap * POLYMAP_WIDTH );
	h = (int)lrint ( zoomMap * POLYMAP_HEIGHT );

	nv_h = (int)lrint ( cx * w - aw / 2 );
	nv_v = (int)lrint ( cy * h - ah / 2 );

	mapCanvas->setGeometry ( 0, 0, (int)(w + aw * 2), (int)(h + ah * 2) );

	polymapScrollArea->horizontalScrollBar ()->setValue ( (int)(nv_h + aw));
	polymapScrollArea->verticalScrollBar ()->setValue ( (int)( nv_v + ah ));
}

void MainWindow::yearChanged (
	int	const	val
	)
{
	if ( ! index )
	{
		return;
	}


	eleanaElection * new_election = new eleanaElection;


	if ( new_election->load ( index, val ) )
	{
		delete new_election;

		QMessageBox::critical ( this, "Error",
			"Unable to change year" );

		return;
	}

	purgeDataLoaded ();		// Remove old year's data
	election = new_election;

	prefs.set ( PREFS_LAST_YEAR, val );

	mapRedraw ();

	clearSeatTable ();

	findTable->clearContents ();
	findTable->setRowCount ( 0 );


	CedSheet	* sheetSum = election->createSummary ();
	int		r, c, rtot = 0, ctot = 0;
	CedCell		* cell;
	char		* csp;
	QTableWidgetItem * twItem;
	QStringList	columnLabels;


	ced_sheet_get_geometry ( sheetSum, &rtot, &ctot );

	tableSummary->setCurrentItem ( NULL );	// Stops double selection of 0,0
	tableSummary->clear ();
	tableSummary->setRowCount ( rtot );
	tableSummary->setColumnCount ( ctot );
	tableSummary->setMinimumHeight ( 0 );
	columnLabels
		<< "Party"
		<< "Votes"
		<< "%"
		<< "Seats"
		<< "Candidates"
		;
	tableSummary->setHorizontalHeaderLabels ( columnLabels );


	for ( r = 1; r <= rtot; r++ )
	{
		for ( c = 1; c <= ctot; c++ )
		{
			cell = ced_sheet_get_cell ( sheetSum, r, c );
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
			twItem->setText ( mtQEX::qstringFromC ( csp ) );
			free ( csp );

			if ( c > 1 )
			{
				twItem->setTextAlignment ( Qt::AlignRight |
					Qt::AlignVCenter );
			}

			tableSummary->setItem ( r - 1, c - 1, twItem );
		}
	}

	tableSummary->setCurrentCell ( 1, 1 );

	mtEleana::tableSetupDetails ( tableSummary );

	comboDiagramLeft->blockSignals ( true );
	comboDiagramRight->blockSignals ( true );


	QString		oldLeft = comboDiagramLeft->currentText ();
	QString		oldRight = comboDiagramRight->currentText ();


	comboDiagramLeft->clear ();
	comboDiagramRight->clear ();


	CedSheet	* sh = ced_sheet_copy_area ( sheetSum, 1, 1, 0, 1 );
	int		cols[] = { 1, 0 };


	ced_sheet_sort_rows ( sh, 4, 0, cols, CED_SORT_MODE_ASCENDING, NULL );

	for ( r = 4; r <= rtot; r++ )
	{
		cell = ced_sheet_get_cell ( sh, r, 1 );
		if ( ! cell || ! cell->text )
		{
			// Should never happen
			continue;
		}

		comboDiagramLeft->addItem( mtQEX::qstringFromC ( cell->text ));
		comboDiagramRight->addItem( mtQEX::qstringFromC ( cell->text ));
	}

	r = MAX ( 0, comboDiagramLeft->findText ( oldLeft ) );
	comboDiagramLeft->setCurrentIndex ( r );

	r = MAX ( 0, comboDiagramRight->findText ( oldRight ) );
	comboDiagramRight->setCurrentIndex ( r );

	ced_sheet_destroy ( sh );
	sh = NULL;

	comboDiagramLeft->blockSignals ( false );
	comboDiagramRight->blockSignals ( false );

	diagramRedraw ();
}

void MainWindow::diagramLeftChanged (
	int	const	ARG_UNUSED ( i )
	)
{
	diagramRedraw ();

	switch ( comboMapMode->currentIndex () )
	{
	case MAP_MODE_PARTY_PLACING:
	case MAP_MODE_PARTY_VOTE_SHARE:
		// Map & Cartogram need to be redrawn to this new setting
		mapRedraw ();
		break;
	}
}

void MainWindow::diagramRightChanged (
	int	const	ARG_UNUSED ( i )
	)
{
	diagramRedraw ();
}

void MainWindow::diagramRedraw ()
{
	diagramWidget->setImage ( election->createDiagram ( index,
		comboDiagramLeft->currentText().toUtf8 ().data (),
		comboDiagramRight->currentText().toUtf8 ().data () ) );
}

