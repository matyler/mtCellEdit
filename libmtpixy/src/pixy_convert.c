/*
	Copyright (C) 2004-2021 Mark Tyler

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



mtPixmap * pixy_pixmap_convert_to_rgb (
	mtPixmap const * const	pixmap
	)
{
	if ( ! pixmap )
	{
		return NULL;
	}

	if ( pixmap->bpp != PIXY_PIXMAP_BPP_INDEXED )
	{
		return NULL;
	}

	mtPixmap * im = pixy_pixmap_new_rgb ( pixmap->width, pixmap->height );
	if ( ! im )
	{
		return NULL;
	}

	pixy_palette_copy ( &im->palette, &pixmap->palette );

	if ( pixy_pixmap_copy_alpha ( im, pixmap ) )
	{
		pixy_pixmap_destroy ( &im );
		return NULL;
	}

	unsigned char	*	dest = im->canvas;
	unsigned char	* const	dlim = dest + pixmap->width * pixmap->height
					* PIXY_PIXMAP_BPP_RGB;
	unsigned char	*	src = pixmap->canvas;
	mtColor	const	*	col = &pixmap->palette.color[0];

	while ( dest < dlim )
	{
		unsigned char const pix = *src++;

		*dest++ = col [ pix ].red;
		*dest++ = col [ pix ].green;
		*dest++ = col [ pix ].blue;
	}

	return im;
}

static void flip_h_mem (
	unsigned char		* const	dest,
	unsigned char	const * const	src,
	int			const	w,
	int			const	h,
	int			const	bpp
	)
{
	if ( ! dest || ! src )
	{
		return;
	}

	for ( int y = 0; y < h; y++ )
	{
		unsigned char * d = dest + bpp * y * w;
		unsigned char const * s = src + bpp * y * w + bpp * (w - 1);

		if ( bpp == 3 )
		{
			for ( int x = 0; x < w; x++ )
			{
				*d++ = s[0];
				*d++ = s[1];
				*d++ = s[2];
				s -= 3;
			}
		}
		else if ( bpp == 1 )
		{
			for ( int x = 0; x < w; x++ )
			{
				*d++ = *s--;
			}
		}
	}
}

mtPixmap * pixy_pixmap_flip_horizontally (
	mtPixmap const * const	pixmap
	)
{
	mtPixmap * const i = pixy_pixmap_duplicate ( pixmap );

	if ( i )
	{
		flip_h_mem ( i->canvas, pixmap->canvas,
			pixmap->width, pixmap->height, pixmap->bpp );
		flip_h_mem ( i->alpha, pixmap->alpha,
			pixmap->width, pixmap->height, 1 );
	}

	return i;
}

static void flip_v_mem (
	unsigned char		* const	dest,
	unsigned char	const * const	src,
	int			const	w,
	int			const	h,
	int			const	bpp
	)
{
	if ( ! dest || ! src )
	{
		return;
	}

	size_t		const	tot = (size_t)(bpp * w);

	for ( int y = 0; y < h; y++ )
	{
		unsigned char * const d = dest + bpp * y * w;
		unsigned char const * const s = src + bpp * (h - y - 1) * w;

		memcpy ( d, s, tot );
	}
}

mtPixmap * pixy_pixmap_flip_vertically (
	mtPixmap const * const	pixmap
	)
{
	mtPixmap * const i = pixy_pixmap_duplicate ( pixmap );

	if ( i )
	{
		flip_v_mem ( i->canvas, pixmap->canvas,
			pixmap->width, pixmap->height, pixmap->bpp );
		flip_v_mem ( i->alpha, pixmap->alpha,
			pixmap->width, pixmap->height, 1 );
	}

	return i;
}

static void rotate_c_mem (
	unsigned char		* const	dest,
	unsigned char	const * const	src,
	int			const	w,
	int			const	h,
	int			const	bpp
	)
{
	if ( ! dest || ! src )
	{
		return;
	}

	unsigned char	const	* s = src;

	for ( int y = 0; y < h; y++ )
	{
		unsigned char * d = dest + bpp * (h - y - 1);

		if ( bpp == 3 )
		{
			for ( int x = 0; x < w; x++ )
			{
				d[0] = *s++;
				d[1] = *s++;
				d[2] = *s++;
				d += h * 3;
			}
		}
		else if ( bpp == 1 )
		{
			for ( int x = 0; x < w; x++ )
			{
				d[0] = *s++;
				d += h;
			}
		}
	}
}

mtPixmap * pixy_pixmap_rotate_clockwise (
	mtPixmap const * const	pixmap
	)
{
	mtPixmap * im = pixy_pixmap_new ( pixmap->bpp, pixmap->height,
		pixmap->width );
	if ( ! im )
	{
		return NULL;
	}

	if ( pixmap->alpha && pixy_pixmap_create_alpha ( im ) )
	{
		pixy_pixmap_destroy ( &im );
		return NULL;
	}

	pixy_palette_copy ( &im->palette, &pixmap->palette );

	rotate_c_mem ( im->canvas, pixmap->canvas, pixmap->width,
		pixmap->height, pixmap->bpp );
	rotate_c_mem ( im->alpha, pixmap->alpha, pixmap->width,
		pixmap->height, 1 );

	return im;
}

static void rotate_a_mem (
	unsigned char		* const	dest,
	unsigned char	const * const	src,
	int			const	w,
	int			const	h,
	int			const	bpp
	)
{
	if ( ! dest || ! src || bpp < 1 )
	{
		return;
	}


	unsigned char	const	* s = src;


	for ( int y = 0; y < h; y++ )
	{
		unsigned char * d = dest + bpp * (y + (w - 1) * h);

		if ( bpp == 3 )
		{
			for ( int x = 0; x < w; x++ )
			{
				d[0] = *s++;
				d[1] = *s++;
				d[2] = *s++;
				d -= h * 3;
			}
		}
		else if ( bpp == 1 )
		{
			for ( int x = 0; x < w; x++ )
			{
				d[0] = *s++;
				d -= h;
			}
		}
	}
}

mtPixmap * pixy_pixmap_rotate_anticlockwise (
	mtPixmap const * const	pixmap
	)
{
	mtPixmap * im = pixy_pixmap_new ( pixmap->bpp, pixmap->height,
		pixmap->width );
	if ( ! im )
	{
		return NULL;
	}

	if ( pixmap->alpha && pixy_pixmap_create_alpha ( im ) )
	{
		pixy_pixmap_destroy ( &im );
		return NULL;
	}

	pixy_palette_copy ( &im->palette, &pixmap->palette );

	rotate_a_mem ( im->canvas, pixmap->canvas, pixmap->width,
		pixmap->height, pixmap->bpp );
	rotate_a_mem ( im->alpha, pixmap->alpha, pixmap->width,
		pixmap->height, 1 );

	return im;
}

