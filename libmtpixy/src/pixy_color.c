/*
	Copyright (C) 2008-2021 Mark Tyler

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



void pixy_color_set_rgb_int (
	mtColor	* const	color,
	int	const	rgb
	)
{
	color->red	= (unsigned char)pixy_int_2_red ( rgb );
	color->green	= (unsigned char)pixy_int_2_green ( rgb );
	color->blue	= (unsigned char)pixy_int_2_blue ( rgb );
}

int pixy_color_get_rgb (
	mtColor	const * const	color
	)
{
	return pixy_rgb_2_int ( color->red, color->green, color->blue );
}

void pixy_transform_color (
	unsigned char * const	buf,
	int		const	buftot,
	int			gam,
	int			bri,
	int			con,
	int			sat,
	int			hue,
	int			pos
	)
{
	if ( ! buf )
	{
		return;
	}


	unsigned char	gamma_table[256];
	unsigned char	bc_table[256];
	unsigned char	ps_table[256];
	unsigned char	rgb[3];
	unsigned char	* img, * imglim;
	int	const	ixx[7] = {0, 1, 2, 0, 1, 2, 0};
	int		do_gamma, do_bc, do_ps = 0;
	int		br, co, sa;
	int		dH, sH, tH, ix0, ix1, ix2, c0, c1, c2, dc = 0;
	int		j, r, g, b;


	// Enforce argument sanity
	gam = MAX ( gam, -100 );
	gam = MIN ( gam, 100 );
	bri = MAX ( bri, -255 );
	bri = MIN ( bri, 255 );
	con = MAX ( con, -100 );
	con = MIN ( con, 100 );
	sat = MAX ( sat, -100 );
	sat = MIN ( sat, 100 );
	hue = MAX ( hue, -1529 );
	hue = MIN ( hue, 1529 );
	pos = MAX ( pos, 1 );
	pos = MIN ( pos, 8 );

	br = bri * 256;
	co = con;

	if (co > 0)
	{
		co *= 3;
	}

	co += 100;
	co = (256 * co) / 100;
	sa = (256 * sat) / 100;
	dH = sH = hue;

	if ( pos != 8 )
	{
		do_ps = 1;
	}

	do_gamma = gam;
	do_bc = br | (co - 256);

	// Prepare posterize table
	if ( do_ps )
	{
		int	const	posm[9] = { 0, 0xFF00, 0x5500, 0x2480, 0x1100,
					0x0840, 0x0410, 0x0204, 0 };
		int	const	pmul = posm[ pos ];


		for ( int i = 0; i < 256; i++ )
		{
			ps_table[i] = (unsigned char)
				(((i >> (8 - pos)) * pmul ) >> 8);
		}
	}

	// Prepare gamma table
	if ( do_gamma )
	{
		double const w = gam < 0 ?
			(1.0 + (double)( 1 - gam ) / 25):
			(1.0 - (double)( 1 + gam ) / 125);

		for ( int i = 0; i < 256; i++ )
		{
			double const d = rint(255.0 * pow((double)i / 255.0,w));
			gamma_table[i] = (unsigned char)d;
		}
	}

	// Prepare brightness-contrast table
	if ( do_bc )
	{
		for ( int i = 0; i < 256; i++ )
		{
			j = ((i + i - 255) * co + (255 * 256)) / 2 + br;
			bc_table[i] = (unsigned char)(
				j < 0 ? 0 : j > (255*256) ? 255 : j >> 8 );
		}
	}

	if ( dH )
	{
		if ( dH < 0 )
		{
			dH += 1530;
		}

		dc = (dH / 510) * 2;
		dH -= dc * 255;

		if ( ( sH = (dH > 255) ) )
		{
			dH = 510 - dH;
			dc = dc < 4 ? dc + 2 : 0;
		}
	}

	ix0 = ixx[ dc ];
	ix1 = ixx[ dc + 1 ];
	ix2 = ixx[ dc + 2 ];

	imglim = buf + buftot * 3;

	for ( img = buf; img < imglim; img += 3 )
	{
		rgb[0] = img[0];
		rgb[1] = img[1];
		rgb[2] = img[2];

		// If we do gamma transform
		if ( do_gamma )
		{
			rgb[0] = gamma_table[ rgb[0] ];
			rgb[1] = gamma_table[ rgb[1] ];
			rgb[2] = gamma_table[ rgb[2] ];
		}

		// If we do hue transform & colour has a hue
		if ( dH && ((rgb[0] ^ rgb[1]) | (rgb[0] ^ rgb[2])) )
		{
			// Min. component
			c2 = dc;

			if ( rgb[ix2] < rgb[ix0] )
			{
				c2++;
			}

			if ( rgb[ixx[c2]] >= rgb[ixx[c2 + 1]] )
			{
				c2++;
			}

			// Actual indices
			c2 = ixx[c2];
			c0 = ixx[c2 + 1];
			c1 = ixx[c2 + 2];

			// Max. component & edge dir
			if ( ( tH = (rgb[c0] <= rgb[c1]) ) )
			{
				c0 = ixx[c2 + 2];
				c1 = ixx[c2 + 1];
			}

			// Do adjustment
			j = dH * (rgb[c0] - rgb[c2]) + 127; // Round up (?)
			j = (j + (j >> 8) + 1) >> 8;

			r = rgb[c0];
			g = rgb[c1];
			b = rgb[c2];

			if ( tH ^ sH ) // Falling edge
			{
				r = g > j + b ? g - j : b;
				rgb[c1] = (unsigned char)r;
				rgb[c2] = (unsigned char)(rgb[c2] + j + r - g);
			}
			else // Rising edge
			{
				b = g < r - j ? g + j : r;
				rgb[c1] = (unsigned char)b;
				rgb[c0] = (unsigned char)(rgb[c0] - (j + g -b));
			}
		}

		r = rgb[ ix0 ];
		g = rgb[ ix1 ];
		b = rgb[ ix2 ];

		// If we do brightness/contrast transform
		if ( do_bc )
		{
			r = bc_table[ r ];
			g = bc_table[ g ];
			b = bc_table[ b ];
		}

		// If we do saturation transform
		if ( sa )
		{
			j = pixy_rgb_2_brightness ( r, g, b );
			r = r * 256 + (r - j) * sa;
			r = r < 0 ? 0 : r > (255 * 256) ? 255 : r >> 8;
			g = g * 256 + (g - j) * sa;
			g = g < 0 ? 0 : g > (255 * 256) ? 255 : g >> 8;
			b = b * 256 + (b - j) * sa;
			b = b < 0 ? 0 : b > (255 * 256) ? 255 : b >> 8;
		}

		// If we do posterize transform
		if ( do_ps )
		{
			r = ps_table[ r ];
			g = ps_table[ g ];
			b = ps_table[ b ];
		}

		img[0] = (unsigned char)r;
		img[1] = (unsigned char)g;
		img[2] = (unsigned char)b;
	}
}

