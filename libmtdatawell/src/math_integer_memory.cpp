/*
	Copyright (C) 2022 Mark Tyler

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

#include "mtdatawell_math.h"



#define HEADER_SIZE		10
#define MPZ_BYTE_ORDER		-1
#define MPZ_BYTE_ENDIAN		-1
#define MPZ_BYTE_NAILS		0



namespace {

static int read_header (
	unsigned char	const * const	mem,
	int				& extra,
	size_t				& size
	)
{
	if ( ! mem )
	{
		std::cerr << "mtDW::IntegerMemory::read_header error: "
			"mem = NULL\n";
		return 1;
	}

	if ( mem[0] != HEADER_SIZE )
	{
		std::cerr << "mtDW::IntegerMemory::read_header error: "
			"bad header size=" << mem[0]
			<< " expected=" << HEADER_SIZE
			<< "\n";
		return 1;
	}

	size_t tot = 0;
	int shift = 0;
	unsigned char const * src = mem + 2;

	for ( size_t i = 0; i < 8; i++ )
	{
		tot += (((size_t)(*src++)) << shift);
		shift += 8;
	}

	extra = (mem[1] & 1) ? 1 : 0;
	size = tot;

	return 0;
}

static int parse_header (
	unsigned char	const * const	mem,
	size_t			const	mem_size,
	int				& extra
	)
{
	size_t tot;

	if ( mem_size < HEADER_SIZE )
	{
		std::cerr << "mtDW::IntegerMemory::parse_header error: "
			"mem_size=" << mem_size << " < HEADER_SIZE="
			<< HEADER_SIZE
			<< "\n";
		return 1;
	}

	if ( read_header ( mem, extra, tot ) )
	{
		// read_header reports all errors
		return 1;
	}

	if ( tot != mem_size )
	{
		std::cerr << "mtDW::IntegerMemory::parse_header error: "
			"memory buffer size mismatch: mem_size=" << mem_size
			<< " counted=" << tot
			<< "\n";
		return 1;
	}

	return 0;
}

} // namespace



int mtDW::IntegerMemory::import_memory (
	unsigned char	const * const	mem,
	size_t			const	mem_size,
	int			const	num_sign
	)
{
	if ( ! mem && mem_size > 0 )
	{
		std::cerr << "mtDW::IntegerMemory::import_memory error: "
			"mem is NULL, but mem_size > 0\n";
		return 1;
	}

	if ( allocate_buffer ( mem_size, num_sign ) )
	{
		// allocate_buffer reports all errors
		return 1;
	}

	if ( mem_size > 0 )
	{
		memcpy ( m_mem + HEADER_SIZE, mem, mem_size );
	}

	return 0;
}

int mtDW::IntegerMemory::import_memory_with_header (
	unsigned char	const * const	mem,
	size_t			const	mem_size
	)
{
	int	tmp_extra;

	if ( parse_header ( mem, mem_size, tmp_extra ) )
	{
		// parse_header reports all errors
		return 1;
	}

	if ( allocate_buffer ( mem_size - HEADER_SIZE, tmp_extra ) )
	{
		// allocate_buffer reports all errors
		return 1;
	}

	memcpy ( m_mem, mem, mem_size );

	return 0;
}

int mtDW::IntegerMemory::import_number ( Integer const & num )
{
	size_t const buflen = (mpz_sizeinbase ( num.get_num(), 2 ) + 7) / 8;
	int const sign = ( num.sign() < 0) ? 1 : 0;

	if ( allocate_buffer ( buflen, sign ) )
	{
		// allocate_buffer reports all errors
		return 1;
	}

	size_t countp = 0;
	unsigned char * const dest = m_mem + HEADER_SIZE;

	mpz_export ( dest, &countp, MPZ_BYTE_ORDER, buflen, MPZ_BYTE_ENDIAN,
		MPZ_BYTE_NAILS, num.get_num() );

	if ( 0 == countp )
	{
		// Ensure that all buffer bytes are initialized
		memset ( dest, 0, buflen );
	}

	return 0;
}

int mtDW::IntegerMemory::import_file ( ::FILE * const fp )
{
	if ( ! fp )
	{
		std::cerr << "mtDW::IntegerMemory::import_file error: "
			"no fp.\n";
		return 1;
	}

	unsigned char header[ HEADER_SIZE ];

	if ( HEADER_SIZE != fread (header, 1, sizeof(header), fp ) )
	{
		std::cerr << "mtDW::IntegerMemory::import_file error: "
			"Unable to read the header from the file.\n";
		return 1;
	}

	int extra;
	size_t size;

	if ( read_header ( header, extra, size ) )
	{
		// read_header reports all errors
		return 1;
	}

	unsigned char * buf = (unsigned char *)malloc ( size );

	if ( ! buf )
	{
		std::cerr << "mtDW::IntegerMemory::import_file error: "
			"malloc fail.\n";
		return 1;
	}

	size_t const todo = size - HEADER_SIZE;

	if ( todo > 0 )
	{
		size_t const actual = fread( buf + sizeof(header), 1, todo, fp);

		if ( todo != actual )
		{
			free ( buf );
			buf = nullptr;

			std::cerr << "mtDW::IntegerMemory::import_file error: "
				"unable to read all bytes from file."
				"expected=" << todo
				<< " actual=" << actual
				<< "\n";

			return 1;
		}
	}

	memcpy ( buf, header, sizeof(header) );
	clear ();

	m_mem = buf;
	m_size = size;
	m_extra = extra;

	return 0;
}

int mtDW::IntegerMemory::import_file ( char const * const filename )
{
	if ( ! filename )
	{
		std::cerr << "mtDW::IntegerMemory::import_file error: "
			"no filename passed.\n";
		return 1;
	}

	mtKit::ByteFileRead file;

	if ( file.open ( filename, 0 ) )
	{
		std::cerr << "mtDW::IntegerMemory::import_file error: unable "
			"to open file: '" << filename << "'\n";
		return 1;
	}

	return import_file ( file.get_fp() );
}

int mtDW::IntegerMemory::export_number ( Integer & num ) const
{
	if ( ! m_mem || HEADER_SIZE == m_size )
	{
		// Nothing to export, so make num zero
		num.set_number ( (signed long int)0 );
		return 0;
	}

	mpz_import ( num.get_num(), m_size - HEADER_SIZE, MPZ_BYTE_ORDER, 1,
		MPZ_BYTE_ENDIAN, MPZ_BYTE_NAILS, m_mem + HEADER_SIZE );

	if ( m_extra & 1 )
	{
		num.negate();
	}

	return 0;
}

int mtDW::IntegerMemory::export_file ( ::FILE * const fp ) const
{
	if ( ! fp )
	{
		std::cerr << "mtDW::IntegerMemory::export_file error: "
			"no fp.\n";
		return 1;
	}

	if ( ! m_mem )
	{
		std::cerr << "mtDW::IntegerMemory::export_file error: "
			"no memory allocated.\n";
		return 1;
	}

	if ( m_size != fwrite ( m_mem, 1, m_size, fp ) )
	{
		std::cerr << "mtDW::IntegerMemory::export_file error: unable "
			"to write all bytes to file.\n";
		return 1;
	}

	return 0;
}

int mtDW::IntegerMemory::export_file ( char const * const filename ) const
{
	if ( ! filename )
	{
		std::cerr << "mtDW::IntegerMemory::export_file error: "
			"no filename passed.\n";
		return 1;
	}

	mtKit::ByteFileWrite file;

	if ( file.open ( filename ) )
	{
		std::cerr << "mtDW::IntegerMemory::export_file error: unable "
			"to open file: '" << filename << "'\n";
		return 1;
	}

	return export_file ( file.get_fp() );
}

int mtDW::IntegerMemory::allocate_buffer (
	size_t	const	bufsize,
	int	const	num_sign
	)
{
	size_t		const	size = bufsize + HEADER_SIZE;
	unsigned char	* const	buf = (unsigned char *)malloc ( size );

	if ( ! buf )
	{
		std::cerr << "mtDW::IntegerMemory::allocate_buffer error: "
			"malloc fail.\n";
		return 1;
	}

	clear ();

	m_mem = buf;
	m_size = size;

	unsigned char * dest = buf;

	*dest++ = HEADER_SIZE;

	if ( num_sign )
	{
		*dest++ = 1;
		m_extra = 1;
	}
	else
	{
		*dest++ = 0;
		m_extra = 0;
	}

	size_t val = size;

	for ( size_t i = 0; i < 8; i++ )
	{
		*dest++ = (unsigned char)(val & 255);
		val = val >> 8;
	}

	return 0;
}

