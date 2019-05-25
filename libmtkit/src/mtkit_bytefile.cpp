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

#include "private.h"



/// READ -----------------------------------------------------------------------



mtKit::ByteFileRead::ByteFileRead ()
	:
	m_fp	(),
	m_pos	( 0 )
{
}

mtKit::ByteFileRead::~ByteFileRead ()
{
	close ();
}

int mtKit::ByteFileRead::open (
	char	const * const	filename,
	uint64_t	const	pos
	)
{
	if ( ! filename )
	{
		return 1;
	}

	FILE * fp = fopen ( filename, "rb" );

	if ( ! fp )
	{
		return 1;
	}

	if ( fseek ( fp, (long)pos, SEEK_SET ) )
	{
		fclose ( fp );
		fp = NULL;
		return 1;
	}

	set_file ( fp );
	m_pos = pos;

	return 0;
}

void mtKit::ByteFileRead::close ()
{
	set_file ( NULL );
	m_pos = 0;
}

size_t mtKit::ByteFileRead::read ( void * const mem, size_t const len )
{
	if ( ! m_fp || ! mem || len < 1 )
	{
		return 0;
	}

	size_t const res = fread ( mem, 1, len, m_fp );

	if ( res < 1 )
	{
		close ();
	}

	m_pos += res;

	return res;
}

void mtKit::ByteFileRead::set_file ( FILE * const fp )
{
	if ( m_fp )
	{
		fclose ( m_fp );
	}

	m_fp = fp;
}



/// WRITE ----------------------------------------------------------------------



mtKit::ByteFileWrite::ByteFileWrite () : m_fp ()
{
}

mtKit::ByteFileWrite::~ByteFileWrite ()
{
	close ();
}

int mtKit::ByteFileWrite::open ( char const * const filename )
{
	if ( ! filename )
	{
		return 1;
	}

	FILE * fp = fopen ( filename, "wb" );

	if ( ! fp )
	{
		return 1;
	}

	set_file ( fp );

	return 0;
}

void mtKit::ByteFileWrite::close ()
{
	set_file ( NULL );
}

int mtKit::ByteFileWrite::write (
	void	* const	mem,
	size_t	const	len
	)
{
	if ( ! m_fp || ! mem || len < 1 )
	{
		return 1;
	}

	if ( len == fwrite ( mem, 1, len, m_fp ) )
	{
		return 0;
	}

	close ();

	return 1;
}

void mtKit::ByteFileWrite::set_file ( FILE * const fp )
{
	if ( m_fp )
	{
		fclose ( m_fp );
	}

	m_fp = fp;
}

