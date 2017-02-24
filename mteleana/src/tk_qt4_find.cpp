/*
	Copyright (C) 2014-2016 Mark Tyler

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



static void find_cb (
	int		const	seat_id,
	char	const * const	seat_text,
	char	const * const	cell_type,
	char	const * const	cell_text,
	void	* ARG_UNUSED (	user )
	)
{
	mainwindow->addFindRow ( seat_text, cell_type, cell_text, seat_id );
}

void MainWindow::addFindRow (
	char	const * const	seat_text,
	char	const * const	cell_type,
	char	const * const	cell_text,
	int		const	seat_id
	)
{
	int		row = findTable->rowCount ();
	QTableWidgetItem * twItem;


	findTable->setRowCount ( row + 1 );

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( seat_text ) );
	findTable->setItem ( row, 0, twItem );
	twItem->setData ( Qt::UserRole, QVariant::fromValue (
		(void *)(intptr_t)seat_id ) );

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( cell_type ) );
	findTable->setItem ( row, 1, twItem );

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( cell_text ) );
	findTable->setItem ( row, 2, twItem );
}

void MainWindow::pressEditFind ()
{
	editFindText->setFocus ();
	editFindText->selectAll ();
	tabWidget->setCurrentIndex ( UI_TAB_FIND );
}

void MainWindow::pressFindButton ()
{
	mainwindow->setEnabled ( false );
	findTable->clearContents ();
	findTable->setRowCount ( 0 );

	election->findText ( editFindText->text ().toUtf8 ().data (), find_cb,
		NULL );

	// This hack ensures that all columns are right width (not just visible)
	findTable->setVisible ( false );
	findTable->resizeColumnsToContents ();
	findTable->setVisible ( true );

	findTable->setCurrentCell ( 0, 0 );
	mainwindow->setEnabled ( true );

	findTable->setFocus ();
}

void MainWindow::findCellChanged (
	int	const	currentRow,
	int	const	ARG_UNUSED ( currentColumn ),
	int	const	previousRow,
	int	const	ARG_UNUSED ( previousColumn )
	)
{
	if ( currentRow == previousRow )
	{
		return;
	}


	QTableWidgetItem * twItem;
	int		i = -1;


	twItem = findTable->item ( currentRow, 0 );
	if ( twItem )
	{
		i = (int)(intptr_t)twItem->data( Qt::UserRole ).value<void *>();
	}
	else
	{
		return;
	}

	setupSeatTable ( i );
	moveMapFocus ( i );
}

