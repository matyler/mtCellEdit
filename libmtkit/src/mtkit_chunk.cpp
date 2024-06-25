/*
	Copyright (C) 2017-2023 Mark Tyler

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



mtKit::ChunkFile::Load::Load ()
	:
	m_fp		()
{
}

mtKit::ChunkFile::Load::~Load ()
{
	close ();
}

int mtKit::ChunkFile::Load::open (
	char	const * const	filename,
	char			id[CHUNK_HEADER_SIZE]
	)
{
	if ( m_fp || ! filename || ! id )
	{
		return INT_ERROR;
	}

	m_fp = fopen ( filename, "rb" );
	if ( ! m_fp )
	{
		return INT_ERROR;
	}

	char header[FILE_HEADER_SIZE];

	if ( get_buf ( (uint8_t *)header, FILE_HEADER_SIZE ) )
	{
		// get_buf() closes file for us
		return INT_ERROR_FATAL;
	}

	if (	header[0] != 0x00	||
		header[1] != 0x6d	||
		header[2] != 0x74	||
		header[3] != 0x43
		)
	{
		close ();
		return INT_ERROR_FATAL;
	}

	id[0] = header[4];
	id[1] = header[5];
	id[2] = header[6];
	id[3] = header[7];

	return 0;
}

void mtKit::ChunkFile::Load::close ()
{
	if ( m_fp )
	{
		fclose ( m_fp );
		m_fp = NULL;
	}
}

int mtKit::ChunkFile::Load::get_chunk (
	uint8_t	**	const	buf,
	uint32_t	* const	buflen,
	char			id[CHUNK_HEADER_SIZE],
	uint32_t	* const	buflen_enc
	)
{
	if ( ! m_fp )
	{
		return INT_ERROR;
	}

	char		header[CHUNK_HEADER_SIZE];
	uint32_t	len_encoded, len_decoded, method;
	int		res = get_buf ( (uint8_t *)header, sizeof(header) );

	if ( res == INT_EOF )
	{
		close ();
		return INT_EOF;
	}
	else if ( res )
	{
		goto error;
	}

	if (	get_uint32 ( len_encoded )	||
		get_uint32 ( len_decoded )	||
		get_uint32 ( method )
		)
	{
		goto error;
	}

	if ( buflen )
	{
		buflen[0] = len_decoded;
	}

	if ( buflen_enc )
	{
		buflen_enc[0] = len_encoded;
	}

	if ( id )
	{
		id[0] = header[0];
		id[1] = header[1];
		id[2] = header[2];
		id[3] = header[3];
	}

	if ( 0 == len_encoded && 0 == len_decoded )
	{
		return INT_SUCCESS;
	}
	else if ( 0 == len_encoded && 0 != len_decoded )
	{
		goto error;
	}
	else if ( 0 != len_encoded && 0 == len_decoded )
	{
		goto error;
	}
	else if ( ! buf )
	{
		if ( len_encoded > 0 )
		{
			if ( fseek ( m_fp, (long)len_encoded, SEEK_CUR ) )
			{
				goto error;
			}
		}

		return INT_SUCCESS;
	}
	else
	{
		uint8_t * enc_buf = (uint8_t *)malloc ( len_encoded );

		if ( ! enc_buf )
		{
			goto error;
		}

		if ( get_buf ( enc_buf, len_encoded ) )
		{
			free ( enc_buf );
			enc_buf = NULL;
			goto error;
		}

		switch ( method )
		{
		case ENCODE_RAW:
			if ( len_decoded != len_encoded )
			{
				free ( enc_buf );
				enc_buf = NULL;
				goto error;
			}

			buf[0] = enc_buf;
			return INT_SUCCESS;

		case ENCODE_DEFLATE:
			{
				unsigned char * dec_buf = NULL;
				res = mtkit_mem_inflate ( enc_buf,
					len_encoded, &dec_buf, len_decoded, 1 );

				free ( enc_buf );
				enc_buf = NULL;

				if ( res )
				{
					goto error;
				}

				buf[0] = dec_buf;
			}

			return INT_SUCCESS;

		default:
			free ( enc_buf );
			enc_buf = NULL;
			goto error;
		}
	}

error:
	close ();
	return INT_ERROR_FATAL;
}

int mtKit::ChunkFile::Load::get_uint32 (
	uint32_t	&num
	)
{
	uint8_t		buf[UINT_SIZE];
	int	const	res = get_buf ( buf, UINT_SIZE );

	if ( 0 == res )
	{
		num = (uint32_t)(
			buf[0] +
			(buf[1] << 8) +
			(buf[2] << 16) +
			(buf[3] << 24) );
	}

	return res;
}

int mtKit::ChunkFile::Load::get_buf (
	uint8_t		* const	buf,
	uint32_t	const	buflen
	)
{
	size_t const tot = fread ( buf, 1, buflen, m_fp );

	if ( tot != buflen )
	{
		if ( tot == 0 && feof ( m_fp ) )
		{
			return INT_EOF;
		}

		close ();

		return INT_ERROR_FATAL;
	}

	return 0;
}



/// ----------------------------------------------------------------------------



mtKit::ChunkFile::Save::Save ()
	:
	m_fp			(),
	m_encoding_type		( ENCODE_RAW ),
	m_deflate_level		( MTKIT_DEFLATE_LEVEL_DEFAULT ),
	m_deflate_model		( MTKIT_DEFLATE_MODEL_DEFAULT )
{
}

mtKit::ChunkFile::Save::~Save ()
{
	close ();
}

int mtKit::ChunkFile::Save::open (
	char	const * const	filename,
	char		const	id[CHUNK_HEADER_SIZE]
	)
{
	if ( m_fp || ! filename || ! id )
	{
		return INT_ERROR;
	}

	m_fp = fopen ( filename, "wb" );
	if ( ! m_fp )
	{
		return INT_ERROR;
	}

	char const header[FILE_HEADER_SIZE] =
		{ 0x00, 0x6d, 0x74, 0x43, id[0], id[1], id[2], id[3] };

	if ( put_buf ( (uint8_t const *)header, sizeof(header) ) )
	{
		// put_buf() closes file on error
		return INT_ERROR_FATAL;
	}

	return 0;
}

void mtKit::ChunkFile::Save::close ()
{
	if ( m_fp )
	{
		fclose ( m_fp );
		m_fp = NULL;
	}
}

void mtKit::ChunkFile::Save::set_encoding_deflate (
	int	const	level,
	int	const	model
	)
{
	m_encoding_type = ENCODE_DEFLATE;
	m_deflate_model = MAX ( MTKIT_DEFLATE_MODEL_MIN,
		MIN ( MTKIT_DEFLATE_MODEL_MAX, model ) );
	m_deflate_level = MAX ( MTKIT_DEFLATE_LEVEL_MIN,
		MIN ( MTKIT_DEFLATE_LEVEL_MAX, level ) );
}

int mtKit::ChunkFile::Save::put_chunk (
	uint8_t	const * const	buf,
	uint32_t	const	buflen,
	char		const	id[CHUNK_HEADER_SIZE]
	)
{
	if ( ! m_fp || ! buf || buflen > CHUNK_SIZE_MAX || ! id )
	{
		return INT_ERROR;
	}

	if ( put_buf ( (uint8_t const *)id, CHUNK_HEADER_SIZE ) )
	{
		// put_buf() closes file on error
		return INT_ERROR_FATAL;
	}

	if (	m_encoding_type == ENCODE_DEFLATE	&&
		m_deflate_level != 0			&&
		buflen > 16
		)
	{
		uint8_t		* zbuf = NULL;
		size_t		zbuflen = 0;

		if ( mtkit_mem_deflate ( buf, buflen, &zbuf, &zbuflen,
			m_deflate_level, m_deflate_model ) )
		{
			close ();
			return INT_ERROR_FATAL;
		}

		if ( (zbuflen + 16) < buflen )
		{
			// Save as a compressed chunk
			int res = INT_SUCCESS;

			if (	put_uint32 ( (uint32_t)zbuflen )||
				put_uint32 ( buflen )		||
				put_uint32 ( ENCODE_DEFLATE )	||
				put_buf ( zbuf, (uint32_t)zbuflen )
				)
			{
				// put_buf() closes file on error
				res = INT_ERROR_FATAL;
			}

			free ( zbuf );
			zbuf = NULL;

			return res;
		}

		// zlib couldn't meaningfully compress so just save the raw data

		free ( zbuf );
		zbuf = NULL;
	}

	// Save as an uncompressed chunk

	if (	put_uint32 ( (uint32_t)buflen )	||
		put_uint32 ( buflen )		||
		put_uint32 ( ENCODE_RAW )	||
		put_buf ( buf, buflen )
		)
	{
		// put_buf() closes file on error
		return INT_ERROR_FATAL;
	}

	return 0;
}

int mtKit::ChunkFile::Save::put_uint32 (
	uint32_t	const	num
	)
{
	uint8_t const buf[UINT_SIZE] = {
		uint8_t(num),
		uint8_t(num >> 8),
		uint8_t(num >> 16),
		uint8_t(num >> 24)
		};

	int res = put_buf ( buf, UINT_SIZE );

	if ( res )
	{
	}

	return res;
}

int mtKit::ChunkFile::Save::put_buf (
	uint8_t	const * const	buf,
	uint32_t	const	buflen
	)
{
	if ( buflen != fwrite ( buf, 1, buflen, m_fp ) )
	{
		close ();
		return INT_ERROR_FATAL;
	}

	return 0;
}

