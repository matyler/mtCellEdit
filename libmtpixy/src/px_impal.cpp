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



static int pal_tree_cmp (
	void	const	* const k1,
	void	const	* const k2
	)
{
	if ( (intptr_t)k1 < (intptr_t)k2 )
	{
		return -1;
	}

	if ( (intptr_t)k1 > (intptr_t)k2 )
	{
		return 1;
	}

	return 0;
}

static void get_histogram (
	unsigned char	const *		img,
	unsigned char	const * const	ilim,
	int			* const	histogram
	)
{
	memset ( histogram, 0, 256 * sizeof( histogram[0] ) );

	for ( ; img < ilim; img++ )
	{
		histogram[ img[0] ]++;
	}
}

static int get_histogram_rgb (
	unsigned char	const *		img,
	unsigned char	const * const	ilim,
	int			* const	histogram,
	mtPixy::Color	const * const	col,
	int			const	coltot
	)
{
	int		rgb, i;
	mtTree		* pal_tree;	// Key=RGB Data=Palette Index
	mtTreeNode	* tnode;


	memset ( histogram, 0, 256 * sizeof( histogram[0] ) );

	pal_tree = mtkit_tree_new ( pal_tree_cmp, NULL );
	if ( ! pal_tree )
	{
		return 1;
	}

	for ( i = 0; i < coltot; i++ )
	{
		rgb = mtPixy::rgb_2_int( col[i].red, col[i].green, col[i].blue);

		if ( 0 == mtkit_tree_node_add ( pal_tree,
			(void *)(intptr_t)rgb,
			(void *)(intptr_t)i )
			)
		{
			mtkit_tree_destroy ( pal_tree );
			return 1;
		}
	}

	for ( ; img < ilim; img += 3 )
	{
		rgb = mtPixy::rgb_2_int ( img[0], img[1], img[2] );

		tnode = mtkit_tree_node_find( pal_tree, (void *)(intptr_t)rgb );
		if ( tnode )
		{
			// Count palette entry frequency
			histogram[ (int)(intptr_t)tnode->data ]++;
		}
	}

	mtkit_tree_destroy ( pal_tree );

	return 0;
}



#define MOD3(x) ((((x) * 5 + 1) >> 2) & 3)



static void rgb2hsv (
	unsigned char	const * const	rgb,
	double			* const hsv
	)
{
	int		c0, c1, c2;


	if ( !((rgb[0] ^ rgb[1]) | (rgb[0] ^ rgb[2]) ) )
	{
		hsv[0] = hsv[1] = 0.0;
		hsv[2] = rgb[0];

		return;
	}

	c2 = rgb[2] < rgb[0] ? 1 : 0;

	if ( rgb[c2] >= rgb[c2 + 1] )
	{
		c2++;
	}

	c0 = MOD3 ( c2 + 1 );
	c1 = (c2 + c0) ^ 3;

	hsv[2] = rgb[c0] > rgb[c1] ? rgb[c0] : rgb[c1];
	hsv[1] = hsv[2] - rgb[c2];
	hsv[0] = c0 * 2 + 1 + (rgb[c1] - rgb[c0]) / hsv[1];
	hsv[1] /= hsv[2];
}

static double rgb_2_hsv (
	int		const	a,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	unsigned char	rgb[3] = { r, g, b };
	double		hsv[3];


	rgb2hsv ( rgb, hsv );

	return hsv[a];
}

