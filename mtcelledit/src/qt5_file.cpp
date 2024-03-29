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



void MainWindow::press_FileNew ()
{
	if ( okToLoseChanges () )
	{
		projectClearAll ();
	}
}

void MainWindow::press_FileOpen ()
{
	if ( okToLoseChanges () )
	{
		QString filename = QFileDialog::getOpenFileName ( this,
			"Load Project File",
			mtQEX::qstringFromC ( mprefs.last_dir.c_str() ),
			NULL, NULL, QFileDialog::DontUseNativeDialog );


		if ( ! filename.isEmpty () )
		{
			projectLoad ( filename.toUtf8 ().data () );
		}
	}
}

void MainWindow::press_FileImport ()
{
	QString filename = QFileDialog::getOpenFileName ( this,
		"Import Project File",
		mtQEX::qstringFromC ( mprefs.last_dir.c_str() ),
		NULL, NULL, QFileDialog::DontUseNativeDialog );


	if ( ! filename.isEmpty () )
	{
		projectImport ( filename.toUtf8 ().data () );
	}
}

void MainWindow::press_FileSave ()
{
	if ( ! cedFile->name || 0 == cedFile->name[0] )
	{
		press_FileSaveAs ();
	}
	else
	{
		projectSave ( cedFile->name, cedFile->type );
	}
}

void MainWindow::press_FileSaveAs ()
{
	QStringList	typelist = getFileExportTypes ();
	mtQEX::SaveFileDialog dialog ( this, "Save Project File", typelist,
				cedFile->type - 1, cedFile->name );

	dialog.setOption ( QFileDialog::DontConfirmOverwrite );

	if ( ! cedFile->name )
	{
		dialog.setDirectory ( mtQEX::qstringFromC ( mprefs.last_dir.
			c_str() ) );
	}

	while ( dialog.exec () )
	{
		QString filename = mtQEX::get_filename ( dialog );

		if ( filename.isEmpty () )
		{
			continue;
		}

		int const format = dialog.getFormat () + 1;
		char * correct = cui_get_correct_sheet_filename (
			filename.toUtf8().data(), format );

		if ( correct )
		{
			filename = QString::fromUtf8 ( correct );
			free ( correct );
			correct = NULL;
		}

		if ( mtQEX::message_file_overwrite ( this, filename ) )
		{
			continue;
		}

		if ( 0 == projectSave ( filename.toUtf8 ().data (), format ) )
		{
			break;
		}
	}
}

void MainWindow::press_FileRecent ( int const i )
{
	if ( okToLoseChanges () )
	{
		projectLoad ( mprefs.recent_file.filename( (size_t)i ).c_str());
	}
}

void MainWindow::press_FileQuit ()
{
	close ();
}

void MainWindow::projectClearAll ()
{
	cui_file_book_new ( cedFile );
	projectSetSheet ();		// Old sheet stale so update renderer
	updateMenus ();

	memChanged = 0;

	updateSheetSelector ();
	updateTitleBar ();

	setCursorRange ( 1, 1, 1, 1, 1, 1, 1 );

	updateGraph ( NULL );
}

int MainWindow::projectLoad (
	char	const	* const	filename
	)
{
	CedSheet	* sheet;
	CedSheetPrefs	* tmp_sheet_prefs = NULL;


	sheet = cui_file_get_sheet ( cedFile );
	if ( sheet )
	{
		if ( ced_file_type_class ( cedFile->type ) == 1 )
		{
			if ( mprefs.sheet_prefs_persist )
			{
				tmp_sheet_prefs = ced_sheet_prefs_new ();
				ced_sheet_prefs_copy ( tmp_sheet_prefs,
					&sheet->prefs );
			}
		}
	}

	if ( cui_file_load ( cedFile, filename, backend->get_force_tsvcsv () ) )
	{
		QMessageBox::critical ( this, "Error",
			QString ("Unable to load project file: %1").
			arg ( mtQEX::qstringFromC ( filename ) ) );

		ced_sheet_prefs_free ( tmp_sheet_prefs );

		return 1;
	}

	cui_file_set_lock ( cedFile, mprefs.file_lock_load );

	projectSetSheet ();		// Old sheet stale so update renderer

	if ( tmp_sheet_prefs )
	{
		if ( ced_file_type_class ( cedFile->type ) == 1 )
		{
			sheet = projectGetSheet ();
			if ( sheet )
			{
				ced_sheet_prefs_copy ( &sheet->prefs,
					tmp_sheet_prefs );
			}
		}

		ced_sheet_prefs_free ( tmp_sheet_prefs );
		tmp_sheet_prefs = NULL;
	}

	updateSheetSelector ();
	updateGraph ( cedFile->cubook->book->prefs.active_graph );

	if (	cedFile->cubook->book->prefs.auto_recalc ||
		! ( cedFile->type == CED_FILE_TYPE_TSV_VAL_BOOK ||
			cedFile->type == CED_FILE_TYPE_LEDGER_VAL_BOOK )
		)
	{
		// Books with values don't need recalculating
		updateRecalcBook ();
	}

	updateMenus ();

	memChanged = 0;

	projectRegister ();

	if ( cedFile->cubook->book->prefs.disable_locks )
	{
		QMessageBox::warning ( this, "Warning",
			"This book has the disable_locks flag set, "
			"which is potentially dangerous." );
	}

	return 0;
}

