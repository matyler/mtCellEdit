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



#ifdef U_JPEG
	#include <jpeglib.h>



struct my_error_mgr
{
	struct jpeg_error_mgr	pub;		// "public" fields
	jmp_buf			setjmp_buffer;	// for return to caller
};



typedef struct my_error_mgr * my_error_ptr;



METHODDEF ( void ) my_error_exit (
	j_common_ptr cinfo
	)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	longjmp ( myerr->setjmp_buffer, 1 );
}



struct my_error_mgr	jerr;



static int jpeg_load_init (
	FILE		* const fp,
	struct jpeg_decompress_struct * const cinfo
	)
{
	jpeg_create_decompress ( cinfo );
	cinfo->err = jpeg_std_error ( &jerr.pub );
	jerr.pub.error_exit = my_error_exit;

	if ( setjmp ( jerr.setjmp_buffer ) )
	{
		return 1;
	}

	jpeg_stdio_src ( cinfo, fp );
	jpeg_read_header ( cinfo, TRUE );
	jpeg_start_decompress ( cinfo );

	return 0;
}

static int jpeg_load_cmyk (
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
	unsigned char	* dest;
	int		i, j, k, r, g, b;


	if ( setjmp ( jerr.setjmp_buffer ) )
	{
		free ( memx );
		return 1;
	}

	for ( i = 0; i < h; i++ )
	{
		src = memx;
		dest = canmem + w * i * canvas_bpp;
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
	unsigned char		* const	canmem,
	int			const	w,
	int			const	h,
	int			const	canvas_bpp,
	struct jpeg_decompress_struct * const cinfo
	)
{
	unsigned char	* dest;
	int		i;


	if ( setjmp ( jerr.setjmp_buffer ) )
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

mtPixy::Image * mtPixy::image_load_jpeg (
	char	const	* const	filename
	)
{
	if ( ! filename )
	{
		return NULL;
	}


	struct jpeg_decompress_struct cinfo;
	FILE		* fp;
	int		w, h;
	Image		* image = NULL;


	fp = fopen ( filename, "rb" );
	if ( NULL == fp )
	{
		return NULL;
	}

	if ( jpeg_load_init ( fp, &cinfo ) )
	{
		goto fail;
	}

	w = (int)cinfo.output_width;
	h = (int)cinfo.output_height;

	switch ( cinfo.out_color_space )
	{
	case JCS_GRAYSCALE:
		image = image_create ( Image::INDEXED, w, h );
		if ( ! image )
		{
			goto fail;
		}

		image->get_palette()->set_grey ();
		image->set_file_flag ( Image::FLAG_PALETTE_LOADED );

		if ( jpeg_load_rgb ( image->get_canvas (), w, h,
			image->get_canvas_bpp (), &cinfo ) )
		{
			goto fail;
		}
		break;

	case JCS_RGB:
		image = image_create ( Image::RGB, w, h );
		if ( ! image )
		{
			goto fail;
		}

		if ( jpeg_load_rgb ( image->get_canvas (), w, h,
			image->get_canvas_bpp (), &cinfo ) )
		{
			goto fail;
		}
		break;

	case JCS_CMYK:
		// Photoshop writes CMYK data inverted
		image = image_create ( Image::RGB, w, h );
		if ( ! image )
		{
			goto fail;
		}

		if ( jpeg_load_cmyk ( image->get_canvas (), w, h,
			image->get_canvas_bpp (), &cinfo )
			)
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

	delete image;

	return NULL;
}

int mtPixy::Image::save_jpeg (
	char	const	* const	filename,
	int		const	compression
	) const
{
	if (	! filename		||
		m_type != RGB		||
		compression < 0		||
		compression > 100
		)
	{
		return 1;
	}


	struct jpeg_compress_struct cinfo;
	JSAMPROW	row_pointer;
	FILE		* fp;
	int		i;


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

	cinfo.image_width = (JDIMENSION)m_width;
	cinfo.image_height = (JDIMENSION)m_height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults ( &cinfo );
	jpeg_set_quality ( &cinfo, compression, TRUE );
	jpeg_start_compress ( &cinfo, TRUE );

	row_pointer = m_canvas;

	for ( i = 0; i < m_height; i++ )
	{
		jpeg_write_scanlines ( &cinfo, &row_pointer, 1 );
		row_pointer += 3 * m_width;
	}

	jpeg_finish_compress ( &cinfo );
	jpeg_destroy_compress ( &cinfo );
	fclose ( fp );

	return 0;
}



#else	// U_JPEG



mtPixy::Image * mtPixy::image_load_jpeg (
	char	const	* const	ARG_UNUSED ( filename )
	)
{
	return NULL;
}

int mtPixy::Image::save_jpeg (
	char	const	* const	ARG_UNUSED ( filename ),
	int		const	ARG_UNUSED ( compression )
	)
{
	return 1;
}



#endif	// U_JPEG

