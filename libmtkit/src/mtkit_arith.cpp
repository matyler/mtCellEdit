/*
	Copyright (C) 2019 Mark Tyler

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

#include "mtkit.h"



static uint64_t const SPAN_MAX = (((uint64_t)1) << 56);



mtKit::ArithDecode::ArithDecode ()
	:
	m_mem		( 0 ),
	m_span_mem	( 1 )
{
}

int mtKit::ArithDecode::push_code (
	int	const	code,
	int	const	span
	)
{
	if (	span < 2
		|| span > 256
		|| code < 0
		|| code > 255
		|| code >= span
		)
	{
		std::cerr << "ArithDecode::push_code bad arg - code="
			<< code << " span=" << span << "\n";
		return 1;
	}

	if ( m_span_mem > SPAN_MAX )
	{
		return 1;
	}

	m_mem += m_span_mem * (uint64_t)code;
	m_span_mem *= (uint64_t)span;

	return 0;
}

int mtKit::ArithDecode::pop_mem (
	uint8_t		* const	dest,
	size_t		& size
	)
{
	if ( ! dest )
	{
		return 1;
	}

	for ( size = 0; size < 7; size++ )
	{
		if ( m_span_mem < 2 )
		{
			break;
		}

		dest[ size ] = (uint8_t)m_mem;

		m_mem /= 256;
		m_span_mem /= 256;
	}

	// Flush object
	m_mem = 0;
	m_span_mem = 1;

	return 0;
}

int mtKit::ArithDecode::get_encoded_byte_count () const
{
	int res = 0;

	for ( uint64_t i = m_span_mem; i > 1; i >>= 8, res++ )
	{
	}

	return res;
}



/// ----------------------------------------------------------------------------



mtKit::ArithEncode::ArithEncode ()
	:
	m_mem		( 0 ),
	m_span_mem	( 1 ),
	m_span_popped	( 1 )
{
}

void mtKit::ArithEncode::push_mem (
	uint8_t	const * const	mem,
	size_t		const	len
	)
{
	if ( ! mem )
	{
		return;
	}

	m_mem = 0;
	m_span_mem = 1;
	m_span_popped = 1;

	size_t const max = MIN ( 7, len );

	for ( size_t i = 0; i < max; i++ )
	{
		m_mem |= ( ((uint64_t)mem[i]) << (8*i) );
		m_span_mem *= 256;
	}
}

int mtKit::ArithEncode::pop_code (
	int	const	span,
	int		& code
	)
{
	if ( span < 2 || span > 256 )
	{
		std::cerr << "ArithEncode::pop_code bad arg - "
			" span=" << span << "\n";
		return 1;
	}

	if ( m_span_mem <= m_span_popped )
	{
		return 1;
	}

	uint64_t const len = (uint64_t)span;

	code = (int)(m_mem % len);

	m_mem /= len;
	m_span_popped *= len;

	return 0;
}

int mtKit::ArithEncode::get_encoded_byte_count () const
{
	int res = 0;

	for ( uint64_t span = 1; span < SPAN_MAX; span <<= 8, res++ )
	{
		if ( m_span_popped < ( span + 1 ) )
		{
			return res;
		}
	}

	return res;
}

