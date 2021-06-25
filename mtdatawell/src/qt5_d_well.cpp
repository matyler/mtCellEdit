/*
	Copyright (C) 2018-2020 Mark Tyler

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



void Mainwindow::press_well_info ()
{
	mtDW::Well * const well = backend.db.get_well ();

	if ( ! well )
	{
		return;
	}

	DialogWellInfo ( *this );

	well->save_state ();
}

void Mainwindow::press_well_reset ()
{
	mtDW::Well * const well = backend.db.get_well ();

	if ( ! well )
	{
		return;
	}

	int const res = QMessageBox::question ( this, "Question",
		"Do you really want to reset the well?",
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

	if ( res == QMessageBox::Yes )
	{
		well->empty ();
		well->set_seed_by_time ();
		well->set_shifts ();
		well->save_state ();
	}
}



#define BUTTON_COL	0
#define LABEL_COL	1
#define INFO_COL	2



DialogWellInfo::DialogWellInfo (
	Mainwindow	&mw
	)
	:
	QDialog		( &mw ),
	m_path		(),
	m_seed		(),
	m_shifts	(),
	m_files_done	(),
	m_files_todo	(),
	m_files_empty	(),
	mainwindow	( mw )
{
	QGroupBox	* groupBox;
	QVBoxLayout	* gvb;
	QHBoxLayout	* hbox;
	QGridLayout	* grid;
	QPushButton	* button;
	QWidget		* w;

	setWindowTitle ( "Well Information" );
	setModal ( true );

	QVBoxLayout * vbox = new QVBoxLayout;
	setLayout ( vbox );

	groupBox = new QGroupBox ( "Settings" );
	vbox->addWidget ( groupBox );

	gvb = new QVBoxLayout;

	groupBox->setLayout ( gvb );

	w = new QWidget;
	gvb->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );
	grid->setColumnStretch ( INFO_COL, 1 ); // Stretch column INFO_COL

	grid->addWidget ( new QLabel ( "Path" ), 0, LABEL_COL );
	grid->addWidget ( new QLabel ( "Seed" ), 1, LABEL_COL );
	grid->addWidget ( new QLabel ( "Shifts" ), 2, LABEL_COL );
	grid->addWidget ( new QLabel ( "Files done" ), 3, LABEL_COL );
	grid->addWidget ( new QLabel ( "Files to do" ), 4, LABEL_COL );

	m_path = new QLineEdit;
	m_path->setReadOnly ( true );
	grid->addWidget ( m_path, 0, INFO_COL );

	m_seed = new QLabel;
	grid->addWidget ( m_seed, 1, INFO_COL );

	m_shifts = new QLabel;
	grid->addWidget ( m_shifts, 2, INFO_COL );

	m_files_done = new QLabel;
	grid->addWidget ( m_files_done, 3, INFO_COL );

	m_files_todo = new QLabel;
	grid->addWidget ( m_files_todo, 4, INFO_COL );

	w = new QWidget;
	gvb->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme("view-refresh"),
		"Reset Seed" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_seed_reset () ) );

	button = new QPushButton ( QIcon::fromTheme("view-refresh"),
		"Reset Shifts" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_shifts_reset () ) );

	button = new QPushButton ( QIcon::fromTheme ( "edit-clear" ),
		"Empty Files" );
	m_files_empty = button;
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_files_empty () ) );

	button = new QPushButton ( QIcon::fromTheme ( "list-add" ),
		"Add Files" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_files_add () ) );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	button_box->setFocus ();

	repopulate ();
	exec ();
}

void DialogWellInfo::press_seed_reset () const
{
	mainwindow.backend.db.get_well ()->set_seed_by_time ();
	repopulate ();
}

void DialogWellInfo::press_shifts_reset () const
{
	mainwindow.backend.db.get_well ()->set_shifts ();
	repopulate ();
}

void DialogWellInfo::press_files_empty ()
{
	int const res = QMessageBox::question ( this, "Question",
		"Do you really want to empty the well files?",
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

	if ( res == QMessageBox::Yes )
	{
		mainwindow.backend.db.get_well ()->empty ();
		repopulate ();
	}
}



class WellScanThread : public QThread
{
public:
	WellScanThread (
		mtQEX::BusyDialog	&busy,
		char	const * const	path,
		mtDW::Well		&well
		)
		:
		m_path	( path ),
		m_busy	( busy ),
		m_well	( well )
	{
	}

	void run ()
	{
		m_well.add_path ( m_path.c_str () );
	}

private:
	std::string	const	m_path;
	mtQEX::BusyDialog	&m_busy;
	mtDW::Well		&m_well;
};



void DialogWellInfo::press_files_add ()
{
	mtKit::UserPrefs &uprefs = mainwindow.uprefs;
	std::string const & dir = uprefs.get_string ( PREFS_LAST_DIRECTORY );
	char const * const last_dir = dir.size() > 0 ? dir.c_str() :
		mtkit_file_home();

	QFileDialog dialog ( this, "Select Directories to Scan",
		mtQEX::qstringFromC ( last_dir ) );

	mtQEX::set_multi_directories ( dialog );

	if ( dialog.exec () )
	{
		mtQEX::BusyDialog busy ( this );

		QStringList const files = dialog.selectedFiles ();

		for ( int i = 0; i < files.size (); i++ )
		{
			WellScanThread work ( busy, files.at(i).toUtf8().data(),
				*mainwindow.backend.db.get_well () );

			work.start ();
			busy.wait_for_thread ( &work );
		}

		uprefs.set ( PREFS_LAST_DIRECTORY, dialog.directory().
			absolutePath ().toUtf8 ().data () );

		repopulate ();
	}
}

void DialogWellInfo::repopulate () const
{
	mtDW::Well * const well = mainwindow.backend.db.get_well ();

	m_path->setText ( mtQEX::qstringFromC( mainwindow.backend.db.get_path ()
		.c_str () ) );
	m_seed->setText ( QString::number ( well->get_seed () ) );

	int shifts[8] = { 0 };
	char txt[128];

	well->get_shifts ( shifts );
	snprintf ( txt, sizeof(txt), "%i, %i, %i, %i, %i, %i, %i, %i",
		shifts[0], shifts[1], shifts[2], shifts[3],
		shifts[4], shifts[5], shifts[6], shifts[7] );

	m_shifts->setText ( mtQEX::qstringFromC ( txt ) );

	int const done = well->get_files_done ();
	int const todo = well->get_files_todo ();

	m_files_done->setText ( QString::number ( done ) );
	m_files_todo->setText ( QString::number ( todo ) );

	if ( (done + todo) > 0 )
	{
		m_files_empty->setEnabled ( true );
	}
	else
	{
		m_files_empty->setEnabled ( false );
	}
}

