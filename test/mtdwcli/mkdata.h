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

#include <string.h>
#include <math.h>
#include <limits.h>

#include <sndfile.h>

#include <mtkit.h>
#include <mtpixy.h>



class CreatePNG
{
public:
	CreatePNG ( char const * path, mtKit::Random & random );
	~CreatePNG ();

private:
	void paint_rectangles ();

/// ----------------------------------------------------------------------------

	static int	const	IMAGE_WIDTH = 1024;
	static int	const	IMAGE_HEIGHT = 1024;

	mtKit::Random	&	m_random;

	mtPixy::Pixmap		m_pixmap;
	std::unique_ptr<mtPixy::Brush> m_brush;

	mtPalette		* m_palette;
	mtColor			* m_color;
};



class AudioWrite
{
public:
	AudioWrite ();
	~AudioWrite ();

	int open ( char const * filename );
	int write ( short const * buf );	// length = BUF_SIZE

/// ----------------------------------------------------------------------------

	static size_t	const	SAMPLE_RATE = 44100;	// Per sec
	static size_t	const	CHANNELS = 1;
	static size_t	const	BUF_SECS = 1;
	static size_t	const	BUF_SIZE = SAMPLE_RATE * CHANNELS * BUF_SECS;

private:
	void close ();

/// ----------------------------------------------------------------------------

	SNDFILE		* m_file;
};



class CreateFLAC
{
public:
	explicit CreateFLAC ( char const * path );
	~CreateFLAC ();

private:
	void create_audio_file ();
	void fill_buffer ();

/// ----------------------------------------------------------------------------

	static size_t	const	AUDIO_SECS = 60;
	static size_t	const	AUDIO_SAMPLES = AudioWrite::BUF_SIZE;

	AudioWrite		m_audio;

	short		* const	m_buf;

	MTKIT_RULE_OF_FIVE( CreateFLAC )
};

