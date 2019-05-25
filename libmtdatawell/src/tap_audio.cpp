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

#include "tap.h"



/// READ -----------------------------------------------------------------------



mtDW::TapAudioRead::TapAudioRead ()
	:
	m_buf		(),
	m_buflen	(0),
	m_file		(),
	m_info		()
{
}

mtDW::TapAudioRead::~TapAudioRead ()
{
	close ();
	free_buf ();
}

void mtDW::TapAudioRead::free_buf ()
{
	free ( m_buf );
	m_buf = NULL;
	m_buflen = 0;
}

int mtDW::TapAudioRead::alloc_buf ()
{
	free_buf ();

	m_buflen = (size_t)(m_info.channels * BUF_FRAMES);

	if ( m_buflen < (size_t)BUF_FRAMES )
	{
		free_buf ();
		return report_error ( ERROR_AUDIO_BAD_CHANNELS );
	}

	m_buf = (short *)calloc ( sizeof(*m_buf), m_buflen );

	if ( ! m_buf )
	{
		free_buf ();
		return report_error ( ERROR_HEAP_EMPTY );
	}

	return 0;
}

int mtDW::TapAudioRead::open (
	char	const * const	filename
	)
{
	close ();

	m_file = sf_open ( filename, SFM_READ, & m_info );

	if ( ! m_file )
	{
		return report_error ( ERROR_AUDIO_OPEN_INPUT );
	}

	int const res = alloc_buf ();

	if ( res )
	{
		close ();

		return res;
	}

	return 0;
}

int mtDW::TapAudioRead::read (
	short	** const	buf,
	size_t		* const	buflen
	)
{
	if ( ! buf || ! buflen || ! m_file || m_info.channels < 1 )
	{
		return report_error ( ERROR_AUDIO_READ );
	}

	sf_count_t const frames = sf_readf_short ( m_file, m_buf, BUF_FRAMES );

	buf[0] = m_buf;
	buflen[0] = (size_t)(frames * m_info.channels);

	return 0;
}

void mtDW::TapAudioRead::close ()
{
	if ( m_file )
	{
		sf_close ( m_file );
		m_file = NULL;
		memset ( &m_info, 0, sizeof(m_info) );
	}
}

int64_t mtDW::TapAudioRead::get_read_capacity () const
{
	return (m_info.frames * m_info.channels) / 8;
}



/// WRITE ----------------------------------------------------------------------



mtDW::TapAudioWrite::TapAudioWrite ()
	:
	m_file (),
	m_info ()
{
}

mtDW::TapAudioWrite::~TapAudioWrite ()
{
	close ();
}

int mtDW::TapAudioWrite::open (
	SF_INFO	const * const	src_info,
	char	const * const	filename
	)
{
	close ();

	if ( src_info )
	{
		m_info.samplerate	= src_info->samplerate;
		m_info.channels		= src_info->channels;
	}

	m_info.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;

	m_file = sf_open ( filename, SFM_WRITE, & m_info );

	if ( ! m_file )
	{
		return report_error ( ERROR_AUDIO_OPEN_OUTPUT );
	}

	return 0;
}

int mtDW::TapAudioWrite::write (
	short	const * const	buf,
	size_t		const	buflen
	)
{
	if ( ! buf || ! m_file || m_info.channels < 1 )
	{
		return report_error ( ERROR_AUDIO_WRITE_INSANITY );
	}

	size_t const chans = (size_t)m_info.channels;
	sf_count_t const frames = (sf_count_t)(buflen / chans);

	if ( frames != sf_writef_short ( m_file, buf, frames ) )
	{
		return report_error ( ERROR_AUDIO_WRITE );
	}

	return 0;
}

void mtDW::TapAudioWrite::close ()
{
	if ( m_file )
	{
		sf_close ( m_file );
		m_file = NULL;
		memset ( &m_info, 0, sizeof(m_info) );
	}
}

