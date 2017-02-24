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



void MainWindow::updateView ()
{
	viewMain->reconfigure ();
	viewTab->reconfigure ();
}

void MainWindow::updateViewConfig ()
{
	updateRender ();
	updateView ();
}



typedef struct
{
	int		check,
			select;

	mtQEX::ButtonMenu * buttonMenu;

	char	const	* active_graph;
} gscb;



static int populate_graph_list_cb (
	CedBook		* const	ARG_UNUSED ( book ),
	char	const	* const	graph_name,
	CedBookFile	* const	ARG_UNUSED ( bookfile ),
	void		* const	user_data
	)
{
	gscb		* const	data = (gscb *)user_data;


	if ( ! mtkit_utf8_string_legal ( (unsigned char const *)graph_name, 0 ))
	{
		fprintf ( stderr, "Bad graph name '%s'\n", graph_name );

		return 0;		// Continue
	}

	data->buttonMenu->addItem ( mtQEX::qstringFromC ( graph_name ) );

	if ( data->check )
	{
		// Select this item from the list if its the current sheet
		if ( ! strcmp ( graph_name, data->active_graph ) )
		{
			data->select = data->buttonMenu->count () - 1;
			data->check = 0;
		}
	}

	return 0;			// Continue
}

void MainWindow::updateGraph (
	char	const	* const	graph_name
	)
{
	gscb		dat = { 1, 0, buttonGraph, graph_name };
	bool		activate;


	if ( ! graph_name )
	{
		dat.check = 0;
	}

	/*
	NOTE: this function is called after loading a new book/sheet so we can't
	save data at this point as we might be storing an old graph script to a
	new file with the same graph name.
	*/

	buttonGraph->blockSignals ( true );
	buttonGraph->clear ();

	cui_graph_scan ( cedFile->cubook->book, populate_graph_list_cb, &dat );

	buttonGraph->setCurrentIndex ( dat.select );
	buttonGraph->blockSignals ( false );

	if ( buttonGraph->count () > 0 )
	{
		graphChanged ( dat.select );

		activate = true;
	}
	else
	{
		graphWidget->setImage ( NULL );
		graphTextEdit->setPlainText ( "" );

		activate = false;
	}

	actGraphDuplicate->setEnabled ( activate );
	actGraphRename->setEnabled ( activate );
	actGraphDelete->setEnabled ( activate );
	actGraphRedraw->setEnabled ( activate );
	actGraphExport->setEnabled ( activate );
	actGraphSClipboard->setEnabled ( activate );

	graphTextEdit->setReadOnly ( ! activate );
}

void MainWindow::updateRecentFiles ()
{
	char		buf[ 2048 ];
	int		i, lim_tot;


	lim_tot = pprfs->getInt ( GUI_INIFILE_RECENT_FILENAME_LEN );

	if ( lim_tot < 50 || lim_tot > 500 )
	{
		lim_tot = 80;
	}

	for ( i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		if ( 1 == mtKit::snip_filename (
			backend->recent_file.get_filename ( i+1 ),
			buf, sizeof ( buf ), lim_tot )
			)
		{
			// Hide if empty
			actFileRecent[ i ]->setVisible ( false );

			continue;
		}

		actFileRecent[ i ]->setText (
			mtQEX::qstringFromC ( buf ) );

		actFileRecent[ i ]->setVisible ( true );
	}

	// Hide separator if not needed
	if ( i > 0 )
	{
		actFileRecentSeparator->setVisible ( true );
	}
	else
	{
		actFileRecentSeparator->setVisible ( false );
	}
}

void MainWindow::updateTitleBar ()
{
	char		txt[ 2048 ];
	int		book;


	book = be_titlebar_text ( cedFile, txt, sizeof ( txt ), memChanged );

	setWindowTitle ( mtQEX::qstringFromC ( txt ) );

	if ( book )
	{
		buttonSheet->show ();
	}
	else
	{
		buttonSheet->hide ();
	}
}

void MainWindow::updateMenus ()
{
	CedSheet	* sheet;


	if ( ! cedFile->cubook->book )
	{
		return;
	}

	sheet = cui_file_get_sheet ( cedFile );

	// Enable/disable undo/redo according to their availability
	if ( cedFile->cubook->undo.undo_step )
	{
		actEditUndo->setEnabled ( true );
	}
	else
	{
		actEditUndo->setEnabled ( false );
	}

	if ( cedFile->cubook->undo.redo_step )
	{
		actEditRedo->setEnabled ( true );
	}
	else
	{
		actEditRedo->setEnabled ( false );
	}

	// Set the lock/unlock option for the sheet
	if ( sheet && sheet->prefs.locked )
	{
		actSheetLock->setText ( "Unlock" );
	}
	else
	{
		actSheetLock->setText ( "Lock" );
	}

	if (	sheet &&
		(sheet->prefs.split_r1 != 0 ||
			sheet->prefs.split_c1 != 0)
		)
	{
		actSheetFreezePanes->setText ( "Unfreeze Panes" );
	}
	else
	{
		actSheetFreezePanes->setText ( "Freeze Panes" );
	}
}



