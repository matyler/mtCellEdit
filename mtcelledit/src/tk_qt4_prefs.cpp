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



int MainWindow::projectSetFont (
	char	const	* const	name,
	int		const	sz
	)
{
	mtPixy::Font	* newfont = NULL;


	newfont = new mtPixy::Font ( name, sz );
	if ( ! newfont )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to open font %1 size %2."
			).arg ( name ).arg ( sz ) );

		return 1;
	}

	delete crendr.font;
	crendr.font = newfont;

	updateViewConfig ();

	return 0;
}

void pref_change_font (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	callback_ptr
	)
{
	char	const	* name;
	int		size;
	mtKit::Prefs	* prefs = (mtKit::Prefs *)callback_ptr;


	name = prefs->getString ( GUI_INIFILE_FONT_PANGO_NAME );
	size = prefs->getInt ( GUI_INIFILE_FONT_SIZE );

	if ( mainwindow->projectSetFont ( name, size ) )
	{
		mainwindow->projectSetFont ( "Sans", 16 );
	}
}

void pref_change_row_pad (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	mainwindow->updateViewConfig ();
}

void pref_change_graph_scale (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	mainwindow->pressGraphRedraw ();
}

void pref_change_recent_filename_len (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	mainwindow->updateRecentFiles ();
}

mtPrefs * Backend::load_pref_window_prefs (
	mtPrefTable	const * const	table
	)
{
	mtPrefs		* mtpr;


	mtpr = mtkit_prefs_new ( table );
	mtKit::prefsInitWindowPrefs ( mtpr );

	if ( mtKit::prefsWindowMirrorPrefs ( mtpr, preferences.getPrefsMem() ))
	{
		mtkit_prefs_destroy ( mtpr );

		return NULL;
	}

	load_pref_window_prefs ( mtpr );

	return mtpr;
}

void fe_commit_prefs_set (
	int		const	pref_id,
	int		const	pref_num,
	char	const	* const	pref_charp,
	int		const	change_cursor,
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	CedSheet	* const sheet = mainwindow->projectGetSheet ();
	int		res;


	if ( ! sheet )
	{
		return;
	}

	res = be_commit_prefs_set ( sheet,
		mainwindow->projectGetCedFile ()->cubook,
		pref_id, pref_num, pref_charp );

	mainwindow->projectReportUpdates ( res );

	if ( be_commit_prefs_set_check ( res, sheet, change_cursor, pref_id ) )
	{
		return;
	}

	mainwindow->updateChangesChores ( 1, 1 );
}

void MainWindow::pressOptionsCellPrefs ()
{
	CedSheet	* sheet = projectGetSheet ();
	mtPrefs		* mtpr;


	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	mtpr = backend->cellpref_init ( sheet, be_cellpref_changed,
		(void *)this );
	if ( ! mtpr )
	{
		return;
	}

	mtQEX::PrefsWindow ( mtpr, "Cell Preferences" );

	backend->save_pref_window_prefs ( mtpr );
	mtKit::prefsWindowMirrorPrefs ( pprfs->getPrefsMem (), mtpr );

	mtkit_prefs_destroy ( mtpr );
	mtpr = NULL;

	be_cellpref_cleanup ( sheet );

	updateView ();
}

void MainWindow::pressOptionsBookPrefs ()
{
	mtPrefs * mtpr = backend->book_prefs_init ( cedFile->cubook->book );


	mtQEX::PrefsWindow ( mtpr, "Book Preferences" );

	backend->save_pref_window_prefs ( mtpr );
	mtKit::prefsWindowMirrorPrefs ( pprfs->getPrefsMem (), mtpr );

	be_book_prefs_finish ( mtpr, cedFile->cubook->book );

	mtkit_prefs_destroy ( mtpr );
}

void MainWindow::pressOptionsProgramPrefs ()
{
	mtQEX::PrefsWindow ( pprfs->getPrefsMem (), "Preferences" );
}

void MainWindow::pressOptionsTextStyle (
	int	const	i
	)
{
	int		res;


	if ( be_prepare_prefs_set ( projectGetSheet () ) )
	{
		return;
	}

	res = cui_cellprefs_text_style ( cedFile, i );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	projectReportUpdates ( res );
	updateChangesChores ( 1, 1 );
	be_cellpref_cleanup ( projectGetSheet () );
}

static void swFunc (
	QString		const	title,
	int		const	pref_id,
	mtKit::Prefs	* const	prefs
	)
{
	if ( be_prepare_prefs_set ( mainwindow->projectGetSheet () ) )
	{
		return;
	}


	SwatchDialog	dialog ( title, prefs );
	int		col = dialog.getColor ();


	if ( col >= 0 )
	{
		fe_commit_prefs_set ( pref_id, col, NULL, 0, NULL );
	}

	be_cellpref_cleanup ( mainwindow->projectGetSheet () );
}

void MainWindow::pressOptionsBackgroundColor ()
{
	swFunc ( "Background Colour", CUI_CELLPREFS_color_background, pprfs );
}

void MainWindow::pressOptionsForegroundColor ()
{
	swFunc ( "Foreground Colour", CUI_CELLPREFS_color_foreground, pprfs );
}

void MainWindow::pressOptionsBorderColor ()
{
	swFunc ( "Border Colour", CUI_CELLPREFS_border_color, pprfs );
}

void MainWindow::pressOptionsBorder (
	int	const	i
	)
{
	int		res;


	if ( be_prepare_prefs_set ( projectGetSheet () ) )
	{
		return;
	}

	res = cui_cellprefs_border ( cedFile, i );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	projectReportUpdates ( res );
	updateChangesChores ( 1, 1 );
	be_cellpref_cleanup ( projectGetSheet () );
}

void fe_book_prefs_changed (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	mainwindow->setChangesFlag ();
}

