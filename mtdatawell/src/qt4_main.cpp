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

#include "qt4.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Backend be;

	if ( be.command_line ( argc, argv ) )
	{
		return be.exit.value ();
	}

	// I don't want Qt snooping or changing my command line.
	int		dummy_argc	= 1;
	char		dummy_str[1]	= { 0 };
	char		* dummy_argv	= dummy_str;
	QApplication	app ( dummy_argc, &dummy_argv );
	Mainwindow	window ( be );

	if ( 0 == be.exit.value () )
	{
		app.exec ();
	}

	return be.exit.value ();
}

Mainwindow::Mainwindow ( Backend &be )
	:
	backend			( be ),
	prefs			( be.prefs ),
	m_statusbar_db		( NULL ),
	m_statusbar_butt	( NULL ),
	m_statusbar_soda	( NULL ),
	m_well_menu		( NULL ),
	m_butt_menu		( NULL ),
	m_soda_menu		( NULL ),
	m_tap_menu		( NULL ),
	m_apps_menu		( NULL ),
	act_db_recent		(),
	act_soda_encrypt	( NULL ),
	m_combo			( NULL ),
	m_input			( NULL ),
	m_bottles		( NULL )
{
	setEnabled ( false );

	setWindowTitle ( VERSION );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/256x256/apps/"
		BIN_NAME ".png" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	create_menu ();
	create_statusbar ();
	create_main_ui ();

	setMinimumSize ( 160, 160 );
	setGeometry ( prefs.getInt ( PREFS_WINDOW_X ),
		prefs.getInt ( PREFS_WINDOW_Y ),
		prefs.getInt ( PREFS_WINDOW_W ),
		prefs.getInt ( PREFS_WINDOW_H ) );

	if ( 1 == prefs.getInt ( PREFS_WINDOW_MAXIMIZED ) )
	{
		showMaximized ();
	}
	else
	{
		show ();
	}

	std::vector<char const *> const &files = backend.get_cline_files ();

	for ( size_t i = 0; i < files.size (); i++ )
	{
		m_input->add_filename ( files[i] );
	}

	m_input->resize_columns ();
	m_input->update_title ();

	setEnabled ( true );

	init_well_success ( backend.init_well () );
}

Mainwindow::~Mainwindow ()
{
	if ( isMaximized () )
	{
		prefs.set ( PREFS_WINDOW_MAXIMIZED, 1 );
	}
	else
	{
		prefs.set ( PREFS_WINDOW_MAXIMIZED, 0 );

		prefs.set ( PREFS_WINDOW_X, geometry().x () );
		prefs.set ( PREFS_WINDOW_Y, geometry().y () );
		prefs.set ( PREFS_WINDOW_W, geometry().width () );
		prefs.set ( PREFS_WINDOW_H, geometry().height () );
	}
}

void Mainwindow::init_well_success ( int const fail )
{
	if ( fail )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to open the database from the filesystem. "
			"Another process could have locked the database. "
			"Try again by opening a different database." );
	}
	else
	{
		m_well_menu->setEnabled ( true );
		m_butt_menu->setEnabled ( true );
		m_soda_menu->setEnabled ( true );
		m_tap_menu->setEnabled ( true );
		m_apps_menu->setEnabled ( true );
	}

	update_statusbar ();
	update_recent_db_menu ();
}

void Mainwindow::database_load ( std::string const & path )
{
	init_well_success ( backend.open_database ( path.c_str () ) );
}

void Mainwindow::create_statusbar ()
{
	QStatusBar	* const	sb = statusBar ();

	sb->setStyleSheet ( "QStatusBar::item { border: 0px}" );

///	LEFT

	m_statusbar_db = new QLabel;
	sb->addWidget ( m_statusbar_db );

	QWidget * const w = new QWidget;
	w->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Preferred );
	sb->addWidget ( w );

	m_statusbar_butt = new QLabel;
	sb->addWidget ( m_statusbar_butt );

///	RIGHT

	m_statusbar_soda = new QLabel;
	sb->addPermanentWidget ( m_statusbar_soda );
}

