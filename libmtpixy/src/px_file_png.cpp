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

#ifdef U_PNG



#include <png.h>

#include "private.h"



static int load_paletted (
	unsigned char	* const	src,
	int		const	w,
	int		const	h,
	png_structp	const	png_ptr,
	png_infop	const	info_ptr,
	png_bytep	* const	row_pointers,
	int		const	color_type,
	int		const	bit_depth
	)
{
	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		return 1;
	}

	png_set_strip_16 ( png_ptr );
	png_set_strip_alpha ( png_ptr );
	png_set_packing ( png_ptr );

	if (	color_type == PNG_COLOR_TYPE_GRAY &&
		bit_depth < 8
		)
	{
		png_set_expand_gray_1_2_4_to_8 ( png_ptr );
	}

	for ( int y = 0; y < h; y++ )
	{
		row_pointers[y] = src + y * w;
	}

	png_read_image ( png_ptr, row_pointers );
	png_read_end ( png_ptr, info_ptr );

	return 0;
}

static int load_rgb (
	unsigned char	* const	canmem,
	unsigned char	* const	alpmem,
	int		const	w,
	int		const	h,
	png_structp	const	png_ptr,
	png_infop	const	info_ptr,
	png_bytep	* const	row_pointers
	)
{
	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		return 1;
	}

	png_set_strip_16 ( png_ptr );
	png_set_expand_gray_1_2_4_to_8 ( png_ptr );
	png_set_palette_to_rgb ( png_ptr );
	png_set_gray_to_rgb ( png_ptr );

	if ( alpmem )
	{
		unsigned char * tmp = (unsigned char *)calloc ( (size_t)(w * h),
					4 );

		if ( ! tmp )
		{
			return 1;
		}

		if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
		{
			free ( tmp );
			return 1;
		}

		for ( int y = 0; y < h; y++ )
		{
			row_pointers[y] = tmp + y * w * 4;
		}

		png_read_image ( png_ptr, row_pointers );

		for ( int y = 0; y < h; y++ )
		{
			unsigned char * src = tmp + y * w * 4;
			unsigned char * dst = canmem + y * w * 3;
			unsigned char * dst_a = alpmem + y * w;

			for ( int x = 0; x < w; x++ )
			{
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;
				*dst_a++ = *src++;
			}
		}

		free ( tmp );
	}
	else
	{
		png_set_strip_alpha ( png_ptr );

		for ( int y = 0; y < h; y++ )
		{
			row_pointers[y] = canmem + y * w * 3;
		}

		png_read_image ( png_ptr, row_pointers );
	}

	png_read_end ( png_ptr, info_ptr );

	return 0;
}



#define PNG_BYTES_TO_CHECK 8



static int load_png_init (
	FILE		* const	fp,
	png_structp	const	png_ptr,
	png_infop	const	info_ptr,
	png_uint_32	* const	pwidth,
	png_uint_32	* const	pheight,
	int		* const	bit_depth,
	int		* const	color_type
	)
{
	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		return 1;
	}

	png_init_io ( png_ptr, fp );
	png_set_sig_bytes ( png_ptr, PNG_BYTES_TO_CHECK );

	png_read_info ( png_ptr, info_ptr );
	png_get_IHDR ( png_ptr, info_ptr, pwidth, pheight, bit_depth,
		color_type, NULL, NULL, NULL );

	return 0;
}

static int load_png_palette (
	mtPixy::Palette	* const	pal,
	png_structp	const	png_ptr,
	png_infop	const	info_ptr,
	mtPixy::Image	* const	image
	)
{
	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		return 1;
	}

	if ( png_get_valid ( png_ptr, info_ptr, PNG_INFO_PLTE ) )
	{
		png_colorp	png_palette;
		int		coltot;
		mtPixy::Color	* const palcol = pal->get_color ();


		png_get_PLTE ( png_ptr, info_ptr, &png_palette, &coltot );

		if ( pal->set_color_total ( coltot ) )
		{
			return 1;
		}

		for ( int i = 0; i < coltot; i++ )
		{
			palcol[i].red = png_palette[i].red;
			palcol[i].green = png_palette[i].green;
			palcol[i].blue = png_palette[i].blue;
		}

		image->set_file_flag ( mtPixy::Image::FLAG_PALETTE_LOADED );
	}

	return 0;
}

