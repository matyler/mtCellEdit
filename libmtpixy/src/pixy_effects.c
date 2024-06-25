/*
	Copyright (C) 2016-2023 Mark Tyler

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



mtPixmap * pixy_effect_transform_color (
	mtPixmap const	* const	pixmap,
	int		const	ga,
	int		const	br,
	int		const	co,
	int		const	sa,
	int		const	hu,
	int		const	po
	)
{
	mtPixmap * const im = pixy_pixmap_duplicate ( pixmap );

	if ( ! im )
	{
		return NULL;
	}

	if ( PIXY_PIXMAP_BPP_RGB == pixmap->bpp )
	{
		pixy_transform_color(im->canvas, pixmap->width * pixmap->height,
			ga, br, co, sa, hu, po );
	}

	pixy_palette_transform_color ( &im->palette, ga, br, co, sa, hu, po );

	return im;
}

mtPixmap * pixy_pixmap_effect_invert (
	mtPixmap const * const	pixmap
	)
{
	mtPixmap * const im = pixy_pixmap_duplicate ( pixmap );

	if ( ! im )
	{
		return NULL;
	}

	if ( PIXY_PIXMAP_BPP_RGB == pixmap->bpp )
	{
		if ( im->canvas )
		{
			unsigned char * dest = im->canvas;
			unsigned char * dlim = im->canvas +
				pixmap->width * pixmap->height * 3;
			unsigned char const * src = pixmap->canvas;

			while ( dest < dlim )
			{
				*dest++ = (unsigned char)(255 - *src++);
				*dest++ = (unsigned char)(255 - *src++);
				*dest++ = (unsigned char)(255 - *src++);
			}
		}
	}
	else if ( PIXY_PIXMAP_BPP_INDEXED == pixmap->bpp )
	{
		pixy_palette_effect_invert ( &im->palette );
	}

	return im;
}

mtPixmap * pixy_pixmap_effect_crt (
	mtPixmap const * const	pixmap,
	int		const	scale
	)
{
	if ( ! pixmap )
	{
		return NULL;
	}

	if (	scale < PIXY_EFFECT_CRT_SCALE_MIN ||
		scale > PIXY_EFFECT_CRT_SCALE_MAX
		)
	{
		fprintf ( stderr, "Image::effect_crt scale out of bounds\n" );
		return NULL;
	}

	mtPixmap	* im = NULL;
	int	const	nw = pixmap->width * scale;
	int	const	nh = pixmap->height * scale;

	if ( PIXY_PIXMAP_BPP_RGB == pixmap->bpp )
	{
		im = pixy_pixmap_scale ( pixmap, nw, pixmap->height,
			PIXY_SCALE_BLOCKY );
	}
	else if ( PIXY_PIXMAP_BPP_INDEXED == pixmap->bpp )
	{
		mtPixmap * tmp = pixy_pixmap_convert_to_rgb ( pixmap );
		if ( ! tmp )
		{
			fprintf(stderr,"Image::effect_crt unable to convert to "
				"RGB\n" );
			return NULL;
		}

		im = pixy_pixmap_scale ( tmp, nw, pixmap->height,
			PIXY_SCALE_BLOCKY );
		pixy_pixmap_destroy ( &tmp );
	}

	if ( ! im )
	{
		fprintf(stderr, "Image::effect_crt unable to scale image(1)\n");
		return NULL;
	}
	else
	{
		mtPixmap * tmp = pixy_pixmap_scale ( im, nw, nh,
			PIXY_SCALE_BLOCKY );
		pixy_pixmap_destroy ( &im );
		im = tmp;
	}

	if ( ! im )
	{
		fprintf(stderr,"Image::effect_crt unable to scale image (2)\n");
		return NULL;
	}

	int const w = im->width;
	int const h = im->height;
	int const rowstride = w*3;

	for ( int i = 1; i < scale; i++ )
	{
		unsigned char	pix[256];
		double	const	mult = 0.5 +
//			(scale - i)/((double)scale) / 2.0;
			(
				sqrt( scale * scale - i*i ) / scale +
				(scale - i)/((double)scale)
			) / 4.0;
			// NOTE: 50% linear, 50% circle

		for ( int p = 0; p < 256; p++ )
		{
			pix[ p ] = (unsigned char)(0.5 + p * mult);
		}

		for ( int y = i; y < h; y += scale )
		{
			unsigned char * dest = im->canvas;

			dest += rowstride * y;

			unsigned char * const dest_end = dest + rowstride;

			for ( ; dest < dest_end; dest+=3 )
			{
				dest[0] = pix[ dest[0] ];
				dest[1] = pix[ dest[1] ];
				dest[2] = pix[ dest[2] ];
			}
		}
	}

	return im;
}

mtPixmap * pixy_pixmap_effect_rgb_action (
	mtPixmap const	* const	pixmap,
	int		const	effect,
	int		const	it
	)
{
	if ( ! pixmap )
	{
		return NULL;
	}

	if ( PIXY_PIXMAP_BPP_RGB != pixmap->bpp || ! pixmap->canvas )
	{
		return NULL;
	}

	mtPixmap * const im = pixy_pixmap_duplicate ( pixmap );

	if ( ! im )
	{
		return NULL;
	}


	double	const	blur = ((double)it) / 200;
	unsigned char *	dest = im->canvas;
	unsigned char const *	src = pixmap->canvas;
	int		x, y, k=0, dxp1, dxm1;
	int	const	ll = pixmap->width * pixmap->bpp;


	for ( y = 0; y < pixmap->height; y++ )
	{
		int const dyp1 = y < pixmap->height - 1 ? ll : -ll;
		int const dym1 = y ? -ll : ll;

		for ( x = 0; x < ll; x++, src++ , dest++ )
		{
			dxp1 = x < ll - pixmap->bpp ?
				pixmap->bpp : -pixmap->bpp;
			dxm1 = x >= pixmap->bpp ?
				-pixmap->bpp : pixmap->bpp;

			switch ( effect )
			{
			case PIXY_EFFECT_EDGE_DETECT:
				k = src[0];
				k = abs(k - src[dym1]) + abs(k - src[dyp1]) +
					abs(k - src[dxm1]) + abs(k - src[dxp1]);
				k += k >> 1;
				break;

			case PIXY_EFFECT_SHARPEN:
				k = src[dym1] + src[dyp1] +
					src[dxm1] + src[dxp1] - 4 * src[0];
				k = (int)(src[0] - blur * k);
				break;

			case PIXY_EFFECT_SOFTEN:
				k = src[dym1] + src[dyp1] +
					src[dxm1] + src[dxp1] - 4 * src[0];
				k = src[0] + (5 * k) / (125 - it);
				break;

			case PIXY_EFFECT_EMBOSS:
				k = src[dym1] + src[dxm1] +
					src[dxm1 + dym1] + src[dxp1 + dym1];
				k = k / 4 - src[0] + 127;
				break;
			}

			if ( k < 0 )
			{
				dest[0] = 0;
			}
			else if ( k > 255 )
			{
				dest[0] = 255;
			}
			else
			{
				dest[0] = (unsigned char)k;
			}
		}
	}

	return im;
}

mtPixmap * pixy_pixmap_effect_edge_detect (
	mtPixmap const	* const	pixmap
	)
{
	return pixy_pixmap_effect_rgb_action ( pixmap, PIXY_EFFECT_EDGE_DETECT,
		0 );
}

mtPixmap * pixy_pixmap_effect_sharpen (
	mtPixmap const	* const	pixmap,
	int		const	n
	)
{
	return pixy_pixmap_effect_rgb_action ( pixmap, PIXY_EFFECT_SHARPEN, n );
}

mtPixmap * pixy_pixmap_effect_soften (
	mtPixmap const	* const	pixmap,
	int		const	n
	)
{
	return pixy_pixmap_effect_rgb_action ( pixmap, PIXY_EFFECT_SOFTEN, n );
}

mtPixmap * pixy_pixmap_effect_emboss (
	mtPixmap const	* const	pixmap
	)
{
	return pixy_pixmap_effect_rgb_action ( pixmap, PIXY_EFFECT_EMBOSS, 0 );
}

mtPixmap * pixy_pixmap_effect_normalize (
	mtPixmap const	* const	pixmap
	)
{
	if ( ! pixmap )
	{
		return NULL;
	}

	if ( PIXY_PIXMAP_BPP_RGB != pixmap->bpp || ! pixmap->canvas )
	{
		return NULL;
	}

	mtPixmap * const im = pixy_pixmap_duplicate ( pixmap );

	if ( ! im )
	{
		return NULL;
	}

	unsigned char	const	* lim = im->canvas + pixmap->width *
					pixmap->height * 3;
	unsigned char		min = 255, max = 0;

	// Pass 1 - get max/min values for R, G, & B
	for ( unsigned char * src = im->canvas; src < lim; src++ )
	{
		min = MIN ( src[0], min );
		max = MAX ( src[0], max );
	}

	// Pass 2 - stretch out R, G, & B values if required
	if (	(min > 0 || max < 255) &&
		min != max
		)
	{
		double	const	fac = 255.0 / (double)(max - min);

		for (	unsigned char * src = im->canvas;
			src < lim;
			src++
			)
		{
			double const val = (double)(src[0] - min) * fac;

			src[0] = (unsigned char)(val + 0.5);
		}
	}

	return im;
}

// Ode to 1994 and my Acorn A3000, mtPaint from 2004, and Dmitry's optimizations
mtPixmap * pixy_pixmap_effect_bacteria (
	mtPixmap const	* const	pixmap,
	int		const	n
	)
{
	if ( ! pixmap )
	{
		return NULL;
	}

	if ( PIXY_PIXMAP_BPP_INDEXED != pixmap->bpp || ! pixmap->canvas )
	{
		return NULL;
	}

	mtPixmap * const im = pixy_pixmap_duplicate ( pixmap );

	if ( ! im )
	{
		return NULL;
	}

	int	const	w = pixmap->width - 2;
	int	const	h = pixmap->height - 2;
	int	const	tot = w * h;
	int		i, j, x, y, pixy, np;
	int	const	coltot = im->palette.size;
	unsigned char * const dest = im->canvas;
	unsigned char *	img;

	for ( i = 0; i < n; i++ )
	{
		for ( j = 0; j < tot; j++ )
		{
			x = rand() % w;
			y = rand() % h;

			img = dest + x + pixmap->width * y;
			pixy = img[0] + img[1] + img[2];

			img += pixmap->width;
			pixy += img[0] + img[1] + img[2];

			img += pixmap->width;
			pixy += img[0] + img[1] + img[2];

			np = ((pixy + pixy + 9) / 18 + 1) % coltot;
			img[ 1 - pixmap->width] = (unsigned char)np;
		}
	}

	return im;
}

