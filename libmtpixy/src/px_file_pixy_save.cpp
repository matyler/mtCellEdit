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




SavePixy::SavePixy (
	mtPixy::Image	const	&image,
	int		const	level
	)
	:
	m_image		( image ),
	m_compr_level	( level < 0 ? 0 : (level % 10) )
{
}

void SavePixy::set_deflate (
	int	const	enc
	)
{
	switch ( enc )
	{
	case ENCODE_RGB1:
	case ENCODE_RGB2:
		m_file.set_encoding_deflate ( m_compr_level,
			MTKIT_DEFLATE_MODEL_RLE );
		break;

	// Most encodings use default zlib
	default:
		m_file.set_encoding_deflate ( m_compr_level,
			MTKIT_DEFLATE_MODEL_DEFAULT );
		break;
	}
}

int SavePixy::put_chunk_info ()
{
	set_deflate ( ENCODE_INFO );

	unsigned char buf [ 16 ];

	mtkit_int32_pack ( buf, 0 );		// Version
	mtkit_int32_pack ( buf + 4, m_image.get_type () );
	mtkit_int32_pack ( buf + 8, m_image.get_width () );
	mtkit_int32_pack ( buf + 12, m_image.get_height () );

	return m_file.put_chunk ( buf, (uint32_t)sizeof(buf), CHEAD_INFO );
}

int SavePixy::put_chunk_pal0 ()
{
	mtPixy::Palette const * const pal = m_image.get_palette ();
	if ( ! pal )
	{
		return 1;
	}

	mtPixy::Color const * const col = pal->get_color ();
	if ( ! col )
	{
		return 1;
	}

	uint8_t		* buf = NULL;
	uint8_t		* dest = NULL;
	uint32_t	buflen = 0;
	int	const	coltot = pal->get_color_total ();

	if ( coltot < 1 )
	{
		return 1;
	}

	buflen = (uint32_t)(coltot * 3);
	buf = (uint8_t *)malloc ( buflen );

	if ( ! buf )
	{
		return 1;
	}

	dest = buf;

	for ( int i = 0; i < coltot; i++ )
	{
		*dest++ = col[i].red;
		*dest++ = col[i].green;
		*dest++ = col[i].blue;
	}

	set_deflate ( ENCODE_PAL0 );

	int const res = m_file.put_chunk ( buf, buflen, CHEAD_PAL0 );

	free ( buf );
	buf = NULL;

	return res;
}

int SavePixy::encode_img0 (
	unsigned char	const * const	mem,
	int			const	w,
	int			const	h,
	int			const	bpp
	)
{
	uint32_t const buflen = (uint32_t)(w * h * bpp );

	set_deflate ( ENCODE_IMG0 );

	return m_file.put_chunk ( mem, buflen, CHEAD_IMG0 );
}

int SavePixy::encode_rgb1 (
	unsigned char	const * const	mem,
	int			const	w,
	int			const	h
	)
{
	double	const	pixtot = w * h;
	uint32_t const	buflen = (uint32_t)(pixtot * 3);
	uint8_t		* buf = (uint8_t *)malloc ( buflen );

	if ( ! buf )
	{
		return 1;
	}

	for ( int y = 0; y < h; y ++ )
	{
		size_t	const	offset = (size_t)(y * w * 3);
		uint8_t	const	* src = mem + offset;
		uint8_t		* dest = buf + offset;

		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;

		for ( int x = 1; x < w; x++, dest += 3, src += 3 )
		{
			dest[0] = (uint8_t)( 128 + src[0] - src[-3] );
			dest[1] = (uint8_t)( 128 + src[1] - src[-2] );
			dest[2] = (uint8_t)( 128 + src[2] - src[-1] );
		}
	}

	set_deflate ( ENCODE_RGB1 );

	int res = m_file.put_chunk ( buf, buflen, CHEAD_RGB1 );

	free ( buf );
	buf = NULL;

	return res;
}

int SavePixy::encode_rgb2 (
	unsigned char	const * const	mem,
	int			const	w,
	int			const	h
	)
{
	double	const	pixtot = w * h;
	uint32_t const	buflen = (uint32_t)(pixtot * 3);
	uint8_t		* buf = (uint8_t *)malloc ( buflen );

	if ( ! buf )
	{
		return 1;
	}

	for ( int y = 0; y < h; y ++ )
	{
		size_t	const	offset = (size_t)(y * w * 3);
		uint8_t	const	* src = mem + offset;
		uint8_t		* dest = buf + offset;

		*dest++ = *src++;
		*dest++ = *src++;
		*dest++ = *src++;

		for ( int x = 1; x < w; x++, dest += 3, src += 3 )
		{
			// Inter pixel delta on green channel
			int const dd = src[1] - src[-2];
			dest[1] = (uint8_t)( 128 + dd );

			// Intra pixel delta's - effective for greyish photos
			dest[0] = (uint8_t)( 128 + (src[0] - src[-3] - dd) );
			dest[2] = (uint8_t)( 128 + (src[2] - src[-1] - dd) );
		}
	}

	set_deflate ( ENCODE_RGB2 );

	int res = m_file.put_chunk ( buf, buflen, CHEAD_RGB2 );

	free ( buf );
	buf = NULL;

	return res;
}