mtPixy::Image * mtPixy::image_load_png (
	char	const	* const	filename
	)
{
	if ( ! filename )
	{
		return NULL;
	}


	Image		* image = NULL;
	png_bytep	* row_pointers = NULL;
	png_structp	png_ptr;
	png_infop	info_ptr;
	png_uint_32	pwidth, pheight;
	char		buf [ PNG_BYTES_TO_CHECK ];
	FILE		* fp;
	int		bit_depth, color_type;
	Palette		* pal;
	unsigned char	* src;


	fp = fopen ( filename, "rb" );
	if ( NULL == fp )
	{
		return NULL;
	}

	if ( PNG_BYTES_TO_CHECK != fread ( buf, 1, PNG_BYTES_TO_CHECK, fp ) )
	{
		fclose ( fp );
		return NULL;
	}

	if ( png_sig_cmp ( (unsigned char *)buf, 0, PNG_BYTES_TO_CHECK ) )
	{
		fclose ( fp );
		return NULL;
	}

	png_ptr = png_create_read_struct ( PNG_LIBPNG_VER_STRING, NULL, NULL,
		NULL);
	if ( ! png_ptr )
	{
		fclose ( fp );
		return NULL;
	}

	png_set_keep_unknown_chunks( png_ptr, PNG_HANDLE_CHUNK_ALWAYS, NULL, 0);

	info_ptr = png_create_info_struct ( png_ptr );
	if ( ! info_ptr )
	{
		goto fail;
	}

	if ( load_png_init ( fp, png_ptr, info_ptr, &pwidth, &pheight,
		&bit_depth, &color_type )
		)
	{
		goto fail;
	}

	// Lower width/height limits are checked in pixy_image_new
	if (	pwidth > Image::WIDTH_MAX	||
		pheight > Image::HEIGHT_MAX
		)
	{
		goto fail;
	}

	if (	color_type != PNG_COLOR_TYPE_PALETTE ||
		bit_depth > 8
		)
	{
		image = image_create ( Image::RGB, (int)pwidth,
			(int)pheight );
	}
	else
	{
		image = image_create ( Image::INDEXED, (int)pwidth,
			(int)pheight );
	}

	if ( ! image )
	{
		goto fail;
	}

	if ( color_type == PNG_COLOR_TYPE_RGB_ALPHA )
	{
		if ( image->create_alpha () )
		{
			goto fail;
		}
	}

	pal = image->get_palette ();
	src = image->get_canvas ();

	row_pointers = (png_bytep *)calloc ( (size_t)(pheight),
		sizeof ( png_bytep ) );
	if ( ! row_pointers )
	{
		goto fail;
	}

	if ( load_png_palette ( pal, png_ptr, info_ptr, image ) )
	{
		goto fail;
	}

	if ( image->get_type () == Image::RGB )
	{
		if ( load_rgb ( src, image->get_alpha (), (int)pwidth,
			(int)pheight, png_ptr, info_ptr, row_pointers ) )
		{
			goto fail;
		}
	}
	else	// Paletted PNG file
	{
		if ( load_paletted ( src, (int)pwidth, (int)pheight,
			png_ptr, info_ptr, row_pointers, color_type, bit_depth )
			)
		{
			goto fail;
		}

		png_unknown_chunkp uk_p;
		png_uint_32 const num_uk = png_get_unknown_chunks ( png_ptr,
						info_ptr, &uk_p );

		for ( png_uint_32 i = 0; i < num_uk; i++ )
		{
			if ( 0 != strcmp( (char const *)uk_p[i].name,
				"alPh" ) )
			{
				continue;
			}

			uLongf dest_len = (uLongf)(pwidth * pheight);

			if (	image->create_alpha ()		||
				! image->get_alpha ()
				)
			{
				goto fail;
			}

			uncompress ( image->get_alpha (), &dest_len,
				uk_p[i].data, uk_p[i].size );
			break;
		}
	}

	free ( row_pointers );
	row_pointers = NULL;

	png_destroy_read_struct ( &png_ptr, &info_ptr, NULL );
	fclose ( fp );

	return image;

fail:
	free ( row_pointers );
	row_pointers = NULL;

	png_destroy_read_struct ( &png_ptr, &info_ptr, NULL );
	fclose ( fp );

	delete image;
	return NULL;
}