void Mainwindow::update_statusbar ()
{
	mtDW::Well * well = backend.db.get_well ();
	mtDW::Butt * butt = backend.db.get_butt ();
	mtDW::Soda * soda = backend.db.get_soda ();

	if ( ! well || ! soda || ! butt )
	{
		return;
	}

	m_statusbar_db->setText (
		mtQEX::qstringFromC ( backend.db.get_path ().c_str () ) );

	QString qtext = "OTP: ";
	std::string const & otp_name = butt->get_otp_name ();

	qtext += mtQEX::qstringFromC ( otp_name.c_str () );
	m_statusbar_butt->setText ( qtext );

	switch ( soda->get_mode () )
	{
	case 0:
		qtext = "Soda: Encrypt";
		act_soda_encrypt->setChecked ( true );
		break;

	case 1:
		qtext = "Soda: Raw";
		act_soda_encrypt->setChecked ( false );
		break;

	default:
		qtext = "Soda: ???";
		break;
	}

	m_statusbar_soda->setText ( qtext );
}

void Mainwindow::create_main_ui ()
{
	QWidget * const w = new QWidget;
	setCentralWidget ( w );

	QVBoxLayout * layv = new QVBoxLayout ( w );

	/// Top Group

	m_combo = new fileCombo ( prefs, "Output Directory",
		QFileDialog::Directory );

	layv->addWidget ( m_combo );

	m_combo->repopulate ( backend.recent_dir );

	/// Bottom section

	QSplitter * const split = new QSplitter;
	layv->addWidget ( split );

	m_input = new groupList ( prefs, "Input Files" );
	split->addWidget ( m_input );

	m_bottles = new groupList ( prefs, "Input Bottles" );
	split->addWidget ( m_bottles );
}

int Mainwindow::validate_output_dir ()
{
	std::string const dir = mtKit::realpath ( m_combo->get_directory ().
		toUtf8 ().data () );

	if ( ! mtkit_file_directory_exists ( dir.c_str () ) )
	{
		QMessageBox::critical( this, "Error", "Bad output directory." );

		return 1;
	}

	// This directory is valid so add to recent list and repopulate combo

	backend.recent_dir.set_filename ( dir.c_str () );
	m_combo->repopulate ( backend.recent_dir );

	return 0;
}

void Mainwindow::press_database_open ()
{
	QString const filename = QFileDialog::getExistingDirectory ( this,
		"Select Database Path", mtQEX::qstringFromC (
		backend.db.get_path ().c_str () ),
		QFileDialog::DontUseNativeDialog );

	if ( filename.isEmpty () )
	{
		return;
	}

	database_load ( filename.toUtf8 ().data () );
}

void Mainwindow::press_database_preferences ()
{
	mtQEX::PrefsWindow ( prefs.getPrefsMem (), "Preferences" );
}

void Mainwindow::press_database_quit ()
{
	close ();
}

void Mainwindow::press_help_help ()
{
	char const * program = prefs.getString ( PREFS_HELP_BROWSER );

	if ( ! program[0] )
	{
		program = getenv ( "BROWSER" );
	}

	if ( ! program || ! program[0] )
	{
		program = "xdg-open";
	}

	std::string path;
	mtKit::get_data_dir ( path, prefs.getString ( PREFS_HELP_FILE ) );
	char const * const html = path.c_str ();

	if ( ! mtkit_file_readable ( html ) )
	{
		QMessageBox::critical ( this, "Error",
			"I am unable to find the documentation.  "
			"You need to set the correct location in the "
			"Preferences." );

		return;
	}


	QStringList list;


	list << mtQEX::qstringFromC ( html );

	if ( ! QProcess::startDetached ( mtQEX::qstringFromC ( program ),
		list ) )
	{
		QMessageBox::critical ( this, "Error",
			"There was a problem running the HTML browser.  "
			"You need to set the correct program name in the "
			"Preferences window." );

		return;
	}
}

void Mainwindow::press_help_about_qt ()
{
	QMessageBox::aboutQt ( this );
}

void Mainwindow::press_help_about ()
{
	QMessageBox::about ( this, "About",
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
}

int report_lib_error (
	QWidget	* const	parent,
	int	const	error
	)
{
	if ( error )
	{
		QString message = "libmtDataWell problem:\n\n";

		message += mtDW::get_error_text ( error );

		QMessageBox::critical ( parent, "Error", message );
	}

	return error;
}

