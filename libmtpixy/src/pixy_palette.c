/*
	Copyright (C) 2008-2020 Mark Tyler

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



static unsigned char rgb_from_f (
	int	const	val,
	int	const	factor
	)
{
	double	const	v = (double)val * 255;
	double	const	f = (double)factor - 1;

	return (unsigned char)lround( v / f );
}

void pixy_palette_init ( mtPalette * const palette )
{
	pixy_palette_set_uniform ( palette, 2 );
}

int pixy_palette_load (
	mtPalette	* const	palette,
	char	const	* const	filename
	)
{
	if ( ! filename )
	{
		return 1;
	}

	int		rgb[3], i, len;
	FILE		* fp;
	char		* input = NULL;
	mtPalette	tmp_palette;
	mtColor		* const tmpcol = &tmp_palette.color[0];

	pixy_palette_init ( &tmp_palette );

	fp = fopen ( filename, "r" );
	if ( NULL == fp )
	{
		return 1;
	}

	input = mtkit_file_readline ( fp, NULL, NULL );
	if (	NULL == input ||
		0 != strncmp ( input, "GIMP Palette", 12 )
		)
	{
		goto error;
	}

	free ( input );
	input = NULL;

	for ( i = 0; i < 256; )
	{
		input = mtkit_file_readline ( fp, &len, NULL );
		if ( ! input )
		{
			// EOF
			break;
		}

		// If line starts with a number or space assume its a
		// palette entry

		if (	input[0] == ' ' ||
			( input[0] >= '0' && input[0] <= '9' )
			)
		{
			if ( 3 != sscanf ( input, "%i %i %i", &rgb[0],
				&rgb[1], &rgb[2] ) )
			{
				fprintf ( stderr, "Error: Bad RGB line '%s'.\n",
					input );
				goto error;
			}

			tmpcol[i].red	= (unsigned char)rgb[0];
			tmpcol[i].green	= (unsigned char)rgb[1];
			tmpcol[i].blue	= (unsigned char)rgb[2];

			i++;
		}

		free ( input );
		input = NULL;
	}

	if ( i < PIXY_PALETTE_COLOR_TOTAL_MIN )
	{
		fprintf ( stderr, "Error: Not enough colours in palette.\n" );
		goto error;
	}

	tmp_palette.size = i;

	pixy_palette_copy ( palette, &tmp_palette );

	fclose ( fp );

	return 0;

error:
	free ( input );
	fclose ( fp );

	return 1;
}

int pixy_palette_save (
	mtPalette const	* const	palette,
	char	const	* const	filename
	)
{
	if ( ! filename )
	{
		return 1;
	}

	FILE * fp = fopen ( filename, "w" );
	if ( NULL == fp )
	{
		return 1;
	}

	if ( 0 > fprintf ( fp,
		"GIMP Palette\n"
		"Name: libmtpixy\n"
		"Columns: 16\n"
		"#\n" )
		)
	{
		goto error;
	}

	for ( int i = 0; i < palette->size; i++ )
	{
		if ( 0 > fprintf ( fp, "%3i %3i %3i\tUntitled\n",
			palette->color[i].red,
			palette->color[i].green,
			palette->color[i].blue
			) )
		{
			goto error;
		}
	}

	fclose ( fp );

	return 0;

error:
	fclose ( fp );

	return 1;
}

int pixy_palette_copy (
	mtPalette	* const	dest,
	mtPalette const	* const	src
	)
{
	if ( src == dest )
	{
		return 0;
	}

	dest->size = src->size;
	memcpy ( &dest->color, src->color, sizeof(dest->color) );

	pixy_palette_set_correct ( dest );

	return 0;
}

int pixy_palette_set_size (
	mtPalette	* const	palette,
	int		const	newtotal
	)
{
	if (	newtotal < PIXY_PALETTE_COLOR_TOTAL_MIN	||
		newtotal > PIXY_PALETTE_COLOR_TOTAL_MAX
		)
	{
		return 1;
	}

	palette->size = newtotal;

	return 0;
}

int pixy_palette_set_correct (
	mtPalette	* const	palette
	)
{
	int		i = palette->size;

	if ( i < 0 )
	{
		i = 0;
	}
	else if ( i > PIXY_PALETTE_COLOR_TOTAL_MAX )
	{
		i = PIXY_PALETTE_COLOR_TOTAL_MAX;
	}

	// Clear missing colours as black
	for ( ; i < PIXY_PALETTE_COLOR_TOTAL_MIN; i++ )
	{
		palette->color[i].red = 0;
		palette->color[i].green = 0;
		palette->color[i].blue = 0;
	}

	palette->size = i;

	return 0;
}

int pixy_palette_set_uniform (
	mtPalette	* const	palette,
	int		const	factor
	)
{
	if (	factor < PIXY_PALETTE_UNIFORM_MIN	||
		factor > PIXY_PALETTE_UNIFORM_MAX
		)
	{
		return 1;
	}

	palette->size = factor * factor * factor;

	int i = 0;

	for ( int b = 0; b < factor; b++ )
	{
		for ( int g = 0; g < factor; g++ )
		{
			for ( int r = 0; r < factor; r++ )
			{
				palette->color[i].red = rgb_from_f ( r, factor);
				palette->color[i].green = rgb_from_f(g, factor);
				palette->color[i].blue = rgb_from_f( b, factor);

				i++;
			}
		}
	}

	return 0;
}

int pixy_palette_set_uniform_balanced (
	mtPalette	* const	palette,
	int		const	factor		// UNIFORM_MIN / MAX
	)
{
	if (	factor < PIXY_PALETTE_UNIFORM_MIN	||
		factor > PIXY_PALETTE_UNIFORM_MAX
		)
	{
		return 1;
	}

	palette->size = factor * factor * factor + 8;

	int i = 0;

	for ( int b = 0; b < factor; b++ )
	{
		for ( int g = 0; g < factor; g++ )
		{
			for ( int r = 0; r < factor; r++ )
			{
				palette->color[i].red =
					rgb_from_f ( r+1, factor+2 );

				palette->color[i].green =
					rgb_from_f ( g+1, factor+2 );

				palette->color[i].blue =
					rgb_from_f ( b+1, factor+2 );

				i++;
			}
		}
	}

	for ( int b = 0; b < 2; b++ )
	{
		for ( int g = 0; g < 2; g++ )
		{
			for ( int r = 0; r < 2; r++ )
			{
				palette->color[i].red = rgb_from_f ( r, 2 );
				palette->color[i].green = rgb_from_f ( g, 2 );
				palette->color[i].blue = rgb_from_f ( b, 2 );

				i++;
			}
		}
	}

	return 0;
}

int pixy_palette_set_grey (
	mtPalette	* const	palette
	)
{
	palette->size = 256;

	for ( int i = 0; i < 256; i++ )
	{
		palette->color[i].red = (unsigned char)i;
		palette->color[i].green = (unsigned char)i;
		palette->color[i].blue = (unsigned char)i;
	}

	return 0;
}

void pixy_palette_effect_invert (
	mtPalette	* const	palette
	)
{
	for ( int i = 0; i < palette->size; i++ )
	{
		palette->color[i].red = (unsigned char)
			(255 - palette->color[i].red);

		palette->color[i].green = (unsigned char)
			(255 - palette->color[i].green);

		palette->color[i].blue = (unsigned char)
			(255 - palette->color[i].blue);
	}
}

int pixy_palette_create_gradient (
	mtPalette	* const	palette,
	unsigned char	const	a,
	unsigned char	const	b
	)
{
	int	const	x = MIN ( a, b );
	int	const	y = MAX ( a, b );
	double	const	delta = (double)(y - x);

	for ( int i = x + 1; i < y; i++ )
	{
		double const p = ((double)(i - x)) / delta;

		palette->color[i].red = (unsigned char)lround (
			(1 - p) * palette->color[x].red +
			p * palette->color[y].red );

		palette->color[i].green = (unsigned char)lround (
			(1 - p) * palette->color[x].green +
			p * palette->color[y].green );

		palette->color[i].blue = (unsigned char)lround (
			(1 - p) * palette->color[x].blue +
			p * palette->color[y].blue );
	}

	return 0;
}

int pixy_palette_append_color (
	mtPalette	* const	palette,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	if ( pixy_palette_set_size ( palette, palette->size + 1 ) )
	{
		return -1;
	}

	palette->color[ palette->size - 1 ].red = r;
	palette->color[ palette->size - 1 ].green = g;
	palette->color[ palette->size - 1 ].blue = b;

	return palette->size - 1;
}

int pixy_palette_get_color_index (
	mtPalette const * const	palette,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	for ( int i = 0; i < palette->size; i++ )
	{
		if (	r == palette->color[i].red	&&
			g == palette->color[i].green	&&
			b == palette->color[i].blue
			)
		{
			return i;
		}
	}

	return -1;
}

void pixy_palette_transform_color (
	mtPalette	* const	palette,
	int		const	ga,
	int		const	br,
	int		const	co,
	int		const	sa,
	int		const	hu,
	int		const	po
	)
{
	if (	palette->size < PIXY_PALETTE_COLOR_TOTAL_MIN ||
		palette->size > PIXY_PALETTE_COLOR_TOTAL_MAX
		)
	{
		return;
	}

	unsigned char	rgb[256*3];

	for ( int i = 0; i < palette->size; i++ )
	{
		rgb[ 3*i + 0 ] = palette->color[i].red;
		rgb[ 3*i + 1 ] = palette->color[i].green;
		rgb[ 3*i + 2 ] = palette->color[i].blue;
	}

	pixy_transform_color ( rgb, palette->size, ga, br, co, sa, hu, po );

	for ( int i = 0; i < palette->size; i++ )
	{
		palette->color[i].red		= rgb[ 3*i + 0 ];
		palette->color[i].green		= rgb[ 3*i + 1 ];
		palette->color[i].blue		= rgb[ 3*i + 2 ];
	}
}

