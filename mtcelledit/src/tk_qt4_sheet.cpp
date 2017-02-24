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



void MainWindow::pressSheetNew ()
{
	int		res;


	res = cui_file_sheet_add ( cedFile );
	if ( res == 1 )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to create a new sheet." );

		return;
	}

	projectReportUpdates ( res );

	if (	res == CUI_ERROR_UNDO_OP ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		// Nothing more to report or do

		return;
	}

	updateSheetSelector ();
	be_update_file_to_book ( cedFile );

	updateChangesChores ( 1, 1 );
}

void MainWindow::pressSheetDuplicate ()
{
	CedSheet	* sheet = projectGetSheet (),
			* newsheet;
	int		res;


	if ( ! sheet || ! sheet->book_tnode )
	{
		return;
	}

	res = cui_book_duplicate_sheet ( cedFile->cubook, sheet, &newsheet );
	projectReportUpdates ( res );

	if ( res )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to duplicate the current sheet." );

		return;
	}

	updateSheetSelector ();

	res = buttonSheet->findText ( mtQEX::qstringFromC (
		(char *)newsheet->book_tnode->key ) );

	if ( res >= 0 )
	{
		buttonSheet->setCurrentIndex ( res );
	}

	be_update_file_to_book ( cedFile );

	updateChangesChores ( 1, 0 );
}

int MainWindow::projectRenameSheet (
	CedSheet	* const	sheet,
	QString		const	new_name
	)
{
	if ( new_name.isEmpty () )
	{
		QMessageBox::critical ( this, "Error",
			"Bad sheet name." );

		return 1;
	}

	if ( ced_book_get_sheet ( cedFile->cubook->book,
		new_name.toUtf8 ().data () ) )
	{
		QMessageBox::critical ( this, "Error",
			"Sheet name already exists." );

		return 1;
	}

	int res = cui_book_page_rename ( cedFile->cubook, sheet,
		new_name.toUtf8 ().data () );

	projectReportUpdates ( res );

	if ( res == CUI_ERROR_NO_CHANGES )
	{
		return 1;		// Try again
	}

	updateSheetSelector ();
	updateChangesChores ( 0, 1 );

	return 0;			// Success
}

void MainWindow::pressSheetRename ()
{
	CedSheet	* sheet = projectGetSheet ();


	if (	! sheet			||
		projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) ||
		! ( cedFile->type == CED_FILE_TYPE_TSV_BOOK ||
			cedFile->type == CED_FILE_TYPE_TSV_VAL_BOOK ||
			cedFile->type == CED_FILE_TYPE_LEDGER_BOOK ||
			cedFile->type == CED_FILE_TYPE_LEDGER_VAL_BOOK
			)		||
		! sheet->book_tnode	||
		! sheet->book_tnode->key
		)
	{
		return;
	}

	for ( int res = 1; res; )
	{
		bool	ok;
		QString new_name = QInputDialog::getText ( this,
			"Rename Sheet",
			"New Sheet Name:",
			QLineEdit::Normal,
			mtQEX::qstringFromC ( (char *)sheet->book_tnode->key ),
			&ok );


		if ( ok )
		{
			res = projectRenameSheet ( sheet, new_name );
		}
		else
		{
			break;
		}
	}
}

void MainWindow::pressSheetDelete ()
{
	CedSheet	* sheet = projectGetSheet ();
	int		page_num, res;


	if ( ! sheet || ! sheet->book_tnode )
	{
		return;
	}

	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	page_num = buttonSheet->currentIndex ();

	res = cui_book_destroy_sheet ( cedFile->cubook,
		(char *)sheet->book_tnode->key );

	// This is needed to replace the stale sheet reference in render
	projectSetSheet ();

	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		buttonSheet->setCurrentIndex ( page_num );

		return;
	}

	updateSheetSelector ();

	if ( page_num > 0 )
	{
		page_num --;
	}

	buttonSheet->setCurrentIndex ( page_num );

	updateChangesChores ( 1, 1 );
}

