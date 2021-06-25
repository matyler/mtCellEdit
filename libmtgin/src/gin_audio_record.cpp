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



mtGin::AudioRecord::AudioRecord ()
{
}

mtGin::AudioRecord::~AudioRecord ()
{
	stop ();
}

void mtGin::AudioRecord::receive_audio (
	Uint8	const * const	stream,
	int		const	len
	)
{
	Uint32	const	deq = (Uint32)len;
	size_t	const	lenshort = deq / sizeof(short);

	if (	m_cb_panic
		|| SDL_AUDIO_PLAYING != m_status
		|| lenshort < 1
		)
	{
		return;
	}

	short	const	* src = (short const *)stream;

	if ( m_file && m_file->write ( (short const *)stream, lenshort ) )
	{
		m_cb_panic = 1;

		return;
	}

	m_vu.m_tick++;

	size_t	const	chantot = m_vu.m_level.size ();
	short	const	* src_end = src + deq / sizeof(src[0]);
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

static void recording_callback (
	void	* const	userdata,
	Uint8	* const	stream,
	int	const	len
	)
{
	auto * const audio = static_cast<mtGin::AudioRecord *>( userdata );

	audio->receive_audio ( stream, len );
}

int mtGin::AudioRecord::open_device (
	int		const	device,
	SDL_AudioSpec		& spec
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
		dev_name = SDL_GetAudioDeviceName ( device, 1 );

		if ( ! dev_name )
		{
			std::cerr << "Unable to open device " << device << "\n";
			return 1;
		}
	}

	stop ();

	spec.callback = recording_callback;
	spec.userdata = this;
	m_dev_in = SDL_OpenAudioDevice ( dev_name, SDL_TRUE, &spec, &m_spec, 0);

	if ( m_dev_in < 1 )
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
		std::cerr << "AudioFileOut::open_device sanity failure "
			"channels=" << m_spec.channels
			<< " freq=" << m_spec.freq
			<< " format=" << m_spec.format
			<< " AUDIO_S16SYS=" << AUDIO_S16SYS
			<< "\n";

		stop ();
		return 1;
	}

	m_vu.m_level.clear ();
	m_vu.m_level.resize ( m_spec.channels );

	m_status = SDL_AUDIO_PLAYING;
	pause ();

	return 0;
}

void mtGin::AudioRecord::set_file ( AudioFileWrite * const file )
{
	m_file = file;
}

int mtGin::AudioRecord::pause ()
{
	if ( m_dev_in < 1 )
	{
		return 1;
	}

	if ( SDL_AUDIO_PLAYING == m_status )
	{
		m_status = SDL_AUDIO_PAUSED;
		SDL_PauseAudioDevice ( m_dev_in, SDL_TRUE );

		return 0;
	}

	return 1;
}

int mtGin::AudioRecord::resume ()
{
	if ( m_dev_in < 1 )
	{
		return 1;
	}

	if ( SDL_AUDIO_PAUSED == m_status )
	{
		m_status = SDL_AUDIO_PLAYING;
		SDL_PauseAudioDevice ( m_dev_in, SDL_FALSE );
		return 0;
	}

	return 1;
}

void mtGin::AudioRecord::toggle_pause_resume ()
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

int mtGin::AudioRecord::get_status ()
{
	if ( m_cb_panic )
	{
		stop ();
	}

	return m_status;
}

void mtGin::AudioRecord::stop ()
{
	if ( m_dev_in > 0 )
	{
		SDL_PauseAudioDevice ( m_dev_in, SDL_TRUE );
		SDL_CloseAudioDevice ( m_dev_in );
		m_dev_in = 0;
	}

	m_status = SDL_AUDIO_STOPPED;
	m_cb_panic = 0;

	m_file = nullptr;
}

