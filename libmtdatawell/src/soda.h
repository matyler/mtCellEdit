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

#ifndef SODA_H_
#define SODA_H_



#include "mtdatawell_sqlite.h"
#include "core.h"



namespace mtDW
{

#define SODA_HEADER_ITEM_MODE	"mode"
#define SODA_HEADER_ITEM_SIZE	"size"
#define SODA_HEADER_ITEM_BUCKET	"bucket"
#define SODA_HEADER_ITEM_POS	"pos"
#define SODA_HEADER_ITEM_BUTT	"butt"

#define SODA_UTREE_ROOT_NAME	"Jug"

#define SODA_FILE_ID		"Soda"
#define SODA_FILE_CHUNK_ID	"Jug0"

#define SODA_CHUNK_SIZE		65536



class Soda::Op
{
public:
	explicit Op ( char const * path = NULL );

	static int decode (
		Butt * butt,
		char const * input,
		char const * output
		);
		// Returns error code.  Interpret via get_error_text()

	int encode (
		Butt * butt,
		char const * input,
		char const * output
		) const;
		// Returns error code.  Interpret via get_error_text()

	static int multi_decode (
		Butt * butt,
		char const * input,
		char const * output
		);
		// Returns error code.  Interpret via get_error_text()

	int multi_encode (
		Butt * butt,
		char const * input,
		char const * output,
		char const * const * otp_names
		) const;
		// Returns error code.  Interpret via get_error_text()

	void set_mode ( int m )	{ m_mode = m; }
	int get_mode () const	{ return m_mode; }

/// ----------------------------------------------------------------------------

	mtDW::Sqlite		m_db;

private:
	int encode_raw () const;
	void db_add_encode (
		char const * filename,
		uint64_t filesize,
		int mode,
		std::string const & otp_name,
		int otp_bucket,
		int bucket_position
		) const;

/// ----------------------------------------------------------------------------

	mtKit::FileLock		m_lock;

	int			m_mode;
};



class SodaTransaction::Op
{
public:
	mtDW::SqliteTransaction trans;

	explicit Op ( mtDW::Sqlite const & db )
		:
		trans ( db )
	{
	}
};



}	// namespace mtDW



#endif		// SODA_H_

