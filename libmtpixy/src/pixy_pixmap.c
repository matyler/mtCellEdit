/*
	Copyright (C) 2008-2021 Mark Tyler

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



mtPixmap * pixy_pixmap_new (
	int	const	type,
	int	const	width,
	int	const	height
	)
{
	if (	width < PIXY_PIXMAP_WIDTH_MIN
		|| width > PIXY_PIXMAP_WIDTH_MAX
		|| height < PIXY_PIXMAP_HEIGHT_MIN
		|| height > PIXY_PIXMAP_HEIGHT_MAX
		)
	{
		return NULL;
	}

	int bpp;

	switch ( type )
	{
	case PIXY_PIXMAP_BPP_RGB:
		bpp = 3;
		break;

	default:
		bpp = 1;
		break;
	}

	void * mem = calloc ( (size_t)bpp, (size_t)(width * height) );
	if ( ! mem )
	{
		return NULL;
	}

	mtPixmap * const pixmap = calloc ( 1, sizeof(mtPixmap) );
	if ( ! pixmap )
	{
		free ( mem );
		return NULL;
	}

	pixmap->width = width;
	pixmap->height = height;
	pixmap->bpp = bpp;
	pixmap->palette_file = 0;

	pixy_palette_init ( & pixmap->palette );

	switch ( type )
	{
	case 0:	// ALPHA
		pixmap->alpha = (unsigned char *)mem;
		break;

	default: // INDEXED, RGB
		pixmap->canvas = (unsigned char *)mem;
		break;
	}

	return pixmap;
}

mtPixmap * pixy_pixmap_new_rgb (
	int	const	width,
	int	const	height
	)
{
	return pixy_pixmap_new ( 3, width, height );
}

mtPixmap * pixy_pixmap_new_indexed (
	int	const	width,
	int	const	height
	)
{
	return pixy_pixmap_new ( 1, width, height );
}

mtPixmap * pixy_pixmap_new_alpha (
	int	const	width,
	int	const	height
	)
{
	return pixy_pixmap_new ( 0, width, height );
}

int pixy_pixmap_create_alpha (
	mtPixmap	* const	pixmap
	)
{
	if ( ! pixmap )
	{
		return 1;
	}

	size_t const pixels = (size_t)(pixmap->width * pixmap->height);
	if ( pixels < 1 )
	{
		return 1;
	}

	unsigned char * mem = calloc ( 1, pixels );
	if ( ! mem )
	{
		return 1;
	}

	free ( pixmap->alpha );
	pixmap->alpha = mem;

	return 0;
}

mtPixmap * pixy_pixmap_duplicate (
	mtPixmap const	* const	pixmap
	)
{
	if ( ! pixmap )
	{
		return NULL;
	}

	mtPixmap * dup = calloc ( 1, sizeof(mtPixmap) );
	if ( ! dup )
	{
		return NULL;
	}

	size_t const pixel_tot = (size_t)(pixmap->width * pixmap->height);

	if ( pixmap->alpha )
	{
		dup->alpha = malloc ( pixel_tot );
		if ( ! dup->alpha )
		{
			goto error;
		}

		memcpy ( dup->alpha, pixmap->alpha, pixel_tot );
	}

	size_t const bpp = (size_t)pixy_pixmap_get_bytes_per_pixel ( pixmap );

	if ( pixmap->canvas && bpp > 0 )
	{
		dup->canvas = malloc ( pixel_tot * bpp );
		if ( ! dup->canvas )
		{
			goto error;
		}

		memcpy ( dup->canvas, pixmap->canvas, pixel_tot * bpp );
	}

	dup->width = pixmap->width;
	dup->height = pixmap->height;
	dup->bpp = pixmap->bpp;
	dup->palette_file = pixmap->palette_file;
	pixy_palette_copy ( &dup->palette, &pixmap->palette );

	return dup;

error:
	pixy_pixmap_destroy ( &dup );
	return NULL;
}

void pixy_pixmap_destroy (
	mtPixmap ** const	pixmap
	)
{
	if ( pixmap )
	{
		if ( pixmap[0] )
		{
			free ( pixmap[0]->canvas );
			free ( pixmap[0]->alpha );
			free ( pixmap[0] );

			pixmap[0] = NULL;
		}
	}
}

int pixy_pixmap_get_width (
	mtPixmap const * const	pixmap
	)
{
	if ( pixmap )
	{
		return pixmap->width;
	}

	return 0;
}

int pixy_pixmap_get_height (
	mtPixmap const * const	pixmap
	)
{
	if ( pixmap )
	{
		return pixmap->height;
	}

	return 0;
}

int pixy_pixmap_get_bytes_per_pixel (
	mtPixmap const * const	pixmap
	)
{
	if ( pixmap )
	{
		return pixmap->bpp;
	}

	return 0;
}

mtPalette * pixy_pixmap_get_palette (
	mtPixmap	* const	pixmap
	)
{
	if ( pixmap )
	{
		return &pixmap->palette;
	}

	return NULL;
}

mtPalette const * pixy_pixmap_get_palette_const (
	mtPixmap const * const	pixmap
	)
{
	if ( pixmap )
	{
		return &pixmap->palette;
	}

	return NULL;
}

int pixy_pixmap_get_palette_size (
	mtPixmap const * const	pixmap
	)
{
	if ( pixmap )
	{
		return pixmap->palette.size;
	}

	return 0;
}

unsigned char * pixy_pixmap_get_canvas (
	mtPixmap	const * const	pixmap
	)
{
	if ( pixmap )
	{
		return pixmap->canvas;
	}

	return NULL;
}

unsigned char * pixy_pixmap_get_alpha (
	mtPixmap	const * const	pixmap
	)
{
	if ( pixmap )
	{
		return pixmap->alpha;
	}

	return NULL;
}

int pixy_pixmap_destroy_alpha (
	mtPixmap	* const	pixmap
	)
{
	if ( ! pixmap || ! pixmap->alpha )
	{
		return 1;
	}

	free ( pixmap->alpha );
	pixmap->alpha = NULL;

	return 0;
}

/*
void pixy_pixmap_set_data (
	mtPixmap	* const	pixmap,
	int		const	w,
	int		const	h,
	unsigned char	* const	canvas,
	unsigned char	* const	alpha
	)
{
	if ( pixmap )
	{
		pixmap->width = w;
		pixmap->height = h;

		free ( pixmap->canvas );
		pixmap->canvas = canvas;

		free ( pixmap->alpha );
		pixmap->alpha = alpha;
	}
}
*/

