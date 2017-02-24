/*
	Copyright (C) 2006-2009 Dmitry Groshev
	Copyright (C) 2009-2013,2016 Mark Tyler

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
These algorithms were first created for mtPaint by Dmitry Groshev. I made a few
very minor tweaks to the interface to get them into an mtPixy context but
otherwise the work and all credit should go to Dmitry.
M.Tyler.
*/



/*
Pairwise Nearest Neighbor quantization algorithm - minimizes mean square
error measure; time used is proportional to number of bins squared - WJ
*/

typedef struct
{
	double		rc,
			gc,
			bc,
			err;
	int		cnt;
	unsigned short	nn,
			fw,
			bk,
			tm,
			mtm;
} pnnbin;



static void find_nn (
	pnnbin		* const	bins,
	int		const	idx
	)
{
	pnnbin		* bin1, * bin2;
	int		i, nn = 0;
	double		n1, n2, dr, dg, db, nerr, err = 1e100;


	bin1 = bins + idx;
	n1 = bin1->cnt;

	for ( i = bin1->fw; i; i = bin2->fw )
	{
		bin2 = bins + i;
		n2 = bin2->cnt;
		dr = bin2->rc - bin1->rc;
		dg = bin2->gc - bin1->gc;
		db = bin2->bc - bin1->bc;
		nerr = ( ( ( dr * dr + dg * dg + db * db ) / ( n1 + n2 ) ) *
			n1 * n2 );

		if ( nerr >= err )
		{
			continue;
		}

		err = nerr;
		nn = i;
	}

	bin1->err = err;
	bin1->nn = (unsigned short)nn;
}

static int pnnquan (
	unsigned char		* inbuf,
	int		const	width,
	int		const	height,
	int		const	quant_to,
	mtPixy::Palette	* const	pal
	)
{
	unsigned short	heap[ 32769 ];
	pnnbin		* bins, * tb, * nb;
	double		d, err, n1, n2;
	int		i, j, k, l, l2, h, b1, maxbins, extbins;
	mtPixy::Color	* const palcol = pal->get_color ();


	heap[0] = 0;			// Empty

	bins = (pnnbin *)calloc ( 32768, sizeof ( pnnbin ) );

	if ( ! bins )
	{
		return 1;
	}

	/* Build histogram */
	k = width * height;
	for ( i = 0; i < k; i++ , inbuf += 3 )
	{
// !!! Can throw gamma correction in here, but what to do about perceptual
// !!! nonuniformity then?
		j = ( ( inbuf[0] & 0xF8 ) << 7 ) +
			( ( inbuf[1] & 0xF8 ) << 2) +
			( inbuf[2] >> 3 );

		tb = bins + j;
		tb->rc += inbuf[0];
		tb->gc += inbuf[1];
		tb->bc += inbuf[2];
		tb->cnt ++;
	}

	/* Cluster nonempty bins at one end of array */
	tb = bins;
	for ( i = 0; i < 32768; i++ )
	{
		if ( ! bins[i].cnt )
		{
			continue;
		}

		*tb = bins[i];
		d = 1.0 / (double)tb->cnt;
		tb->rc *= d; tb->gc *= d; tb->bc *= d;
		tb ++;
	}

	maxbins = (int)(tb - bins);

	for ( i = 0; i < maxbins - 1; i++ )
	{
		bins[i].fw	= (short unsigned int)(i + 1);
		bins[i + 1].bk	= (short unsigned int)(i);
	}

// !!! Already zeroed out by calloc ()
//	bins[0].bk = bins[i].fw = 0;

	/* Initialize nearest neighbors and build heap of them */
	for ( i = 0; i < maxbins; i++ )
	{
		find_nn ( bins, i );
		/* Push slot on heap */
		err = bins[i].err;

		for ( l = ++heap[0]; l > 1; l = l2 )
		{
			l2 = l >> 1;

			if ( bins[h = heap[l2]].err <= err )
			{
				break;
			}

			heap[l] = (short unsigned int)(h);
		}

		heap[l] = (short unsigned int)(i);
	}

	/* Merge bins which increase error the least */
	extbins = maxbins - quant_to;
	for ( i = 0; i < extbins; )
	{
		/* Use heap to find which bins to merge */
		while ( 1 )
		{
			/* One with least error */
			tb = bins + ( b1 = heap[1] );

			/* Is stored error up to date? */

			if (	( tb->tm >= tb->mtm ) &&
				( bins[tb->nn].mtm <= tb->tm )
				)
			{
				break;
			}

			if ( tb->mtm == 0xFFFF )
			{
				/* Deleted node */
				b1 = heap[1] = heap[ heap[0] -- ];
			}
			else
			{
				/* Too old error value */
				find_nn ( bins, b1 );
				tb->tm = (short unsigned int)(i);
			}

			/* Push slot down */
			err = bins[b1].err;

			for ( l = 1; (l2 = l + l) <= heap[0]; l = l2 )
			{
				if (	l2 < heap[0] &&
					bins[heap[l2]].err >
						bins[ heap[ l2 + 1 ] ].err
					)
				{
					l2++;
				}

				if ( err <= bins[ h = heap[ l2 ] ].err )
				{
					break;
				}

				heap[l] = (short unsigned int)(h);
			}

			heap[l] = (short unsigned int)(b1);
		}

		/* Do a merge */
		nb = bins + tb->nn;
		n1 = tb->cnt;
		n2 = nb->cnt;
		d = 1.0 / ( n1 + n2 );
		tb->rc = d * rint ( n1 * tb->rc + n2 * nb->rc );
		tb->gc = d * rint ( n1 * tb->gc + n2 * nb->gc );
		tb->bc = d * rint ( n1 * tb->bc + n2 * nb->bc );
		tb->cnt += nb->cnt;
		tb->mtm = (short unsigned int)(++i);

		/* Unchain deleted bin */
		bins[ nb->bk ].fw = nb->fw;
		bins[ nb->fw ].bk = nb->bk;
		nb->mtm = 0xFFFF;
	}

	/* Fill palette */
	i = j = 0;
	while ( 1 )
	{
		palcol[j].red	= (unsigned char)lrint ( bins[i].rc );
		palcol[j].green	= (unsigned char)lrint ( bins[i].gc );
		palcol[j].blue	= (unsigned char)lrint ( bins[i].bc );

		j++;

		if ( ! (i = bins[i].fw) )
		{
			break;
		}
	}

	pal->set_color_total ( MAX ( j, mtPixy::Palette::COLOR_TOTAL_MIN ) );

	free ( bins );

	return 0;
}


int mtPixy::Image::quantize_pnn (
	int		const	coltot,
	Palette		* const	pal
	)
{
	if (	! m_canvas				||
		m_type != RGB				||
		coltot < Palette::COLOR_TOTAL_MIN	||
		coltot > Palette::COLOR_TOTAL_MAX
		)
	{
		return 1;
	}

	if ( pal )
	{
		if ( pnnquan ( m_canvas, m_width, m_height, coltot, pal ) )
		{
			return 1;
		}

		pal->set_color_total ( coltot );
	}
	else
	{
		if ( pnnquan( m_canvas, m_width, m_height, coltot, &m_palette ))
		{
			return 1;
		}
	}

	return 0;
}

