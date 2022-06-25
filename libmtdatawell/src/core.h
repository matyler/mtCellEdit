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

#include <mtkit_sqlite.h>

#include "mtdatawell.h"

#include <string.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>



namespace mtDW
{



std::string prepare_path ( char const * path );

void get_temp_filename (
	std::string	&	filename,
	char	const * const	prefix
	);
	// filename = "prefix_01" or other numbers up to _99 for unused file

int remove_dir (
	std::string	const	&path	// MUST end in MTKIT_DIR_SEP
	);



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



#define RETURN_ON_ERROR( A )					\
	{							\
		int const roe = A;				\
		if ( roe ) return roe;				\
	}



enum
{
	ERROR_MIN			= -999999999,

	ERROR_ANALYSIS_INSANITY		,

	ERROR_AUDIO_BAD_CHANNELS	,
	ERROR_AUDIO_DECODE_EXCEPTION	,
	ERROR_AUDIO_DECODE_INSANITY	,
	ERROR_AUDIO_ENCODE		,
	ERROR_AUDIO_ENCODE_INSANITY	,
	ERROR_AUDIO_OPEN_INPUT		,
	ERROR_AUDIO_OPEN_OUTPUT		,
	ERROR_AUDIO_READ		,
	ERROR_AUDIO_TOO_SMALL		,
	ERROR_AUDIO_WRITE		,
	ERROR_AUDIO_WRITE_INSANITY	,
	ERROR_AUDIO_ZERO_INPUT		,

	ERROR_BUTT_OTP_DELETE_ACTIVE	,
	ERROR_BUTT_OTP_EXISTS		,
	ERROR_BUTT_OTP_MISSING		,
	ERROR_BUTT_OTP_NO_WELL		,
	ERROR_BUTT_OTP_OPEN_BUCKET	,
	ERROR_BUTT_OTP_READ_BUFFER	,
	ERROR_BUTT_OTP_READ_ONLY	,

	ERROR_DISK_OTP_READ_ONLY	,

	ERROR_HEAP_EMPTY		,

	ERROR_IMAGE_DECODE_EXCEPTION	,
	ERROR_IMAGE_ENCODE_INSANITY	,
	ERROR_IMAGE_INVALID_BOTTLE	,
	ERROR_IMAGE_OPEN_OUTPUT		,
	ERROR_IMAGE_TOO_SMALL		,
	ERROR_IMAGE_WRITE		,

	ERROR_IMPORT_OTP_BAD_DIR	,
	ERROR_IMPORT_OTP_EXISTS		,
	ERROR_IMPORT_OTP_OPEN_DIR	,

	ERROR_LOAD_INPUT		,

	ERROR_OTP_NAME_ILLEGAL		,
	ERROR_OTP_NAME_TOO_LARGE	,
	ERROR_OTP_NAME_TOO_SMALL	,

	ERROR_SODA_BAD_CHUNK_ID		,
	ERROR_SODA_BAD_HEADER		,
	ERROR_SODA_BAD_HEADER_ID	,
	ERROR_SODA_BIG_CHUNK		,
	ERROR_SODA_DECODE_INSANITY	,
	ERROR_SODA_DECODE_NO_BUTT	,
	ERROR_SODA_DECODE_NO_XOR	,
	ERROR_SODA_ENCODE_EXCEPTION	,
	ERROR_SODA_ENCODE_INSANITY	,
	ERROR_SODA_ENCODE_SIZE		,
	ERROR_SODA_ENCODE_WRITE		,
	ERROR_SODA_FILE_CHUNK_1		,
	ERROR_SODA_FILE_ID		,
	ERROR_SODA_FILE_WRITE		,
	ERROR_SODA_MISSING_DATA		,
	ERROR_SODA_OPEN_INFO		,
	ERROR_SODA_OPEN_INPUT		,
	ERROR_SODA_OPEN_INSANITY	,
	ERROR_SODA_OPEN_OUTPUT		,
	ERROR_SODA_UTREE_ALLOC		,

	ERROR_TAP_BOTTLE_INVALID	,
	ERROR_TAP_DECODE_INSANITY	,
	ERROR_TAP_ENCODE_BAD_BOTTLE	,
	ERROR_TAP_ENCODE_INSANITY	,
	ERROR_TAP_ENCODE_SAVE_PNG	,
	ERROR_TAP_OPEN_SODA_INSANITY	,
	ERROR_TAP_UNKNOWN_BOTTLE	,

	ERROR_WELL_SAVE_FILE_INSANITY	,
	ERROR_WELL_SAVE_FILE_OPEN	,
	ERROR_WELL_SAVE_FILE_WRITE	,

	ERROR_MAX			,

	ERROR_NONE			= 0
};