int SavePixy::encode_idx (
	unsigned char	const * const	mem,
	int			const	w,
	int			const	h,
	int			const	enc
	)
{
	int		bit_tot;
	char	const	* chunk_name;

	switch ( enc )
	{
	case ENCODE_IDX1:	bit_tot = 1; chunk_name = CHEAD_IDX1; break;
	case ENCODE_IDX2:	bit_tot = 2; chunk_name = CHEAD_IDX2; break;
	case ENCODE_IDX4:	bit_tot = 4; chunk_name = CHEAD_IDX4; break;
	default:		return 1;
	}

	unsigned char		const * const	srclim = mem + w * h;
	mtKit::BitPackWrite	pack;

	for ( unsigned char const * s = mem; s < srclim; s++ )
	{
		if ( pack.write ( s[0], bit_tot ) )
		{
			return 1;
		}
	}

	unsigned char const * const buf = pack.get_buf ();
	size_t const buf_len = pack.get_buf_len ();

	set_deflate ( enc );

	if ( m_file.put_chunk( buf, (uint32_t)buf_len, chunk_name ) )
	{
		return 1;
	}

	return 0;
}

static int get_bits_used (
	int	const	n
	)
{
	if	( n < 2 )	return 0;
	else if ( n < 3 )	return 1;
	else if ( n < 5 )	return 2;
	else if ( n < 9 )	return 3;
	else if ( n < 17 )	return 4;
	else if ( n < 33 )	return 5;
	else if ( n < 65 )	return 6;
	else if ( n < 129 )	return 7;

	return 8;
}

static int analyse_idx (
	int	const	coltot
	)
{
	int const bits = get_bits_used ( coltot );

	if	( bits < 2 )	return ENCODE_IDX1;
	else if ( bits < 3 )	return ENCODE_IDX2;
	else if ( bits < 5 )	return ENCODE_IDX4;

	return ENCODE_IMG0;
}

static int analyse_rgb (
	uint8_t	const * const	buf,
	int		const	bpp,
	int		const	w,
	int		const	h
	)
{
	if ( (w * h) < 100 )
	{
		return ENCODE_IMG0;
	}

	uint8_t	const	* bufend = buf + bpp * w * h;
	int64_t		rgb_intra = 0;
	int64_t		rgb_inter[3] = { 0, 0, 0 };
	int	const	tot = w * h;
	int		rle_tot = 0;

	for ( uint8_t const * src = buf + 3; src < bufend; src += 3 )
	{
		rgb_intra += MAX ( abs(src[0] - src[1]),
			MAX ( abs(src[0] - src[2]), abs(src[1] - src[2]) ) );

		rgb_inter[0] += abs(src[0] - src[-3]);
		rgb_inter[1] += abs(src[1] - src[-2]);
		rgb_inter[2] += abs(src[2] - src[-1]);

		if (	src[0] == src[-3] &&
			src[1] == src[-2] &&
			src[2] == src[-1] )
		{
			rle_tot++;
		}
	}

	int const intra_average = (int)(rgb_intra / tot);
	int const inter_average = (int)(MAX ( (rgb_inter[0] / tot),
		MAX ( (rgb_inter[1] / tot), (int)(rgb_inter[2] / tot) ) ) );
	int const rle_perc = (100 * rle_tot)/tot;

	if ( rle_perc > 30 )
	{
		// Lots of sequences of the same pixels, so save as raw
		return ENCODE_IMG0;
	}

	if ( inter_average < 20 )
	{
		if ( intra_average < 50 )
		{
			// Grey enough to benefit from intra delta
			return ENCODE_RGB2;
		}

		return ENCODE_RGB1;
	}

	// No benefits to any delta
	return ENCODE_IMG0;
}

int SavePixy::get_coltot () const
{
	mtPixy::Palette const * const pal = m_image.get_palette ();
	if ( ! pal )
	{
		return -1;
	}

	mtPixy::Color const * const col = pal->get_color ();
	if ( ! col )
	{
		return -1;
	}

	return pal->get_color_total ();
}

int SavePixy::put_chunk_canvas ()
{
	uint8_t const * const buf = m_image.get_canvas ();
	if ( ! buf )
	{
		return 0;
	}

	int const bpp = m_image.get_canvas_bpp ();
	int const w = m_image.get_width ();
	int const h = m_image.get_height ();
	int enc = ENCODE_IMG0;

	if ( bpp == 1 )
	{
		int	const	coltot = get_coltot ();

		if ( coltot < 0 )
		{
			return 1;
		}

		enc = analyse_idx ( coltot );
	}
	else if ( bpp == 3 )
	{
		enc = analyse_rgb ( buf, bpp, w, h );
	}
	else
	{
		return 1;
	}

	switch ( enc )
	{
	case ENCODE_IMG0:
		return encode_img0 ( buf, w, h, bpp );

	case ENCODE_RGB1:
		return encode_rgb1 ( buf, w, h );

	case ENCODE_RGB2:
		return encode_rgb2 ( buf, w, h );

	case ENCODE_IDX1:
	case ENCODE_IDX2:
	case ENCODE_IDX4:
		return encode_idx ( buf, w, h, enc );

	default:
		break;
	}

	return 1;
}

int SavePixy::put_chunk_alp0 ()
{
	if ( ! m_image.get_alpha () )
	{
		return 0;
	}

	uint32_t const buflen = (uint32_t)(m_image.get_width() *
		m_image.get_height() );

	set_deflate ( ENCODE_ALP0 );

	return m_file.put_chunk ( m_image.get_alpha (), buflen, CHEAD_ALP0 );
}

int SavePixy::open (
	char	const * const	filename
	)
{
	if ( m_file.open ( filename, "Pixy" ) )
	{
		return 1;
	}

	if (	put_chunk_info ()	||
		put_chunk_pal0 ()	||
		put_chunk_alp0 ()	||
		put_chunk_canvas ()
		)
	{
		return 1;
	}

	return 0;
}

int mtPixy::Image::save_pixy (
	char	const * const	filename,
	int		const	compression
	) const
{
	SavePixy sv ( *this, compression );

	return sv.open ( filename );
}

