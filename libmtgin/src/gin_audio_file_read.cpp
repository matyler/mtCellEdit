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



class mtGin::AudioFileRead::Op
{
public:
	Op ();
	~Op ();

	int open ( char const * filename, SDL_AudioSpec const & spec );
	size_t read ( short * buf, size_t buflen );

private:
	void close ();

/// ----------------------------------------------------------------------------

	SNDFILE		* m_sndfile	= nullptr;
	SF_INFO		m_sfinfo;

	MTKIT_RULE_OF_FIVE( Op )
};



mtGin::AudioFileRead::Op::Op ()
{
}

mtGin::AudioFileRead::Op::~Op ()
{
	close ();
}

void mtGin::AudioFileRead::Op::close ()
{
	if ( m_sndfile )
	{
		sf_close ( m_sndfile );
		m_sndfile = nullptr;
	}
}

int mtGin::AudioFileRead::Op::open (
	char	const * const	filename,
	SDL_AudioSpec	const	& spec
	)
{
	if ( ! filename )
	{
		std::cerr << "open failed : bad filename\n";
		return 1;
	}

	close ();

	memset ( &m_sfinfo, 0, sizeof(m_sfinfo) );

	m_sfinfo.channels = spec.channels;
	m_sfinfo.samplerate = spec.freq;
	m_sfinfo.format = SF_FORMAT_PCM_16;

	m_sndfile = sf_open ( filename, SFM_READ, & m_sfinfo );

	if ( ! m_sndfile )
	{
		std::cerr << "sf_open failed : " << filename << "\n";
		return 1;
	}

	return 0;
}

size_t mtGin::AudioFileRead::Op::read (
	short	* const	buf,
	size_t	const	buflen
	)
{
	if ( ! buf || ! m_sndfile )
	{
		return 1;
	}

	sf_count_t const frames = ((sf_count_t)buflen) / m_sfinfo.channels;
	if ( frames < 1 )
	{
		return 1;
	}

	sf_count_t const in = sf_readf_short ( m_sndfile, buf, frames );

	if ( in < 1 )
	{
		close ();
		return 0;
	}

	return (size_t)in;
}



/// ----------------------------------------------------------------------------



mtGin::AudioFileRead::AudioFileRead ()
	:
	m_op		( new AudioFileRead::Op )
{
}

mtGin::AudioFileRead::~AudioFileRead ()
{
}

int mtGin::AudioFileRead::open (
	char	const * const	filename,
	SDL_AudioSpec	const	& spec
	) const
{
	return m_op->open ( filename, spec );
}

size_t mtGin::AudioFileRead::read ( short * buf, size_t buflen ) const
{
	return m_op->read ( buf, buflen );
}