///	SAVE	----------------------------------------------------------------



static int save_init (
	png_structp		&png_ptr,
	png_infop		&info_ptr,
	mtPixy::Image::Type const type,
	int		const	w,
	int		const	h,
	unsigned char	* const	alpha,
	int		const	compression,
	FILE		* const	fp,
	mtPixy::Palette	const	&palette
	)
{
	png_ptr = png_create_write_struct ( PNG_LIBPNG_VER_STRING, NULL, NULL,
		NULL );
	if ( ! png_ptr )
	{
		return 1;
	}

	info_ptr = png_create_info_struct ( png_ptr );
	if ( ! info_ptr )
	{
		return 1;
	}

	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		return 1;
	}

	png_init_io ( png_ptr, fp );
	png_set_compression_level ( png_ptr, compression );

	if ( type == mtPixy::Image::INDEXED )
	{
		png_set_IHDR( png_ptr, info_ptr, (png_uint_32)w, (png_uint_32)h,
			8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
	}
	else if ( type == mtPixy::Image::RGB )
	{
		png_set_IHDR( png_ptr, info_ptr, (png_uint_32)w, (png_uint_32)h,
			8,
			alpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT );
	}

	png_color	pal[256];
	int	const	coltot = palette.get_color_total ();

	if ( coltot > 0 )
	{
		mtPixy::Color const * const palcol = palette.get_color ();

		for ( int i = 0; i < coltot; i++ )
		{
			pal[i].red = palcol[i].red;
			pal[i].green = palcol[i].green;
			pal[i].blue = palcol[i].blue;
		}
	}

	png_set_PLTE ( png_ptr, info_ptr, pal, coltot );
	png_write_info ( png_ptr, info_ptr );

	return 0;
}

static int save_flat (
	png_structp		&png_ptr,
	unsigned char	* const	m_canvas,
	int		const	ww,
	int		const	h
	)
{
	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		return 1;
	}

	// Flat RGB/Indexed image
	for ( int y = 0; y < h; y++ )
	{
		png_write_row ( png_ptr, (png_bytep)( m_canvas + y * ww ) );
	}

	return 0;
}

static int save_rgba (
	png_structp		&png_ptr,
	unsigned char	* const	m_canvas,
	unsigned char	* const	m_alpha,
	int		const	w,
	int		const	h
	)
{
	unsigned char * rgba_row = (unsigned char *)malloc ( (size_t)(w * 4) );
	if ( ! rgba_row )
	{
		return 1;
	}

	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		free ( rgba_row );
		return 1;
	}

	unsigned char * tmi = m_canvas;
	unsigned char * tma = m_alpha;

	for ( int y = 0; y < h; y++ )
	{
		unsigned char * tmp = rgba_row;

		// Combine RGB and alpha
		for ( int x = 0; x < w; x++ )
		{
			tmp[0] = tmi[0];
			tmp[1] = tmi[1];
			tmp[2] = tmi[2];
			tmp[3] = tma[0];
			tmp += 4;
			tmi += 3;
			tma ++;
		}

		png_write_row ( png_ptr, (png_bytep)rgba_row );
	}

	free ( rgba_row );

	return 0;
}

