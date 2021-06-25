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



void MainWindow::create_menu ()
{
	QAction * act_menu_open;
	QAction * act_menu_close;
	QAction * act_menu_next;
	QAction * act_menu_previous;
	QAction * act_menu_copy;
	QAction * act_menu_quit;

	QEX_MENU( menu_open, "Open ...", "Ctrl+O", "document-open" )
	QEX_MENU( menu_close, "Close", "Ctrl+W", "window-close" )
	QEX_MENU( menu_next, "Next", "Ctrl+Page Up", "go-next" )
	QEX_MENU( menu_previous, "Previous","Ctrl+Page Down","go-previous")
	QEX_MENU( menu_copy, "Copy", "Ctrl+C", "edit-copy" )
	QEX_MENU( menu_quit, "Quit", "Ctrl+Q", "application-exit" )

	QMenu * menu = menuBar ()->addMenu ( "&Menu" );
	menu->setTearOffEnabled ( true );
	menu->addAction ( act_menu_open );
	menu->addAction ( act_menu_close );
	menu->addSeparator ();
	menu->addAction ( act_menu_next );
	menu->addAction ( act_menu_previous );
	menu->addSeparator ();
	menu->addAction ( act_menu_copy );
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

void MainWindow::press_menu_open ()
{
	QString const filename = QFileDialog::getExistingDirectory ( this,
		"Select Directory to Analyse", NULL,
		QFileDialog::DontUseNativeDialog );

	if ( ! filename.isEmpty () )
	{
		analyse ( filename.toUtf8 ().data () );
	}
}

void MainWindow::press_menu_close ()
{
	QWidget * const w = m_tab_widget->currentWidget ();

	if ( w )
	{
		int const table_id = w->property ( TABLE_ID ).toInt ();

		m_table_map.remove ( table_id );
	}

	m_tab_widget->removeTab ( m_tab_widget->currentIndex () );
}

void MainWindow::press_menu_next ()
{
	m_tab_widget->setCurrentIndex ( m_tab_widget->currentIndex () - 1 );
}

void MainWindow::press_menu_previous ()
{
	m_tab_widget->setCurrentIndex ( m_tab_widget->currentIndex () + 1 );
}

void MainWindow::press_menu_copy ()
{
	QWidget * const w = m_tab_widget->currentWidget ();

	if ( w )
	{
		int const table_id = w->property ( TABLE_ID ).toInt ();

		TableAnalysis * const tab = m_table_map.value ( table_id );

		if ( tab )
		{
			tab->copy_to_clipboard ();
		}
	}
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

