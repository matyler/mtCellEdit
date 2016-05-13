/*
	Copyright (C) 2004-2016 Mark Tyler

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

#include <cairo.h>
#include "private.h"



#define MTKIT_MAX_WIDTH		30000
#define MTKIT_MAX_HEIGHT	30000



struct mtImage
{
	int		width;
	int		height;
	unsigned char	* rgb;		// 3bpp, R, G, B memory
	unsigned char	* alpha;	// 1bpp, alpha memory
};



static mtImage * mtkit_image_new (
	int	const	width,
	int	const	height,
	int	const	chan_mask
	)
{
	mtImage		* image;


	if (	width < 1			||
		width > MTKIT_MAX_WIDTH		||
		height < 1			||
		height > MTKIT_MAX_HEIGHT
		)
	{
		return NULL;
	}

	image = calloc ( sizeof ( mtImage ), 1 );
	if ( ! image )
	{
		return NULL;
	}

	image->width = width;
	image->height = height;

	if ( chan_mask & 1 )
	{
		image->rgb = calloc ( (size_t)(3 * width), (size_t)height );
		if ( ! image->rgb )
		{
			goto error;
		}
	}

	if ( chan_mask & 2 )
	{
		image->alpha = calloc ( (size_t)width, (size_t)height );
		if ( ! image->alpha )
		{
			goto error;
		}
	}

	return image;

error:
	mtkit_image_destroy ( image );

	return NULL;
}

mtImage * mtkit_image_new_rgb (
	int	const	width,
	int	const	height
	)
{
	return mtkit_image_new ( width, height, 1 );
}

int mtkit_image_destroy (
	mtImage	* const image
	)
{
	if ( ! image )
	{
		return 1;
	}

	free ( image->rgb );
	free ( image->alpha );
	free ( image );

	return 0;
}

int mtkit_image_get_width (
	mtImage	* const image
	)
{
	if ( image )
	{
		return image->width;
	}

	return 0;
}

int mtkit_image_get_height (
	mtImage	* const image
	)
{
	if ( image )
	{
		return image->height;
	}

	return 0;
}

unsigned char * mtkit_image_get_rgb (
	mtImage	* const	image
	)
{
	if ( image )
	{
		return image->rgb;
	}

	return NULL;
}

unsigned char * mtkit_image_get_alpha (
	mtImage	* const image
	)
{
	if ( image )
	{
		return image->alpha;
	}

	return NULL;
}

mtImage * mtkit_image_new_data (
	int		const	width,
	int		const	height,
	unsigned char	* const	rgb,
	unsigned char	* const	alpha
	)
{
	mtImage		* image;


	image = mtkit_image_new ( width, height, 0 );
	if ( ! image )
	{
		return NULL;
	}

	image->rgb = rgb;
	image->alpha = alpha;

	return image;
}

mtImage * mtkit_image_from_cairo (
	cairo_surface_t	* const	surface
	)
{
	mtImage		* image = NULL;


#ifdef CAIRO_HAS_PNG_FUNCTIONS

	cairo_format_t	format;
	int		w, h, y, s_stride, d_stride;
	unsigned char	* src, * dest, * s, * d, * s_end;


	src = cairo_image_surface_get_data ( surface );
	if ( ! src )
	{
		return NULL;
	}

	format = cairo_image_surface_get_format ( surface );
	if ( format != CAIRO_FORMAT_ARGB32 && format != CAIRO_FORMAT_RGB24 )
	{
		return NULL;
	}

	w = cairo_image_surface_get_width ( surface );
	h = cairo_image_surface_get_height ( surface );
	image = mtkit_image_new_rgb ( w, h );

	if ( ! image )
	{
		return NULL;
	}

	dest = mtkit_image_get_rgb ( image );
	if ( ! dest )
	{
		mtkit_image_destroy ( image );

		return NULL;
	}


	// Copy data from Cairo to mtImage
	s_stride = cairo_image_surface_get_stride ( surface );

	d_stride = w * 3;


	uint32_t	one = 1,
			le = ( (uint8_t *) &one )[0];
			// 1 = Little endian 0 = Big endian


	if ( le ) for ( y = 0; y < h; y++ )	// Little endian
	{
		s = src + y * s_stride;
		d = dest + y * d_stride;
		s_end = s + w * 4;

		for ( ; s < s_end; s += 4 )
		{
			*d++ = s[2];		// Red
			*d++ = s[1];		// Green
			*d++ = s[0];		// Blue
		}
	}
	else for ( y = 0; y < h; y++ )		// Big endian
	{
		s = src + y * s_stride;
		d = dest + y * d_stride;
		s_end = s + w * 4;

		for ( ; s < s_end; s += 4 )
		{
			*d++ = s[1];		// Red
			*d++ = s[2];		// Green
			*d++ = s[3];		// Blue
		}
	}

#endif

	return image;
}

mtImage * mtkit_image_load_png (
	char	const * const	filename
	)
{
	mtImage		* image = NULL;

#ifdef CAIRO_HAS_PNG_FUNCTIONS

	cairo_surface_t * sf;


	sf = cairo_image_surface_create_from_png ( filename );
	if ( ! sf )
	{
		return NULL;
	}

	image = mtkit_image_from_cairo ( sf );

	cairo_surface_destroy ( sf );

#endif

	return image;
}

