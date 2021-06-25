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



int MainWindow::projectSetFont (
	char	const	* const	name,
	int		const	sz
	)
{
	std::unique_ptr<mtPixy::Font> newfont ( new mtPixy::Font ( name, sz ) );

	if ( ! newfont )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to open font %1 size %2."
			).arg ( name ).arg ( sz ) );

		return 1;
	}

	cui_render.font = newfont.get();
	cui_font.reset ( newfont.release() );

	updateViewConfig ();

	return 0;
}

void MainWindow::pref_change_font ()
{
	if ( projectSetFont ( mprefs.font_pango_name.c_str(), mprefs.font_size))
	{
		projectSetFont ( "Sans", 16 );
	}
}

void MainWindow::commit_prefs_set (
	int		const	pref_id,
	int		const	pref_num,
	char	const	* const	pref_charp,
	int		const	change_cursor,
	CuiCellPrefChange	& change
	)
{
	CedSheet * const sheet = projectGetSheet ();
	if ( ! sheet )
	{
		return;
	}

	int const res = change.commit_prefs_set ( sheet,
		projectGetCedFile ()->cubook,
		pref_id, pref_num, pref_charp );

	projectReportUpdates ( res );

	if ( change.commit_prefs_set_check( res, sheet, change_cursor, pref_id))
	{
		return;
	}

	updateChangesChores ( 1, 1 );
}

void MainWindow::press_OptionsCellPrefs ()
{
	CedSheet * const sheet = projectGetSheet ();
	if ( projectReportUpdates ( cui_check_sheet_lock ( sheet ) ) )
	{
		return;
	}

	CuiCellPrefChange	change;
	if ( change.init ( sheet ) )
	{
		QMessageBox::critical ( this, "Error", "Unable to set cell "
			"prefs" );
		return;
	}

	mtKit::UPrefUIEdit	ui_edit;
	mtKit::UserPrefs	prefs;

	prefs.add_ui_defaults ( ui_edit );
	backend->ui_shared_prefs_init ( prefs );

	change.cellpref_init ( prefs );

	auto cb_gui = [this, &change]( int const code, int num,
		char const * const chp )
		{
			commit_prefs_set ( code, num, chp, 1, change );
		};

	prefs.set_callback ( CED_FILE_PREFS_CELL_ALIGN_HORIZONTAL,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_align_horizontal,
				change.cell_vals.align_horizontal, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_COLOR_BACKGROUND,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_color_background,
				change.cell_vals.color_background, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_COLOR_FOREGROUND,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_color_foreground,
				change.cell_vals.color_foreground, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_FORMAT,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_format, change.cell_vals.format,
				nullptr);
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_FORMAT_DATETIME,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_format_datetime, 0,
				change.cell_format_datetime.c_str() );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_NUM_DECIMAL_PLACES,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_num_decimal_places,
				change.cell_vals.num_decimal_places, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_NUM_THOUSANDS,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_num_thousands, 0,
				change.cell_num_thousands.c_str() );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_NUM_ZEROS,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_num_zeros,
				change.cell_vals.num_zeros, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_TEXT_STYLE,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_text_style,
				change.cell_vals.text_style, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_LOCKED,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_locked,
				change.cell_vals.locked, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_BORDER_TYPE,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_border_type,
				change.cell_vals.border_type, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_BORDER_COLOR,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_border_color,
				change.cell_vals.border_color, nullptr );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_TEXT_PREFIX,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_text_prefix, 0,
				change.cell_text_prefix.c_str() );
		} );

	prefs.set_callback ( CED_FILE_PREFS_CELL_TEXT_SUFFIX,
		[&cb_gui, &change]()
		{
			cb_gui ( CUI_CELLPREFS_text_suffix, 0,
				change.cell_text_suffix.c_str() );
		} );

	mtQEX::prefs_window ( this, prefs, "Cell Preferences" );

	backend->ui_shared_prefs_finish ( prefs );

	updateView ();
}

