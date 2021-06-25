/*
	Copyright (C) 2021 Mark Tyler

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

#include "private.h"



mtGin::AudioPlay::AudioPlay ()
{
}

mtGin::AudioPlay::~AudioPlay ()
{
	stop ();
}

int mtGin::AudioPlay::open_device (
	int		const	device,
	SDL_AudioSpec	const &	spec
	)
{
	if ( spec.format != AUDIO_S16SYS )
	{
		std::cerr << "open_device fail : bad format "
			"(only AUDIO_S16SYS is valid)\n";
		return 1;
	}

	char const * dev_name = NULL;

	if ( device >= 0 )
	{
		dev_name = SDL_GetAudioDeviceName ( device, 0 );

		if ( ! dev_name )
		{
			std::cerr << "Unable to open device " << device << "\n";
			return 1;
		}
	}

	stop ();

	m_dev_out = SDL_OpenAudioDevice ( dev_name, 0, &spec, &m_spec, 0 );

	if ( m_dev_out < 1 )
	{
		std::cerr << "Unable to open device " << device << " : "
			<< dev_name << "\n";
		return 1;
	}

	// Sanity checking
	if (	m_spec.freq < 8000
		|| m_spec.format != AUDIO_S16SYS
		|| m_spec.channels < 1
		|| m_spec.samples < 1
		)
	{
		std::cerr << "AudioPlay::open_device sanity failure "
			"channels=" << m_spec.channels
			<< " freq=" << m_spec.freq
			<< " format=" << m_spec.format
			<< " AUDIO_S16SYS=" << AUDIO_S16SYS
			<< "\n";

		stop ();
		return 1;
	}

	// Prepare the buffer for libsndfile to use later

	if ( m_buf )
	{
		free ( m_buf );
	}

	m_bufframes = m_spec.samples;
	m_bufbytes = (Uint32)(m_spec.channels * m_bufframes * sizeof(m_buf[0]));

	m_buf = (short *)calloc ( m_spec.channels * m_bufframes,
		sizeof(m_buf[0]) );

	if ( ! m_buf )
	{
		std::cerr << "AudioPlay unable to allocate buffer size="
			<< m_bufbytes << "\n";
		stop ();
		return 1;
	}

	m_vu.m_level.clear ();
	m_vu.m_level.resize ( m_spec.channels );

	m_status = SDL_AUDIO_PLAYING;
	pause ();

	return 0;
}

void mtGin::AudioPlay::set_file ( AudioFileRead * const file )
{
	m_file = file;
}

int mtGin::AudioPlay::queue_data (
	short	const * const	buf,
	size_t		const	buflen
	)
{
	if ( SDL_AUDIO_PLAYING != m_status )
	{
		return 1;
	}

	Uint32 const bytes_tot = (Uint32)(buflen * sizeof(buf[0]));
	SDL_QueueAudio ( m_dev_out, buf, bytes_tot );

	return 0;
}

int mtGin::AudioPlay::queue_file_data ()
{
	while ( SDL_AUDIO_PLAYING == m_status )
	{
		Uint32 const q_size = SDL_GetQueuedAudioSize ( m_dev_out );
		if ( q_size > (m_bufbytes * 2) )
		{
			return 0;
		}

		if ( ! m_file )
		{
			if ( q_size < 1 )
			{
				// Only stop when audio has finished playing
				stop ();
			}

			return 0;
		}

		size_t	const	chantot = m_vu.m_level.size ();
		size_t	const	frames = m_file->read ( m_buf, m_bufframes );

		if ( frames < 1 )
		{
			// EOF
			m_file = nullptr;

			for ( size_t chan = 0; chan < chantot; chan++ )
			{
				m_vu.m_level[ chan ] = 0;
			}

			return -1;
		}

		Uint32 const bytes_tot = (Uint32)(
			frames * sizeof(m_buf[0]) * chantot );

		SDL_QueueAudio ( m_dev_out, m_buf, bytes_tot );

		short	const	* src = m_buf;
		short	const	* src_end = src + ((size_t)frames) * chantot;
		std::vector<short> min;
		std::vector<short> max;

		min.resize ( chantot );
		max.resize ( chantot );

		// Get initial max/min
		for ( size_t chan = 0; chan < chantot; chan++ )
		{
			min[ chan ] = max[ chan ] = *src++;
		}

		while ( src < src_end )
		{
			for ( size_t chan = 0; chan < chantot; chan++ )
			{
				short const level = *src++;
				min[ chan ] = MIN ( min[ chan ], level );
				max[ chan ] = MAX ( max[ chan ], level );
			}
		}

		// Calculate VU levels for each channel
		for ( size_t chan = 0; chan < chantot; chan++ )
		{
			m_vu.m_level[ chan ] = max[ chan ] - min[ chan ];
		}
	}

	return 1;
}

int mtGin::AudioPlay::pause ()
{
	if ( m_dev_out < 1 )
	{
		return 1;
	}

	if ( SDL_AUDIO_PLAYING == m_status )
	{
		m_status = SDL_AUDIO_PAUSED;
		SDL_PauseAudioDevice ( m_dev_out, SDL_TRUE );

		return 0;
	}

	return 1;
}

int mtGin::AudioPlay::resume ()
{
	if ( m_dev_out < 1 )
	{
		return 1;
	}

	if ( SDL_AUDIO_PAUSED == m_status )
	{
		m_status = SDL_AUDIO_PLAYING;
		SDL_PauseAudioDevice ( m_dev_out, SDL_FALSE );
		return 0;
	}

	return 1;
}

void mtGin::AudioPlay::stop ()
{
	if ( m_dev_out > 0 )
	{
		SDL_PauseAudioDevice ( m_dev_out, SDL_TRUE );
		SDL_CloseAudioDevice ( m_dev_out );
		m_dev_out = 0;
	}

	m_status = SDL_AUDIO_STOPPED;

	free ( m_buf );
	m_buf = nullptr;
	m_bufframes = 0;
	m_bufbytes = 0;
}

void mtGin::AudioPlay::toggle_pause_resume ()
{
	switch ( m_status )
	{
	case SDL_AUDIO_PAUSED:
		resume ();
		break;

	case SDL_AUDIO_PLAYING:
		pause ();
		break;

	default:
		std::cerr << "Toggle status failure\n";
		break;
	}
}

