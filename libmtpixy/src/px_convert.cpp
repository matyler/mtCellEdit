/*
	Copyright (C) 2016-2021 Mark Tyler

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



/*
Dumb (but fast) Floyd-Steinberg dithering in RGB space, loosely based on
Dennis Lee's dithering implementation from dl3quant.c, in turn based on
dithering code from the IJG's jpeg library - WJ
*/

static void dumb_dither (
	mtColor		const * const	pal,
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
	unsigned char		clamp[768];
	int			k, j0, dj, r, g, b, rlen, serpent = 2;


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
		unsigned char const * src = mem_src + i * width * 3;
		unsigned char * dest = mem_dest + i * width;

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

		int const dj3 = dj * 3;

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


struct DithCol
{
	DithCol ( int ia, int ib, int r, int g, int b )
		:
		index_a	( ia ),
		index_b	( ib ),
		red	( r ),
		green	( g ),
		blue	( b )
	{}

	int	const	index_a;
	int	const	index_b;

	int	const	red;
	int	const	green;
	int	const	blue;
};



class DithTable
{
public:
	DithTable ( mtColor const * pal, int cols )
		:
		m_pal	( pal ),
		m_cols	( cols )
	{}

	void init ();

	void match ( int r, int g, int b, int & idx_a, int & idx_b ) const;

private:
	mtColor		const * const	m_pal;
	int			const	m_cols;
	std::vector< DithCol >		m_table;
};



void DithTable::init ()
{
	// 255/3 = 85
	int const limit = m_cols > 32 ? 86 : 255;

	for ( int i = 0; i < m_cols; i++ )
	{
		for ( int j = i; j < m_cols; j++ )
		{
			mtColor	const & ca = m_pal[ i ];
			mtColor	const & cb = m_pal[ j ];

			if (	abs( ca.red - cb.red )		> limit
				|| abs( ca.green - cb.green )	> limit
				|| abs( ca.blue - cb.blue )	> limit
				)
			{
				continue;
			}

			int const r = ((int)ca.red + (int)cb.red) / 2;
			int const g = ((int)ca.green + (int)cb.green) / 2;
			int const b = ((int)ca.blue + (int)cb.blue) / 2;

			m_table.emplace_back ( i, j, r, g, b );
		}
	}
}

void DithTable::match (
	int	const	r,
	int	const	g,
	int	const	b,
	int		& idx_a,
	int		& idx_b
	) const
{
	size_t best_i = 0;
	int best_d = 1<<30;

	for ( size_t i = 0; i < m_table.size(); i++ )
	{
		DithCol		const & col = m_table[ i ];
		mtColor		const & ca = m_pal[ col.index_a ];
		mtColor		const & cb = m_pal[ col.index_b ];

		int const d =
			3*abs( r - col.red )	+
			3*abs( g - col.green )	+
			3*abs( b - col.blue )	+
/*
NOTE: we also add the delta between each of the indices to avoid the grey
problem whereby two greys far away from the image colour average to exactly
the required value crowding out a near miss, e.g.

image pixel	= (100,100,100)
palette a	= (50,50,50)
palette b	= (90,90,90)
palette c	= (150,150,150)

b&b is the better choice than a&c, but without checking the total deltas of
the indices we would otherwise choose the extremes instead as they average 100
for a perfect match.
MT 2020-8-9
*/
			abs( r - ca.red )	+
			abs( g - ca.green )	+
			abs( b - ca.blue )	+

			abs( r - cb.red )	+
			abs( g - cb.green )	+
			abs( b - cb.blue )
			;

		if ( d < best_d )
		{
			best_i = i;
			best_d = d;
		}
	}

	idx_a = m_table[ best_i ].index_a;
	idx_b = m_table[ best_i ].index_b;
}



static void average_dither (
	mtColor		const * const	pal,
	int			const	ncols,
	unsigned char	const * const	mem_src,
	unsigned char		* const	mem_dest,
	int			const	w,
	int			const	h
	)
{
	DithTable table ( pal, ncols );

	try
	{
		table.init ();
	}
	catch (...)
	{
		return;
	}

	unsigned char		* dest = mem_dest;
	unsigned char	const	* src = mem_src;

	for ( int j = 0; j < h; j++ )		// Convert RGB to indexed
	{
		for ( int i = 0; i < w; i++ )
		{
			int const r = *src++;
			int const g = *src++;
			int const b = *src++;
			int const dither = ( i + j ) & 1;

			int kt[2];
			table.match ( r, g, b, kt[0], kt[1] );

			*dest++ = (unsigned char)kt[ dither ];
		}
	}
}

static void basic_dither (
	mtColor		const * const	pal,
	int			const	ncols,
	unsigned char	const * const	mem_src,
	unsigned char		* const	mem_dest,
	int			const	w,
	int			const	h
	)
{
	unsigned char		* dest = mem_dest;
	unsigned char	const	* src = mem_src;

	for ( int j = 0; j < h; j++ )		// Convert RGB to indexed
	{
		for ( int i = 0; i < w; i++ )
		{
			int const r = *src++;
			int const g = *src++;
			int const b = *src++;

			int best_i = 0; // 1st Closest palette item to pixel
			int best_d = 100000000;
			int last_i = 0; // 2nd Closest palette item to pixel
			int last_d = 100000000;

			int k;
			for ( k = 0; k < ncols; k++ )
			{
				int test_d =
					abs (r - (int)pal[k].red ) +
					abs (g - (int)pal[k].green )+
					abs (b - (int)pal[k].blue );

				if ( test_d < best_d )
				{
					last_i = best_i;
					last_d = best_d;
					best_i = k;
					best_d = test_d;

					if ( test_d == 0 )
					{
						break;
					}
				}
				else
				{
					if ( test_d < last_d )
					{
						last_i = k;
						last_d = test_d;
					}
				}
			}

			if ( last_d == 100000000 )
			{
				best_d = 0;
			}

			if ( best_d == 0 )
			{
				k = best_i;
			}
			else
			{
				if ( best_d * 0.9 < (last_d - best_d) )
				{
					k = best_i;
				}
				else
				{
					int const kt[2] = { best_i, last_i };
					int const dither = ( i + j ) & 1;

					if ( best_i > last_i )
					{
						k = kt[ dither ];
					}
					else
					{
						k = kt[ 1 - dither ];
					}
				}
			}

			*dest++ = (unsigned char)k;
		}
	}
}

mtPixmap * pixy_pixmap_convert_to_indexed (
	mtPixmap const	* const	pixmap,
	int		const	dt
	)
{
	if ( pixmap->bpp != PIXY_PIXMAP_BPP_RGB )
	{
		return NULL;
	}

	mtPixmap * im = pixy_pixmap_new_indexed( pixmap->width, pixmap->height);
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

	switch ( dt )
	{
	case PIXY_DITHER_NONE:
		dumb_dither ( &pixmap->palette.color[0], pixmap->palette.size,
			pixmap->canvas, im->canvas, pixmap->width,
			pixmap->height, 0 );
		break;

	case PIXY_DITHER_BASIC:
		basic_dither ( &pixmap->palette.color[0], pixmap->palette.size,
			pixmap->canvas, im->canvas, pixmap->width,
			pixmap->height );
		break;

	case PIXY_DITHER_AVERAGE:
		average_dither ( &pixmap->palette.color[0],
			pixmap->palette.size, pixmap->canvas,
			im->canvas, pixmap->width, pixmap->height );
		break;

	case PIXY_DITHER_FLOYD:
		dumb_dither ( &pixmap->palette.color[0],
			pixmap->palette.size, pixmap->canvas, im->canvas,
			pixmap->width, pixmap->height, 1 );
		break;

	default:
		pixy_pixmap_destroy ( &im );
		return NULL;
	}

	return im;
}

