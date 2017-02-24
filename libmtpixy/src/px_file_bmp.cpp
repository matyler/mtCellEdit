/*
	Copyright (C) 2016 Mark Tyler

	Code ideas and portions from mtPaint:
	Copyright (C) 2004-2006 Mark Tyler
	Copyright (C) 2006-2016 Dmitry Groshev

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



mtPixy::Image * mtPixy::image_load_bmp (
	char	const	* const	filename
	)
{
	if ( ! filename )
	{
		return NULL;
	}


	Image		* image = NULL;
	unsigned char	head[1024], * buf = NULL, pix, * src, * dest;
	int		w, h, bitcount, compression, cols, i, j, k, uc;
	FILE		* fp;
	size_t		readin, bpl = 0, ssize;


	fp = fopen ( filename, "rb" );
	if ( fp == NULL )
	{
		return NULL;
	}

	readin = fread ( head, 1, 54, fp );
	if ( readin < 54 )
	{
		goto fail;		// No proper header
	}

	if ( head[0] != 'B' || head[1] != 'M' )
	{
		goto fail;		// Signature
	}

	w = head[18] + (head[19] << 8) + (head[20] << 16) + (head[21] << 24);
	h = head[22] + (head[23] << 8) + (head[24] << 16) + (head[25] << 24);
	bitcount = head[28] + (head[29] << 8);
	compression = head[30] + (head[31] << 8) + (head[32] << 16) +
		(head[33] << 24);

	cols = head[46] + (head[47] << 8) + (head[48] << 16) + (head[49] << 24);

	if (	bitcount != 1 &&
		bitcount != 4 &&
		bitcount != 8 &&
		bitcount != 24 )
	{
		goto fail;
	}

	if ( compression != 0 )
	{
		goto fail;
	}

	if ( bitcount == 24 )
	{
		image = image_create ( Image::RGB, w, h );
	}
	else
	{
		image = image_create ( Image::INDEXED, w, h );
	}

	if ( ! image )
	{
		goto fail;
	}

	bpl = (size_t)w;		// Bytes per line

	if ( bitcount == 24 )
	{
		bpl = bpl * 3;
	}

	if ( bitcount == 4 )
	{
		bpl = (bpl + 1) / 2;
	}

	if ( bitcount == 1 )
	{
		bpl = (bpl + 7) / 8;
	}

	if ( bpl % 4 != 0 )
	{
		// 4 byte boundary for pixels
		bpl = bpl + 4 - ( bpl % 4 );
	}

	buf = (unsigned char *)malloc ( bpl );
	if ( ! buf )
	{
		goto fail;
	}

	if ( bitcount == 24 )		// RGB image
	{
		for ( j = 0; j < h; j++ )
		{
			// Read in line of pixels
			readin = fread ( buf, 1, bpl, fp );

			if ( readin != bpl )
			{
				goto fail;
			}

			src = buf;
			dest = image->get_canvas ();
			dest += 3 * w * ( h - 1 - j );

			for ( i = 0; i < w; i++ )
			{
				dest[2] = *src++;
				dest[1] = *src++;
				dest[0] = *src++;
				dest += 3;
			}
		}
	}
	else				// Indexed palette image
	{
		Palette		* const pal = image->get_palette ();
		Color		* const pal_col = pal->get_color ();


		if ( cols == 0 )
		{
			cols = 1 << bitcount;
		}

		if ( pal->set_color_total ( cols ) )
		{
			goto fail;
		}

		// Read in colour table
		ssize = (size_t)(cols * 4);

		readin = fread ( head, 1, ssize, fp );
		if ( readin != ssize )
		{
			goto fail;
		}

		for ( i = 0; i < cols; i++ )
		{
			pal_col[i].red = head[2 + 4 * i];
			pal_col[i].green = head[1 + 4 * i];
			pal_col[i].blue = head[0 + 4 * i];
		}

		image->set_file_flag ( Image::FLAG_PALETTE_LOADED );

		for ( j = 0; j < h; j++ )
		{
			// Read in line of pixels
			readin = fread ( buf, 1, bpl, fp );

			if ( readin != bpl )
			{
				goto fail;
			}

			src = buf;
			dest = image->get_canvas ();
			dest += w * ( h - 1 - j );

			if ( bitcount == 8 )
			{
				memcpy ( dest, src, (size_t)w );
			}
			else if ( bitcount == 4 )
			{
				for ( i = 0; i < w; i += 2 )
				{
					pix = *src++;
					*dest++ = (unsigned char)(pix >> 4);

					if ( (i + 1) < w )
					{
						*dest++ = (unsigned char)
							(pix % 16);
					}
				}
			}
			else if ( bitcount == 1 )
			{
				for ( i = 0; i < w; i += 8 )
				{
					pix = *src++;
					k = 0;

					while ( k < 8 && (i + k) < w )
					{
						uc = (pix >> (7 - k) ) % 2;
						*dest++ = (unsigned char)uc;
						k++;
					}
				}
			}
		}
	}

	free ( buf );
	fclose ( fp );

	return image;

fail:
	free ( buf );
	fclose ( fp );

	delete image;

	return NULL;
}

int mtPixy::Image::save_bmp (
	char	const	* const	filename
	) const
{
	if ( ! filename )
	{
		return 1;
	}


	unsigned char	head[1024] = {0}, * buf = NULL, * src, * dest;
	int		i, j, cols;
	FILE		* fp;
	size_t		bpl, filesize, headsize, ssize;


	switch ( m_type )
	{
	case INDEXED:
	case RGB:
		break;

	default:
		return 1;
	}

	cols = m_palette.get_color_total ();

	bpl = (size_t)(m_width * m_canvas_bpp);

	if ( bpl % 4 != 0 )
	{
		bpl = bpl + 4 - (bpl % 4);	// Adhere to 4 byte boundaries
	}

	filesize = 54 + bpl * (size_t)m_height;

	if ( m_canvas_bpp == 1 )
	{
		filesize = filesize + (size_t)(cols * 4);
	}

	headsize = filesize - (bpl * (size_t)m_height);

	fp = fopen ( filename, "wb" );
	if ( fp == NULL )
	{
		return 1;
	}

	head[0] = 'B';
	head[1] = 'M';

	head[2] = (unsigned char)(filesize % 256);
	head[3] = (unsigned char)((filesize >> 8) % 256);
	head[4] = (unsigned char)((filesize >> 16) % 256);
	head[5] = (unsigned char)((filesize >> 24) % 256);

	head[10] = (unsigned char)(headsize % 256);
	head[11] = (unsigned char)(headsize / 256);

	head[14] = 40;
	head[26] = 1;

	head[18] = (unsigned char)(m_width % 256);
	head[19] = (unsigned char)(m_width / 256);
	head[20] = 0;
	head[21] = 0;
	head[22] = (unsigned char)(m_height % 256);
	head[23] = (unsigned char)(m_height / 256);
	head[24] = 0;
	head[25] = 0;

	head[28] = (unsigned char)(m_canvas_bpp * 8);
	head[29] = 0;			// Bits per pixel
	head[30] = 0;
	head[31] = 0;
	head[32] = 0;
	head[33] = 0;			// No compression

	head[34] = (unsigned char)(bpl * (size_t)m_height % 256);
	head[35] = (unsigned char)((bpl * (size_t)m_height >> 8) % 256);
	head[36] = (unsigned char)((bpl * (size_t)m_height >> 16) % 256);
	head[37] = (unsigned char)((bpl * (size_t)m_height >> 24) % 256);

	head[38] = 18;
	head[39] = 11;
	head[42] = 18;
	head[43] = 11;

	if ( m_canvas_bpp == 1 )
	{
		head[46] = (unsigned char)(cols % 256);
		head[47] = (unsigned char)(cols / 256);
		head[50] = head[46];
		head[51] = head[47];
	}

	if ( 54 != fwrite ( head, 1, 54, fp ) )
	{
		goto fail;
	}

	buf = (unsigned char *)calloc ( bpl, 1 );
	if ( ! buf )
	{
		goto fail;
	}

	if ( m_canvas_bpp == 3 )		// RGB image
	{
		for ( j = m_height - 1; j >= 0; j-- )
		{
			src = m_canvas + 3 * m_width * j;
			dest = buf;

			for ( i = 0; i < m_width; i++ )
			{
				*dest++ = src[2];
				*dest++ = src[1];
				*dest++ = src[0];
				src += 3;
			}

			if ( fwrite ( buf, 1, bpl, fp ) != bpl )
			{
				goto fail;
			}
		}
	}

	if ( m_canvas_bpp == 1 )		// Indexed palette image
	{
		Color	const * const pal_col = m_palette.get_color ();


		for ( i = 0; i < cols; i++ )
		{
			head[3 + 4 * i] = 0;
			head[2 + 4 * i] = pal_col[i].red;
			head[1 + 4 * i] = pal_col[i].green;
			head[0 + 4 * i] = pal_col[i].blue;
		}

		ssize = (size_t)(cols * 4);
		if ( fwrite ( head, 1, ssize, fp ) != ssize )
		{
			goto fail;
		}

		for ( j = m_height - 1; j >= 0; j-- )
		{
			src = m_canvas + m_width * j;

			// We can't save directly from the image as bpl might
			// be > w

			memcpy ( buf, src, (size_t)m_width );

			if ( fwrite ( buf, 1, bpl, fp ) != bpl )
			{
				goto fail;
			}
		}
	}

	free ( buf );
	fclose ( fp );

	return 0;

fail:
	free ( buf );
	fclose ( fp );

	return 1;
}