mtPixmap * pixy_pixmap_from_cairo (
	cairo_surface_t	* const	surface
	)
{
	if ( ! surface )
	{
		return NULL;
	}

	unsigned char const * const src = cairo_image_surface_get_data(surface);
	if ( ! src )
	{
		return NULL;
	}

	cairo_format_t const format = cairo_image_surface_get_format( surface );
	int const bpp =
		(format == CAIRO_FORMAT_ARGB32)		? 4 :
		(format == CAIRO_FORMAT_RGB24)		? 3 : 0;

	if ( bpp < 3 )
	{
		return NULL;
	}

	int const w = cairo_image_surface_get_width ( surface );
	int const h = cairo_image_surface_get_height ( surface );

	mtPixmap * image = pixy_pixmap_new_rgb ( w, h );
	if ( ! image )
	{
		return NULL;
	}

	if ( 4 == bpp )
	{
		pixy_pixmap_create_alpha ( image );
	}

	unsigned char * const dest = image->canvas;
	unsigned char * const alpha = image->alpha;
	if ( ! dest )
	{
		pixy_pixmap_destroy ( &image );
		return NULL;
	}

	// Copy data from Cairo to mtImage
	int	const	s_stride = cairo_image_surface_get_stride ( surface );
	int	const	d_stride = w * 3;
	uint32_t const	one = 1;
	uint32_t const	le = ( (uint8_t const *) &one )[0];
			// 1 = Little endian 0 = Big endian

	if ( le ) for ( int y = 0; y < h; y++ )	// Little endian
	{
		unsigned char		const	* s = src + y * s_stride;
		unsigned char			* d = dest + y * d_stride;
		unsigned char	const * const	s_end = s + w * 4;

		for ( ; s < s_end; s += 4 )
		{
			*d++ = s[2];		// Red
			*d++ = s[1];		// Green
			*d++ = s[0];		// Blue
		}

		if ( alpha )
		{
			d = alpha + y * w;
			s = src + y * s_stride;

			for ( ; s < s_end; s += 4 )
			{
				*d++ = s[3];		// Alpha
			}
		}
	}
	else for ( int y = 0; y < h; y++ )	// Big endian
	{
		unsigned char		const	* s = src + y * s_stride;
		unsigned char			* d = dest + y * d_stride;
		unsigned char	const * const	s_end = s + w * 4;

		for ( ; s < s_end; s += 4 )
		{
			*d++ = s[1];		// Red
			*d++ = s[2];		// Green
			*d++ = s[3];		// Blue
		}

		if ( alpha )
		{
			d = alpha + y * w;
			s = src + y * s_stride;

			for ( ; s < s_end; s += 4 )
			{
				*d++ = s[0];		// Alpha
			}
		}
	}

	return image;
}

