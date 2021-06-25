/*
	Copyright (C) 2018-2021 Mark Tyler

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
#define PREFS_BUF_FILE_TOT	"buf_file_tot"
#define PREFS_BUF_FILE_POS	"buf_file_pos"
#define PREFS_BUF_PRNG_TOT	"buf_prng_tot"
#define PREFS_BUF_PRNG_POS	"buf_prng_pos"
#define PREFS_BUF_ZLIB_POS	"buf_zlib_pos"
#define PREFS_FILE_ID		"file_id"
#define PREFS_FILE_POS_LSI	"file_pos_lsi"
#define PREFS_FILE_POS_MSI	"file_pos_msi"
#define PREFS_TODO_TABLE	"todo_table"
#define PREFS_BITSHIFT_		"bitshift_"
#define PREFS_BITSHIFT_SALT	"bitshift_salt"
#define PREFS_BITSHIFT_POS	"bitshift_pos"

#define PREFS_FILENAME		"well.prefs"
#define DB_FILENAME		"well.sqlite"
#define BUF_FILE_FILENAME	"well_file.tmp"
#define BUF_PRNG_FILENAME	"well_prng.tmp"
#define BUF_ZLIB_FILENAME	"well_zlib.tmp"

#define WELL_PATH		"well"

#define BUFFER_SIZE		8192



mtDW::Well::Op::Op ( char const * const path )
	:
	m_well_root	( mtDW::prepare_path(path) + WELL_PATH + MTKIT_DIR_SEP),
	m_file		( m_file_db ),
	m_file_buffer	( BUFFER_SIZE ),
	m_prng_buffer	( BUFFER_SIZE )
{
	mtkit_mkdir ( m_well_root.c_str() );

	if ( m_lock.set ( m_well_root + "well.lock" ) )
	{
		throw 123;
	}

	if ( m_file_db.open ( m_well_root + DB_FILENAME ) )
	{
		throw 123;
	}

	// Prefs file ----------------------------------------------------------

	new_well_prefs ();

	std::string const fn = m_well_root + PREFS_FILENAME;
	m_prefs_well->uprefs.load ( fn.c_str (), NULL );

	restore_well_state ();
}

mtDW::Well::Op::~Op ()
{
	save_state ();
}

void mtDW::Well::Op::store_well_state ()
{
	m_file_buffer.save ( m_well_root + BUF_FILE_FILENAME );
	m_prng_buffer.save ( m_well_root + BUF_PRNG_FILENAME );

	m_file.get_zlib ().save ( m_well_root + BUF_ZLIB_FILENAME );

	WellPrefs * const prefs = m_prefs_well.get ();

	if ( prefs )
	{
		prefs->buf_file_tot = (int)m_file_buffer.get_tot();
		prefs->buf_file_pos = (int)m_file_buffer.get_pos();

		prefs->buf_prng_tot = (int)m_prng_buffer.get_tot();
		prefs->buf_prng_pos = (int)m_prng_buffer.get_pos();

		prefs->buf_zlib_pos = (int)m_file.get_zlib ().get_pos();

		uint64_t const seed = m_random.get_seed ();

		prefs->seed_lsi = (int)(seed & 0xFFFFFFFF);
		prefs->seed_msi = (int)((seed >> 32) & 0xFFFFFFFF);

		prefs->file_id = m_file_db.get_file_id ();

		uint64_t const & pos = m_file.get_pos ();

		prefs->file_pos_lsi = (int)(pos & 0xFFFFFFFF);
		prefs->file_pos_msi = (int)((pos >> 32) & 0xFFFFFFFF);

		prefs->bitshift_salt = m_bitshift.get_salt ();
		prefs->bitshift_pos = m_bitshift.get_pos ();

		m_bitshift.get_shifts ( prefs->bitshift );
	}
}

void mtDW::Well::Op::restore_well_state ()
{
	WellPrefs * const prefs = m_prefs_well.get ();

	if ( prefs )
	{
		m_file_db.set_file_id ( prefs->file_id );

		uint64_t lsi, msi;

		lsi = (uint64_t)prefs->seed_lsi;
		msi = (uint64_t)prefs->seed_msi;
		uint64_t const seed = (lsi & 0xFFFFFFFF) | (msi << 32);

		lsi = (uint64_t)prefs->file_pos_lsi;
		msi = (uint64_t)prefs->file_pos_msi;
		size_t const file_pos = (size_t)
			((lsi & 0xFFFFFFFF) | (msi << 32));

		// Buffer files ------------------------------------------------

		m_file_buffer.load_fill ( m_well_root + BUF_FILE_FILENAME );
		m_file_buffer.set_pos ( (size_t)prefs->buf_file_pos );
		m_file_buffer.set_tot ( MIN( (size_t)prefs->buf_file_tot,
			m_file_buffer.get_tot() ) );

		m_prng_buffer.load_fill ( m_well_root + BUF_PRNG_FILENAME );
		m_prng_buffer.set_pos ( (size_t)prefs->buf_prng_pos );
		m_prng_buffer.set_tot ( MIN( (size_t)prefs->buf_prng_tot,
			m_prng_buffer.get_tot() ) );

		m_file.get_zlib ().load_whole( m_well_root + BUF_ZLIB_FILENAME);
		m_file.get_zlib ().set_pos ( (size_t)prefs->buf_zlib_pos );

		// Finalize ----------------------------------------------------

		if ( 0 == seed )
		{
			m_random.set_seed_by_time ();
			m_bitshift.set_shifts ( m_random );
		}
		else
		{
			m_random.set_seed ( seed );

			if ( m_bitshift.set_shifts ( prefs->bitshift ) )
			{
				m_bitshift.set_shifts ( m_random );
			}
			else
			{
				m_bitshift.set_salt ( prefs->bitshift_salt );
				m_bitshift.set_pos ( prefs->bitshift_pos );
			}
		}

		if ( 0 != file_pos )
		{
			m_file.open ( file_pos );
		}
	}
}

mtDW::WellPrefs * mtDW::Well::Op::create_well_prefs ()
{
	auto prefs = new WellPrefs;

	prefs->uprefs.add_int ( PREFS_SEED_LSI, prefs->seed_lsi, 0 );
	prefs->uprefs.add_int ( PREFS_SEED_MSI, prefs->seed_msi, 0 );
	prefs->uprefs.add_int ( PREFS_BUF_FILE_TOT, prefs->buf_file_tot, 0 );
	prefs->uprefs.add_int ( PREFS_BUF_FILE_POS, prefs->buf_file_pos, 0 );
	prefs->uprefs.add_int ( PREFS_BUF_PRNG_TOT, prefs->buf_prng_tot, 0 );
	prefs->uprefs.add_int ( PREFS_BUF_PRNG_POS, prefs->buf_prng_pos, 0 );
	prefs->uprefs.add_int ( PREFS_BUF_ZLIB_POS, prefs->buf_zlib_pos, 0 );
	prefs->uprefs.add_int ( PREFS_FILE_ID, prefs->file_id, 1 );
	prefs->uprefs.add_int ( PREFS_FILE_POS_LSI, prefs->file_pos_lsi, 0 );
	prefs->uprefs.add_int ( PREFS_FILE_POS_MSI, prefs->file_pos_msi, 0 );
	prefs->uprefs.add_int ( PREFS_TODO_TABLE, prefs->todo_table, 0 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_SALT, prefs->bitshift_salt, 0 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_POS, prefs->bitshift_pos, 0 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "0", prefs->bitshift[0], -1 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "1", prefs->bitshift[1], -1 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "2", prefs->bitshift[2], -1 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "3", prefs->bitshift[3], -1 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "4", prefs->bitshift[4], -1 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "5", prefs->bitshift[5], -1 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "6", prefs->bitshift[6], -1 );
	prefs->uprefs.add_int ( PREFS_BITSHIFT_ "7", prefs->bitshift[7], -1 );

	return prefs;
}

void mtDW::Well::Op::new_well_prefs ()
{
	if ( m_prefs_well.get () )
	{
		// We have old prefs so save them
		store_well_state ();
	}

	m_prefs_well.reset ( create_well_prefs () );
}

void mtDW::Well::Op::save_state ()
{
	WellPrefs * const prefs = m_prefs_well.get ();

	if ( prefs )
	{
		store_well_state ();
		prefs->uprefs.save ();
	}
}

int mtDW::Well::Op::count_files_done () const
{
	return (m_file_db.get_file_id () - 1);
}

int mtDW::Well::Op::count_files_todo () const
{
	return (m_file_db.count_files () - m_file_db.get_file_id () + 1);
}

int mtDW::Well::Op::get_int ()
{
	uint8_t		buf[4] = {0};

	get_data ( buf, sizeof(buf) );

	return (int)(	buf[0]
			| (buf[1] << 8)
			| (buf[2] << 16)
			| (buf[3] << 24)
			);
}

int mtDW::Well::Op::get_int (
	int	const	modulo
	)
{
	if ( modulo < 2 )
	{
		return 0;
	}

	int	const	floor = ((INT_MAX - modulo) + 1) % modulo;
	int		res = 0;

	do
	{
		// Lose negative int's
		res = get_int () & INT_MAX;

	} while ( res < floor );

	return res % modulo;
}

void mtDW::Well::Op::get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	size_t	const	tot = m_prng_buffer.get_size ();
	size_t		todo = buflen;
	size_t		done = 0;
	bool		files_ok = true;

	for ( ; todo > 0; todo -= done )
	{
		done = 0;

		if ( m_prng_buffer.get_pos () < m_prng_buffer.get_tot () )
		{
			done = MIN ( todo, (m_prng_buffer.get_tot () -
				m_prng_buffer.get_pos () ) );

			memcpy ( buf + (buflen - todo), m_prng_buffer.get_buf ()
				+ m_prng_buffer.get_pos (), done );

			m_prng_buffer.set_pos (m_prng_buffer.get_pos () + done);

			continue;
		}

		if ( files_ok )
		{
			if ( m_file.read ( m_file_buffer ) )
			{
				// Don't bother trying again, no files available
				files_ok = false;
			}
			else
			{
				uint8_t * const dest = m_file_buffer.get_buf ();

				for ( size_t i = 0; i < tot; i++ )
				{
					dest[i] = m_bitshift.get_byte (dest[i]);
				}
			}
		}

		uint8_t * const pmem = m_prng_buffer.get_buf ();

		m_random.get_data ( pmem, tot );

		// PRNG buffer is full again
		m_prng_buffer.set_tot ( tot );
		m_prng_buffer.set_pos ( 0 );

		if ( files_ok )
		{
			// We only need to XOR if the files delivered something

			uint8_t const * const fmem = m_file_buffer.get_buf ();

			for ( size_t i = 0; i < tot; i++ )
			{
				pmem[i] = fmem[i] ^ pmem[i];
			}
		}
	}
}

int mtDW::Well::Op::save_file (
	int		const	bytes,
	char	const * const	filename
	)
{
	if ( bytes < 0 || ! filename )
	{
		return report_error ( ERROR_WELL_SAVE_FILE_INSANITY );
	}

	mtKit::ByteFileWrite file;

	if ( file.open ( filename ) )
	{
		return report_error ( ERROR_WELL_SAVE_FILE_OPEN );
	}

	uint8_t		buf[ BUFFER_SIZE ];
	size_t		todo = (size_t)bytes;
	size_t		done = 0;

	mtKit::SqliteTransaction trans ( m_file_db.m_db );
	WellOpSaveState woss ( this );

	for ( ; todo > 0; todo -= done )
	{
		done = MIN ( todo, sizeof(buf) );

		get_data ( buf, done );

		if ( file.write ( buf, done ) )
		{
			return report_error ( ERROR_WELL_SAVE_FILE_WRITE );
		}
	}

	return 0;
}

