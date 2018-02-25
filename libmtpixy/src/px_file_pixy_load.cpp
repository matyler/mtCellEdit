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
#include "px_file_pixy.h"



LoadPixy::LoadPixy ()
	:
	m_h		( 0 ),
	m_p		( -1 ),
	m_v		( 0 ),
	m_w		( 0 ),
	m_image		( NULL )
{
}

LoadPixy::~LoadPixy ()
{
	flush_image ();
}

int LoadPixy::get_first_chunk ()
{
	uint8_t		* buf;
	uint32_t	buflen;
	char		id [ mtKit::ChunkFile::CHUNK_HEADER_SIZE ];
	int		res = m_file.get_chunk ( &buf, &buflen, id, NULL );

	if ( res != mtKit::ChunkFile::INT_SUCCESS )
	{
		return 1;
	}

	res = 1;

	if ( get_chunk ( buf, buflen, id, CHEAD_INFO ) )
	{
		goto error;
	}

	flush_image ();
	m_image = mtPixy::image_create ( (mtPixy::Image::Type)m_p, m_w, m_h );
	if ( ! m_image )
	{
		goto error;
	}

	res = 0;

error:
	free ( buf );
	buf = NULL;

	return res;
}

int LoadPixy::decode_info (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	if ( buflen < 16 )
	{
		return 1;
	}

	m_v = mtkit_int32_unpack ( buf );
	m_p = mtkit_int32_unpack ( buf + 4 );
	m_w = mtkit_int32_unpack ( buf + 8 );
	m_h = mtkit_int32_unpack ( buf + 12 );

	return 0;
}

int LoadPixy::decode_pal0 (
	uint8_t	const * const	buf,
	size_t		const	buflen
	)
{
	mtPixy::Palette * const pal = m_image->get_palette ();
	if ( ! pal )
	{
		return 1;
	}

	mtPixy::Color * const col = pal->get_color ();
	if ( ! col )
	{
		return 1;
	}

	int	const	coltot = (int)(buflen / 3);
	if ( pal->set_color_total ( coltot ) )
	{
		return 1;
	}

	uint8_t const * src = buf;

	for ( int i = 0; i < coltot; i++ )
	{
		col[i].red = *src++;
		col[i].green = *src++;
		col[i].blue = *src++;
	}

	m_image->set_file_flag ( mtPixy::Image::FLAG_PALETTE_LOADED );

	return 0;
}

int LoadPixy::decode_img0 (
	uint8_t	const * const	buf,
	size_t		const	buflen
	)
{
	size_t const destlen = (size_t)(m_image->get_width() *
		m_image->get_height() * m_image->get_canvas_bpp () );

	if ( buflen != destlen )
	{
		return 1;
	}

	memcpy ( m_image->get_canvas (), buf, buflen );

	return 0;
}

int LoadPixy::decode_rgb2 (
	uint8_t	const * const	buf,
	size_t		const	buflen
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	uint8_t		* const	can = m_image->get_canvas ();
	int		const	rowtot = m_image->get_height ();
	int		const	coltot = m_image->get_width ();
	double		const	pixtot = rowtot * coltot;

	if ( ! can || buflen != (size_t)(pixtot * 3) )
	{
		return 1;
	}

	for ( int row = 0; row < rowtot; row ++ )
	{
		size_t	const	offset = (size_t)(row * coltot * 3);
		uint8_t		* dest = can + offset;
		uint8_t	const	* src = buf + offset;

		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;

		for ( int col = 1; col < coltot; col ++, dest += 3, src += 3 )
		{
			// Inter pixel delta on mid channel
			dest[1] = (uint8_t)( src[1] - 128 + dest[-2] );

			int const dd = dest[1] - dest[-2];

			// Intra pixel delta's
			dest[0] = (uint8_t)( src[0] - 128 + dd + dest[-3] );
			dest[2] = (uint8_t)( src[2] - 128 + dd + dest[-1] );
		}
	}

	return 0;
}

int LoadPixy::decode_rgb1 (
	uint8_t	const * const	buf,
	size_t		const	buflen
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	uint8_t		* const	can = m_image->get_canvas ();
	int		const	rowtot = m_image->get_height ();
	int		const	coltot = m_image->get_width ();
	double		const	pixtot = rowtot * coltot;

	if ( ! can || buflen != (size_t)(pixtot * 3) )
	{
		return 1;
	}

	for ( int row = 0; row < rowtot; row ++ )
	{
		size_t	const	offset = (size_t)(row * coltot * 3);
		uint8_t		* dest = can + offset;
		uint8_t	const	* src = buf + offset;

		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;

		for ( int col = 1; col < coltot; col ++, dest += 3, src += 3 )
		{
			dest[0] = (uint8_t)( src[0] - 128 + dest[-3] );
			dest[1] = (uint8_t)( src[1] - 128 + dest[-2] );
			dest[2] = (uint8_t)( src[2] - 128 + dest[-1] );
		}
	}

	return 0;
}

