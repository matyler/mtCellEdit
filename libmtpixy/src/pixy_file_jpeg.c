/*
	Copyright (C) 2008-2021 Mark Tyler

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

#include <jpeglib.h>



typedef struct
{
	struct jpeg_error_mgr	pub;		// "public" fields
	jmp_buf			setjmp_buffer;	// for return to caller

} my_error_mgr;



METHODDEF ( void ) my_error_exit (
	j_common_ptr cinfo
	)
{
	my_error_mgr * myerr = (my_error_mgr *) cinfo->err;
	longjmp ( myerr->setjmp_buffer, 1 );
}



static int jpeg_load_init (
	my_error_mgr		* const	jerr,
	FILE			* const	fp,
	struct jpeg_decompress_struct * const cinfo
	)
{
	jpeg_create_decompress ( cinfo );
	cinfo->err = jpeg_std_error ( &jerr->pub );
	jerr->pub.error_exit = my_error_exit;

	if ( setjmp ( jerr->setjmp_buffer ) )
	{
		return 1;
	}

	jpeg_stdio_src ( cinfo, fp );
	jpeg_read_header ( cinfo, TRUE );
	jpeg_start_decompress ( cinfo );

	return 0;
}

static int jpeg_load_cmyk (
	my_error_mgr		* const	jerr,
	unsigned char		* const	canmem,
	int			const	w,
	int			const	h,
	int			const	canvas_bpp,
	struct jpeg_decompress_struct * const cinfo
	)
{
	unsigned char	* const memx = (unsigned char *)malloc (
					cinfo->output_width * 4 );

	if ( ! memx )
	{
		return 1;
	}


	unsigned char	const xb = cinfo->saw_Adobe_marker ? 0 : 255;
	unsigned char	* src;
	int		i, j, k, r, g, b;


	if ( setjmp ( jerr->setjmp_buffer ) )
	{
		free ( memx );
		return 1;
	}

	for ( i = 0; i < h; i++ )
	{
		src = memx;
		unsigned char * dest = canmem + w * i * canvas_bpp;
		jpeg_read_scanlines ( cinfo, &src, 1 );

		for ( j = 0; j < w; j++ , src += 4 , dest += 3 )
		{
			k = src[3] ^ xb;

			r = (src[0] ^ xb) * k;
			r = (r + (r >> 8) + 1) >> 8;
			dest[0] = (unsigned char)r;

			g = (src[1] ^ xb) * k;
			g = (g + (g >> 8) + 1) >> 8;
			dest[1] = (unsigned char)g;

			b = (src[2] ^ xb) * k;
			b = (b + (b >> 8) + 1) >> 8;
			dest[2] = (unsigned char)b;
		}
	}

	jpeg_finish_decompress ( cinfo );
	free ( memx );

	return 0;
}

static int jpeg_load_rgb (
	my_error_mgr		* const	jerr,
	unsigned char		* const	canmem,
	int			const	w,
	int			const	h,
	int			const	canvas_bpp,
	struct jpeg_decompress_struct * const cinfo
	)
{
	unsigned char	* dest;
	int		i;


	if ( setjmp ( jerr->setjmp_buffer ) )
	{
		return 1;
	}

	for ( i = 0; i < h; i++ )
	{
		dest = canmem + w * i * canvas_bpp;
		jpeg_read_scanlines ( cinfo, &dest, 1 );
	}

	jpeg_finish_decompress ( cinfo );

	return 0;
}

mtPixmap * pixy_pixmap_load_jpeg ( char const * const filename )
{
	if ( ! filename )
	{
		return NULL;
	}


	my_error_mgr	jerr;
	struct jpeg_decompress_struct cinfo;
	FILE		* fp;
	int		w, h;
	mtPixmap	* image = NULL;

	memset ( &jerr, 0, sizeof(jerr) );

	fp = fopen ( filename, "rb" );
	if ( NULL == fp )
	{
		return NULL;
	}

	if ( jpeg_load_init ( &jerr, fp, &cinfo ) )
	{
		goto fail;
	}

	w = (int)cinfo.output_width;
	h = (int)cinfo.output_height;

	switch ( cinfo.out_color_space )
	{
	case JCS_GRAYSCALE:
		image = pixy_pixmap_new_indexed ( w, h );
		if ( ! image )
		{
			goto fail;
		}

		pixy_palette_set_grey ( &image->palette );
		image->palette_file = 1;

		if ( jpeg_load_rgb ( &jerr, image->canvas, w, h, 1, &cinfo ) )
		{
			goto fail;
		}
		break;

	case JCS_RGB:
		image = pixy_pixmap_new_rgb ( w, h );
		if ( ! image )
		{
			goto fail;
		}

		if ( jpeg_load_rgb ( &jerr, image->canvas, w, h, 3, &cinfo ) )
		{
			goto fail;
		}
		break;

	case JCS_CMYK:
		// Photoshop writes CMYK data inverted
		image = pixy_pixmap_new_rgb ( w, h );
		if ( ! image )
		{
			goto fail;
		}

		if ( jpeg_load_cmyk ( &jerr, image->canvas, w, h, 3, &cinfo ) )
		{
			goto fail;
		}
		break;

	default:
		// Unsupported colorspace
		goto fail;
	}

	jpeg_destroy_decompress ( &cinfo );
	fclose ( fp );

	return image;

fail:
	jpeg_destroy_decompress ( &cinfo );
	fclose ( fp );

	pixy_pixmap_destroy ( &image );

	return NULL;
}

int pixy_pixmap_save_jpeg (
	mtPixmap	const * const	pixmap,
	char		const * const	filename,
	int			const	compression
	)
{
	if (	! pixmap		||
		! filename		||
		pixy_pixmap_get_bytes_per_pixel ( pixmap ) != 3	||
		compression < 0		||
		compression > 100
		)
	{
		return 1;
	}


	my_error_mgr	jerr;
	struct jpeg_compress_struct cinfo;
	JSAMPROW	row_pointer;
	FILE		* fp;
	int		i;

	memset ( &jerr, 0, sizeof(jerr) );

	fp = fopen ( filename, "wb" );
	if ( NULL == fp )
	{
		return 1;
	}

	cinfo.err = jpeg_std_error ( &jerr.pub );
	jerr.pub.error_exit = my_error_exit;

	if ( setjmp ( jerr.setjmp_buffer ) )
	{
		jpeg_destroy_compress ( &cinfo );
		fclose ( fp );

		return 1;
	}

	jpeg_create_compress ( &cinfo );
	jpeg_stdio_dest ( &cinfo, fp );

	cinfo.image_width = (JDIMENSION)pixmap->width;
	cinfo.image_height = (JDIMENSION)pixmap->height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults ( &cinfo );
	jpeg_set_quality ( &cinfo, compression, TRUE );
	jpeg_start_compress ( &cinfo, TRUE );

	row_pointer = pixmap->canvas;

	for ( i = 0; i < pixmap->height; i++ )
	{
		jpeg_write_scanlines ( &cinfo, &row_pointer, 1 );
		row_pointer += 3 * pixmap->width;
	}

	jpeg_finish_compress ( &cinfo );
	jpeg_destroy_compress ( &cinfo );
	fclose ( fp );

	return 0;
}

