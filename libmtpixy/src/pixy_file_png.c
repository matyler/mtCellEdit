/*
	Copyright (C) 2008-2023 Mark Tyler

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
	mtPalette	* const	pal,
	png_structp	const	png_ptr,
	png_infop	const	info_ptr,
	mtPixmap	* const	image
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
		mtColor		* const palcol = &pal->color[0];


		png_get_PLTE ( png_ptr, info_ptr, &png_palette, &coltot );

		if ( pixy_palette_set_size ( pal, coltot ) )
		{
			return 1;
		}

		for ( int i = 0; i < coltot; i++ )
		{
			palcol[i].red = png_palette[i].red;
			palcol[i].green = png_palette[i].green;
			palcol[i].blue = png_palette[i].blue;
		}

		image->palette_file = 1;
	}

	return 0;
}

mtPixmap * pixy_pixmap_load_png ( char const * const filename )
{
	if ( ! filename )
	{
		return NULL;
	}


	mtPixmap	* image = NULL;
	png_bytep	* row_pointers = NULL;
	png_structp	png_ptr;
	png_infop	info_ptr;
	png_uint_32	pwidth, pheight;
	char		buf [ PNG_BYTES_TO_CHECK ];
	FILE		* fp;
	int		bit_depth, color_type;
	mtPalette	* pal;
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

	// Width/height limits are checked in pixy_image_new

	if (	color_type != PNG_COLOR_TYPE_PALETTE ||
		bit_depth > 8
		)
	{
		image = pixy_pixmap_new_rgb ( (int)pwidth, (int)pheight );
	}
	else
	{
		image = pixy_pixmap_new_indexed ( (int)pwidth, (int)pheight );
	}

	if ( ! image )
	{
		goto fail;
	}

	if ( color_type == PNG_COLOR_TYPE_RGB_ALPHA )
	{
		if ( pixy_pixmap_create_alpha ( image ) )
		{
			goto fail;
		}
	}

	pal = &image->palette;
	src = image->canvas;

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

	if ( pixy_pixmap_get_bytes_per_pixel ( image ) == 3 )
	{
		if ( load_rgb ( src, image->alpha, (int)pwidth,
			(int)pheight, png_ptr, info_ptr, row_pointers ) )
		{
			fprintf(stderr, "Unable to load RGB image\n");
			goto fail;
		}
	}
	else	// Paletted PNG file
	{
		if ( load_paletted ( src, (int)pwidth, (int)pheight,
			png_ptr, info_ptr, row_pointers, color_type, bit_depth )
			)
		{
			fprintf(stderr, "Unable to load palette based image\n");
			goto fail;
		}

		png_unknown_chunkp uk_p;
		int const num_uk = (int)png_get_unknown_chunks ( png_ptr,
						info_ptr, &uk_p );

		for ( int i = 0; i < num_uk; i++ )
		{
			if ( 0 != strcmp( (char const *)uk_p[i].name,
				"alPh" ) )
			{
				continue;
			}

			unsigned char * alp;

			if (	pixy_pixmap_create_alpha ( image )	||
				! (alp = image->alpha)			||
				mtkit_mem_inflate ( uk_p[i].data,
					uk_p[i].size, &alp, pwidth * pheight,
					0 )
				)
			{
				fprintf(stderr, "Unable to load alpha\n");
				// Don't abort here, just give up quietly
			}

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

	pixy_pixmap_destroy ( &image );
	return NULL;
}

static int save_init (
	png_structp	* const	png_ptr,
	png_infop	* const	info_ptr,
	int		const	bpp,
	int		const	w,
	int		const	h,
	unsigned char	* const	alpha,
	int		const	compression,
	FILE		* const	fp,
	mtPalette const * const	palette
	)
{
	*png_ptr = png_create_write_struct ( PNG_LIBPNG_VER_STRING, NULL, NULL,
		NULL );
	if ( ! *png_ptr )
	{
		return 1;
	}

	*info_ptr = png_create_info_struct ( *png_ptr );
	if ( ! *info_ptr )
	{
		return 1;
	}

	if ( setjmp ( png_jmpbuf ( *png_ptr ) ) )
	{
		return 1;
	}

	png_init_io ( *png_ptr, fp );
	png_set_compression_level ( *png_ptr, compression );

	if ( bpp == 1 )
	{
		png_set_IHDR( *png_ptr, *info_ptr,
			(png_uint_32)w, (png_uint_32)h,
			8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
	}
	else if ( bpp == 3 )
	{
		png_set_IHDR( *png_ptr, *info_ptr,
			(png_uint_32)w, (png_uint_32)h, 8,
			alpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT );
	}

	png_color	pal[256];
	int	const	coltot = palette->size;

	if ( coltot > 0 )
	{
		mtColor const * const palcol = &palette->color[0];

		for ( int i = 0; i < coltot; i++ )
		{
			pal[i].red = palcol[i].red;
			pal[i].green = palcol[i].green;
			pal[i].blue = palcol[i].blue;
		}
	}

	png_set_PLTE ( *png_ptr, *info_ptr, pal, coltot );
	png_write_info ( *png_ptr, *info_ptr );

	return 0;
}

static int save_flat (
	png_structp	* const png_ptr,
	unsigned char	* const	canvas,
	int		const	ww,
	int		const	h
	)
{
	if ( setjmp ( png_jmpbuf ( *png_ptr ) ) )
	{
		return 1;
	}

	// Flat RGB/Indexed image
	for ( int y = 0; y < h; y++ )
	{
		png_write_row ( *png_ptr, (png_bytep)( canvas + y * ww ) );
	}

	return 0;
}

static int save_rgba (
	png_structp	* const	png_ptr,
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

	if ( setjmp ( png_jmpbuf ( *png_ptr ) ) )
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

		png_write_row ( *png_ptr, (png_bytep)rgba_row );
	}

	free ( rgba_row );

	return 0;
}

static int save_idx_alpha (
	png_structp	* const	png_ptr,
	png_infop	* const	info_ptr,
	unsigned char	* const	m_alpha,
	int		const	w,
	int		const	h,
	int		const	compression
	)
{
	unsigned char	* tmp;
	size_t		res_len;

	if ( mtkit_mem_deflate ( m_alpha, (size_t)(w * h), &tmp, &res_len,
		compression, MTKIT_DEFLATE_MODEL_DEFAULT ) )
	{
		return 1;
	}

	png_unknown_chunk	uk_chnk;

	strncpy ( (char *)uk_chnk.name, "alPh", 5 );
	uk_chnk.data = tmp;
	uk_chnk.size = res_len;
	uk_chnk.location = PNG_AFTER_IDAT;

	if ( setjmp ( png_jmpbuf ( *png_ptr ) ) )
	{
		free ( tmp );
		return 1;
	}

	png_set_unknown_chunks ( *png_ptr, *info_ptr, &uk_chnk, 1 );

#if PNG_LIBPNG_VER < 10600	// before 1.6
	png_set_unknown_chunk_location( *png_ptr, *info_ptr, 0, PNG_AFTER_IDAT);
#endif

	free ( tmp );

	return 0;
}

static int save_write_end (
	png_structp	* const	png_ptr,
	png_infop	* const	info_ptr
	)
{
	if ( setjmp ( png_jmpbuf ( *png_ptr ) ) )
	{
		return 1;
	}

	png_write_end ( *png_ptr, *info_ptr );

	return 0;
}

int pixy_pixmap_save_png (
	mtPixmap	const * const	pixmap,
	char		const * const	filename,
	int			const	compression
	)
{
	if (	! pixmap		||
		! filename		||
		compression < 0		||
		compression > 9		||
		! pixmap->canvas
		)
	{
		return 1;
	}


	png_structp	png_ptr		= NULL;
	png_infop	info_ptr	= NULL;
	int		res		= 1;
	int	const	bpp = pixy_pixmap_get_bytes_per_pixel ( pixmap );


	FILE * fp = fopen ( filename, "wb" );
	if ( fp == NULL )
	{
		goto fail;
	}

/// PREPARE LIBPNG

	if ( save_init ( &png_ptr, &info_ptr, bpp,
		pixmap->width, pixmap->height,
		pixmap->alpha, compression, fp, &pixmap->palette ) )
	{
		goto fail;
	}

/// SAVE IMAGE CANVAS

	if ( bpp == 1 || ! pixmap->alpha )
	{
		if ( save_flat ( &png_ptr, pixmap->canvas,
			pixmap->width * bpp, pixmap->height ) )
		{
			goto fail;
		}
	}
	else if ( bpp == 3 && pixmap->alpha )
	{
		if ( save_rgba( &png_ptr, pixmap->canvas, pixmap->alpha,
			pixmap->width, pixmap->height ) )
		{
			goto fail;
		}
	}

/// EXTRA CHUNK

	if ( bpp == 1 && pixmap->alpha )
	{
		if ( save_idx_alpha ( &png_ptr, &info_ptr, pixmap->alpha,
			pixmap->width, pixmap->height, compression ) )
		{
			goto fail;
		}
	}

	if ( save_write_end ( &png_ptr, &info_ptr ) )
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