int LoadPixy::decode_alp0 (
	uint8_t	const * const	buf,
	size_t		const	buflen
	)
{
	if ( ! m_image->get_alpha () )
	{
		return 1;
	}

	size_t const destlen = (size_t)(m_image->get_width () *
		m_image->get_height () );

	if ( buflen != destlen )
	{
		return 1;
	}

	memcpy ( m_image->get_alpha (), buf, buflen );

	return 0;
}

int LoadPixy::decode_idx (
	uint8_t	const * const	buf,
	size_t		const	buflen,
	int		const	bit_tot
	)
{
	unsigned char * dest = m_image->get_canvas ();
	if ( ! dest )
	{
		return 1;
	}

	mtKit::BitPackRead pack ( buf, buflen );
	int const pixtot = m_image->get_width () * m_image->get_height ();
	unsigned char * const destlim = dest + pixtot;

	for ( ; dest < destlim; )
	{
		int byte;

		if ( pack.read ( byte, bit_tot ) )
		{
			// Quiet failure - allow user to retrieve image decoded
			return 0;
		}

		*dest++ = (unsigned char)byte;
	}

	return 0;
}

static int cmp_id (
	char	const *	id,
	char	const * txt
	)
{
	if ( 0 == memcmp ( id, txt, 4 ) )
	{
		return 1;
	}

	return 0;
}

int LoadPixy::get_chunk (
	uint8_t		* const	buf,
	size_t		const	buflen,
	char	const * const	id,
	char	const * const	force_id
	)
{
	if ( force_id && memcmp ( force_id, id, 4 ) )
	{
		return 1;
	}

	if (     cmp_id(id, CHEAD_INFO )) return decode_info ( buf, buflen );
	else if (cmp_id(id, CHEAD_PAL0 )) return decode_pal0 ( buf, buflen );
	else if (cmp_id(id, CHEAD_IMG0 )) return decode_img0 ( buf, buflen );
	else if (cmp_id(id, CHEAD_ALP0 )) return decode_alp0 ( buf, buflen );
	else if (cmp_id(id, CHEAD_RGB1 )) return decode_rgb1 ( buf, buflen );
	else if (cmp_id(id, CHEAD_RGB2 )) return decode_rgb2 ( buf, buflen );
	else if (cmp_id(id, CHEAD_IDX1 )) return decode_idx  ( buf, buflen, 1 );
	else if (cmp_id(id, CHEAD_IDX2 )) return decode_idx  ( buf, buflen, 2 );
	else if (cmp_id(id, CHEAD_IDX4 )) return decode_idx  ( buf, buflen, 4 );

	// Unknown chunk so quietly ignore it
	return 0;
}

mtPixy::Image * LoadPixy::take_image ()
{
	mtPixy::Image * i = m_image;

	m_image = NULL;

	return i;
}

void LoadPixy::flush_image ()
{
	if ( m_image )
	{
		delete m_image;
		m_image = NULL;
	}
}

mtPixy::Image * LoadPixy::open (
	char	const * const	filename
	)
{
	char id [ mtKit::ChunkFile::CHUNK_HEADER_SIZE ];

	if (	m_file.open ( filename, id )	||
		memcmp ( id, "Pixy", 4 )
		)
	{
		return NULL;
	}

	if ( get_first_chunk () )
	{
		return NULL;
	}

	for ( int res = 0; res == 0; )
	{
		uint8_t		* buf;
		uint32_t	buflen;

		res = m_file.get_chunk ( &buf, &buflen, id, NULL );

		switch ( res )
		{
		case mtKit::ChunkFile::INT_SUCCESS:
			{
				int const r = get_chunk( buf, buflen, id, NULL);

				if ( r >= 0 )
				{
					free ( buf );
					buf = NULL;
				}

				if ( r < 1 )
				{
					break;
				}
			}

		// Fall through

		case mtKit::ChunkFile::INT_ERROR:
		case mtKit::ChunkFile::INT_ERROR_FATAL:
		default:
			flush_image ();
			break;

		case mtKit::ChunkFile::INT_EOF:
			break;
		}
	}

	return take_image ();
}

mtPixy::Image * mtPixy::image_load_pixy (
	char	const * const	filename
	)
{
	LoadPixy file;

	return file.open ( filename );
}

