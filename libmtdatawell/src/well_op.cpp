/*
	Copyright (C) 2018 Mark Tyler

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

#include "well.h"



#define PREFS_SEED_LSI		"seed_lsi"
#define PREFS_SEED_MSI		"seed_msi"
#define PREFS_BUF_POS		"buf_pos"
#define PREFS_FILE_ID		"file_id"
#define PREFS_FILE_POS_LSI	"file_pos_lsi"
#define PREFS_FILE_POS_MSI	"file_pos_msi"
#define PREFS_TODO_TABLE	"todo_table"
#define PREFS_BITSHIFT_		"bitshift_"
#define PREFS_BITSHIFT_SALT	"bitshift_salt"
#define PREFS_BITSHIFT_POS	"bitshift_pos"

#define PREFS_FILENAME		"well.prefs"
#define DB_FILENAME		"well.sqlite"
#define BUFFER_FILENAME		"well.tmp"
#define BUFFER_SIZE		8192



mtDW::WellOp::WellOp ( char const * const path )
	:
	m_file		( m_file_db ),
	m_file_buffer	( BUFFER_SIZE ),
	m_prng_buffer	( BUFFER_SIZE )
{
	std::string const real_path = mtDW::prepare_path ( path );

	if ( m_file_db.open ( real_path + DB_FILENAME ) )
	{
		throw 123;
	}

	// Prefs file ----------------------------------------------------------

	static mtPrefTable const prefs_table[] = {
	{ PREFS_SEED_LSI,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_SEED_MSI,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BUF_POS,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_FILE_ID,	MTKIT_PREF_TYPE_INT, "1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_FILE_POS_LSI,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_FILE_POS_MSI,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_TODO_TABLE,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_SALT,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_POS,	MTKIT_PREF_TYPE_INT, "0", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "0",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "1",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "2",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "3",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "4",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "5",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "6",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ PREFS_BITSHIFT_ "7",	MTKIT_PREF_TYPE_INT, "-1", NULL, NULL, 0, NULL, NULL },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	m_prefs.addTable ( prefs_table );

	std::string const fn = real_path + PREFS_FILENAME;

	m_prefs.load ( fn.c_str (), NULL );

	m_file_db.set_file_id ( m_prefs.getInt ( PREFS_FILE_ID ) );

	uint64_t lsi, msi;

	lsi = (uint64_t)m_prefs.getInt ( PREFS_SEED_LSI );
	msi = (uint64_t)m_prefs.getInt ( PREFS_SEED_MSI );
	uint64_t const seed = (lsi & 0xFFFFFFFF) | (msi << 32);

	size_t const buf_pos = (size_t)m_prefs.getInt ( PREFS_BUF_POS );

	lsi = (uint64_t)m_prefs.getInt ( PREFS_FILE_POS_LSI );
	msi = (uint64_t)m_prefs.getInt ( PREFS_FILE_POS_MSI );
	size_t const file_pos = (size_t)((lsi & 0xFFFFFFFF) | (msi << 32));

	// Buffer file ---------------------------------------------------------

	std::string const buffer_filename = real_path + BUFFER_FILENAME;

	m_file_buffer.load ( buffer_filename );

	if ( BUFFER_SIZE == m_file_buffer.tot )
	{
		// Loaded correctly
		m_file_buffer.pos = buf_pos;
	}
	else
	{
		// Something went wrong so flush and restart
		memset ( m_file_buffer.array, 0, BUFFER_SIZE );
		m_file_buffer.pos = 0;
		m_file_buffer.tot = BUFFER_SIZE;
	}

	// Finalize ------------------------------------------------------------

	if ( 0 == seed )
	{
		m_random.set_seed_by_time ();
		m_bitshift.set_shifts ( m_random );
	}
	else
	{
		m_random.set_seed ( seed );

		int const shifts[8] = {
			m_prefs.getInt ( PREFS_BITSHIFT_ "0" ),
			m_prefs.getInt ( PREFS_BITSHIFT_ "1" ),
			m_prefs.getInt ( PREFS_BITSHIFT_ "2" ),
			m_prefs.getInt ( PREFS_BITSHIFT_ "3" ),
			m_prefs.getInt ( PREFS_BITSHIFT_ "4" ),
			m_prefs.getInt ( PREFS_BITSHIFT_ "5" ),
			m_prefs.getInt ( PREFS_BITSHIFT_ "6" ),
			m_prefs.getInt ( PREFS_BITSHIFT_ "7" )
			};

		if ( m_bitshift.set_shifts ( shifts ) )
		{
			m_bitshift.set_shifts ( m_random );
		}
		else
		{
			m_bitshift.set_salt ( m_prefs.getInt (
				PREFS_BITSHIFT_SALT ) );

			m_bitshift.set_pos ( m_prefs.getInt (
				PREFS_BITSHIFT_POS ) );
		}
	}

	m_path = real_path;

	if ( 0 != file_pos )
	{
		m_file.open ( file_pos );
	}
}

mtDW::WellOp::~WellOp ()
{
	if ( m_file_buffer.save ( m_path + BUFFER_FILENAME ) )
	{
		std::cerr << "Unable to save buffer\n";
		m_file_buffer.pos = 0;
	}

	m_prefs.set ( PREFS_BUF_POS, (int)m_file_buffer.pos );

	uint64_t const seed = m_random.get_seed ();

	m_prefs.set ( PREFS_SEED_LSI, (int)(seed & 0xFFFFFFFF) );
	m_prefs.set ( PREFS_SEED_MSI, (int)((seed >> 32) & 0xFFFFFFFF) );

	m_prefs.set ( PREFS_FILE_ID, m_file_db.get_file_id () );

	uint64_t const & pos = m_file.get_pos ();

	m_prefs.set ( PREFS_FILE_POS_LSI, (int)(pos & 0xFFFFFFFF) );
	m_prefs.set ( PREFS_FILE_POS_MSI, (int)((pos >> 32) & 0xFFFFFFFF) );

	m_prefs.set ( PREFS_BITSHIFT_SALT, m_bitshift.get_salt () );
	m_prefs.set ( PREFS_BITSHIFT_POS, m_bitshift.get_pos () );

	int shifts[8] = { 0 };

	m_bitshift.get_shifts ( shifts );

	m_prefs.set ( PREFS_BITSHIFT_ "0", shifts[0] );
	m_prefs.set ( PREFS_BITSHIFT_ "1", shifts[1] );
	m_prefs.set ( PREFS_BITSHIFT_ "2", shifts[2] );
	m_prefs.set ( PREFS_BITSHIFT_ "3", shifts[3] );
	m_prefs.set ( PREFS_BITSHIFT_ "4", shifts[4] );
	m_prefs.set ( PREFS_BITSHIFT_ "5", shifts[5] );
	m_prefs.set ( PREFS_BITSHIFT_ "6", shifts[6] );
	m_prefs.set ( PREFS_BITSHIFT_ "7", shifts[7] );

	// Well prefs always saved when mtKit::Prefs destroyed
}

int mtDW::WellOp::count_files_done () const
{
	return (m_file_db.get_file_id () - 1);
}

int mtDW::WellOp::count_files_todo () const
{
	return (m_file_db.count_files () - m_file_db.get_file_id () + 1);
}

