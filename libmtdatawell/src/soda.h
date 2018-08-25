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

#define HEADER_ITEM_MODE	"mode"
#define HEADER_ITEM_SIZE	"size"
#define HEADER_ITEM_BUCKET	"bucket"
#define HEADER_ITEM_POS		"pos"
#define HEADER_ITEM_BUTT	"butt"

#define UTREE_ROOT_NAME		"Jug"

#define FILE_ID			"Soda"
#define FILE_CHUNK_ID		"Jug0"

#define CHUNK_SIZE		65536



class SodaOp
{
public:
	explicit SodaOp ( char const * path = NULL );
	~SodaOp ();

	static int decode (
		Butt * butt,
		char const * input,
		char const * output
		);

	int encode (
		Butt * butt,
		char const * input,
		char const * output
		) const;

	static int multi_decode (
		Butt * butt,
		char const * input,
		char const * output
		);

	int multi_encode (
		Butt * butt,
		char const * input,
		char const * output,
		char const * const * butt_names
		) const;

	inline void set_mode ( int m )	{ m_mode = m; }
	inline int get_mode () const	{ return m_mode; }

private:
	int encode_raw () const;
	void db_add_encode (
		char const * filename,
		uint64_t filesize,
		int mode,
		std::string const & bucket_name,
		int butt_bucket,
		int bucket_position
		) const;

/// ----------------------------------------------------------------------------

	int			m_mode;

	mtKit::Sqlite		m_db;
};



}	// namespace mtDW

