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

/*	-------------
	CHUNK HEADERS
	-------------

	-------
	General
	-------
*/

#define CHEAD_INFO	"Info" // First header: image geometry
#define CHEAD_PAL0	"Pal0" // Artists palette: 2-256 colours in packed RGB
#define CHEAD_ALP0	"Alp0" // Alpha channel: 1 byte per pixel
#define CHEAD_IMG0	"Img0" // Raw image format: 1/3 bytes per pixel

/*	----------------------
	Indexed palette images
	----------------------
*/

#define CHEAD_IDX1	"Idx1" // 1 bit per pixel
#define CHEAD_IDX2	"Idx2" // 2 bits per pixel
#define CHEAD_IDX4	"Idx4" // 4 bits per pixel

/*	-----------------
	24 bit RGB images
	-----------------
*/

#define CHEAD_RGB1	"Rgb1" // Inter pixel Delta (photo)
#define CHEAD_RGB2	"Rgb2" // Intra pixel Delta (greyish photo)



enum	// INTERNAL use ONLY!
{
	ENCODE_MIN		= 0,

	ENCODE_INFO		= 0,
	ENCODE_PAL0		= 1,
	ENCODE_ALP0		= 2,
	ENCODE_IMG0		= 3,	// Raw compress bytes as is
	ENCODE_RGB1		= 4,	// Inter pixel delta
	ENCODE_RGB2		= 5,	// Intra & Inter pixel delta
	ENCODE_IDX1		= 6,	// 1 bit per pixel
	ENCODE_IDX2		= 7,	// 2 bit per pixel
	ENCODE_IDX4		= 8,	// 4 bit per pixel

	ENCODE_MAX		= 8
};



inline int get_bit_tot (
	int	const	num
	)
{
	if ( num < 2 )		return 0;
	else if ( num < 3 )	return 1;
	else if ( num < 5 )	return 2;
	else if ( num < 9 )	return 3;
	else if ( num < 17 )	return 4;
	else if ( num < 33 )	return 5;
	else if ( num < 65 )	return 6;
	else if ( num < 129 )	return 7;

	return 8;
}



class LoadPixy;
class SavePixy;



class LoadPixy
{
public:
	LoadPixy ();
	~LoadPixy ();

	mtPixy::Image * open ( char const * filename );

private:
	int get_first_chunk ();
	int get_chunk (
		uint8_t * buf,
		size_t buflen,
		char const * id,
		char const * force_id
		);

	// decode_* return 0=success >0=fail -1=success but don't free buf

	int decode_info ( uint8_t * buf, size_t buflen );
	int decode_pal0 ( uint8_t const * buf, size_t buflen );
	int decode_alp0 ( uint8_t const * buf, size_t buflen );
	int decode_img0 ( uint8_t const * buf, size_t buflen );
	int decode_rgb1 ( uint8_t const * buf, size_t buflen );
	int decode_rgb2 ( uint8_t const * buf, size_t buflen );
	int decode_idx ( uint8_t const * buf, size_t buflen, int bit_tot );

	mtPixy::Image * take_image ();
	void flush_image ();

/// ----------------------------------------------------------------------------

	// These state variables can be changed by "Info" chunks at any time.

	int		m_h;		// Height
	int		m_p;		// Pixel canvas type: image.get_type ();
	int		m_v;		// Version
	int		m_w;		// Width

	mtKit::ChunkFile::Load	m_file;
	mtPixy::Image		* m_image;
};



class SavePixy
{
public:
	SavePixy ( mtPixy::Image const &image, int level );

	int open ( char const * filename );

private:

	void set_deflate ( int enc );

	int put_chunk_info ();
	int put_chunk_canvas ();
	int put_chunk_pal0 ();
	int put_chunk_alp0 ();

	int encode_img0 ( unsigned char const * mem, int w, int h, int bpp );
	int encode_rgb1 ( unsigned char const * mem, int w, int h );
	int encode_rgb2 ( unsigned char const * mem, int w, int h );
	int encode_idx ( unsigned char const * mem, int w, int h, int enc );

	int get_coltot () const;

/// ----------------------------------------------------------------------------

	mtPixy::Image		const	&m_image;
	int			const	m_compr_level;

	mtKit::ChunkFile::Save		m_file;
};

