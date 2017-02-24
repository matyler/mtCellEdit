/*
	Copyright (C) 2016 Mark Tyler

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



static void scale_indexed_nn (
	unsigned char		* const	dest,
	int			const	nw,
	int			const	nh,
	unsigned char	const * const	src,
	int			const	ow,
	int			const	oh
	)
{
	int		i, j, oi, oj;


	for ( j = 0; j < nh; j++ )
	{
		oj = (int)( ( (float)oh ) * ( (float)j ) / ( (float)nh ) );

		for ( i = 0; i < nw; i++ )
		{
			oi = (int)(((float)ow) * ( (float) i ) / ((float)nw) );

			dest[ i + nw * j ] = src[ oi + ow * oj ];
		}
	}
}

static void scale_indexed_smooth (
	unsigned char		* const	dest,
	int			const	nw,
	int			const	nh,
	unsigned char	const * const	src,
	int			const	ow,
	int			const	oh
	)
{
	int		i, j, oi, oj, oi1, oj1, oi2, oj2;
	int		a = 0, b = 0, c = 0;
	int		d = 0, e = 0, xlen, ylen, avi, avj;
	float		foi, foj, foi1, foj1, foi2, foj2;
	float		ab, ac, xf1, xf2, yf1, yf2, av_tot, pf1, pf2, ff;


	for ( j = 0; j < nh; j++ )
	{
		foj = ( (float) oh - 1 ) * ( (float) j ) / ( (float)nh - 1 );

		for ( i = 0; i < nw; i++ )
		{
			foi = ( (float) ow - 1) * ( (float) i ) /
				( (float)nw - 1 );

			oi  = (int)foi;
			oj  = (int)foj;

			if ( nw >= ow )	// Expanding X axis
			{
				oi2 = (int)(foi + 0.9999f);
				ab = foi - (float)( (int) foi );

				a = src[ oi + ow * oj ];
				b = src[ oi2 + ow * oj ];
			}
			else
			{
				ab = 0.0;
			}

			if ( nh >= oh )		// Expanding Y axis
			{
				oi2 = (int)(foi + 0.9999f);
				oj2 = (int)(foj + 0.9999f);
				ac = foj - (float)( (int) foj );

				c = src[ oi + ow * oj2 ];
				d = src[ oi2 + ow * oj2 ];
			}
			else
			{
				ac = 0.0;
			}

			if ( nw < ow || nh < oh )	// Shrinking
			{
			 av_tot = 0;
			 ff = 0;

			 foi1 = ( (float) ow ) * ( (float) i ) / (float)nw;
			 foj1 = ( (float) oh ) * ( (float) j ) / (float)nh;
			 foi2 = ( (float) ow ) * ( (float) i + 1 ) / (float)nw;
			 foj2 = ( (float) oh ) * ( (float) j + 1 ) / (float)nh;

			 oi1 = (int)foi1;
			 oj1 = (int)foj1;
			 oi2 = (int)foi2;
			 oj2 = (int)foj2;

			 if ( ( foi2 - (float)oi2 ) < 0.00001f )
			 {
				oi2 = oi2 - 1;
			 }

			 if ( ( foj2 - (float)oj2 ) < 0.00001f )
			 {
				oj2 = oj2 - 1;
			 }

			 // X Fraction of 1st pixel to sample from
			 // original
			 xf1 = 1 - ( foi1 - (float)oi1 );

			 // Y Fraction of 1st pixel to sample from
			 // original
			 yf1 = 1 - ( foj1 - (float)oj1 );

			 // X Fraction of last pixel to sample from
			 // original
			 xf2 = foi2 - (float)oi2;

			 // Y Fraction of last pixel to sample from
			 // original
			 yf2 = foj2 - (float)oj2;

			 // X pixels to sample from original
			 xlen = oi2 - oi1;

			 // Y pixels to sample from original
			 ylen = oj2 - oj1;

			 if ( xlen == 0 ) xf1 = 1.0;
			 if ( ylen == 0 ) yf1 = 1.0;

			 for ( avj = 0; avj <= ylen; avj ++ )
			 {
			   pf1 = 1.0;

			   if ( avj == 0 )
			   {
				pf1 = yf1;
			   }
			   else if ( avj == ylen )
			   {
				pf1 = yf2;
			   }

			   for ( avi = 0; avi <= xlen; avi ++ )
			   {
			      pf2 = pf1;

			      if ( avi == 0 )
			      {
				pf2 = pf1 * xf1;
			      }
			      else if ( avi == xlen )
			      {
				pf2 = pf1 * xf2;
			      }

				ff = ff + pf2 * ( src[ oi1 + avi +
					ow * ( oj1 + avj ) ] );

			      av_tot = av_tot + pf2;
			   }
			 }

			 // Average colour
			 a = (int)( ( 0.49999f ) + ( (float)ff ) / av_tot );
			}

			e = (int)(	(1 - ac) * ((1 - ab) * ((float)a) +
					ab * ((float)b) ) +
					ac * ( ( 1 - ab ) * ((float)c) +
					ab * ((float)d) )
					);

			dest[ i + nw * j ] = (unsigned char)e;
		}
	}
}

static void scale_rgb_nn (
	unsigned char		* const	dest,
	int			const	nw,
	int			const	nh,
	unsigned char	const * const	src,
	int			const	ow,
	int			const	oh
	)
{
	int		i, j, oi, oj;


	for ( j = 0; j < nh; j++ )
	{
		oj = (int)( ( (float)oh ) * ( (float)j ) / ( (float)nh ) );

		for ( i = 0; i < nw; i++ )
		{
			oi = (int)(((float)ow) * ( (float) i ) / ((float)nw) );

			dest[ 3 * ( i + nw * j ) ] =
				src[ 3 * ( oi + ow * oj ) ];

			dest[ 1 + 3 * ( i + nw * j ) ] =
				src[ 1 + 3 * ( oi + ow * oj ) ];

			dest[ 2 + 3 * ( i + nw * j ) ] =
				src[ 2 + 3 * ( oi + ow * oj ) ];
		}
	}
}

static void scale_rgb_smooth (
	unsigned char		* const	dest,
	int			const	nw,
	int			const	nh,
	unsigned char	const * const	src,
	int			const	ow,
	int			const	oh
	)
{
	int		i, j, oi, oj, k, oi1, oj1, oi2, oj2;
	int		a[3] = { 0 }, b[3] = { 0 }, c[3] = { 0 };
	int		d[3] = { 0 }, e[3] = { 0 }, xlen, ylen, avi, avj;
	float		foi, foj, foi1, foj1, foi2, foj2;
	float		ab, ac, xf1, xf2, yf1, yf2, av_tot, pf1, pf2, ff[3];


	for ( j = 0; j < nh; j++ )
	{
		foj = ( (float) oh - 1 ) * ( (float) j ) / ( (float)nh - 1 );

		for ( i = 0; i < nw; i++ )
		{
			foi = ( (float) ow - 1) * ( (float) i ) /
				( (float)nw - 1 );

			oi  = (int)foi;
			oj  = (int)foj;

			if ( nw >= ow )	// Expanding X axis
			{
				oi2 = (int)(foi + 0.9999f);
				ab = foi - (float)( (int) foi );

				for ( k = 0; k < 3; k++ )
				{
					a[k] = src[ k + 3 *
						( oi + ow * oj ) ];
					b[k] = src[ k + 3 *
						( oi2 + ow * oj ) ];
				}
			}
			else
			{
				ab = 0.0;
			}

			if ( nh >= oh )		// Expanding Y axis
			{
				oi2 = (int)(foi + 0.9999f);
				oj2 = (int)(foj + 0.9999f);
				ac = foj - (float)( (int) foj );

				for ( k = 0; k < 3; k++ )
				{
					c[k] = src[ k + 3 * ( oi
						+ ow * oj2 ) ];
					d[k] = src[ k + 3 * ( oi2
						+ ow * oj2 ) ];
				}
			}
			else
			{
				ac = 0.0;
			}

			if ( nw < ow || nh < oh )	// Shrinking
			{
			 av_tot = 0;
			 ff[0] = 0; ff[1] = 0; ff[2] = 0;

			 foi1 = ( (float) ow ) * ( (float) i ) / (float)nw;
			 foj1 = ( (float) oh ) * ( (float) j ) / (float)nh;
			 foi2 = ( (float) ow ) * ( (float) i + 1 ) / (float)nw;
			 foj2 = ( (float) oh ) * ( (float) j + 1 ) / (float)nh;

			 oi1 = (int)foi1;
			 oj1 = (int)foj1;
			 oi2 = (int)foi2;
			 oj2 = (int)foj2;

			 if ( ( foi2 - (float)oi2 ) < 0.00001f )
			 {
				oi2 = oi2 - 1;
			 }

			 if ( ( foj2 - (float)oj2 ) < 0.00001f )
			 {
				oj2 = oj2 - 1;
			 }

			 // X Fraction of 1st pixel to sample from
			 // original
			 xf1 = 1 - ( foi1 - (float)oi1 );

			 // Y Fraction of 1st pixel to sample from
			 // original
			 yf1 = 1 - ( foj1 - (float)oj1 );

			 // X Fraction of last pixel to sample from
			 // original
			 xf2 = foi2 - (float)oi2;

			 // Y Fraction of last pixel to sample from
			 // original
			 yf2 = foj2 - (float)oj2;

			 // X pixels to sample from original
			 xlen = oi2 - oi1;

			 // Y pixels to sample from original
			 ylen = oj2 - oj1;

			 if ( xlen == 0 ) xf1 = 1.0;
			 if ( ylen == 0 ) yf1 = 1.0;

			 for ( avj = 0; avj <= ylen; avj ++ )
			 {
			   pf1 = 1.0;

			   if ( avj == 0 )
			   {
				pf1 = yf1;
			   }
			   else if ( avj == ylen )
			   {
				pf1 = yf2;
			   }

			   for ( avi = 0; avi <= xlen; avi ++ )
			   {
			      pf2 = pf1;

			      if ( avi == 0 )
			      {
				pf2 = pf1 * xf1;
			      }
			      else if ( avi == xlen )
			      {
				pf2 = pf1 * xf2;
			      }

			      for ( k = 0; k < 3; k++ )
			      {
				ff[k] = ff[k] + pf2 * (
					src[ k + 3 * ( oi1 + avi +
					ow * ( oj1 + avj ) ) ] );
			      }

			      av_tot = av_tot + pf2;
			   }
			 }

			 // Average colour
			 for ( k = 0; k < 3; k++ )
			 {
				a[k] = (int)(
					( 0.49999f ) +
					( (float)ff[k] ) / av_tot
					);
			 }
			}

			for ( k = 0; k < 3; k++ )
			{
				e[k] = (int)(
					(1 - ac) * ((1 - ab) * ((float)a[k]) +
					ab * ((float)b[k]) ) +
					ac * ( ( 1 - ab ) * ((float)c[k]) +
					ab * ((float)d[k]) )
					);
			}

			dest[ 3 * ( i + nw * j ) ] = (unsigned char)e[0];
			dest[ 1 + 3 * ( i + nw * j ) ] = (unsigned char)e[1];
			dest[ 2 + 3 * ( i + nw * j ) ] = (unsigned char)e[2];
		}
	}
}

mtPixy::Image * mtPixy::Image::scale (
	int		const	w,
	int		const	h,
	ScaleType	const	scaletype
	)
{
	switch ( scaletype )
	{
	case BLOCKY:
		break;

	case SMOOTH:
		// Smooth scaling only allowed for RGB images
		if ( m_type != RGB )
		{
			return NULL;
		}
		break;

	default:
		return NULL;
	}


	Image	* newim;


	newim = image_create ( m_type, w, h );
	if ( ! newim )
	{
		return NULL;
	}

	newim->m_palette.copy ( &m_palette );

	if ( m_alpha && newim->create_alpha () )
	{
		delete newim;
		return NULL;
	}

	switch ( m_type )
	{
	case RGB:
		if ( scaletype == BLOCKY )
		{
			scale_rgb_nn ( newim->m_canvas, w, h, m_canvas,
				m_width, m_height );
		}
		else if ( scaletype == SMOOTH )
		{
			scale_rgb_smooth ( newim->m_canvas, w, h, m_canvas,
				m_width, m_height );
		}
		break;

	case INDEXED:
		scale_indexed_nn ( newim->m_canvas, w, h, m_canvas, m_width,
			m_height );
		break;

	default:
		// Ignore other image types
		break;
	}

	if ( m_alpha )
	{
		if ( m_type == 0 )
		{
			scale_indexed_nn ( newim->m_alpha, w, h,
				m_alpha, m_width, m_height );
		}
		else
		{
			scale_indexed_smooth ( newim->m_alpha, w, h,
				m_alpha, m_width, m_height );
		}
	}

	return newim;
}

