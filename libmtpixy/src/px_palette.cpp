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



mtPixy::Palette::Palette (
	int	const	paltype
	)
	:
	m_color_total	( 0 )
{
	if ( set_uniform ( paltype ) )
	{
		// Bad argument so set to default 8 colours
		set_uniform ( 2 );
	}
}

mtPixy::Palette::~Palette ()
{
}

static unsigned char rgb_from_f (
	int	const	val,
	int	const	factor
	)
{
	double	const	v = (double)val * 255;
	double	const	f = (double)factor - 1;

	return (unsigned char)lround( v / f );
}

int mtPixy::Palette::set_uniform (
	int	const	factor
	)
{
	if (	factor < UNIFORM_MIN	||
		factor > UNIFORM_MAX
		)
	{
		return 1;
	}


	int	r, g, b, i;


	m_color_total = factor * factor * factor;
	i = 0;

	for ( b = 0; b < factor; b++ )
	{
		for ( g = 0; g < factor; g++ )
		{
			for ( r = 0; r < factor; r++ )
			{
				m_color[i].red = rgb_from_f ( r, factor );
				m_color[i].green = rgb_from_f ( g, factor );
				m_color[i].blue = rgb_from_f ( b, factor );

				i++;
			}
		}
	}

	return 0;
}

int mtPixy::Palette::set_uniform_balanced (
	int	const	factor
	)
{
	if (	factor < UNIFORM_MIN	||
		factor > UNIFORM_MAX
		)
	{
		return 1;
	}


	int	r, g, b, i;


	m_color_total = factor * factor * factor + 8;
	i = 0;

	for ( b = 0; b < factor; b++ )
	{
		for ( g = 0; g < factor; g++ )
		{
			for ( r = 0; r < factor; r++ )
			{
				m_color[i].red = rgb_from_f ( r+1, factor+2 );
				m_color[i].green = rgb_from_f ( g+1, factor+2 );
				m_color[i].blue = rgb_from_f ( b+1, factor+2 );

				i++;
			}
		}
	}

	for ( b = 0; b < 2; b++ )
	{
		for ( g = 0; g < 2; g++ )
		{
			for ( r = 0; r < 2; r++ )
			{
				m_color[i].red = rgb_from_f ( r, 2 );
				m_color[i].green = rgb_from_f ( g, 2 );
				m_color[i].blue = rgb_from_f ( b, 2 );

				i++;
			}
		}
	}

	return 0;
}

int mtPixy::Palette::load (
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
	Palette		tmp_palette;
	Color		* const tmpcol = tmp_palette.get_color ();


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
		if ( ! input || len > 100 )
		{
			// EOF or line too long
			break;
		}

		// If line starts with a number or space assume its a
		// palette entry

		if (	input[0] == ' ' ||
			( input[0] >= '0' && input[0] <= '9')
			)
		{
			if ( 3 != sscanf ( input, "%3i %3i %3i", &rgb[0],
				&rgb[1], &rgb[2] ) )
			{
				goto error;
			}

			tmpcol[i].red = (unsigned char)rgb[0];
			tmpcol[i].green = (unsigned char)rgb[1];
			tmpcol[i].blue = (unsigned char)rgb[2];

			i++;
		}

		free ( input );
		input = NULL;
	}

	if ( i < COLOR_TOTAL_MIN )
	{
		goto error;
	}

	tmp_palette.set_color_total ( i );
	copy ( &tmp_palette );

	fclose ( fp );

	return 0;

error:
	free ( input );
	fclose ( fp );

	return 1;
}

