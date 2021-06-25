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

	DialogBottleInfo ( *this, list.first () );
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
	setModal ( true );
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

	std::string const directory = m_combo->get_directory().toUtf8().data();
	int error = 0;

	mtQEX::BusyDialog busy ( this, nullptr,
	[this, &error, &directory, &bottle_list, &file_list]()
	{
		mtDW::Well	* const	well = backend.db.get_well ();
		mtDW::Butt	* const	butt = backend.db.get_butt ();
		mtDW::Soda	* const	soda = backend.db.get_soda ();
		mtDW::Tap	* const	tap = backend.db.get_tap ();

		try
		{
			for ( int i = 0; i < file_list.size (); i++ )
			{
				std::string const &bottle = bottle_list.at (i);
				std::string const &input = file_list.at (i);
				std::string output ( directory );

				output += MTKIT_DIR_SEP;
				output += mtKit::basename ( input );

				mtDW::TapFile	tapfile;
				int		type;

				error = tapfile.open_info(bottle.c_str(), type);
				if ( error )
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

				error = tap->encode ( well, butt, soda,
					bottle.c_str (), input.c_str (),
					output.c_str () );

				if ( error )
				{
					return;
				}
			}
		}
		catch ( ... )
		{
			error = -1;
		}
	});

	busy.wait_for_thread ();

	report_lib_error ( this, error );
}

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
	int error = 0;

	mtQEX::BusyDialog busy ( this, nullptr,
	[this, &error, &directory, &list]()
	{
		mtDW::Butt	* const	butt = backend.db.get_butt ();

		try
		{
			for ( int i = 0; i < list.size (); i++ )
			{
				std::string const &input = list.at ( i );

				std::string output ( directory );
				output += MTKIT_DIR_SEP;
				output += mtKit::basename ( input );

				if ( 0 == mtKit::string_strip_extension (output,
					"flac") )
				{
					mtKit::string_strip_extension ( output,
						"png" );
				}

				error = mtDW::Tap::multi_decode ( butt,
					input.c_str (), output.c_str () );

				if ( error )
				{
					return;
				}
			}
		}
		catch ( ... )
		{
			error = -1;
		}
	});

	busy.wait_for_thread ();

	report_lib_error ( this, error );
}

