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

#include "soda.h"



int mtDW::Soda::Op::decode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	)
{
	if ( ! input || ! output )
	{
		return report_error ( ERROR_SODA_DECODE_INSANITY );
	}

	SodaFile	soda_file;

	RETURN_ON_ERROR ( soda_file.open ( input ) )

	ByteBuf		xorb;

	if ( ! soda_file.m_mode_raw )
	{
		if ( ! butt )
		{
			return report_error ( ERROR_SODA_DECODE_NO_BUTT );
		}

		if ( xorb.allocate ( SODA_CHUNK_SIZE ) )
		{
			return report_error ( ERROR_SODA_DECODE_NO_XOR );
		}

		RETURN_ON_ERROR ( butt->read_set_otp ( soda_file.m_otp_name,
			soda_file.m_bucket, soda_file.m_bucket_pos ) )
	}

	mtKit::ByteFileWrite file_out;

	if ( file_out.open ( output ) )
	{
		return report_error ( ERROR_SODA_OPEN_OUTPUT );
	}

	char		id[ mtKit::ChunkFile::CHUNK_HEADER_SIZE ] = {0};
	uint8_t		* buf;
	uint32_t	buflen;
	ByteBuf		membuf;
	uint64_t	todo = soda_file.m_filesize;

	while ( todo > 0 )
	{
		membuf.set ( NULL, 0 );

		if ( soda_file.m_chunk.get_chunk( &buf, &buflen, id, NULL ) )
		{
			return report_error ( ERROR_SODA_MISSING_DATA );
		}

		if ( 0 != memcmp ( id, SODA_FILE_CHUNK_ID, sizeof(id) ) )
		{
			return report_error ( ERROR_SODA_BAD_CHUNK_ID );
		}

		membuf.set ( buf, buflen );

		if ( ! soda_file.m_mode_raw )
		{
			if ( buflen > xorb.get_size () )
			{
				return report_error ( ERROR_SODA_BIG_CHUNK );
			}

			uint8_t * const src = xorb.get_buf ();

			RETURN_ON_ERROR ( butt->read_get_data ( src, buflen ) )

			for ( size_t i = 0; i < buflen; i++ )
			{
				buf[i] ^= src[i];
			}
		}

		size_t const tot = (size_t)MIN ( todo, buflen );

		if ( tot > 0 )
		{
			if ( file_out.write ( buf, tot ) )
			{
				return report_error ( ERROR_SODA_FILE_WRITE );
			}

			todo -= tot;
		}
	}

	return 0;
}

int mtDW::Soda::Op::multi_decode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	)
{
	if ( ! input || ! output )
	{
		return report_error ( ERROR_SODA_DECODE_INSANITY );
	}

	FilenameSwap	name ( output );

	name.m_res = decode ( butt, input, output );
	if ( name.m_res )
	{
		// We must decode at least once (and remove rogue output temp)
		return name.m_res;
	}

	// Some errors are not really errors in this context
	mtDW::set_stderr_less ();

	do
	{
		name.m_res = decode ( butt, name.f1, name.f2 );
		name.swap ();

	} while ( 0 == name.m_res );

	mtDW::set_stderr_more ();

	// Certain failures are allowed, i.e. we have extracted the input file
	// which will NOT have a valid Soda header.
	switch ( name.m_res )
	{
	case ERROR_SODA_OPEN_INPUT:
	case ERROR_SODA_FILE_ID:
	case ERROR_SODA_FILE_CHUNK_1:
	case ERROR_SODA_BAD_HEADER_ID:
	case ERROR_SODA_BAD_HEADER:
		name.m_res = 0;		// Keep output file
		return 0;
	}

//	Report any error as it was suppressed earlier on
	return report_error ( name.m_res );
}

