/*
	Copyright (C) 2018-2022 Mark Tyler

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

#ifndef CORE_H_
#define CORE_H_



#include "mtdatawell_sqlite.h"

#include <string.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>



namespace mtDW
{



class ArithEncode;
class ArithDecode;
class BitShifter;
class ByteBuf;
class FilenameSwap;
class OpenDir;



std::string prepare_path ( char const * path );

void get_temp_filename (
	std::string	&	filename,
	char	const * const	prefix
	);
	// filename = "prefix_01" or other numbers up to _99 for unused file

int remove_dir (
	std::string	const	&path	// MUST end in MTKIT_DIR_SEP
	);



class ArithEncode
{
public:
	ArithEncode ();

	void push_mem ( uint8_t const * mem, size_t len ); // 1 <= len <= 7

	int pop_code ( int span, int & code );		// 2 <= span <= 256
		// 0 = OK, data left to encode
		// 1 = OK, data all encoded

	int get_encoded_byte_count () const;

private:
	uint64_t	m_mem;		// Current data
	uint64_t	m_span_mem;	// Current total span in m_mem
	uint64_t	m_span_popped;	// Current total popped
};



class ArithDecode
{
public:
	ArithDecode ();

	int push_code (
		int code,	// 0 <= code <= 255
		int span	// 2 <= span <= 256
		);
		// 0 = OK, code packed
		// 1 = not sent, full (i.e. span * m_span_mem > 7 bytes)
		// -1 = Error

	int pop_mem ( uint8_t * dest, size_t & size );
			// dest Must be >= 7 bytes

	int get_encoded_byte_count () const;

private:
	uint64_t	m_mem;		// Current data
	uint64_t	m_span_mem;	// Current total span in m_mem
};



class BitShifter
{
public:
	BitShifter ();
	~BitShifter ();

	// NOTE: random must be seeded by the caller.
	int set_shifts ( mtKit::Random &random );
	int set_shifts ( int const shifts[8] );	// shifts[] contains *ALL* 0..7
	inline void set_salt ( int i )	{ m_salt = i; }
	inline void set_pos ( int i )	{ m_pos = i; }

	uint8_t get_byte ( uint8_t input );
	void get_shifts ( int shifts[8] ) const;
	inline int get_salt () const	{ return m_salt; }
	inline int get_pos () const	{ return m_pos; }

protected:
	int		m_pos;
	int		m_shifts[ 8 ];
	int		m_salt;
};



class ByteBuf
{
public:
	ByteBuf ();
	explicit ByteBuf ( size_t const size );

	~ByteBuf ();

	int allocate ( size_t size );
	void set ( uint8_t * buf, size_t size );

	int save ( std::string const &filename ) const;
	void load_fill ( std::string const &filename );
	void load_whole ( std::string const &filename );

	inline uint8_t * get_buf () const { return m_buf; }
	inline size_t get_size () const { return m_size; }
	inline size_t get_tot () const { return m_tot; }
	inline size_t get_pos () const { return m_pos; }

	inline void set_tot ( size_t const tot ) { m_tot = tot; }
	inline void set_pos ( size_t const pos ) { m_pos = pos; }

private:
	uint8_t		* m_buf = nullptr;
	size_t		m_size = 0;
	size_t		m_tot = 0;	// Current items in array
	size_t		m_pos = 0;	// Current position in array

	MTKIT_RULE_OF_FIVE( ByteBuf )
};



class OpenDir
{
public:
	inline explicit OpenDir ( std::string const & path )
	{
		dp = opendir ( path.c_str () );
	}

	inline ~OpenDir ()
	{
		if ( dp )
		{
			closedir ( dp );
			dp = NULL;
		}
	}

/// ----------------------------------------------------------------------------

	DIR * dp;
};



class FilenameSwap
{
public:
	explicit FilenameSwap ( char const * const output );
	~FilenameSwap ();

	void swap ();

/// ----------------------------------------------------------------------------

	char	const * f1;
	char	const * f2;

	std::string	m_tmp;
	int		m_res;

private:
	char	const * const	m_prefix;
};



}	// namespace mtDW



/// ERROR HANDLING -------------------------------------------------------------



int report_error ( int error );		// Output get_error_text to stderr
	// = error



#endif		// CORE_H_