QStringList MainWindow::getFileExportTypes ()
{
	QStringList	list;
	int		i;


	for (	i = CED_FILE_TYPE_NONE + 1;
		i <= CED_FILE_TYPE_LEDGER_VAL_BOOK;
		i ++
		)
	{
		list << mtQEX::qstringFromC ( ced_file_type_text ( i ) );
	}

	return list;
}

void MainWindow::pressSheetExport ()
{
	CedSheet	* sheet = projectGetSheet ();


	if ( ! sheet )
	{
		return;
	}


	QStringList	list = getFileExportTypes ();
	QString		last = mtQEX::qstringFromC ( pprfs->getString (
				GUI_INIFILE_LAST_DIR ) ) +
				MTKIT_DIR_SEP + "export";


	while ( 1 )
	{
		mtQEX::SaveFileDialog dialog ( this, "Export Sheet", list,
			cedFile->type - 1, last.toUtf8 ().data () );

		if ( ! dialog.exec () )
		{
			break;
		}

		QStringList	fileList = dialog.selectedFiles ();
		QString		filename = fileList.at ( 0 );
		int		format = dialog.getFormat ();


		if ( ! filename.isEmpty () )
		{
			if ( be_export_sheet ( sheet,
				filename.toUtf8 ().data (), format + 1 ) )
			{
				QMessageBox::critical ( this, "Error",
					"Unable to export sheet." );

				continue;
			}

			projectRegister ();
		}

		break;
	}
}

void MainWindow::pressSheetExportOutput ()
{
	CedSheet	* const	sheet = projectGetSheet ();


	if ( ! sheet )
	{
		return;
	}


	QStringList	list;
	QString		last = mtQEX::qstringFromC ( pprfs->getString (
				GUI_INIFILE_LAST_DIR ) ) +
				MTKIT_DIR_SEP + "export";


	// Order as per CED_SHEET_EXPORT_*
	list	<< "EPS"
		<< "HTML"
		<< "PDF"
		<< "PDF Paged"
		<< "PNG"
		<< "PS"
		<< "SVG"
		<< "TSV"
		<< "TSV Quoted"
		;

	while ( 1 )
	{
		mtQEX::SaveFileDialog dialog ( this, "Export Sheet Output",
			list, lastExportSheetType, last.toUtf8 ().data () );

		if ( ! dialog.exec () )
		{
			break;
		}

		QStringList	fileList = dialog.selectedFiles ();
		QString		filename = fileList.at ( 0 );


		if ( ! filename.isEmpty () )
		{
			lastExportSheetType = dialog.getFormat ();

			if ( cui_export_output ( pprfs->getPrefsMem (), sheet,
				filename.toUtf8 ().data (),
				cedFile->name, lastExportSheetType,
				pprfs->getInt ( GUI_INIFILE_ROW_PAD ),
				pprfs->getString ( GUI_INIFILE_FONT_PANGO_NAME),
				pprfs->getInt ( GUI_INIFILE_FONT_SIZE ) )
				)
			{
				QMessageBox::critical ( this, "Error",
					"Unable to export sheet output." );

				continue;
			}

			backend->remember_last_dir( filename.toUtf8 ().data ());
		}

		break;
	}
}

void MainWindow::pressSheetFreezePanes ()
{
	CedSheet	* sheet = projectGetSheet ();
	int		r1, c1, r2, c2, srow, scol, set_pos = 0;


	if ( ! sheet )
	{
		return;
	}

	if (	sheet->prefs.split_r1 != 0 ||
		sheet->prefs.split_c1 != 0
		)
	{
		// Split currently in force so turn current one off

		srow = sheet->prefs.split_r1;
		scol = sheet->prefs.split_c1;

		r1 = 0;
		c1 = 0;
		r2 = 0;
		c2 = 0;

		set_pos = 1;
	}
	else
	{
		// No split yet so set one up

		r1 = sheet->prefs.start_row;
		c1 = sheet->prefs.start_col;
		r2 = MAX ( sheet->prefs.cursor_r1 - 1, r1 );
		c2 = MAX ( sheet->prefs.cursor_c1 - 1, c1 );

		if ( sheet->prefs.cursor_r1 == 1 )
		{
			r1 = 0;
			r2 = 0;
		}

		if ( sheet->prefs.cursor_c1 == 1 )
		{
			c1 = 0;
			c2 = 0;
		}
	}

	sheet->prefs.split_r1 = r1;
	sheet->prefs.split_r2 = r2;
	sheet->prefs.split_c1 = c1;
	sheet->prefs.split_c2 = c2;

	updateViewConfig ();

	if ( set_pos )
	{
		viewMain->ensureVisible ( sheet, srow, scol );
	}

	updateChangesChores ( 0, 1 );
}

