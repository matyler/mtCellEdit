/*
	Copyright (C) 2017 Mark Tyler

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



#define BUF_CHUNK 32768



static int get_bit_mask (
	int	const	n
	)
{
	switch ( n )
	{
	case 1:	return 1;
	case 2:	return 3;
	case 3:	return 7;
	case 4:	return 15;
	case 5:	return 31;
	case 6:	return 63;
	case 7:	return 127;
	}

	return 255;
}

mtKit::BitPackWrite::BitPackWrite ()
	:
	m_buf		( NULL ),
	m_buflim	( NULL ),
	m_cwl		( NULL ),
	m_bit_next	( 0 ),
	m_buf_size	( 0 )
{
}

mtKit::BitPackWrite::~BitPackWrite ()
{
	free ( m_buf );
	m_buf = NULL;

	m_buflim = NULL;
	m_cwl = NULL;
	m_bit_next = 0;
	m_buf_size = 0;
}

int mtKit::BitPackWrite::write (
	int	const	byte,
	int	const	bit_tot
	)
{
	if ( bit_tot < 1 )
	{
		return 0;
	}

	if ( bit_tot > 8 )
	{
		return 1;
	}

	if ( m_cwl >= m_buflim && buf_expand () )
	{
		return 1;
	}

	m_cwl[0] = (unsigned char)(m_cwl[0] | (byte << m_bit_next));

	int const bits_done = 8 - m_bit_next;

	m_bit_next += bit_tot;

	if ( m_bit_next < 8 )
	{
		return 0;
	}

	m_bit_next -= 8;
	m_cwl++;

	if ( 0 == m_bit_next )
	{
		return 0;
	}

	if ( m_cwl >= m_buflim && buf_expand () )
	{
		return 1;
	}

	m_cwl[0] = (unsigned char)( byte >> bits_done );

	return 0;
}

unsigned char const * mtKit::BitPackWrite::get_buf () const
{
	return m_buf;
}

size_t mtKit::BitPackWrite::get_buf_len () const
{
	if ( 0 == m_bit_next )
	{
		return (size_t)(m_cwl - m_buf);
	}

	return (size_t)(m_cwl - m_buf + 1);
}

int mtKit::BitPackWrite::buf_expand ()
{
	if ( ! m_buf )
	{
		// Initial allocation

		m_buf_size = BUF_CHUNK;
		m_buf = (unsigned char *)calloc ( m_buf_size, 1 );

		if ( ! m_buf )
		{
			m_buf_size = 0;
			return 1;
		}

		m_buflim = m_buf + m_buf_size;
		m_cwl = m_buf;
		m_bit_next = 0;

		return 0;
	}

	// Expand current allocation

	size_t	const	nsize = m_buf_size + BUF_CHUNK;
	unsigned char	* nbuf = (unsigned char *)realloc ( m_buf, nsize );

	if ( ! nbuf )
	{
		return 1;
	}

	memset ( nbuf + m_buf_size, 0, nsize - m_buf_size );

	m_buflim = nbuf + nsize;
	m_cwl = nbuf + (m_cwl - m_buf);
	m_buf_size = nsize;
	m_buf = nbuf;

	return 0;
}



///	------------------------------------------------------------------------



mtKit::BitPackRead::BitPackRead (
	unsigned char	const * const	mem,
	size_t			const	memlen
	)
	:
	m_mem_start	( mem ),
	m_mem		( mem ),
	m_memlim	( mem + memlen ),
	m_bit_next	( 0 )
{
}

int mtKit::BitPackRead::read (
	int		&byte,
	int	const	bit_tot
	)
{
	if ( bit_tot < 1 )
	{
		return 0;
	}

	if ( m_mem >= m_memlim || bit_tot > 8 )
	{
		return 1;
	}

	int const bit_mask = get_bit_mask ( bit_tot );
	int const shift = m_bit_next;

	byte = (m_mem[0] >> shift) & bit_mask;

	m_bit_next += bit_tot;

	if ( m_bit_next < 8 )
	{
		return 0;
	}

	m_bit_next -= 8;
	m_mem++;

	if ( 0 == m_bit_next )
	{
		return 0;
	}

	if ( m_mem >= m_memlim )
	{
		return 1;
	}

	int const lshift = (8 - shift);

	byte |= ( (m_mem[0] & get_bit_mask ( m_bit_next )) << lshift );

	return 0;
}

void mtKit::BitPackRead::restart (
	unsigned char	const * const	mem,
	size_t			const	memlen
	)
{
	m_mem_start	= mem;
	m_mem		= mem;
	m_memlim	= mem + memlen;
	m_bit_next	= 0;
}

size_t mtKit::BitPackRead::bytes_left () const
{
	if ( m_mem >= m_memlim )
	{
		return 0;
	}

	return (size_t)(m_memlim - m_mem - 1 + (0==m_bit_next ? 1 : 0 ) );
}

