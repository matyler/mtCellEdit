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

#include "core.h"



namespace mtDW
{



class TapAudioRead
{
public:
	TapAudioRead ();
	~TapAudioRead ();

	int open ( char const * filename );
		// Returns error code.  Interpret via get_error_text()

	int read ( short ** buf, size_t * buflen );
		// Returns error code.  Interpret via get_error_text()

	int64_t get_read_capacity () const;

	inline SF_INFO const * get_info () const { return & m_info; }

	static int const BUF_FRAMES = 8192;

private:
	void close ();
	void free_buf ();

	int alloc_buf ();
		// Returns error code.  Interpret via get_error_text()

/// ----------------------------------------------------------------------------

	short		* m_buf;
	size_t		m_buflen;

	SNDFILE		* m_file;
	SF_INFO		m_info;
};



class TapAudioWrite
{
public:
	TapAudioWrite ();
	~TapAudioWrite ();

	int open ( SF_INFO const * src_info, char const * filename );
		// Returns error code.  Interpret via get_error_text()

	int write ( short const * buf, size_t buflen );
		// Returns error code.  Interpret via get_error_text()

private:
	void close ();

/// ----------------------------------------------------------------------------

	SNDFILE		* m_file;
	SF_INFO		m_info;
};



class Tap::Op
{
public:
	Op ();
	~Op ();

	static int encode_image (
		Well * well,
		mtPixy::Image * image,
		char const * input	// Soda file
		);
		// Returns error code.  Interpret via get_error_text()

	static int decode_image (
		mtPixy::Image * image,
		char const * output,	// Soda file
		int &type
		);
		// Returns error code.  Interpret via get_error_text()
		// type = TYPE_RGB_1 (op->filename = Soda file)
		// type = TYPE_INVALID (empty image)

	static int encode_audio (
		TapAudioRead * audio_in,
		Well * well,
		char const * input,	// Soda file
		char const * output	// FLAC file
		);
		// Returns error code.  Interpret via get_error_text()

	static int decode_audio (
		char const * input,	// Bottle file
		char const * output,	// Soda file
		int &type
		);
		// Returns error code.  Interpret via get_error_text()
		// type = TYPE_SND_1 (op->filename = Soda file)
		// type = TYPE_INVALID (empty audio)
};



class TapFile::Op
{
public:
	Op ();
	~Op ();

	void delete_soda_filename ();
	void set_soda_filename ( std::string const & filename );

/// ----------------------------------------------------------------------------

	std::string	m_soda_file;	// Temp file (0 size = no Soda)

	mtKit::unique_ptr<mtPixy::Image> m_image;
	mtKit::unique_ptr<TapAudioRead> m_audio;

	size_t		m_capacity;
};



}	// namespace mtDW

