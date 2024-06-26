/*
	Copyright (C) 2018-2024 Mark Tyler

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

#ifndef WELL_H_
#define WELL_H_



#include "mtdatawell_sqlite.h"
#include "core.h"



#define SCHEMA_VERSION		3202

#define DB_TABLE_FILES		"Files"

#define DB_FIELD_ID		"id"
#define DB_FIELD_FILENAME	"filename"



namespace mtDW
{

class FileScan;
class FileStream;



class FileDB
{
public:
	FileDB ();
	~FileDB ();

	int open ( std::string const & filename );

	std::string const get_todo_filename ();
	void remove_todo_filename ();

	int count_files () const;
	void remove_all_files ();

	int get_file_id () const	{ return (int)m_file_id; }
	void set_file_id ( int id )	{ m_file_id = (uint32_t)id; }
	void increment_file_id ()	{ m_file_id++; }

/// ----------------------------------------------------------------------------

	mtDW::Sqlite	m_db;

private:
	void get_todo_filename_internal ( std::string & res );

/// ----------------------------------------------------------------------------

	uint32_t	m_file_id;
};



class FileStream
{
public:
	explicit FileStream ( FileDB & db );

	int open ( uint64_t pos = 0 );		// Open next file from the db
		// 0 = open, ready for reading
		// 1 = no files available

	int read ( ByteBuf & buf );		// Fills whole buffer
		// 0 = filled
		// 1 = not filled

	uint64_t get_pos () const	{ return m_file.get_pos (); }

	ByteBuf &get_zlib ()		{ return m_buf_zlib; }

private:

/// ----------------------------------------------------------------------------

	ByteBuf			m_buf_file;	// Raw from files
	ByteBuf			m_buf_zlib;	// Deflated m_buf_file

	mtKit::ByteFileRead	m_file;

	FileDB			&m_file_db;
};



struct WellPrefs
{
	int			seed_lsi	= 0;
	int			seed_msi	= 0;
	int			buf_file_tot	= 0;
	int			buf_file_pos	= 0;
	int			buf_prng_tot	= 0;
	int			buf_prng_pos	= 0;
	int			buf_zlib_pos	= 0;
	int			file_id		= 0;
	int			file_pos_lsi	= 0;
	int			file_pos_msi	= 0;
	int			todo_table	= 0;
	int			bitshift_salt	= 0;
	int			bitshift_pos	= 0;
	int			bitshift[8]	= {0};

	// Below must be destroyed first before the above items
	mtKit::UserPrefs	uprefs;
};



class Well::Op
{
public:
	explicit Op ( char const * path );
	~Op ();

	int save_file ( int const bytes, char const * filename );
		// Returns error code.  Interpret via get_error_text()

	int count_files_done () const;
	int count_files_todo () const;

	int get_int ();			// = INT_MIN..INT_MAX
	int get_int ( int modulo );	// = 0..(modulo - 1)
	void get_data ( uint8_t * buf, size_t buflen );	// buf != NULL

	void save_state ();

/// ----------------------------------------------------------------------------

	mtKit::Random		m_random;
	mtDW::BitShifter	m_bitshift;

	FileDB			m_file_db;

private:
	static WellPrefs * create_well_prefs ();
	void new_well_prefs ();
	void store_well_state ();
	void restore_well_state ();

/// ----------------------------------------------------------------------------

	mtKit::FileLock		m_lock;

	std::string	const	m_well_root;	// <m_path> / well /

	std::unique_ptr<WellPrefs> m_prefs_well;

	FileStream		m_file;

	ByteBuf			m_file_buffer;
	ByteBuf			m_prng_buffer;
};



class WellOpSaveState
{
public:
	explicit WellOpSaveState ( Well::Op * op ) : m_op ( op ) {}
	~WellOpSaveState ()	{ if ( m_op ) m_op->save_state (); }

private:
	Well::Op	* const m_op;
};



class FileScan
{
public:
	FileScan ( FileDB & db, std::string const & path );

private:
	void path_recurse ( std::string const & path );

/// ----------------------------------------------------------------------------

	FileDB		& m_file_db;
	mtDW::SqliteAddRecord m_rec;
};



}	// namespace mtDW



#endif		// WELL_H_

