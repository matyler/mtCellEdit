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



void MainWindow::press_FindWildcards ()
{
	if ( act_FindWildcards->isChecked () )
	{
		uprefs.set ( PREFS_FIND_WILDCARDS, 1 );
	}
	else
	{
		uprefs.set ( PREFS_FIND_WILDCARDS, 0 );
	}
}

void MainWindow::press_FindCase ()
{
	if ( act_FindCase->isChecked () )
	{
		uprefs.set ( PREFS_FIND_CASE_SENSITIVE, 1 );
	}
	else
	{
		uprefs.set ( PREFS_FIND_CASE_SENSITIVE, 0 );
	}
}

void MainWindow::press_FindValue ()
{
	if ( act_FindValue->isChecked () )
	{
		uprefs.set ( PREFS_FIND_VALUE, 1 );
	}
	else
	{
		uprefs.set ( PREFS_FIND_VALUE, 0 );
	}
}

void MainWindow::press_FindSheets ()
{
	if ( act_FindSheets->isChecked () )
	{
		uprefs.set ( PREFS_FIND_ALL_SHEETS, 1 );
	}
	else
	{
		uprefs.set ( PREFS_FIND_ALL_SHEETS, 0 );
	}
}



enum
{
	FIND_COL_SHEET,
	FIND_COL_ROW,
	FIND_COL_COLUMN,
	FIND_COL_CONTENT,

	FIND_COL_TOTAL
};



int MainWindow::addFindRow (
	CedSheet	* const	sheet,
	CedCell	const	* const	cell,
	int		const	r,
	int		const	c
	)
{
	int	const	row = findTable->rowCount ();
	char	const	* sheet_name = NULL;


	if ( ! sheet || ! cell || row >= FIND_MAX_MATCHES )
	{
		return 1;
	}

	findTable->setRowCount ( row + 1 );

	if ( sheet->book_tnode )
	{
		sheet_name = (char *)sheet->book_tnode->key;
	}


	QTableWidgetItem * twItem;


	twItem = new QTableWidgetItem;
	twItem->setText ( QString ( "%1" ).arg ( r ) );
	findTable->setItem ( row, FIND_COL_ROW, twItem );
	twItem->setData ( Qt::UserRole, QVariant::fromValue (
		(void *)(intptr_t)r ) );

	twItem = new QTableWidgetItem;
	twItem->setText ( QString ( "%1" ).arg ( c ) );
	findTable->setItem ( row, FIND_COL_COLUMN, twItem );
	twItem->setData ( Qt::UserRole, QVariant::fromValue (
		(void *)(intptr_t)c ) );

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( sheet_name ) );
	findTable->setItem ( row, FIND_COL_SHEET, twItem );

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( cell->text ) );
	findTable->setItem ( row, FIND_COL_CONTENT, twItem );

	return 0;
}

static int be_find_cb (
	CedSheet	* const	sheet,
	CedCell		* const	cell,
	int		const	r,
	int		const	c,
	void		* const	user_data
	)
{
	MainWindow	* const mw = static_cast<MainWindow *>(user_data);


	if ( mw->addFindRow ( sheet, cell, r, c ) )
	{
		return 1;
	}

	return 0;
}

void MainWindow::press_Find ()
{
	CedSheet * const sheet = projectGetSheet ();

	if ( ! sheet )
	{
		return;
	}

	if ( mprefs.find_all_sheets )
	{
		findTable->setColumnHidden ( FIND_COL_SHEET, false );
	}
	else
	{
		findTable->setColumnHidden ( FIND_COL_SHEET, true );
	}

	uprefs.set ( PREFS_FIND_TEXT, editFindText->text ().toUtf8 ().data () );

	setEnabled ( false );
	findTable->clearContents ();
	findTable->setRowCount ( 0 );

	be_find ( cedFile, sheet,
		editFindText->text ().toUtf8 ().data (),
		mprefs.find_wildcards,
		mprefs.find_case_sensitive,
		mprefs.find_value,
		mprefs.find_all_sheets,
		be_find_cb, (void *)this
		);

	// This hack ensures that all columns are right width (not just visible)
	findTable->setVisible ( false );
	findTable->resizeColumnsToContents ();
	findTable->setVisible ( true );

	findTable->setCurrentCell ( 0, 1 );
	setEnabled ( true );

	findTable->setFocus ( Qt::OtherFocusReason );
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
	int		r = 0,
			c = 0;


	twItem = findTable->item ( currentRow, FIND_COL_ROW );
	if ( twItem )
	{
		r = (int)(intptr_t)twItem->data( Qt::UserRole ).value<void *>();
	}

	twItem = findTable->item ( currentRow, FIND_COL_COLUMN );
	if ( twItem )
	{
		c = (int)(intptr_t)twItem->data( Qt::UserRole ).value<void *>();
	}

	if ( r < 1 || c < 1 )
	{
		return;
	}

	if ( ! findTable->isColumnHidden ( FIND_COL_SHEET ) )
	{
		twItem = findTable->item ( currentRow, FIND_COL_SHEET );
		if ( twItem )
		{
			int		i;


			i = buttonSheet->findText ( twItem->text () );
			if ( i >= 0 )
			{
				if ( i != buttonSheet->currentIndex () )
				{
					buttonSheet->setCurrentIndex ( i );
				}
			}
			else
			{
			// Sheet doesn't exist so list is old and needs clearing

				findTable->clearContents ();
				findTable->setRowCount ( 0 );
			}
		}
	}

	setCursorRange ( r, c, r, c, 1, 0, 0 );
	findTable->setFocus ( Qt::OtherFocusReason );
}

void MainWindow::press_OptionsFind ()
{
	if ( editFindText->hasFocus () )
	{
		tabWidget->hide ();
		viewMain->setFocus ();
	}
	else
	{
		tabWidget->show ();
		tabWidget->setCurrentIndex ( TAB_FIND );
		editFindText->setFocus ( Qt::OtherFocusReason );
		editFindText->selectAll ();
	}
}

void MainWindow::press_OptionsView ()
{
	if ( viewTab->hasFocus () )
	{
		tabWidget->hide ();
		viewMain->setFocus ();
	}
	else
	{
		tabWidget->show ();
		tabWidget->setCurrentIndex ( TAB_VIEW );
		viewTab->setFocus ();
	}
}

