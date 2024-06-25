/*
	Copyright (C) 2024 Mark Tyler

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

#include "wasp_qt.h"



void MainWindow::create_menu ()
{
	QAction * act_file_new;
	QAction * act_file_open;
	QAction * act_file_save;
	QAction * act_file_save_as;
	QAction * act_file_export_wave;
	QAction * act_file_quit;

	QEX_MENU( file_new, "New", "Ctrl+N", "document-new" )
	QEX_MENU( file_open, "Open ...", "Ctrl+O", "document-open" )
	QEX_MENU( file_save, "Save", "Ctrl+S", "document-save" )
	QEX_MENU( file_save_as, "Save As ...", "Shift+Ctrl+S", "document-save-as" )
	QEX_MENU( file_export_wave, "Export Wave ...", nullptr, "document-save-as" )
	QEX_MENU( file_quit, "Quit", "Ctrl+Q", "application-exit" )

	QMenu * menu = menuBar ()->addMenu ( "&File" );
	menu->setTearOffEnabled ( true );
	menu->addAction ( act_file_new );
	menu->addAction ( act_file_open );
	menu->addSeparator ();
	menu->addAction ( act_file_save );
	menu->addAction ( act_file_save_as );
	menu->addSeparator ();
	menu->addAction ( act_file_export_wave );
	act_file_recent_separator = menu->addSeparator ();

	for ( size_t i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		act_file_recent[i] = new QAction ( "", this );

		connect ( act_file_recent[i], &QAction::triggered, [i,this]
			{
				press_file_recent ( i + 1 );
			} );

		if ( i < 10 )
		{
			act_file_recent[i]->setShortcut (
				QString ( "Ctrl+%1" ).arg ( (i + 1) % 10 ) );
		}

		menu->addAction ( act_file_recent[i] );
	}

	menu->addSeparator ();
	menu->addAction ( act_file_quit );

/// ----------------------------------------------------------------------------

	QAction * act_edit_prefs;

	QEX_MENU ( edit_prefs, "Preferences ...", "Ctrl+P", "preferences-other")

	QMenu * const edit_menu = menuBar ()->addMenu ( "&Edit" );
	edit_menu->setTearOffEnabled ( true );

	edit_menu->addAction ( act_edit_prefs );

/// ----------------------------------------------------------------------------

	QAction * act_audio_play;
	QAction * act_audio_stop;
	QAction * act_audio_set_audio_device;

	QEX_MENU( audio_play, "Play", "F4", "media-playback-start" )
	QEX_MENU( audio_stop, "Stop", "F5", "media-playback-stop" )
	QEX_MENU( audio_set_audio_device, "Set Audio Device...", NULL,
		"preferences-other" )

	menu = menuBar ()->addMenu ( "&Audio" );
	menu->setTearOffEnabled ( true );
	menu->addAction ( act_audio_play );
	menu->addAction ( act_audio_stop );
	menu->addSeparator ();
	menu->addAction ( act_audio_set_audio_device );

/// ----------------------------------------------------------------------------

	QAction * act_help_about_qt;
	QAction * act_help_about;

	QEX_MENU( help_about_qt, "About Qt ...", NULL, NULL )
	QEX_MENU( help_about, "About ...", "F1", "help-about" )

	QMenu * help_menu = menuBar ()->addMenu ( "&Help" );
	help_menu->setTearOffEnabled ( true );

	help_menu->addAction ( act_help_about_qt );
	help_menu->addAction ( act_help_about );
}

void MainWindow::press_edit_prefs ()
{
	mtQEX::prefs_window ( this, m_uprefs, "Preferences" );
}

void MainWindow::press_help_about_qt ()
{
	QMessageBox::aboutQt ( this );
}

void MainWindow::press_help_about ()
{
	mtQEX::DialogAbout dialog ( this, VERSION );

	dialog.add_info ( "About",
		VERSION"\n"
	"\n"
	"Copyright (C) " MT_COPYRIGHT_YEARS " Mark Tyler.\n"
	"\n"
	"https://www.marktyler.org/\n"
	"\n"
	"This program is free software: you can redistribute it and/or modify "
	"it under the terms of the GNU General Public License as published by "
	"the Free Software Foundation, either version 3 of the License, or "
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful, "
	"but WITHOUT ANY WARRANTY; without even the implied warranty of "
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License "
	"along with this program.  If not, see http://www.gnu.org/licenses/\n"
		);

	dialog.exec ();
}