void MainWindow::pressSheetLock ()
{
	CedSheet	* const	sheet = projectGetSheet ();


	if ( ! sheet )
	{
		return;
	}

	sheet->prefs.locked = ! sheet->prefs.locked;

	// Update the lock/unlock menu and the other gubbins
	updateChangesChores ( 0, 1 );
}

void MainWindow::pressSheetPrevious ()
{
	int		i = buttonSheet->currentIndex () - 1;


	buttonSheet->setCurrentIndex ( MAX ( i, 0 ) );
}

void MainWindow::pressSheetNext ()
{
	int	const	i = buttonSheet->currentIndex () + 1;
	int	const	max = buttonSheet->count () - 1;


	buttonSheet->setCurrentIndex ( MIN ( i, max ) );
}

void MainWindow::coreRecalcBook ()
{
	ced_book_recalculate ( cedFile->cubook->book, 0 );
	ced_book_recalculate ( cedFile->cubook->book, 1 );
}

void MainWindow::coreRecalcSheet ()
{
	CedSheet	* const	sheet = projectGetSheet ();


	ced_sheet_recalculate ( sheet, NULL, 0 );
	ced_sheet_recalculate ( sheet, NULL, 1 );
}

void MainWindow::pressSheetRecalcBook ()
{
	coreRecalcBook ();
	updateChangesChores ( 0, 1 );
}

void MainWindow::pressSheetRecalc ()
{
	coreRecalcSheet ();
	updateChangesChores ( 0, 1 );
}

int MainWindow::getSelectionPosition (
	CedSheet	* const	sheet,
	int		* const	row,
	int		* const	col
	)
{
	if ( ! sheet )
	{
		return 1;
	}

	if ( row )
	{
		row[0] = sheet->prefs.cursor_r1;
	}

	if ( col )
	{
		col[0] = sheet->prefs.cursor_c1;
	}

	if ( ! clipboardObtainPaste () )
	{
		return 1;		// No paste found
	}

	return 0;
}

void MainWindow::pressRowInsert ()
{
	CedSheet	* const	sheet = projectGetSheet ();
	int		row,
			rowtot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_row_extent ( sheet, &row, &rowtot ) )
	{
		return;
	}

	res = cui_sheet_insert_row ( cedFile->cubook, sheet, row, rowtot );
	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateChangesChores ( 1, 0 );
}

void MainWindow::pressRowInsertPasteHeight ()
{
	CedSheet	* const	sheet = projectGetSheet ();
	int		row,
			res;


	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if (	getSelectionPosition ( sheet, &row, NULL ) ||
		row < 1 )
	{
		return;
	}

	res = cui_sheet_insert_row ( cedFile->cubook, sheet, row,
		cedClipboard->rows );
	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateChangesChores ( 1, 0 );
}

void MainWindow::pressRowDelete ()
{
	CedSheet	* const	sheet = projectGetSheet ();
	int		row,
			rowtot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_row_extent ( sheet, &row, &rowtot ) )
	{
		return;
	}

	res = cui_sheet_delete_row ( cedFile->cubook, sheet, row, rowtot );
	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateChangesChores ( 1, 0 );
}

void MainWindow::pressColumnInsert ()
{
	CedSheet	* const	sheet = projectGetSheet ();
	int		col,
			coltot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_col_extent ( sheet, &col, &coltot ) )
	{
		return;
	}

	res = cui_sheet_insert_column ( cedFile->cubook, sheet, col, coltot );
	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateViewConfig ();
	updateChangesChores ( 1, 0 );
}

