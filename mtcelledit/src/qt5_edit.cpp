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

#include "qt5.h"



void MainWindow::press_EditUndo ()
{
	int		res;


	res = cui_book_undo_step ( cedFile->cubook );

	// This is needed in case we remove the last sheet
	projectSetSheet ();
	projectReportUpdates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return;
	}

	updateSheetSelector ();
	updateViewConfig ();
	updateChangesChores ( 1, 0 );	// Update menus/titles
}

void MainWindow::press_EditRedo ()
{
	int		res;


	res = cui_book_redo_step ( cedFile->cubook );

	// This is needed in case we remove the last sheet
	projectSetSheet ();
	projectReportUpdates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return;
	}

	updateSheetSelector ();
	updateViewConfig ();
	updateChangesChores ( 1, 0 );	// Update menus/titles
}

void MainWindow::press_EditFixYears ()
{
	int const res = be_fix_years ( cedFile, mprefs.year_start_2d );

	switch ( res )
	{
	case 1:
		QMessageBox::critical ( this, "Error",
			"Unable to fix years." );
		break;

	case 2:
		QMessageBox::critical ( this, "Error",
			"Unexpected error in system date." );
		return;
	}

	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL	||
		res == CUI_ERROR_LOCKED_SHEET	||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;			// Nothing changed
	}

	updateChangesChores ( 1, 0 );
}

void MainWindow::press_EditSelectAll ()
{
	setCursorRange ( 1, 1, MAX (1,sheetRows), MAX(1,sheetCols), 0, 0, 0 );
}

void MainWindow::press_OptionsEditCell ()
{
	editCelltext->setFocus ( Qt::OtherFocusReason );
}

void MainWindow::quicksumChanged ( int ARG_UNUSED ( i ) )
{
	updateQuicksumLabel ();
}

void MainWindow::createQuicksum ()
{
	connect ( buttonQuicksum, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( quicksumChanged ( int ) ) );

	buttonQuicksum->addItem ( "None" );
	buttonQuicksum->addItem ( "Sum" );
	buttonQuicksum->addItem ( "Min" );
	buttonQuicksum->addItem ( "Max" );
	buttonQuicksum->addItem ( "Max - Min" );
	buttonQuicksum->addItem ( "Average" );
	buttonQuicksum->addItem ( "Median" );
	buttonQuicksum->addItem ( "Count" );
	buttonQuicksum->addItem ( "Counta" );
}

