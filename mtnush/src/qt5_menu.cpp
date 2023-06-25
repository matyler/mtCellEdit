/*
	Copyright (C) 2022-2023 Mark Tyler

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



void MainWindow::create_menu ()
{
	QAction * act_menu_preferences;
	QAction * act_menu_quit;

	QEX_MENU( menu_preferences, "Preferences ...", "Ctrl+P",
		"preferences-other" )
	QEX_MENU( menu_quit, "Quit", "Ctrl+Q", "application-exit" )

	QMenu * menu = menuBar ()->addMenu ( "&Menu" );
	menu->setTearOffEnabled ( true );
	menu->addAction ( act_menu_preferences );
	menu->addSeparator ();
	menu->addAction ( act_menu_quit );


	QAction * act_help_about_qt;
	QAction * act_help_about;

	QEX_MENU( help_about_qt, "About Qt ...", NULL, NULL )
	QEX_MENU( help_about, "About ...", "F1", "help-about" )

	QMenu * help_menu = menuBar ()->addMenu ( "&Help" );
	help_menu->setTearOffEnabled ( true );

	help_menu->addAction ( act_help_about_qt );
	help_menu->addAction ( act_help_about );
}

void MainWindow::press_menu_preferences ()
{
	mtQEX::prefs_window ( this, uprefs, "Preferences" );
}

void MainWindow::press_menu_quit ()
{
	close ();
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

