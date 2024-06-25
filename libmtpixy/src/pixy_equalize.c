/*
	Copyright (C) 2023-2024 Mark Tyler

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



void pixy_palette_effect_equalize_info (
	mtPalette	const * const	palette,
	unsigned char			rgb_min_max[6]
	)
{
	rgb_min_max[0] = rgb_min_max[2] = rgb_min_max[4] = 255;
	rgb_min_max[1] = rgb_min_max[3] = rgb_min_max[5] = 0;

	for ( int i = 0; i < palette->size; i++ )
	{
		rgb_min_max[0] = MIN( rgb_min_max[0], palette->color[i].red );
		rgb_min_max[2] = MIN( rgb_min_max[2], palette->color[i].green );
		rgb_min_max[4] = MIN( rgb_min_max[4], palette->color[i].blue );

		rgb_min_max[1] = MAX( rgb_min_max[1], palette->color[i].red );
		rgb_min_max[3] = MAX( rgb_min_max[3], palette->color[i].green );
		rgb_min_max[5] = MAX( rgb_min_max[5], palette->color[i].blue );
	}
}

static void prep_lookup (
	unsigned char	const	smin,		// Source min/max
	unsigned char	const	smax,
	unsigned char	const	dmin,		// Destination min/max
	unsigned char	const	dmax,
	unsigned char		rgblt[256]	// Translation table src -> dest
	)
{
	memset ( rgblt, 0, 256 );

	double const srange = (smax == smin) ? (1) : (smax - smin);
	double const drange = (dmax == dmin) ? (1) : (dmax - dmin);

	for ( int i = smin; i <= smax; i++ )
	{
		double const sp = (i - smin) / srange;
		double const col = 0.5 + dmin + sp * drange;

		rgblt[i] = (unsigned char)col;
	}
}

void pixy_palette_effect_equalize (
	mtPalette	* const	palette,
	unsigned char	const	rgb_min_max[6]
	)
{
	unsigned char src[6], rlt[256], glt[256], blt[256];

	pixy_palette_effect_equalize_info ( palette, src );

	// Prepare the red, green, blue lookup tables
	prep_lookup ( src[0], src[1], rgb_min_max[0], rgb_min_max[1], rlt );
	prep_lookup ( src[2], src[3], rgb_min_max[2], rgb_min_max[3], glt );
	prep_lookup ( src[4], src[5], rgb_min_max[4], rgb_min_max[5], blt );

	for ( int i = 0; i < palette->size; i++ )
	{
		palette->color[i].red	= rlt[ palette->color[i].red ];
		palette->color[i].green	= glt[ palette->color[i].green ];
		palette->color[i].blue	= blt[ palette->color[i].blue ];
	}
}

int pixy_pixmap_equalize_image_info (
	mtPixmap	const * const	pixmap,
	unsigned char			rgb_min_max[6]
	)
{
	if ( ! pixmap || ! pixmap->canvas )
	{
		return 1;
	}

	if ( pixmap->bpp < 3 )
	{
		pixy_palette_effect_equalize_info ( &pixmap->palette,
			rgb_min_max );
		return 0;
	}

	unsigned char const
		* dest = pixmap->canvas,
		* const dest_end = dest + pixmap->width * pixmap->height * 3;

	rgb_min_max[0] = rgb_min_max[2] = rgb_min_max[4] = 255;
	rgb_min_max[1] = rgb_min_max[3] = rgb_min_max[5] = 0;

	for ( ; dest < dest_end; dest += 3 )
	{
		rgb_min_max[0] = MIN( rgb_min_max[0], dest[0] );
		rgb_min_max[2] = MIN( rgb_min_max[2], dest[1] );
		rgb_min_max[4] = MIN( rgb_min_max[4], dest[2] );

		rgb_min_max[1] = MAX( rgb_min_max[1], dest[0] );
		rgb_min_max[3] = MAX( rgb_min_max[3], dest[1] );
		rgb_min_max[5] = MAX( rgb_min_max[5], dest[2] );
	}

	return 0;
}

int pixy_pixmap_equalize_palette_info (
	mtPixmap	const * const	pixmap,
	unsigned char			rgb_min_max[6]
	)
{
	if ( ! pixmap )
	{
		return 1;
	}

	pixy_palette_effect_equalize_info ( &pixmap->palette, rgb_min_max );

	return 0;
}

mtPixmap * pixy_pixmap_equalize_image (
	mtPixmap 	const * const	pixmap,
	unsigned char		const	rgb_min_max[6]
	)
{
	if ( ! pixmap || ! pixmap->canvas )
	{
		return NULL;
	}

	mtPixmap * const im = pixy_pixmap_duplicate ( pixmap );

	if ( ! im )
	{
		return NULL;
	}

	if ( im->bpp < 3 )
	{
		pixy_palette_effect_equalize ( &im->palette, rgb_min_max );
		return im;
	}

	unsigned char src[6], rlt[256], glt[256], blt[256];

	if ( pixy_pixmap_equalize_image_info ( im, src ) )
	{
		return im;
	}

	// Prepare the red, green, blue lookup tables
	prep_lookup ( src[0], src[1], rgb_min_max[0], rgb_min_max[1], rlt );
	prep_lookup ( src[2], src[3], rgb_min_max[2], rgb_min_max[3], glt );
	prep_lookup ( src[4], src[5], rgb_min_max[4], rgb_min_max[5], blt );

	unsigned char
		* dest = im->canvas,
		* const dest_end = dest + pixmap->width * pixmap->height * 3;

	for ( ; dest < dest_end; dest += 3 )
	{
		dest[0] = rlt[ dest[0] ];
		dest[1] = glt[ dest[1] ];
		dest[2] = blt[ dest[2] ];
	}

	return im;
}

mtPixmap * pixy_pixmap_equalize_palette (
	mtPixmap	const * const	pixmap,
	unsigned char		const	rgb_min_max[6]
	)
{
	mtPixmap * const im = pixy_pixmap_duplicate ( pixmap );

	if ( ! im )
	{
		return NULL;
	}

	pixy_palette_effect_equalize ( &im->palette, rgb_min_max );

	return im;
}

