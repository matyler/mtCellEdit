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



mtPixy::Image * mtPixy::Image::convert_to_rgb ()
{
	if ( m_type != INDEXED )
	{
		return NULL;
	}

	Image * im = image_create ( RGB, m_width, m_height );
	if ( ! im )
	{
		return NULL;
	}

	im->m_palette.copy ( &m_palette );

	if ( im->copy_alpha ( this ) )
	{
		delete im;
		return NULL;
	}

	unsigned char	*	dest = im->m_canvas;
	unsigned char	* const	dlim = dest + m_width * m_height * 3;
	unsigned char	*	src = m_canvas;
	unsigned char		pix;
	Color	const	*	col = m_palette.get_color ();

	while ( dest < dlim )
	{
		pix = *src++;

		*dest++ = col [ pix ].red;
		*dest++ = col [ pix ].green;
		*dest++ = col [ pix ].blue;
	}

	return im;
}


/*
Dumb (but fast) Floyd-Steinberg dithering in RGB space, loosely based on
Dennis Lee's dithering implementation from dl3quant.c, in turn based on
dithering code from the IJG's jpeg library - WJ
*/

static void dumb_dither (
	mtPixy::Color	const * const	pal,
	int			const	ncols,
	unsigned char	const * const	mem_src,
	unsigned char		* const	mem_dest,
	int			const	width,
	int			const	height,
	int			const	dither
	)
{
	unsigned short		cols[32768], sqrtb[512], * sqr;
	short			limtb[512], * lim, fr[3] = { 0, 0, 0 };
	short			* rows = NULL, * row0 = fr, * row1 = fr;
	unsigned char		clamp[768], * dest;
	unsigned char	const	* src;
	int			k, j0, dj, dj3, r, g, b, rlen, serpent = 2;


	// Allocate working space
	rlen = ( width + 2 ) * 3;

	if ( dither )
	{
		rows = (short *)calloc ( (size_t)(rlen * 2), sizeof( *rows ) );

		if ( ! rows )
		{
			return;
		}

		serpent = 0;
	}

	// Color cache, squares table, clamp table
	memset ( cols, 0, sizeof ( cols ) );
	sqr = sqrtb + 256;

	for ( int i = -255; i < 256; i++ )
	{
		sqr[i] = (unsigned short)(i * i);
	}

	memset ( clamp, 0, 256 );
	memset ( clamp + 512, 255, 256 );

	for ( int i = 0; i < 256; i++ )
	{
		clamp[i + 256] = (unsigned char)i;
	}

	// Error limiter table, Dennis Lee's way
#define ERR_LIM 20

	lim = limtb + 256;

	for ( int i = 0; i < ERR_LIM; i++ )
	{
		lim[i] = (short)i;
		lim[-i] = (short)(-i);
	}

	for ( int i = ERR_LIM ; i < 256; i++ )
	{
		lim[i] = ERR_LIM;
		lim[-i] = -ERR_LIM;
	}

#undef ERR_LIM

	// Process image
	for ( int i = 0; i < height; i++ )
	{
		src = mem_src + i * width * 3;
		dest = mem_dest + i * width;

		if ( serpent ^= 1 )
		{
			j0 = 0; dj = 1;
		}
		else
		{
			j0 = ( width - 1 ) * 3; dj = -1;
			dest += width - 1;
		}

		if ( dither )
		{
			row0 = row1 = rows + 3;
			*( serpent ? &row1 : &row0 ) += rlen;
			memset ( row1 - 3, 0, (size_t)rlen * sizeof(short) );
			src += j0;
			row0 += j0;
			row1 += j0;
		}

		dj3 = dj * 3;

		for ( int j = 0; j < width; j++ , src += dj3 , dest += dj )
		{
			r = clamp[src[0] + ( ( row0[0] + 0x1008 ) >> 4)];
			g = clamp[src[1] + ( ( row0[1] + 0x1008 ) >> 4)];
			b = clamp[src[2] + ( ( row0[2] + 0x1008 ) >> 4)];

			k =	( ( r & 0xF8 ) << 7) +
				( ( g & 0xF8 ) << 2) +
				( b >> 3 );

			if ( ! cols[k] ) // Find nearest color in RGB
			{
				int	n = 0, l = 1000000;

/*
Searching for color nearest to first color in cell, instead of to cell
itself, looks like a bug, but works like a feature - makes FS dither less
prone to patterning. This trick I learned from Dennis Lee's code - WJ
*/
				for ( int ii = 0; ii < ncols; ii++ )
				{
					int const jj = sqr[ r - pal[ii].red ] +
						sqr[ g - pal[ii].green ] +
						sqr[ b - pal[ii].blue ];

					if ( jj >= l )
					{
						continue;
					}

					l = jj;
					n = ii;
				}

				cols[k] = (short unsigned)(n + 1);
			}

			k = cols[k] - 1;
			dest[0] = (unsigned char)k;

			if ( ! dither )
			{
				continue;
			}

			r = lim[ r - pal[k].red   ];
			g = lim[ g - pal[k].green ];
			b = lim[ b - pal[k].blue  ];

			k = r + r;
			row1[0 + dj3] = (short)(row1[0 + dj3] + r);
			row1[0 - dj3] = (short)(row1[0 - dj3] + (r += k) );
			row1[0 + 0  ] = (short)(row1[0 + 0  ] + (r += k) );
			row0[0 + dj3] = (short)(row0[0 + dj3] + r + k);

			k = g + g;
			row1[1 + dj3] = (short)(row1[1 + dj3] + g );
			row1[1 - dj3] = (short)(row1[1 - dj3] + (g += k) );
			row1[1 + 0  ] = (short)(row1[1 + 0  ] + (g += k) );
			row0[1 + dj3] = (short)(row0[1 + dj3] + g + k);

			k = b + b;
			row1[2 + dj3] = (short)(row1[2 + dj3] + b);
			row1[2 - dj3] = (short)(row1[2 - dj3] + (b += k));
			row1[2 + 0  ] = (short)(row1[2 + 0  ] + (b += k) );
			row0[2 + dj3] = (short)(row0[2 + dj3] + b + k);

			row0 += dj3;
			row1 += dj3;
		}
	}

	free ( rows );
}