typedef struct
{
	int		check,
			select;
	mtQEX::ButtonMenu * buttonMenu;
	char	const	* active_sheet;
} sscb;



static int sselpop_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	char	const	* const	name,
	void		* const	user_data
	)
{
	sscb		* const	data = (sscb *)user_data;


	if ( ! mtkit_utf8_string_legal ( (unsigned char const *)name, 0 ) )
	{
		fprintf ( stderr, "Bad sheet name '%s'\n", name );

		return 0;		// Continue
	}

	data->buttonMenu->addItem ( mtQEX::qstringFromC ( name ) );

	if ( data->check )
	{
		// Select this item from the list if its the current sheet
		if ( ! strcmp ( name, data->active_sheet ) )
		{
			data->select = data->buttonMenu->count () - 1;
			data->check = 0;
		}
	}

	return 0;			// Continue
}

void MainWindow::updateSheetSelector ()
{
	sscb		dat = { 1, 0, buttonSheet, NULL };


	dat.active_sheet = cedFile->cubook->book->prefs.active_sheet;

	if ( ! dat.active_sheet )
	{
		dat.check = 0;
	}

	buttonSheet->blockSignals ( true );
	buttonSheet->clear ();

	ced_book_scan ( cedFile->cubook->book, sselpop_cb, &dat );

	buttonSheet->setCurrentIndex ( dat.select );
	buttonSheet->blockSignals ( false );

	sheetChanged ( dat.select );
}

void MainWindow::updateEntryCellref ()
{
	char		txt [ 128 ];


	be_sheet_ref ( projectGetSheet (), txt, sizeof ( txt ) );

	editCellref->setText ( mtQEX::qstringFromC ( txt ) );
}

void MainWindow::updateEntryCelltext ()
{
	CedCell		* cell = NULL;
	CedSheet	* sheet;
	QString		newtext;


	sheet = projectGetSheet ();

	if ( sheet )
	{
		cell = ced_sheet_get_cell ( sheet, sheet->prefs.cursor_r1,
			sheet->prefs.cursor_c1 );
	}

	editCelltext->setText ( newtext );

	if ( cell && cell->text )
	{
		if ( cell->type == CED_CELL_TYPE_TEXT_EXPLICIT )
		{
			newtext = "'";
		}

		newtext = newtext + mtQEX::qstringFromC ( cell->text );
		editCelltext->setText ( newtext );

		if ( cell->type == CED_CELL_TYPE_ERROR )
		{
			int		e = (int)(cell->value / 1000);


			editCelltext->setSelection ( 0, e );
		}
	}

	if (	sheet &&
		( sheet->prefs.locked ||
			( cell && cell->prefs && cell->prefs->locked ) ) &&
		! cedFile->cubook->book->prefs.disable_locks
		)
	{
		editCelltext->setEnabled ( false );
	}
	else
	{
		editCelltext->setEnabled ( true );
	}
}

void MainWindow::updateRecalcBook ()
{
	ced_book_recalculate ( cedFile->cubook->book, 0 );
	ced_book_recalculate ( cedFile->cubook->book, 1 );
}

void MainWindow::updateQuicksumLabel ()
{
	int		op = buttonQuicksum->currentIndex ();


	if ( op == 0 )
	{
		labelQuicksum->hide ();

		return;
	}


	char		txt [ 256 ];


	be_quicksum_label ( projectGetSheet (), txt, sizeof ( txt ), op );

	labelQuicksum->setText ( mtQEX::qstringFromC ( txt ) );
	labelQuicksum->show ();
}