int mtPixy::Image::palette_sort (
	unsigned char	const	i_start,
	unsigned char	const	i_end,
	PaletteSortType	const	s_type,
	bool		const	reverse
	)
{
	int		const	start = MIN ( i_start, i_end );
	int		const	end = MAX ( i_start, i_end );
	int			tab0[256], tab1[256], tmp, i, j, histogram[256];
	int		const	coltot = m_palette.get_color_total ();
	Color		* const	col = m_palette.get_color ();


	if ( coltot < end )
	{
		return 1;
	}

	if ( s_type == SORT_FREQUENCY )
	{
		if ( ! m_canvas )
		{
			return 1;
		}

		unsigned char const * const img = m_canvas;
		unsigned char const * const ilim = img + m_width * m_height *
						m_canvas_bpp;

		if ( RGB == m_type )
		{
			if ( get_histogram_rgb ( img, ilim, histogram, col,
				coltot ) )
			{
				return 1;
			}
		}
		else if ( INDEXED == m_type )
		{
			get_histogram ( img, ilim, histogram );
		}
		else
		{
			return 1;
		}
	}

	for ( i = 0; i < 256; i++ )
	{
		tab0[i] = i;
	}

	for ( i = start; i <= end; i++ )
	{
		switch ( s_type )
		{
		case SORT_HUE:
			tab1[i] = (int)( 0.5 + 1000 * rgb_2_hsv ( 0,
				col[i].red, col[i].green, col[i].blue ) );
			break;

		case SORT_SATURATION:
			tab1[i] = MAX ( col[i].red,
				MAX ( col[i].green, col[i].blue ) ) -
				MIN ( col[i].red,
				MIN ( col[i].green, col[i].blue ) );
			break;

		case SORT_VALUE:
			tab1[i] = MAX ( col[i].red,
				MAX ( col[i].green, col[i].blue ) );
			break;

		case SORT_MIN:
			tab1[i] = MIN ( col[i].red,
				MIN ( col[i].green, col[i].blue ) );
			break;

		case SORT_BRIGHTNESS:
			tab1[i] = rgb_2_brightness ( col[i].red, col[i].green,
				col[i].blue );
			break;

		case SORT_RED:
			tab1[i] = col[i].red;
			break;

		case SORT_GREEN:
			tab1[i] = col[i].green;
			break;

		case SORT_BLUE:
			tab1[i] = col[i].blue;
			break;

		case SORT_FREQUENCY:
			tab1[i] = histogram[i];
			break;

		default:
			break;
		}
	}

	for ( j = end; j > start; j-- )		// The venerable bubble sort
	{
		for ( i = start; i < j; i++ )
		{
			if (	(!reverse && tab1[i] > tab1[i + 1] ) ||
				( reverse && tab1[i] < tab1[i + 1] )
				)
			{
				tmp = tab0[i];
				tab0[i] = tab0[i + 1];
				tab0[i + 1] = tmp;

				tmp = tab1[i];
				tab1[i] = tab1[i + 1];
				tab1[i + 1] = tmp;
			}
		}
	}


	Palette			np;
	Color		* const	oc = np.get_color ();


	np.copy( &m_palette );

	for ( i = start; i <= end; i++ )
	{
		col[i] = oc[ tab0[i] ];
	}

	if ( INDEXED == m_type )
	{
		// Adjust canvas pixels if in indexed palette mode
		for ( i = 0; i < 256; i++ )
		{
			tab1[ tab0[i] ] = i;
		}


		unsigned char			* img = m_canvas;
		unsigned char const * const	ilim = img + m_width * m_height;


		for ( ; img < ilim; img++ )
		{
			img[0] = (unsigned char)tab1[ img[0] ];
		}
	}

	return 0;
}

int mtPixy::Image::palette_merge_duplicates (
	int	* const	tot
	)
{
	if ( m_type != INDEXED || ! m_canvas )
	{
		return 1;
	}

	unsigned char		dupes[256];
	int			i, j, found = 0;
	int		const	coltot = m_palette.get_color_total ();
	Color	const * const	col = m_palette.get_color ();

	dupes[0] = 0;

	for ( i = coltot - 1; i > 0; i-- )
	{
		dupes[i] = (unsigned char)i;

		for ( j = 0; j < i; j++ )
		{
			if (	col[i].red	== col[j].red &&
				col[i].green	== col[j].green &&
				col[i].blue	== col[j].blue
				)
			{
				found++;
				dupes[i] = (unsigned char)j;

				break;
			}
		}
	}

	unsigned char		* img = m_canvas;
	unsigned char	* const	ilim = img + m_width * m_height;

	for ( ; img < ilim; img++ )
	{
		img[0] = dupes[ img[0] ];
	}

	if ( tot )
	{
		tot[0] = found;
	}

	return 0;
}

int mtPixy::Image::palette_remove_unused (
	int	* const	tot
	)
{
	if ( m_type != INDEXED || ! m_canvas )
	{
		return 1;
	}


	unsigned char		conv[ Palette::COLOR_TOTAL_MAX ];
	int			i, j, found;
	int			histogram[ Palette::COLOR_TOTAL_MAX ];
	Color		* const	col = m_palette.get_color ();
	int		const	coltot = m_palette.get_color_total ();
	unsigned char		* img = m_canvas;
	unsigned char	* const	ilim = img + m_width * m_height;


	get_histogram ( img, ilim, histogram );

	for ( found = 0, i = 0; i < coltot; i++ )
	{
		if ( 0 == histogram[i] )
		{
			found++;
		}
	}

	// We do full 256 to catch rogue orphan pixels
	for ( i = j = 0; i < Palette::COLOR_TOTAL_MAX; i++ )
	{
		if ( histogram[i] )
		{
			col[j] = col[i];

			conv[i] = (unsigned char)( j++ );
		}
	}

	for ( ; img < ilim; img++ )
	{
		img[0] = conv[ img[0] ];
	}

	m_palette.set_color_total ( MAX ( coltot - found,
		Palette::COLOR_TOTAL_MIN ) );

	if ( tot )
	{
		tot[0] = found;
	}

	return 0;
}

