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



void Mainwindow::create_menu ()
{
	QAction * act_database_open;
	QAction * act_database_preferences;
	QAction * act_database_quit;

	QEX_MENU ( database_open, "Open ...", "Ctrl+O", "document-open" )
	QEX_MENU ( database_preferences, "Preferences ...", "Ctrl+P",
		"preferences-other" )
	QEX_MENU ( database_quit, "Quit", "Ctrl+Q", "application-exit" )

	QMenu * database_menu = menuBar ()->addMenu ( "&Database" );
	database_menu->setTearOffEnabled ( true );
	database_menu->addAction ( act_database_open );
	database_menu->addSeparator ();


	QSignalMapper * signal_mapper = new QSignalMapper ( this );

	for ( int i = 0; i < PREFS_RECENT_DB_TOTAL; i++ )
	{
		act_db_recent[i] = new QAction ( "", this );

		connect ( act_db_recent[i], SIGNAL ( triggered () ),
			signal_mapper, SLOT ( map () ) );
		signal_mapper->setMapping ( act_db_recent [ i ], i + 1 );

		database_menu->addAction ( act_db_recent[i] );
	}

	connect ( signal_mapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( press_db_recent ( int ) ) );

	database_menu->addSeparator ();
	database_menu->addAction ( act_database_preferences );
	database_menu->addSeparator ();
	database_menu->addAction ( act_database_quit );


	QAction * act_well_info;
	QAction * act_well_reset;

	QEX_MENU ( well_info, "Information ...", NULL, "document-properties" )
	QEX_MENU ( well_reset, "Reset ...", NULL, "view-refresh" )

	m_well_menu = menuBar ()->addMenu ( "&Well" );
	m_well_menu->setTearOffEnabled ( true );
	m_well_menu->addAction ( act_well_info );
	m_well_menu->addSeparator ();
	m_well_menu->addAction ( act_well_reset );
	m_well_menu->setEnabled ( false );


	QAction * act_butt_info;
	QAction * act_butt_analysis;

	QEX_MENU ( butt_info, "Information ...", NULL, "document-properties" )
	QEX_MENU ( butt_analysis, "Analysis ...", NULL, NULL )

	m_butt_menu = menuBar ()->addMenu ( "&Butt" );
	m_butt_menu->setTearOffEnabled ( true );
	m_butt_menu->addAction ( act_butt_info );
	m_butt_menu->addAction ( act_butt_analysis );
	m_butt_menu->setEnabled ( false );


	QAction * act_soda_info;
	QAction * act_soda_create;
	QAction * act_soda_extract_file;

	QEX_MENU ( soda_info, "File Information ...",NULL,"document-properties")
	QEX_MENU ( soda_create, "Create File(s)", NULL, "document-new" )
	QEX_MENU ( soda_extract_file, "Extract File(s)", NULL, NULL )
	QEX_MENU ( soda_encrypt, "Encrypt", NULL, NULL )

	m_soda_menu = menuBar ()->addMenu ( "&Soda" );
	m_soda_menu->setTearOffEnabled ( true );
	m_soda_menu->addAction ( act_soda_info );
	m_soda_menu->addAction ( act_soda_create );
	m_soda_menu->addAction ( act_soda_extract_file );
	m_soda_menu->addSeparator ();
	m_soda_menu->addAction ( act_soda_encrypt );
	m_soda_menu->setEnabled ( false );

	act_soda_encrypt->setCheckable ( true );

	QAction * act_tap_bottle_info;
	QAction * act_tap_create_bottle;
	QAction * act_tap_extract_file;

	QEX_MENU ( tap_bottle_info, "Bottle Information ...", NULL,
		"document-properties" )
	QEX_MENU ( tap_create_bottle, "Create Bottle(s)", NULL, "document-new" )
	QEX_MENU ( tap_extract_file, "Extract File(s)", NULL, NULL )

	m_tap_menu = menuBar ()->addMenu ( "&Tap" );
	m_tap_menu->setTearOffEnabled ( true );
	m_tap_menu->addAction ( act_tap_bottle_info );
	m_tap_menu->addAction ( act_tap_create_bottle );
	m_tap_menu->addAction ( act_tap_extract_file );
	m_tap_menu->setEnabled ( false );


	QAction * act_apps_binfile;
	QAction * act_apps_cards;
	QAction * act_apps_coins;
	QAction * act_apps_declist;
	QAction * act_apps_dice;
	QAction * act_apps_unicodefonts;
	QAction * act_apps_homoglyphs;
	QAction * act_apps_intlist;
	QAction * act_apps_numshuff;
	QAction * act_apps_passwords;
	QAction * act_apps_pins;

	QEX_MENU ( apps_binfile, "Binary File ...", NULL, NULL )
	QEX_MENU ( apps_cards, "Card Shuffle ...", NULL, NULL )
	QEX_MENU ( apps_coins, "Coin Toss ...", NULL, NULL )
	QEX_MENU ( apps_declist, "Decimal List ...", NULL, NULL )
	QEX_MENU ( apps_dice, "Dice Rolls ...", NULL, NULL )
	QEX_MENU ( apps_unicodefonts, "Fonts ...", NULL, NULL )
	QEX_MENU ( apps_homoglyphs, "Homoglyphs ...", NULL, NULL )
	QEX_MENU ( apps_intlist, "Integer List ...", NULL, NULL )
	QEX_MENU ( apps_numshuff, "Number Shuffle ...", NULL, NULL )
	QEX_MENU ( apps_passwords, "Passwords ...", NULL, NULL )
	QEX_MENU ( apps_pins, "Pin Numbers ...", NULL, NULL )

	m_apps_menu = menuBar ()->addMenu ( "&Apps" );
	m_apps_menu->setTearOffEnabled ( true );
	m_apps_menu->addAction ( act_apps_binfile );
	m_apps_menu->addAction ( act_apps_cards );
	m_apps_menu->addAction ( act_apps_coins );
	m_apps_menu->addAction ( act_apps_declist );
	m_apps_menu->addAction ( act_apps_dice );
	m_apps_menu->addAction ( act_apps_intlist );
	m_apps_menu->addAction ( act_apps_numshuff );
	m_apps_menu->addAction ( act_apps_passwords );
	m_apps_menu->addAction ( act_apps_pins );
	m_apps_menu->setEnabled ( false );

	QMenu * menuUnicode = m_apps_menu->addMenu ( "Unicode" );
	menuUnicode->setTearOffEnabled ( true );
	menuUnicode->addAction ( act_apps_unicodefonts );
	menuUnicode->addAction ( act_apps_homoglyphs );


	QAction * act_help_help;
	QAction * act_help_about_qt;
	QAction * act_help_about;

	QEX_MENU ( help_help, "Help ...", "F1", "help-contents" )
	QEX_MENU ( help_about_qt, "About Qt ...", NULL, NULL )
	QEX_MENU ( help_about, "About ...", NULL, "help-about" )

	QMenu * help_menu = menuBar ()->addMenu ( "&Help" );
	help_menu->setTearOffEnabled ( true );

	help_menu->addAction ( act_help_help );
	help_menu->addSeparator ();
	help_menu->addAction ( act_help_about_qt );
	help_menu->addAction ( act_help_about );
}

void Mainwindow::press_db_recent ( int const i )
{
	database_load ( backend.recent_db.get_filename ( i ) );
}

void Mainwindow::update_recent_db_menu ()
{
	char buf[ PATH_MAX ];

	for ( int i = 0; i < PREFS_RECENT_DB_TOTAL; i++ )
	{
		if ( 1 == mtkit_snip_filename (
			backend.recent_db.get_filename ( i + 1 ),
			buf, sizeof ( buf ), 80 )
			)
		{
			// Hide if empty
			act_db_recent[ i ]->setVisible ( false );

			continue;
		}

		act_db_recent[ i ]->setText (
			mtQEX::qstringFromC ( buf ) );

		act_db_recent[ i ]->setVisible ( true );
	}
}