static void basic_dither (
	mtPixy::Color	const * const	pal,
	int			const	ncols,
	unsigned char	const * const	mem_src,
	unsigned char		* const	mem_dest,
	int			const	w,
	int			const	h
	)
{
	unsigned char		* dest, pcol[3];
	unsigned char	const	* old_mem_image;
	int			k, closest[3][2];


	dest = mem_dest;
	old_mem_image = mem_src;

	for ( int j = 0; j < h; j++ )		// Convert RGB to indexed
	{
		for ( int i = 0; i < w; i++ )
		{
			pcol[0] = old_mem_image[ 3 * ( i + w * j ) ];
			pcol[1] = old_mem_image[ 1 + 3 * ( i + w * j ) ];
			pcol[2] = old_mem_image[ 2 + 3 * ( i + w * j ) ];

			closest[0][0] = 0; // 1st Closest palette item to pixel
			closest[1][0] = 100000000;
			closest[0][1] = 0; // 2nd Closest palette item to pixel
			closest[1][1] = 100000000;

			for ( k = 0; k < ncols; k++ )
			{
				closest[2][0] =
					abs ( pcol[0] - pal[k].red ) +
					abs ( pcol[1] - pal[k].green ) +
					abs ( pcol[2] - pal[k].blue );

				if ( closest[2][0] < closest[1][0] )
				{
					closest[0][1] = closest[0][0];
					closest[1][1] = closest[1][0];
					closest[0][0] = k;
					closest[1][0] = closest[2][0];
				}
				else
				{
					if ( closest[2][0] < closest[1][1] )
					{
						closest[0][1] = k;
						closest[1][1] = closest[2][0];
					}
				}
			}

			if ( closest[1][1] == 100000000 )
			{
				closest[1][0] = 0;
			}

			if ( closest[1][0] == 0 )
			{
				k = closest[0][0];
			}
			else
			{
				if ( closest[1][1] * 0.67 <
					( closest[1][1] - closest[1][0] ) )
				{
					k = closest[0][0];
				}
				else
				{
				  	if ( closest[0][0] > closest[0][1] )
					{
						k = closest[0][ ( i + j ) % 2 ];
					}
					else
					{
						k = closest[0][ (i+j+1) % 2 ];
					}
				}
			}

			*dest++ = (unsigned char)k;
		}
	}
}