void MainWindow::setCursorRange (
	int		r1,
	int		c1,
	int		r2,
	int		c2,
	int	const	cursor_visible,
	int	const	force_update,
	int	const	follow_all
	)
{
	CedSheet	* const	sheet = projectGetSheet ();
	int		mr1 = -1,
			mr2,
			mc1,
			mc2;


	if ( ! sheet )
	{
		return;
	}

	if ( r1 < 1 ) r1 = 1;
	if ( r2 < 1 ) r2 = 1;
	if ( c1 < 1 ) c1 = 1;
	if ( c2 < 1 ) c2 = 1;
	if ( r1 > CED_MAX_ROW ) r1 = CED_MAX_ROW;
	if ( r2 > CED_MAX_ROW ) r2 = CED_MAX_ROW;
	if ( c1 > CED_MAX_COLUMN ) c1 = CED_MAX_COLUMN;
	if ( c2 > CED_MAX_COLUMN ) c2 = CED_MAX_COLUMN;

	if (	! force_update &&
		sheet->prefs.cursor_r1 == r1 &&
		sheet->prefs.cursor_c1 == c1 &&
		sheet->prefs.cursor_r2 == r2 &&
		sheet->prefs.cursor_c2 == c2
		)
	{
		return;		// No update required
	}


/*
For ultra optimization spot when rows or cols are the same in old & new and only
one row/col is different.  Then just update the difference in rows or cols
between the old and new.  e.g. when the user is enlarging or shrinking the
selection area with the mouse or the arrow keys.
*/

/*
	if (	sheet->prefs.cursor_r1 == r1 &&
		sheet->prefs.cursor_r2 == r2 )
	{
		if ( sheet->prefs.cursor_c1 == c1 )
		{
			mr1 = r1;
			mr2 = r2;
			mc1 = c2;
			mc2 = sheet->prefs.cursor_c2;
		}
		else if ( sheet->prefs.cursor_c2 == c2 )
		{
			mr1 = r1;
			mr2 = r2;
			mc1 = c1;
			mc2 = sheet->prefs.cursor_c1;
		}
	}

NOTE - we disallow horizontal optimization because sometimes long text lines
need to be fully redrawn rather than partially by a single column.  For example
in the test suite there is the long line test which doesn't redraw properly when
the user expands the cursor selection in a particular way.  This is because of
the different text colour that is rendered if the text cell is selected, or
painted over if the text cell isn't selected.
*/

	if (	sheet->prefs.cursor_c1 == c1 &&
		sheet->prefs.cursor_c2 == c2
		)
	{
		if ( sheet->prefs.cursor_r1 == r1 )
		{
			mr1 = sheet->prefs.cursor_r2;
			mr2 = r2;
			mc1 = c1;
			mc2 = c2;
		}
		else if ( sheet->prefs.cursor_r2 == r2 )
		{
			mr1 = sheet->prefs.cursor_r1;
			mr2 = r1;
			mc1 = c1;
			mc2 = c2;
		}
	}

	if ( mr1 == -1 )
	{
		viewMain->redrawArea ( sheet,
			sheet->prefs.cursor_r1, sheet->prefs.cursor_c1,
			sheet->prefs.cursor_r2, sheet->prefs.cursor_c2
			);
		viewTab->redrawArea ( sheet,
			sheet->prefs.cursor_r1, sheet->prefs.cursor_c1,
			sheet->prefs.cursor_r2, sheet->prefs.cursor_c2
			);
	}

	sheet->prefs.cursor_r1 = r1;
	sheet->prefs.cursor_c1 = c1;
	sheet->prefs.cursor_r2 = r2;
	sheet->prefs.cursor_c2 = c2;

	if ( mr1 == -1 )
	{
		viewMain->redrawArea ( sheet, r1, c1, r2, c2 );
		viewTab->redrawArea ( sheet, r1, c1, r2, c2 );
	}
	else
	{
		viewMain->redrawArea ( sheet, mr1, mc1, mr2, mc2 );
		viewTab->redrawArea ( sheet, mr1, mc1, mr2, mc2 );
	}

	viewMain->setScrollbars ();
	viewTab->setScrollbars ();

	if ( cursor_visible )
	{
		if ( follow_all )
		{
			viewMain->ensureVisible ( sheet, r1, c1 );
			viewTab->ensureVisible ( sheet, r1, c1 );
		}
		else
		{
			if ( viewTab->hasFocus () )
			{
				viewTab->ensureVisible ( sheet, r1, c1 );
			}
			else
			{
				viewMain->ensureVisible ( sheet, r1, c1 );
			}
		}
	}

	updateEntryCellref ();
	updateEntryCelltext ();
	updateQuicksumLabel ();
}

void MainWindow::updateRender ()
{
	crendr.sheet = projectSetSheet ();
	crendr.row_pad = pprfs->getInt ( GUI_INIFILE_ROW_PAD );

	be_cedrender_set_font_width ( &crendr );
}

CuiRender * MainWindow::projectGetRender ()
{
	return &crendr;
}

void MainWindow::setChangesFlag ()
{
	memChanged = 1;
	updateTitleBar ();
}

void MainWindow::updateChangesChores (
	int	const	new_geometry,
	int	const	block_sheet_recalcs
	)
{
	if (	! block_sheet_recalcs &&
		projectGetSheet () &&
		cedFile->cubook->book->prefs.auto_recalc
		)
	{
		// Always update sheet (must be done before book updates)
		coreRecalcSheet ();

		switch ( cedFile->cubook->book->prefs.auto_recalc )
		{
		case 2:
			coreRecalcBook ();
			break;
		}
	}

	setChangesFlag ();
	updateMainArea ();
	updateMenus ();
	updateEntryCelltext ();
	updateQuicksumLabel ();

	if ( new_geometry )
	{
		projectSetSheetGeometry ();

		viewMain->updateGeometry ();
		viewTab->updateGeometry ();
	}
}

void MainWindow::updateMainArea ()
{
	viewMain->updateRedraw ();
	viewTab->updateRedraw ();
}

int MainWindow::projectReportUpdates (
	int	const	error
	)
{
	char	const	* msg;
	char		buf [ 2048 ];


	msg = be_get_error_update_text ( error, buf, sizeof ( buf ) );

	if ( ! msg )
	{
		return 0;
	}

	// Report this error to the user
	QMessageBox::critical ( this, "Error",
		mtQEX::qstringFromC ( msg ) );

	return error;
}

