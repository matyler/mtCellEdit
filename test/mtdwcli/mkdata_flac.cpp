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

#include "mkdata.h"



CreateFLAC::CreateFLAC (
	char	const * const	path
	)
	:
	m_buf		( (short *)calloc ( sizeof(m_buf[0]), AUDIO_SAMPLES ) )
{
	if ( ! m_buf )
	{
		std::cerr << "Unable to allocate audio buffer\n";
		throw 123;
	}

	std::string filename;

	filename += path;
	filename += MTKIT_DIR_SEP;
	filename += "bottle.flac";

	if ( m_audio.open ( filename.c_str () ) )
	{
		throw 123;
	}

	create_audio_file ();
}

CreateFLAC::~CreateFLAC ()
{
	free ( m_buf );
}

void CreateFLAC::create_audio_file ()
{
	for ( size_t i = 0; i < AUDIO_SECS; i++ )
	{
		fill_buffer ();

		m_audio.write ( m_buf );
	}
}

void CreateFLAC::fill_buffer ()
{
	double const tot = AudioWrite::BUF_SIZE;
	double const max = MIN ( SHRT_MAX, -SHRT_MIN );

	for ( size_t i = 0; i < AudioWrite::BUF_SIZE; i++ )
	{
		double const k = 2 * M_PI * ((double)i / tot);
		double const m1 = sin ( k * 100 );
		double const m2 = sin ( k );

		m_buf[ i ] = (short)(max * m1 * m2);
	}
}



/// ----------------------------------------------------------------------------



AudioWrite::AudioWrite ()
	:
	m_file ()
{
}

AudioWrite::~AudioWrite ()
{
	close ();
}

int AudioWrite::open (
	char	const * const	filename
	)
{
	close ();

	SF_INFO			info = { 0, 0, 0, 0, 0, 0 };

	info.samplerate		= SAMPLE_RATE;
	info.channels		= CHANNELS;
	info.format		= SF_FORMAT_FLAC | SF_FORMAT_PCM_16;

	m_file = sf_open ( filename, SFM_WRITE, & info );

	if ( ! m_file )
	{
		std::cerr << "Error opening output audio file.\n";
		return 1;
	}

	return 0;
}

int AudioWrite::write (
	short	const * const	buf
	)
{
	if ( ! buf || ! m_file )
	{
		return 1;
	}

	if ( BUF_SIZE != (size_t)sf_writef_short ( m_file, buf, BUF_SIZE ) )
	{
		std::cerr << "Error writing to audio file.\n";
		return 1;
	}

	return 0;
}

void AudioWrite::close ()
{
	if ( m_file )
	{
		sf_close ( m_file );
		m_file = NULL;
	}
}

