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



class TapAudioRead
{
public:
	TapAudioRead ();
	~TapAudioRead ();

	int open ( char const * filename );
	int read ( short ** buf, size_t * buflen );

	int64_t get_read_capacity () const;

	inline SF_INFO const * get_info () const { return & m_info; }

	static int const BUF_FRAMES = 8192;

private:
	void close ();
	void free_buf ();
	int alloc_buf ();

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
	int write ( short const * buf, size_t buflen );

private:
	void close ();

/// ----------------------------------------------------------------------------

	SNDFILE		* m_file;
	SF_INFO		m_info;
};



class TapOp
{
public:
	TapOp ();
	~TapOp ();

	static int encode_image (
		Butt * butt,
		mtPixy::Image * image,
		char const * input	// Soda file
		);

	static int decode_image (
		mtPixy::Image * image,
		char const * output	// Soda file
		);
		// = TYPE_RGB_1 (op->filename = Soda file)
		// = TYPE_INVALID

	static int encode_audio (
		TapAudioRead * audio_in,
		Butt * butt,
		char const * input,	// Soda file
		char const * output	// FLAC file
		);

	static int decode_audio (
		char const * input,	// Bottle file
		char const * output	// Soda file
		);
		// = TYPE_SND_1 (op->filename = Soda file)
		// = TYPE_INVALID
};



class TapFileOp
{
public:
	TapFileOp ();
	~TapFileOp ();

	void delete_soda_filename ();
	void set_soda_filename ( std::string const & filename );

/// ----------------------------------------------------------------------------

	std::string	m_soda_file;	// Temp file (0 size = no Soda)

	mtKit::unique_ptr<mtPixy::Image> m_image;
	mtKit::unique_ptr<TapAudioRead> m_audio;

	size_t		m_capacity;
};



}	// namespace mtDW

