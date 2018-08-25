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

#include "soda.h"



int mtDW::SodaOp::decode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	)
{
	SodaFile soda;

	if ( soda.open ( input ) )
	{
		return 1;
	}

	mtKit::ByteBuf	xorb;

	if ( ! soda.m_mode_raw )
	{
		if ( ! butt )
		{
			std::cerr << "No butt available.\n";
			return 1;
		}

		xorb.array_len = CHUNK_SIZE;
		xorb.array = (uint8_t *)calloc ( 1, CHUNK_SIZE );

		if ( ! xorb.array )
		{
			std::cerr << "Unable to allocate XOR buffer.\n";
			return 1;
		}

		if ( butt->read_set_butt ( soda.m_butt_name, soda.m_bucket,
			soda.m_bucket_pos ) )
		{
			std::cerr << "Unable to prepare butt.\n";
			return 1;
		}
	}

	mtKit::ByteFileWrite file_out;

	if ( file_out.open ( output ) )
	{
		std::cerr << "Unable to open output file.\n";
		return 1;
	}

	char		id[ mtKit::ChunkFile::CHUNK_HEADER_SIZE ] = {0};
	uint8_t		* buf;
	uint32_t	buflen;
	mtKit::ByteBuf	membuf;
	uint64_t	todo = soda.m_filesize;

	while ( todo > 0 )
	{
		membuf.set ( NULL, 0 );

		if ( soda.m_chunk.get_chunk( &buf, &buflen, id, NULL ) )
		{
			std::cerr << "Missing bytes not found: " << todo <<"\n";
			return 1;
		}

		if ( 0 != memcmp ( id, FILE_CHUNK_ID, sizeof(id) ) )
		{
			std::cerr << "Invalid Soda file data chunk ID.\n";
			return 1;
		}

		membuf.set ( buf, buflen );

		if ( ! soda.m_mode_raw )
		{
			if ( buflen > xorb.array_len )
			{
				std::cerr << "Chunk too large.\n";
				return 1;
			}

			if ( butt->read_get_data ( xorb.array, buflen ) )
			{
				std::cerr << "Unable to get butt data.\n";
				return 1;
			}

			for ( size_t i = 0; i < buflen; i++ )
			{
				buf[i] ^= xorb.array[i];
			}
		}

		size_t const tot = (size_t)MIN ( todo, buflen );

		if ( tot > 0 )
		{
			if ( file_out.write ( buf, tot ) )
			{
				std::cerr <<"Problem writing to output file.\n";
				return 1;
			}

			todo -= tot;
		}
	}

	return 0;
}

int mtDW::SodaOp::multi_decode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	)
{
	FilenameSwap	name ( output );
	int		tot = 0;

	name.m_res = decode ( butt, input, output );

	if ( 0 == name.m_res )
	{
		tot ++;

		while ( 1 )
		{
			name.m_res = decode ( butt, name.f1, name.f2 );

			name.swap ();

			if ( name.m_res )
			{
				break;
			}
		}
	}

	if ( tot > 0 )
	{
		name.m_res = 0;
	}
	else
	{
		name.m_res = 1;
	}

	return name.m_res;
}