void MainWindow::projectRegister ()
{
	if ( backend->register_project ( cedFile ) )
	{
		return;
	}

	updateTitleBar ();
	updateRecentFiles ();
}

void MainWindow::reportLargeTSV ()
{
	QMessageBox::critical ( this, "Error",
		"The sheet geometry is too large to save in this file"
		" format." );
}

int MainWindow::projectSave (
	char	const	* const	filename,
	int		const	format
	)
{
	int		res;


	if (	format == CED_FILE_TYPE_TSV_BOOK	||
		format == CED_FILE_TYPE_TSV_VAL_BOOK	||
		format == CED_FILE_TYPE_LEDGER_BOOK	||
		format == CED_FILE_TYPE_LEDGER_VAL_BOOK
		)
	{
		projectGraphStoreChanges ();
	}

	res = cui_file_save ( cedFile, filename, format );
	if ( res == 1 )
	{
		reportLargeTSV ();

		return 1;
	}

	if ( res )
	{
		QMessageBox::critical ( this, "Error",
			QString ("Unable to save file: %1").
			arg ( mtQEX::qstringFromC ( filename ) ) );

		return 1;
	}

	cui_file_set_lock ( cedFile, mprefs.file_lock_save );

	memChanged = 0;
	projectRegister ();

	return 0;
}

int MainWindow::projectImport (
	char	const	* const	filename
	)
{
	CuiFile		* uifile;
	int		sheet_tot, sheet_fail, file_tot, file_fail, res;


	uifile = cui_file_new ();
	if ( ! uifile )
	{
		QMessageBox::critical ( this, "Error",
			QString("Unable to allocate memory to import file: %1").
			arg ( mtQEX::qstringFromC ( filename ) ) );

		return 1;
	}

	if ( cui_file_load ( uifile, filename, backend->get_force_tsvcsv () ) ||
		! uifile->cubook			||
		! uifile->cubook->book
		)
	{
		cui_file_free ( uifile );

		QMessageBox::critical ( this, "Error",
			QString ("Unable to import file: %1").
			arg ( mtQEX::qstringFromC ( filename ) ) );

		return 1;
	}

	res = cui_book_merge ( cedFile->cubook, uifile->cubook->book,
		&sheet_tot, &sheet_fail, &file_tot, &file_fail );

	cui_file_free ( uifile );

	projectReportUpdates ( res );

	updateSheetSelector ();		// Done here as need for success/fail

	// Update graph selector
	updateGraph ( cedFile->cubook->book->prefs.active_graph );

	QMessageBox::information ( this, "Information",
		QString (
		"%1 sheets imported.\n"
		"%2 sheets not imported due to identical names.\n"
		"%3 graphs/files imported.\n"
		"%4 graphs/files not imported due to identical names." )
		.arg ( sheet_tot )
		.arg ( sheet_fail )
		.arg ( file_tot )
		.arg ( file_fail )
		);


	if (	res == CUI_ERROR_LOCKED_CELL	||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	if ( sheet_tot > 0 )
	{
		be_update_file_to_book ( cedFile );
	}

	updateChangesChores ( 0, 1 );

	return 0;			// Success
}