int pixy_pixmap_create_indexed_canvas (
	mtPixmap	* const pixmap
	)
{
	if ( ! pixmap )
	{
		return 1;
	}

	unsigned char * canvas = calloc( PIXY_PIXMAP_BPP_INDEXED,
		(size_t)(pixmap->width * pixmap->height));
	if ( ! canvas )
	{
		return 1;
	}

	pixmap->bpp = PIXY_PIXMAP_BPP_INDEXED;

	free ( pixmap->canvas );
	pixmap->canvas = canvas;

	return 0;
}

int pixy_pixmap_create_rgb_canvas (
	mtPixmap	* const pixmap
	)
{
	if ( ! pixmap )
	{
		return 1;
	}

	unsigned char * canvas = calloc( PIXY_PIXMAP_BPP_RGB,
		(size_t)(pixmap->width * pixmap->height));
	if ( ! canvas )
	{
		return 1;
	}

	pixmap->bpp = PIXY_PIXMAP_BPP_RGB;

	free ( pixmap->canvas );
	pixmap->canvas = canvas;

	return 0;
}

mtPixmap * pixy_pixmap_from_data (
	int		const	type,
	int		const	width,
	int		const	height,
	unsigned char	* const	canvas,
	unsigned char	* const	alpha
	)
{
	if (	width < PIXY_PIXMAP_WIDTH_MIN
		|| width > PIXY_PIXMAP_WIDTH_MAX
		|| height < PIXY_PIXMAP_HEIGHT_MIN
		|| height > PIXY_PIXMAP_HEIGHT_MAX
		|| type < 0
		)
	{
		return NULL;
	}

	switch ( type )
	{
	case PIXY_PIXMAP_BPP_ALPHA_ONLY:
	case PIXY_PIXMAP_BPP_INDEXED:
	case PIXY_PIXMAP_BPP_RGB:
		break;

	default:
		return NULL;
	}

	mtPixmap * const pixmap = calloc ( 1, sizeof(mtPixmap) );
	if ( ! pixmap )
	{
		return NULL;
	}

	pixmap->width = width;
	pixmap->height = height;
	pixmap->bpp = type;
	pixmap->palette_file = 0;
	pixmap->canvas = canvas;
	pixmap->alpha = alpha;

	pixy_palette_init ( & pixmap->palette );

	return pixmap;
}

void pixy_pixmap_print_geometry (
	mtPixmap const * const	pixmap,
	char		* const	buf,
	size_t		const	buflen
	)
{
	if ( ! pixmap )
	{
		snprintf ( buf, buflen, "NONE" );
		return;
	}

	switch ( pixmap->bpp )
	{
	case PIXY_PIXMAP_BPP_RGB:
		snprintf ( buf, buflen, "%i x %i x RGB",
			pixmap->width, pixmap->height );
		break;

	case PIXY_PIXMAP_BPP_INDEXED:
		snprintf ( buf, buflen, "%i x %i x %i",
			pixmap->width, pixmap->height,
			pixmap->palette.size );
		break;

	case PIXY_PIXMAP_BPP_ALPHA_ONLY:
		snprintf ( buf, buflen, "%i x %i (alpha)",
			pixmap->width, pixmap->height );
		break;

	default:
		snprintf ( buf, buflen, "%i x %i x ???",
			pixmap->width, pixmap->height );
		break;
	}

	if ( pixmap->alpha )
	{
		mtkit_strnncat ( buf, " + A", buflen );
	}
}

int pixy_pixmap_copy_alpha (
	mtPixmap	* const	dest,
	mtPixmap const * const	src
	)
{
	if (	! dest				||
		! src				||
		dest->width != src->width	||
		dest->height != src->height
		)
	{
		return 1;
	}

	if ( ! src->alpha )
	{
		free ( dest->alpha );
		dest->alpha = NULL;
		return 0;
	}

	size_t const tot = (size_t)(src->width * src->height);

	dest->alpha = (unsigned char *)malloc ( tot );
	if ( ! dest->alpha )
	{
		return 1;
	}

	memcpy ( dest->alpha, src->alpha, tot );

	return 0;
}

int pixy_pixmap_move_alpha (
	mtPixmap	* const	dest,
	mtPixmap	* const	src
	)
{
	if (	! src
		|| ! dest
		|| dest->width != src->width
		|| dest->height != src->height
		)
	{
		return 1;
	}

	free ( dest->alpha );
	dest->alpha = src->alpha;
	src->alpha = NULL;

	return 0;
}