mtPixy::Image * mtPixy::Image::convert_to_indexed (
	DitherType	const	dt
	)
{
	if ( m_type != RGB )
	{
		return NULL;
	}

	Image * im = image_create ( INDEXED, m_width, m_height );
	if ( ! im )
	{
		return NULL;
	}

	im->m_palette.copy ( &m_palette );

	if ( im->copy_alpha ( this ) )
	{
		delete im;
		return NULL;
	}

	switch ( dt )
	{
	case DITHER_NONE:
		dumb_dither( m_palette.get_color(), m_palette.get_color_total(),
			m_canvas, im->m_canvas, m_width, m_height, 0 );
		break;

	case DITHER_BASIC:
		basic_dither ( m_palette.get_color (),
			m_palette.get_color_total(), m_canvas, im->m_canvas,
			m_width, m_height );
		break;

	case DITHER_FLOYD:
		dumb_dither( m_palette.get_color(), m_palette.get_color_total(),
			m_canvas, im->m_canvas, m_width, m_height, 1 );
		break;

	default:
		delete im;
		return NULL;
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


	unsigned char		* d;
	unsigned char	const	* s;


	for ( int y = 0; y < h; y++ )
	{
		d = dest + bpp * y * w;
		s = src + bpp * y * w + bpp * (w - 1);

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

mtPixy::Image * mtPixy::Image::flip_horizontally ()
{
	Image		* i = duplicate ();


	if ( i )
	{
		flip_h_mem ( i->m_canvas, m_canvas, m_width, m_height,
			m_canvas_bpp );
		flip_h_mem ( i->m_alpha, m_alpha, m_width, m_height, 1 );
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


	unsigned char		* d;
	unsigned char	const	* s;
	size_t		const	tot = (size_t)(bpp * w);


	for ( int y = 0; y < h; y++ )
	{
		d = dest + bpp * y * w;
		s = src + bpp * (h - y - 1) * w;

		memcpy ( d, s, tot );
	}
}

mtPixy::Image * mtPixy::Image::flip_vertically ()
{
	Image		* i = duplicate ();


	if ( i )
	{
		flip_v_mem ( i->m_canvas, m_canvas, m_width, m_height,
			m_canvas_bpp );
		flip_v_mem ( i->m_alpha, m_alpha, m_width, m_height, 1 );
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


	unsigned char		* d;
	unsigned char	const	* s = src;


	for ( int y = 0; y < h; y++ )
	{
		d = dest + bpp * (h - y - 1);

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

mtPixy::Image * mtPixy::Image::rotate_clockwise ()
{
	Image * im = image_create ( m_type, m_height, m_width );
	if ( ! im )
	{
		return NULL;
	}

	if ( m_alpha && im->create_alpha () )
	{
		delete im;
		return NULL;
	}

	im->m_palette.copy ( &m_palette );

	rotate_c_mem ( im->m_canvas, m_canvas, m_width, m_height, m_canvas_bpp);
	rotate_c_mem ( im->m_alpha, m_alpha, m_width, m_height, 1 );

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
	if ( ! dest || ! src )
	{
		return;
	}


	unsigned char		* d;
	unsigned char	const	* s = src;


	for ( int y = 0; y < h; y++ )
	{
		d = dest + bpp * (y + (w - 1) * h);

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

mtPixy::Image * mtPixy::Image::rotate_anticlockwise ()
{
	Image * im = image_create ( m_type, m_height, m_width );
	if ( ! im )
	{
		return NULL;
	}

	if ( m_alpha && im->create_alpha () )
	{
		delete im;
		return NULL;
	}

	im->m_palette.copy ( &m_palette );

	rotate_a_mem ( im->m_canvas, m_canvas, m_width, m_height, m_canvas_bpp);
	rotate_a_mem ( im->m_alpha, m_alpha, m_width, m_height, 1 );

	return im;
}