static int save_idx_alpha (
	png_structp		&png_ptr,
	png_infop		&info_ptr,
	unsigned char	* const	m_alpha,
	int		const	w,
	int		const	h,
	int		const	compression
	)
{
	uLongf		const	dest_len = compressBound( (uLong)(w*h));
	unsigned char		* tmp;


	tmp = (unsigned char *)malloc ( dest_len );
	if ( ! tmp )
	{
		return 1;
	}

	uLongf res_len = dest_len;

	if ( compress2 ( tmp, &res_len, m_alpha, (uLong)(w * h),
		compression ) != Z_OK )
	{
		free ( tmp );
		return 1;
	}

	png_unknown_chunk	uk_chnk;

	strncpy ( (char *)uk_chnk.name, "alPh", 5 );
	uk_chnk.data = tmp;
	uk_chnk.size = res_len;
	uk_chnk.location = PNG_AFTER_IDAT;

	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		free ( tmp );
		return 1;
	}

	png_set_unknown_chunks ( png_ptr, info_ptr, &uk_chnk, 1 );

#if PNG_LIBPNG_VER < 10600	// before 1.6
	png_set_unknown_chunk_location ( png_ptr, info_ptr, 0, PNG_AFTER_IDAT );
#endif

	free ( tmp );

	return 0;
}

static int save_write_end (
	png_structp	&png_ptr,
	png_infop	&info_ptr
	)
{
	if ( setjmp ( png_jmpbuf ( png_ptr ) ) )
	{
		return 1;
	}

	png_write_end ( png_ptr, info_ptr );

	return 0;
}

int mtPixy::Image::save_png (
	char	const	* const	filename,
	int		const	compression
	) const
{
	if (	! filename		||
		compression < 0		||
		compression > 9		||
		! m_canvas
		)
	{
		return 1;
	}


	png_structp	png_ptr		= NULL;
	png_infop	info_ptr	= NULL;
	FILE		* fp		= NULL;
	int		res		= 1;


	fp = fopen ( filename, "wb" );
	if ( fp == NULL )
	{
		goto fail;
	}

/// PREPARE LIBPNG

	if ( save_init ( png_ptr, info_ptr, m_type, m_width, m_height, m_alpha,
		compression, fp, m_palette ) )
	{
		goto fail;
	}

/// SAVE IMAGE CANVAS

	if ( m_type == INDEXED || ! m_alpha )
	{
		if ( save_flat ( png_ptr, m_canvas, m_width * m_canvas_bpp,
			m_height ) )
		{
			goto fail;
		}
	}
	else if ( m_type == RGB && m_alpha )
	{
		if ( save_rgba( png_ptr, m_canvas, m_alpha, m_width, m_height ))
		{
			goto fail;
		}
	}

/// EXTRA CHUNK

	if ( m_type == INDEXED && m_alpha )
	{
		if ( save_idx_alpha ( png_ptr, info_ptr, m_alpha, m_width,
			m_height, compression ) )
		{
			goto fail;
		}
	}

	if ( save_write_end ( png_ptr, info_ptr ) )
	{
		goto fail;
	}

	res = 0;

fail:
	if ( png_ptr || info_ptr )
	{
		png_destroy_write_struct ( &png_ptr, &info_ptr );
		png_ptr = NULL;
		info_ptr = NULL;
	}

	if ( fp )
	{
		fclose ( fp );
		fp = NULL;
	}

	return res;
}



#else	// U_PNG



#include "private.h"



mtPixy::Image * mtPixy::image_load_png (
	char	const	* const	ARG_UNUSED ( filename )
	)
{
	return NULL;
}

int mtPixy::Image::save_png (
	char	const	* const	ARG_UNUSED ( filename ),
	int		const	ARG_UNUSED ( compression )
	)
{
	return 1;
}



#endif	// U_PNG