int mtPixy::Image::get_information (
	int		&urp,
	int		&pnip,
	int	* const	pf,
	int		&pt
	)
{
	unsigned char const * const lim =
			m_canvas + m_canvas_bpp * m_width * m_height;
	int	const	coltot = m_palette.get_color_total ();
	int		i;


	urp = 0;
	pnip = 0;
	pt = coltot;

	for ( i = 0; i < Palette::COLOR_TOTAL_MAX; i++ )
	{
		pf [ i ] = 0;
	}

	if ( ! m_canvas )
	{
		return 0;
	}

	if ( RGB == m_type )
	{
		unsigned char * cube = (unsigned char *)calloc( 256*256, 256 );
		if ( ! cube )
		{
			return 1;
		}


		Color	const * const	col = m_palette.get_color ();
		int			rgb;


		if ( get_histogram_rgb ( m_canvas, lim, pf, col, coltot ) )
		{
			free ( cube );
			return 1;
		}

		for ( unsigned char const * s = m_canvas; s < lim; s += 3 )
		{
			rgb = mtPixy::rgb_2_int ( s[0], s[1], s[2] );
			cube[ rgb ] = 1;
		}

		// Count unique pixel values
		for ( i = 0; i < 16777216; i++ )
		{
			if ( cube[i] > 0 )
			{
				urp++;
			}
		}

		free ( cube );
	}
	else if ( INDEXED == m_type )
	{
		get_histogram ( m_canvas, lim, pf );

		// Count unique pixel values
		for ( i = 0; i < Palette::COLOR_TOTAL_MAX; i++ )
		{
			if ( pf[i] > 0 )
			{
				urp++;
			}
		}
	}

	pnip = m_width * m_height;

	for ( i = 0; i < coltot; i++ )
	{
		pnip -= pf[i];
	}

	return 0;
}

int mtPixy::Image::palette_create_from_canvas ()
{
	if ( m_type != RGB || ! m_canvas )
	{
		return 1;
	}


	mtTree		* pal_tree;		// Key=RGB Data=index
	mtTreeNode	* tnode;
	int		i = 0, rgb;
	Palette		pal;
	Color		* col = pal.get_color ();
	unsigned char const * src;
	unsigned char const * slim = m_canvas + m_width * m_height * 3;


	pal_tree = mtkit_tree_new ( pal_tree_cmp, NULL );
	if ( ! pal_tree )
	{
		return 1;
	}

	pal.set_color_total ( 2 );

	for ( src = m_canvas; src < slim; src += 3 )
	{
		rgb = mtPixy::rgb_2_int ( src[0], src[1], src[2] );

		tnode = mtkit_tree_node_find( pal_tree, (void *)(intptr_t)rgb );
		if ( tnode )
		{
			// Colour is already in the palette
			continue;
		}

		if ( i >= Palette::COLOR_TOTAL_MAX )
		{
			// Too many colours for the palette to hold
			break;
		}

		col[i].red = src[0];
		col[i].green = src[1];
		col[i].blue = src[2];

		if ( 0 == mtkit_tree_node_add ( pal_tree,
			(void *)(intptr_t)rgb, (void *)(intptr_t)i )
			)
		{
			mtkit_tree_destroy ( pal_tree );
			return 1;
		}

		i++;
	}

	mtkit_tree_destroy ( pal_tree );
	pal_tree = NULL;

	if ( i >= Palette::COLOR_TOTAL_MAX )
	{
		return 1;
	}

	pal.set_color_total ( i );
	m_palette.copy ( &pal );

	return 0;
}

void mtPixy::Image::palette_move_color (
	unsigned char	const	idx,
	unsigned char	const	new_idx
	)
{
	if ( idx == new_idx )
	{
		return;
	}

	int	const	delta = idx < new_idx ? 1 : -1;
	Color	* const	dst = m_palette.get_color ();
	Color	const	tcol = dst[idx];

	for ( int i = idx; i != new_idx; i += delta )
	{
		dst[i] = dst[i + delta];
	}

	dst[ new_idx ] = tcol;

	if ( INDEXED == m_type && m_canvas )
	{
		unsigned char	map[256];
		unsigned char	* img = m_canvas;
		unsigned char	* const ilim = img + m_width * m_height;

		for ( int i = 0; i < 256; i++ )
		{
			map[i] = (unsigned char)i;
		}

		for ( int i = new_idx; i != idx; i -= delta )
		{
			map[i] = (unsigned char)(i - delta);
		}

		map[idx] = new_idx;

		for ( ; img < ilim; img++ )
		{
			img[0] = map[ img[0] ];
		}
	}
}

void mtPixy::Image::palette_set_default (
	int	const	pal_type,
	int	const	pal_num
	)
{
	if ( pal_type == 0 )
	{
		m_palette.set_uniform ( pal_num );
	}
	else
	{
		m_palette.set_uniform_balanced ( pal_num );
	}
}