void MainWindow::press_OptionsBookPrefs ()
{
	CedBook		* const	book = cedFile->cubook->book;

	std::string	book_author;
	std::string	book_comment;
	int		disable_locks;
	int		auto_recalc;

	bool		changed = false;
	auto set_changed = [&changed](){ changed = true; };

	mtKit::UPrefUIEdit	ui_edit;
	mtKit::UserPrefs	prefs;

	prefs.add_ui_defaults ( ui_edit );

	backend->ui_shared_prefs_init ( prefs );

	prefs.add_string_multi ( CED_FILE_PREFS_BOOK_AUTHOR, book_author, "" );
	prefs.add_string_multi ( CED_FILE_PREFS_BOOK_COMMENT, book_comment, "");
	prefs.add_bool ( CED_FILE_PREFS_BOOK_DISABLE_LOCKS, disable_locks, 0 );
	prefs.add_option ( CED_FILE_PREFS_BOOK_AUTO_RECALC, auto_recalc, 1,
		{"None", "Sheet", "Book"} );

	book_author = book->prefs.author ? book->prefs.author : "";
	book_comment = book->prefs.comment ? book->prefs.comment : "";
	disable_locks = book->prefs.disable_locks;
	auto_recalc = book->prefs.auto_recalc;

	prefs.set_description ( CED_FILE_PREFS_BOOK_AUTHOR, "Book Author" );
	prefs.set_description ( CED_FILE_PREFS_BOOK_COMMENT, "Book Comment" );
	prefs.set_description ( CED_FILE_PREFS_BOOK_DISABLE_LOCKS,
		"Disable all cell locks" );
	prefs.set_description ( CED_FILE_PREFS_BOOK_AUTO_RECALC,
		"Automatically recalculate the sheet or book after changes" );

	prefs.set_callback ( CED_FILE_PREFS_BOOK_AUTHOR, set_changed );
	prefs.set_callback ( CED_FILE_PREFS_BOOK_COMMENT, set_changed );
	prefs.set_callback ( CED_FILE_PREFS_BOOK_DISABLE_LOCKS, set_changed );
	prefs.set_callback ( CED_FILE_PREFS_BOOK_AUTO_RECALC, set_changed );

	mtQEX::prefs_window ( this, prefs, "Book Preferences" );

	backend->ui_shared_prefs_finish ( prefs );

	if ( changed )
	{
		mtkit_strfreedup ( &book->prefs.author, book_author.c_str() );
		mtkit_strfreedup ( &book->prefs.comment, book_comment.c_str() );

		book->prefs.disable_locks = disable_locks;
		book->prefs.auto_recalc = auto_recalc;

		setChangesFlag ();
	}
}

void MainWindow::press_OptionsProgramPrefs ()
{
	mtQEX::prefs_window ( this, uprefs, "Preferences" );
}

void MainWindow::press_OptionsTextStyle ( int const i )
{
	int const res = cui_cellprefs_text_style ( cedFile, i );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		QMessageBox::critical ( this, "Error", "Unable to set style" );
		return;
	}

	projectReportUpdates ( res );
	updateChangesChores ( 1, 1 );
}

void MainWindow::swatch_dialog (
	char	const * const	title,
	int		const	pref_id
	)
{
	CuiCellPrefChange	change;

	if ( change.init ( projectGetSheet () ) )
	{
		return;
	}

	SwatchDialog	dialog ( title, mprefs.font_size );
	int	const	col = dialog.getColor ();

	if ( col >= 0 )
	{
		commit_prefs_set ( pref_id, col, NULL, 0, change );
	}
}

void MainWindow::press_OptionsBackgroundColor ()
{
	swatch_dialog ( "Background Colour", CUI_CELLPREFS_color_background );
}

void MainWindow::press_OptionsForegroundColor ()
{
	swatch_dialog ( "Foreground Colour", CUI_CELLPREFS_color_foreground );
}

void MainWindow::press_OptionsBorderColor ()
{
	swatch_dialog ( "Border Colour", CUI_CELLPREFS_border_color );
}

void MainWindow::press_OptionsBorder ( int const i )
{
	int const res = cui_cellprefs_border ( cedFile, i );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		QMessageBox::critical ( this, "Error", "Unable to set border" );
		return;
	}

	projectReportUpdates ( res );
	updateChangesChores ( 1, 1 );
}

