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

#include "core.h"



namespace mtDW
{

class FileScan;
class FileStream;



class FileDB : public mtKit::Sqlite
{
public:
	FileDB ();
	~FileDB ();

	int open ( std::string const & filename );

	int add_todo_filename ( std::string const & filename ) const;
	std::string const get_todo_filename ();
	void remove_todo_filename ();

	int count_files () const;
	void remove_all_files () const;

	inline int get_file_id () const { return (int)m_file_id; }
	inline void set_file_id ( int id ) { m_file_id = (uint32_t)id; }
	inline void increment_file_id () { m_file_id++; }

private:
	std::string const get_todo_filename_internal ();

	int add_table_filename (
		std::string const & filename,
		char const * table
		) const;

/// ----------------------------------------------------------------------------

	uint32_t	m_file_id;
};



class FileStream
{
public:
	explicit FileStream ( FileDB & db );
	~FileStream ();

	int open ( uint64_t pos = 0 );		// Open next file from the db
		// 0 = open, ready for reading
		// 1 = no files available

	int read ( mtKit::ByteBuf & buf );	// Fills whole buffer
		// 0 = filled
		// 1 = not filled

	inline uint64_t get_pos () const { return m_pos; }

private:
	void set_file ( FILE * fp );
	void free_zlib ();

/// ----------------------------------------------------------------------------

	mtKit::ByteBuf	buf_file;	// Raw from files

	unsigned char	* m_zlib;	// Deflated buf_file
	size_t		m_zlib_len;
	size_t		m_zlib_pos;

	uint64_t	m_pos;		// m_fp file position
	FILE		* m_fp;

	FileDB		& m_file_db;
};



class WellOp
{
public:
	explicit WellOp ( char const * path );
	~WellOp ();

	int save_file ( int const bytes, char const * filename );

	inline std::string const & get_path () const { return m_path; }

	int count_files_done () const;
	int count_files_todo () const;

/// ----------------------------------------------------------------------------

	mtKit::Random		m_random;
	mtKit::BitShifter	m_bitshift;

	FileDB			m_file_db;

private:
	std::string		m_path;

	mtKit::Prefs		m_prefs;

	FileStream		m_file;

	mtKit::ByteBuf		m_file_buffer;
	mtKit::ByteBuf		m_prng_buffer;
};



class FileScan
{
public:
	FileScan ( FileDB & db, std::string const & path );
	~FileScan ();

private:
	void path_recurse ( std::string const & path );

/// ----------------------------------------------------------------------------

	FileDB		& m_file_db;
};



}	// namespace mtDW

