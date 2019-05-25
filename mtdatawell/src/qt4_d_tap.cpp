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



class DialogBottleInfo : public QDialog
{
public:
	DialogBottleInfo ( Mainwindow &mw, std::string const &filename );
};



void Mainwindow::press_tap_bottle_info ()
{
	QList<std::string> list;

	if ( m_bottles->get_filenames ( list ) || list.isEmpty () )
	{
		QMessageBox::critical( this, "Error",
			"Unable to get bottle filename." );

		return;
	}

	DialogBottleInfo dialog ( *this, list.first () );
}



#define TITLE_COL	0
#define INFO_COL	1



DialogBottleInfo::DialogBottleInfo (
	Mainwindow		&mw,
	std::string	const	&filename
	)
	:
	QDialog		( &mw )
{
	mtDW::TapFile	tap;
	std::string	type;
	int		filetype;

	if ( report_lib_error( &mw, tap.open_soda( filename.c_str(), filetype)))
	{
		return;
	}

	switch ( filetype )
	{
	case mtDW::TapFile::TYPE_RGB:
		type = "Empty RGB Image";
		break;

	case mtDW::TapFile::TYPE_SND:
		type = "Empty Audio";
		break;

	case mtDW::TapFile::TYPE_RGB_1:
		type = "Soda RGB Image (3bpp)";
		break;

	case mtDW::TapFile::TYPE_SND_1:
		type = "Soda Audio";
		break;

	// Future additions go here

	case mtDW::TapFile::TYPE_INVALID:
		type = "Invalid bottle";
		break;

	default:
		type = "Unknown";
		break;
	}

	setWindowTitle ( "Bottle Information" );
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
	grid->addWidget ( new QLabel ( "Type" ), 1, TITLE_COL );
	grid->addWidget ( new QLabel ( "Capacity (bytes)" ), 2, TITLE_COL );

	QLineEdit * ledit = new QLineEdit;
	ledit->setReadOnly ( true );
	ledit->setText ( mtQEX::qstringFromC ( filename.c_str () ) );

	grid->addWidget ( ledit, 0, INFO_COL );
	grid->addWidget ( new QLabel ( type.c_str () ), 1, INFO_COL );
	grid->addWidget ( new QLabel ( QString::number ( tap.get_capacity () )),
		2, INFO_COL );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	exec ();
}



class TapCreateThread : public QThread
{
public:
	TapCreateThread (
		QList<std::string>	&file_list,
		QList<std::string>	&bottle_list,
		std::string	const 	&output,
		Backend			&be
		)
		:
		m_error		( 0 ),
		m_output	( output ),
		m_file_list	( file_list ),
		m_bottle_list	( bottle_list ),
		m_well		( be.db.get_well () ),
		m_butt		( be.db.get_butt () ),
		m_soda		( be.db.get_soda () ),
		m_tap		( be.db.get_tap () )
	{
	}

	void run ()
	{
		try
		{
			for ( int i = 0; i < m_file_list.size (); i++ )
			{
				std::string const &bottle = m_bottle_list.at ( i );
				std::string const &input = m_file_list.at ( i );
				std::string output ( m_output );

				output += MTKIT_DIR_SEP;
				output += mtKit::basename ( input );

				mtDW::TapFile	tap;
				int		type;

				m_error = tap.open_info( bottle.c_str(), type );
				if ( m_error )
				{
					return;
				}

				switch ( type )
				{
					case mtDW::TapFile::TYPE_RGB:
					case mtDW::TapFile::TYPE_RGB_1:
						output += ".png";
						break;

					case mtDW::TapFile::TYPE_SND:
					case mtDW::TapFile::TYPE_SND_1:
						output += ".flac";
						break;
				}

				m_error = m_tap->encode ( m_well, m_butt, m_soda,
					bottle.c_str (), input.c_str (),
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
	QList<std::string>	&m_file_list;
	QList<std::string>	&m_bottle_list;
	mtDW::Well	* const	m_well;
	mtDW::Butt	* const	m_butt;
	mtDW::Soda	* const	m_soda;
	mtDW::Tap	* const	m_tap;
};



void Mainwindow::press_tap_create_bottle ()
{
	if ( validate_output_dir () )
	{
		return;
	}

	QList<std::string> file_list;

	if ( m_input->get_filenames ( file_list ) )
	{
		QMessageBox::critical( this, "Error",
			"Unable to get input filenames." );

		return;
	}

	QList<std::string> bottle_list;

	if ( m_bottles->get_filenames ( bottle_list ) )
	{
		QMessageBox::critical( this, "Error",
			"Unable to get bottle filenames." );

		return;
	}

	if ( file_list.size () != bottle_list.size () )
	{
		QMessageBox::critical( this, "Error",
			"The number of files MUST equal the number of bottles."
			);

		return;
	}

	std::string const directory = m_combo->get_directory().toUtf8().data ();

	mtQEX::BusyDialog busy ( this );
	TapCreateThread work ( file_list, bottle_list, directory, backend );

	work.start ();
	busy.wait_for_thread ( work );

	report_lib_error ( this, work.error () );
}



class TapExtractThread : public QThread
{
public:
	TapExtractThread (
		QList<std::string>	&list,
		std::string	const 	&output,
		Backend			&be
		)
		:
		m_error		( 0 ),
		m_output	( output ),
		m_list		( list ),
		m_butt		( be.db.get_butt () )
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

				if ( 0 == mtKit::string_strip_extension (output,
					"flac") )
				{
					mtKit::string_strip_extension ( output,
						"png" );
				}

				m_error = mtDW::Tap::multi_decode ( m_butt,
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
};



void Mainwindow::press_tap_extract_file ()
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
	TapExtractThread work ( list, directory, backend );

	work.start ();
	busy.wait_for_thread ( work );

	report_lib_error ( this, work.error () );
}