int mtPixy::Palette::save (
	char	const	* const	filename
	) const
{
	if ( ! filename )
	{
		return 1;
	}


	FILE		* fp;
	int		i;


	fp = fopen ( filename, "w" );
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

	for ( i = 0; i < m_color_total; i++ )
	{
		if ( 0 > fprintf ( fp, "%3i %3i %3i\tUntitled\n",
			m_color[i].red, m_color[i].green, m_color[i].blue
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

int mtPixy::Palette::set_correct ()
{
	int		i = m_color_total;


	if ( i < 0 )
	{
		i = 0;
	}
	else if ( i > COLOR_TOTAL_MAX )
	{
		i = COLOR_TOTAL_MAX;
	}

	// Clear missing colours as black
	for ( ; i < COLOR_TOTAL_MIN; i++ )
	{
		m_color[i].red = 0;
		m_color[i].green = 0;
		m_color[i].blue = 0;
	}

	m_color_total = i;

	return 0;
}

int mtPixy::Palette::set_grey ()
{
	int		i;


	m_color_total = 256;

	for ( i = 0; i < 256; i++ )
	{
		m_color[i].red = (unsigned char)i;
		m_color[i].green = (unsigned char)i;
		m_color[i].blue = (unsigned char)i;
	}

	return 0;
}

int mtPixy::Palette::set_color_total (
	int	const	newtotal
	)
{
	if (	newtotal < COLOR_TOTAL_MIN	||
		newtotal > COLOR_TOTAL_MAX
		)
	{
		return 1;
	}

	m_color_total = newtotal;

	return 0;
}

int mtPixy::Palette::copy (
	Palette		* const	src
	)
{
	if ( ! src )
	{
		return 1;
	}

	if ( src == this )
	{
		return 0;
	}

	m_color_total = src->m_color_total;
	memcpy ( &m_color, src->m_color, sizeof(m_color) );

	set_correct ();

	return 0;
}

int mtPixy::Palette::get_color_total () const
{
	return m_color_total;
}

int mtPixy::Palette::get_color_index (
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	) const
{
	int		i;


	for ( i = 0; i < m_color_total; i++ )
	{
		if (	r == m_color[i].red	&&
			g == m_color[i].green	&&
			b == m_color[i].blue
			)
		{
			return i;
		}
	}

	return -1;
}

void mtPixy::Palette::effect_invert ()
{
	for ( int i = 0; i < m_color_total; i++ )
	{
		m_color[i].red = (unsigned char)(255 - m_color[i].red);
		m_color[i].green = (unsigned char)(255 - m_color[i].green);
		m_color[i].blue = (unsigned char)(255 - m_color[i].blue);
	}
}

int mtPixy::Palette::create_gradient (
	unsigned char	const	a,
	unsigned char	const	b
	)
{
	int	const	x = MIN ( a, b );
	int	const	y = MAX ( a, b );
	double	const	delta = (double)(y - x);
	double		p;
	int		i;


	for ( i = x + 1; i < y; i++ )
	{
		p = ((double)(i - x)) / delta;

		m_color[i].red = (unsigned char)lround (
			(1 - p) * m_color[x].red + p * m_color[y].red );

		m_color[i].green = (unsigned char)lround (
			(1 - p) * m_color[x].green + p * m_color[y].green );

		m_color[i].blue = (unsigned char)lround (
			(1 - p) * m_color[x].blue + p * m_color[y].blue );
	}

	return 0;
}

int mtPixy::Palette::append_color (
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	if ( set_color_total ( m_color_total + 1 ) )
	{
		return -1;
	}

	m_color[ m_color_total - 1 ].red = r;
	m_color[ m_color_total - 1 ].green = g;
	m_color[ m_color_total - 1 ].blue = b;

	return m_color_total - 1;
}

mtPixy::Color * mtPixy::Palette::get_color ()
{
	return m_color;
}

mtPixy::Color const * mtPixy::Palette::get_color () const
{
	return m_color;
}

void mtPixy::Palette::transform_color (
	int	const	ga,
	int	const	br,
	int	const	co,
	int	const	sa,
	int	const	hu,
	int	const	po
	)
{
	if ( m_color_total < COLOR_TOTAL_MIN || m_color_total > COLOR_TOTAL_MAX)
	{
		return;
	}


	unsigned char	* rgb;
	int		i;


	rgb = (unsigned char *)malloc ( (size_t)(m_color_total * 3) );
	if ( ! rgb )
	{
		return;
	}

	for ( i = 0; i < m_color_total; i++ )
	{
		rgb[ 3*i + 0 ] = m_color[i].red;
		rgb[ 3*i + 1 ] = m_color[i].green;
		rgb[ 3*i + 2 ] = m_color[i].blue;
	}

	mtPixy::transform_color ( rgb, m_color_total, ga, br, co, sa, hu, po );

	for ( i = 0; i < m_color_total; i++ )
	{
		m_color[i].red		= rgb[ 3*i + 0 ];
		m_color[i].green	= rgb[ 3*i + 1 ];
		m_color[i].blue		= rgb[ 3*i + 2 ];
	}

	free ( rgb );
}

mtPixy::Color::Color (
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	red = r;
	green = g;
	blue = b;
}

mtPixy::Color::Color ()
{
	red = 0;
	green = 0;
	blue = 0;
}

mtPixy::Color::~Color ()
{
}