void MainWindow::pressColumnInsertPasteWidth ()
{
	CedSheet	* const	sheet = projectGetSheet ();
	int		col,
			res;


	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if (	getSelectionPosition ( sheet, NULL, &col ) ||
		col < 1
		)
	{
		return;
	}

	res = cui_sheet_insert_column ( cedFile->cubook, sheet, col,
		cedClipboard->cols );
	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateViewConfig ();
	updateChangesChores ( 1, 0 );
}

void MainWindow::pressColumnDelete ()
{
	CedSheet	* const	sheet = projectGetSheet ();
	int		col,
			coltot,
			res;


	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	if ( be_selection_col_extent ( sheet, &col, &coltot ) )
	{
		return;
	}

	res = cui_sheet_delete_column ( cedFile->cubook, sheet, col, coltot );
	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateViewConfig ();
	updateChangesChores ( 1, 0 );
}

void MainWindow::pressColumnSetWidth ()
{
	CedSheet	* const	sheet = projectGetSheet ();
	CedCell		* cell;
	int		w;


	if ( ! sheet )
	{
		return;
	}

	// We check here to tell user action is invalid before wasting any
	// CPU/user time.

	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	cell = ced_sheet_get_cell ( sheet, 0, sheet->prefs.cursor_c1 );

	if ( cell && cell->prefs )
	{
		w = cell->prefs->width;
	}
	else
	{
		w = 0;
	}

	bool ok;

	w = QInputDialog::getInt ( this, "Set Column Width",
		"Set Column Width", w, 0, CED_MAX_COLUMN_WIDTH, 1, &ok );

	if ( ! ok )
	{
		return;
	}

	int c = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );

	int ctot = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 )
		- c + 1;

	int res = cui_sheet_set_column_width ( cedFile->cubook, sheet, c, ctot,
		w );

	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateViewConfig ();
	updateChangesChores ( 1, 1 );
}

void MainWindow::pressColumnSetWidthAuto ()
{
	int		res,
			c,
			ctot;
	CedSheet	* const	sheet = projectGetSheet ();


	if ( ! sheet )
	{
		return;
	}

	c = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	ctot = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 ) - c + 1;

	res = cui_sheet_set_column_width_auto ( cedFile->cubook, sheet, c,
		ctot );

	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	updateViewConfig ();
	updateChangesChores ( 0, 1 );
}

void MainWindow::sheetChanged (
	int		const	ARG_UNUSED ( i )
	)
{
	QString		text;
	CedBook		* const	book = cedFile->cubook->book;


	text = buttonSheet->text ();

	if ( text.isEmpty () )
	{
		mtkit_strfreedup ( &book->prefs.active_sheet, NULL );
	}
	else
	{
		mtkit_strfreedup ( &book->prefs.active_sheet,
			text.toUtf8 ().data () );
	}

	updateViewConfig ();
	updateEntryCelltext ();
	updateEntryCellref ();
	updateQuicksumLabel ();
	updateMenus ();

	viewMain->setFocus ();
}

CedSheet * MainWindow::projectGetSheet ()
{
	return crendr.sheet;
}

CuiFile * MainWindow::projectGetCedFile ()
{
	return cedFile;
}

void MainWindow::projectSetSheetGeometry ()
{
	sheetRows = 0;
	sheetCols = 0;

	ced_sheet_get_geometry ( projectGetSheet (), &sheetRows, &sheetCols );
}

CedSheet * MainWindow::projectSetSheet ()
{
	// Automatically update renderer to this new value
	crendr.sheet = cui_file_get_sheet ( cedFile );

	projectSetSheetGeometry ();

	return crendr.sheet;
}

void MainWindow::projectGetSheetGeometry (
	int		* const	r,
	int		* const	c
	)
{
	if ( r )
	{
		r[0] = sheetRows;
	}

	if ( c )
	{
		c[0] = sheetCols;
	}
}

