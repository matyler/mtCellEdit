/*
	Copyright (C) 2013-2018 Mark Tyler

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

#include "qt4.h"



MainWindow::MainWindow ()
	:
	m_tab_id	( 0 ),
	m_tab_widget	()
{
	setWindowTitle ( VERSION );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/256x256/apps/"
		BIN_NAME ".png" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	QWidget * widget = new QWidget;
	setCentralWidget ( widget );

	create_menu ();

	QVBoxLayout * vbox = new QVBoxLayout;
	vbox->setMargin ( 5 );

	m_tab_widget = new QTabWidget;
	vbox->addWidget ( m_tab_widget );

	widget->setLayout ( vbox );

	mtPrefTable const prefs_table[] = {
	{ PREFS_WINDOW_X, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_Y, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_W, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_H, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	prefs.addTable ( prefs_table );
	prefs.load ( NULL, BIN_NAME );

	setMinimumSize ( 160, 160 );

	setGeometry ( prefs.getInt ( PREFS_WINDOW_X ),
		prefs.getInt ( PREFS_WINDOW_Y ),
		prefs.getInt ( PREFS_WINDOW_W ),
		prefs.getInt ( PREFS_WINDOW_H ) );

	show ();
}

MainWindow::~MainWindow ()
{
	prefs.set ( PREFS_WINDOW_X, geometry().x () );
	prefs.set ( PREFS_WINDOW_Y, geometry().y () );
	prefs.set ( PREFS_WINDOW_W, geometry().width () );
	prefs.set ( PREFS_WINDOW_H, geometry().height () );
}



// Forward declaration
static int raftScan ( void * user_data );



class workerThread : public QThread
{
public:
	workerThread ( mtQEX::BusyDialog &busy, char const * const path )
		:
		m_sheet	( NULL ),
		m_busy	( busy ),
		m_path	( path )
	{
	}

	~workerThread ()
	{
	}

	void run ()
	{
		if ( raft_scan_sheet ( m_path, &m_sheet, raftScan, this ) )
		{
			// Fail
		}
	}

/// ----------------------------------------------------------------------------

	CedSheet		* m_sheet;
	mtQEX::BusyDialog	&m_busy;

private:
	char	const * const	m_path;
};



static int raftScan (
	void	* const	user_data
	)
{
	workerThread * const worker = static_cast<workerThread *>(user_data);

	if ( worker->m_busy.aborted () )
	{
		return 1;		// User wants to stop
	}

	return 0;			// Keep scanning
}

void MainWindow::analyse (
	char	const * const	path
	)
{
	char * new_path = raft_path_check ( path );
	if ( ! new_path )
	{
		return;
	}

	mtQEX::BusyDialog busy ( this );
	busy.show_abort ();

	workerThread work ( busy, new_path );
	work.start ();

	busy.wait_for_thread ( work );

	if ( work.m_sheet )
	{
		m_tab_id++;

		QWidget * widget = new QWidget;
		m_tab_widget->addTab( widget, QString ("%1").arg (m_tab_id) );
		m_tab_widget->setCurrentWidget ( widget );

		widget->setProperty ( TABLE_ID, m_tab_id );

		QVBoxLayout * vbox = new QVBoxLayout;
		vbox->setMargin ( 0 );
		widget->setLayout ( vbox );

		TableAnalysis * const tab = new TableAnalysis ( work.m_sheet,
			new_path, *this, vbox );

		m_table_map.insert ( m_tab_id, tab );

		mtQEX::process_qt_pending ();

		tab->setFocus ();
	}

	free ( new_path );
	new_path = NULL;
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

