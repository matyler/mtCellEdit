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



MainWindow::MainWindow ()
{
	setWindowTitle ( VERSION );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/scalable/apps/"
		BIN_NAME ".svg" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	QWidget * widget = new QWidget;
	setCentralWidget ( widget );

	create_menu ();

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setContentsMargins ( 5, 5, 5, 5 );

	m_tab_widget = new QTabWidget;
	vbox->addWidget ( m_tab_widget );

	widget->setLayout ( vbox );

	m_prefs.add_int ( PREFS_WINDOW_X, m_win_x, 50 );
	m_prefs.add_int ( PREFS_WINDOW_Y, m_win_y, 50 );
	m_prefs.add_int ( PREFS_WINDOW_W, m_win_w, 500 );
	m_prefs.add_int ( PREFS_WINDOW_H, m_win_h, 500 );

	m_prefs.load ( nullptr, BIN_NAME );

	setMinimumSize ( 160, 160 );

	setGeometry ( m_win_x, m_win_y, m_win_w, m_win_h );

	show ();
}

MainWindow::~MainWindow ()
{
	// Saved to disk by m_prefs
	m_win_x = geometry().x ();
	m_win_y = geometry().y ();
	m_win_w = geometry().width ();
	m_win_h = geometry().height ();
}

void MainWindow::analyse ( char const * const path )
{
	std::string new_path = raft_path_check ( path );
	if ( new_path.empty() )
	{
		return;
	}

	CedSheet * sheet = nullptr;

	mtQEX::BusyDialog busy ( this, nullptr,
	[this, &busy, &sheet, new_path]()
	{
		raft_scan_sheet ( new_path, &sheet, *busy.get_busy() );
	});

	busy.show_abort ();
	busy.wait_for_thread ();

	if ( sheet )
	{
		m_tab_id++;

		QWidget * widget = new QWidget;
		m_tab_widget->addTab( widget, QString ("%1").arg (m_tab_id) );
		m_tab_widget->setCurrentWidget ( widget );

		widget->setProperty ( TABLE_ID, m_tab_id );

		QVBoxLayout * vbox = new QVBoxLayout;
		vbox->setContentsMargins ( 0, 0, 0, 0 );
		widget->setLayout ( vbox );

		TableAnalysis * const tab = new TableAnalysis ( sheet, new_path,
			*this, vbox );

		m_table_map.insert ( m_tab_id, tab );

		mtQEX::process_qt_pending ();

		tab->setFocus ();
	}
}

void MainWindow::closeEvent (
	QCloseEvent	* const	ev
	)
{
	ev->accept ();
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	char const * path = NULL;

	if ( raft_cline ( argc, argv, &path ) )
	{
		return 0;
	}

	// I don't want Qt snooping or changing my command line.
	int	dummy_argc	= 1;
	char	dummy_str[1]	= { 0 };
	char	* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	MainWindow	window;

	window.analyse ( path );

	return app.exec ();
}

