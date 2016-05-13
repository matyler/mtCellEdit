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



int MainWindow :: projectSetFont (
	char	const	* const	name,
	int		const	size
	)
{
	mtFont		* newfont = NULL;


	newfont = cui_font_new_pango ( name, size );
	if ( ! newfont )
	{
		QMessageBox :: critical ( this, tr ( "Error" ),
			QString ( tr ( "Unable to open font %1 size %2." )
			).arg ( name ).arg ( size ) );

		return 1;
	}

	cui_font_destroy ( render.font );
	render.font = newfont;

	updateViewConfig ();

	return 0;
}

void pref_change_font (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	ARG_UNUSED ( callback_ptr )
	)
{
	char	const	* name;
	int		size;


	name = prefs_get_string ( GUI_INIFILE_FONT_PANGO_NAME );
	size = prefs_get_int ( GUI_INIFILE_FONT_SIZE );

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

void fe_save_pref_window_prefs (
	mtPrefs		* const	prefs
	)
{
	be_save_pref_window_prefs ( prefs );
	mtQEX :: prefsWindowMirrorPrefs ( prefs_file (), prefs );
}

mtPrefs * fe_load_pref_window_prefs (
	mtPrefTable	const * const	table
	)
{
	mtPrefs		* prefs;


	prefs = mtkit_prefs_new ( table );
	mtQEX :: prefsInitPrefs ( prefs );

	if ( mtQEX :: prefsWindowMirrorPrefs ( prefs, prefs_file () ) )
	{
		mtkit_prefs_destroy ( prefs );

		return NULL;
	}

	be_load_pref_window_prefs ( prefs );

	return prefs;
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

void MainWindow :: pressOptionsCellPrefs ()
{
	CedSheet	* sheet = projectGetSheet ();
	mtPrefs		* prefs;


	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	prefs = be_cellpref_init ( sheet, be_cellpref_changed, (void *)this );
	if ( ! prefs )
	{
		return;
	}

	qexPrefsWindow ( prefs, "Cell Preferences" );

	fe_save_pref_window_prefs ( prefs );

	mtkit_prefs_destroy ( prefs );
	prefs = NULL;

	be_cellpref_cleanup ( sheet );

	updateView ();
}

void MainWindow :: pressOptionsBookPrefs ()
{
	mtPrefs * prefs = be_book_prefs_init ( cedFile->cubook->book );

	qexPrefsWindow ( prefs, "Book Preferences" );
	fe_save_pref_window_prefs ( prefs );
	be_book_prefs_finish ( prefs, cedFile->cubook->book );

	mtkit_prefs_destroy ( prefs );
}

void MainWindow :: pressOptionsProgramPrefs ()
{
	qexPrefsWindow ( prefs_file (), "Preferences" );
}

void MainWindow :: pressOptionsTextStyle (
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
	int		const	pref_id
	)
{
	if ( be_prepare_prefs_set ( mainwindow->projectGetSheet () ) )
	{
		return;
	}


	SwatchDialog	dialog ( title );
	int		col = dialog.getColor ();


	if ( col >= 0 )
	{
		fe_commit_prefs_set ( pref_id, col, NULL, 0, NULL );
	}

	be_cellpref_cleanup ( mainwindow->projectGetSheet () );
}

void MainWindow :: pressOptionsBackgroundColor ()
{
	swFunc ( tr ( "Background Colour" ), CUI_CELLPREFS_color_background );
}

void MainWindow :: pressOptionsForegroundColor ()
{
	swFunc ( tr ( "Foreground Colour" ), CUI_CELLPREFS_color_foreground );
}

void MainWindow :: pressOptionsBorderColor ()
{
	swFunc ( tr ( "Border Colour" ), CUI_CELLPREFS_border_color );
}

void MainWindow :: pressOptionsBorder (
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

