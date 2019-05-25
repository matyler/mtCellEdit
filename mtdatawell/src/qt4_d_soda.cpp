/*
	Copyright (C) 2018-2019 Mark Tyler

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



class DialogSodaInfo : public QDialog
{
public:
	DialogSodaInfo ( Mainwindow &mw, std::string const &filename );
};



void Mainwindow::press_soda_info ()
{
	QList<std::string> list;

	if ( m_input->get_filenames ( list ) || list.isEmpty () )
	{
		QMessageBox::critical( this, "Error",
			"Unable to get input filename." );

		return;
	}

	DialogSodaInfo dialog ( *this, list.first () );
}



#define TITLE_COL	0
#define INFO_COL	1



DialogSodaInfo::DialogSodaInfo (
	Mainwindow		&mw,
	std::string	const	&filename
	)
	:
	QDialog		( &mw )
{
	mtDW::SodaFile soda;

	if ( report_lib_error ( &mw, soda.open ( filename.c_str () ) ) )
	{
		return;
	}

	setWindowTitle ( "Soda Information" );
	resize ( mw.width (), 10 );

	QVBoxLayout * vbox = new QVBoxLayout;
	setLayout ( vbox );

	QGroupBox	* groupBox;
	QVBoxLayout	* gvb;
	QGridLayout	* grid;
	QWidget		* w;

	groupBox = new QGroupBox ( "File" );
	vbox->addWidget ( groupBox );

	gvb = new QVBoxLayout;

	groupBox->setLayout ( gvb );

	w = new QWidget;
	gvb->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );
	grid->setColumnStretch ( INFO_COL, 1 ); // Stretch column INFO_COL

	grid->addWidget ( new QLabel ( "Name" ), 0, TITLE_COL );
	grid->addWidget ( new QLabel ( "Soda size" ), 1, TITLE_COL );
	grid->addWidget ( new QLabel ( "Mode" ), 2, TITLE_COL );

	QLineEdit * ledit = new QLineEdit;
	ledit->setReadOnly ( true );
	ledit->setText ( mtQEX::qstringFromC ( filename.c_str () ) );

	grid->addWidget ( ledit, 0, INFO_COL );
	grid->addWidget ( new QLabel ( QString::number ( soda.m_filesize ) ), 1,
		INFO_COL );
	grid->addWidget ( new QLabel ( QString::number ( soda.m_mode_raw ) ), 2,
		INFO_COL );

	if ( ! soda.m_mode_raw )
	{
		grid->addWidget ( new QLabel ( "Butt" ), 3, TITLE_COL );
		grid->addWidget ( new QLabel ( "Bucket" ), 4, TITLE_COL );
		grid->addWidget ( new QLabel ( "Bucket Position"), 5,TITLE_COL);

		grid->addWidget ( new QLabel ( soda.m_otp_name.c_str () ), 3,
			INFO_COL);
		grid->addWidget ( new QLabel ( QString::number( soda.m_bucket)),
			4, INFO_COL );
		grid->addWidget ( new QLabel ( QString::number (
			soda.m_bucket_pos ) ), 5, INFO_COL );
	}

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	exec ();
}



class SodaCreateThread : public QThread
{
public:
	SodaCreateThread (
		QList<std::string>	&list,
		std::string	const 	&output,
		Backend			&be
		)
		:
		m_error		( 0 ),
		m_output	( output ),
		m_list		( list ),
		m_butt		( be.db.get_butt () ),
		m_soda		( be.db.get_soda () )
	{
	}

	void run ()
	{
		try
		{
			for ( int i = 0; i < m_list.size (); i++ )
			{
				std::string const &input = m_list.at ( i );
				std::string output ( m_output );

				output += MTKIT_DIR_SEP;
				output += mtKit::basename ( input );
				output += ".soda";

				m_error = m_soda->encode( m_butt, input.c_str(),
					output.c_str () );

				if ( m_error )
				{
					return;
				}
			}
		}
		catch ( ... )
		{
			m_error = -1;
		}
	}

	inline int error () { return m_error; }

private:
	int			m_error;
	std::string	const	&m_output;
	QList<std::string>	&m_list;
	mtDW::Butt	* const	m_butt;
	mtDW::Soda	* const	m_soda;
};



void Mainwindow::press_soda_create ()
{
	if ( validate_output_dir () )
	{
		return;
	}

	QList<std::string> list;

	if ( m_input->get_filenames ( list ) )
	{
		QMessageBox::critical( this, "Error",
			"Unable to get input filenames." );

		return;
	}

	std::string const directory = m_combo->get_directory().toUtf8().data ();

	mtQEX::BusyDialog busy ( this );
	SodaCreateThread work ( list, directory, backend );

	// Very necessary for large numbers of files
	mtDW::SodaTransaction trans ( *backend.db.get_soda () );

	work.start ();
	busy.wait_for_thread ( work );

	report_lib_error ( this, work.error () );
}



class SodaExtractThread : public QThread
{
public:
	SodaExtractThread (
		QList<std::string>	&list,
		std::string	const 	&output,
		Backend			&be
		)
		:
		m_error		( 0 ),
		m_output	( output ),
		m_list		( list ),
		m_butt		( be.db.get_butt () ),
		m_soda		( be.db.get_soda () )
	{
	}

	void run ()
	{
		try
		{
			for ( int i = 0; i < m_list.size (); i++ )
			{
				std::string const &input = m_list.at ( i );
				std::string output ( m_output );

				output += MTKIT_DIR_SEP;
				output += mtKit::basename ( input );

				mtKit::string_strip_extension ( output, "soda");

				m_error = m_soda->multi_decode ( m_butt,
					input.c_str (), output.c_str () );

				if ( m_error )
				{
					return;
				}
			}
		}
		catch ( ... )
		{
			m_error = -1;
		}
	}

	inline int error () { return m_error; }

private:
	int			m_error;
	std::string	const	&m_output;
	QList<std::string>	&m_list;
	mtDW::Butt	* const	m_butt;
	mtDW::Soda	* const	m_soda;
};



void Mainwindow::press_soda_extract_file ()
{
	if ( validate_output_dir () )
	{
		return;
	}

	QList<std::string> list;

	if ( m_input->get_filenames ( list ) )
	{
		QMessageBox::critical( this, "Error",
			"Unable to get input filenames." );

		return;
	}

	std::string const directory = m_combo->get_directory().toUtf8().data ();

	mtQEX::BusyDialog busy ( this );
	SodaExtractThread work ( list, directory, backend );

	work.start ();
	busy.wait_for_thread ( work );

	report_lib_error ( this, work.error () );
}

void Mainwindow::press_soda_encrypt ()
{
	if ( act_soda_encrypt->isChecked () )
	{
		backend.db.get_soda ()->set_mode ( 0 );
	}
	else
	{
		backend.db.get_soda ()->set_mode ( 1 );
	}

	update_statusbar ();
}

