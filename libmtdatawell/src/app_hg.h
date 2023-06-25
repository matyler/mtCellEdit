/*
	Copyright (C) 2019-2022 Mark Tyler

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

#ifndef APP_HG_H_
#define APP_HG_H_



#include "core.h"



class FileOps;



class FileOps
{
public:
	explicit FileOps ( mtDW::GlyphIndex const * index );
	~FileOps ();

	// Disk operations
	int load_input_file ( char const * filename );
	int load_covert_file ( char const * filename );
	int open_output_file ( char const * filename );

	// Memory operations
	int open_output_mem ();
	int get_output_mem_utf8 ( std::string &result );

	int load_input_utf8 ( std::string const &input );
	int load_covert_utf8 ( std::string const &covert );
	int analyse_input ( std::string &info ) const;

	int encode_hg_file ( mtDW::Well * well );
	int decode_hg_file ();
	int encode_utf8font_file ( int type );
	int cleanse_file ();

	inline int get_file_size () const { return m_file_in_len; }
	inline int get_covert_size () const { return m_file_covert_len; }
	inline int get_utf8_len () const { return m_file_in_utf8; }
		// -1=not UTF-8 else glyph total
	inline int get_utf8_bit_capacity () const { return m_file_in_utf8_bits;}
	inline int get_utf8_byte_capacity () const{return m_file_in_utf8_bytes;}
	inline int get_utf8_encoded_tot () const{return m_file_in_utf8_encoded;}
	inline int get_utf8_roots_tot () const { return m_file_in_utf8_roots; }
	inline int get_utf8_misc_tot () const { return m_file_in_utf8_misc; }

private:
	void file_in_set ( char * buf, int len );
	void file_covert_set ( char * buf, int len );
	void file_out_set ( mtFile * file_out );

/// ----------------------------------------------------------------------------

	char		* m_file_in_buf;
	int		m_file_in_len;		// Bytes
	int		m_file_in_utf8;		// Glyphs, -1=Not UTF-8
	int		m_file_in_utf8_bits;
	int		m_file_in_utf8_bytes;	// Arithmetic coder extent
	int		m_file_in_utf8_encoded;	// Encoded glyphs (non-ASCII)
	int		m_file_in_utf8_roots;	// Encoded root glyphs
	int		m_file_in_utf8_misc;	// Misc ASCII, e.g. \n \t

	char		* m_file_covert_buf;
	int		m_file_covert_len;	// Bytes

	mtFile		* m_file_out;

	mtDW::GlyphIndex const * const m_hg_index;

	MTKIT_RULE_OF_FIVE( FileOps )
};



#endif		// APP_HG_H_

