/*
	Copyright (C) 2016-2021 Mark Tyler

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



class FloodStack
{
public:
	explicit FloodStack ();
	~FloodStack ();

	void push ( int y, int minx, int maxx );	// Throws on fail
	int pop ( int &y, int &minx, int &maxx );	// 0=pop 1=no data

private:
	void get_next_chunk ();

	typedef struct
	{
		int m_y;
		int m_minx;
		int m_maxx;
	} Item;

	enum
	{
		ITEM_CHUNK = 1000
	};

	Item		* m_item;
	size_t		m_item_tot;
	size_t		m_item_pos;	// Next free slot
};



FloodStack::FloodStack ()
	:
	m_item		(),
	m_item_tot	( 0 ),
	m_item_pos	( 0 )
{
}

FloodStack::~FloodStack ()
{
	free ( m_item );
	m_item = NULL;
}

void FloodStack::get_next_chunk ()
{
	size_t	const	neo_size = m_item_tot + ITEM_CHUNK;
	Item	* const	neo_mem = (Item *)realloc( m_item, neo_size *
				sizeof(*m_item) );
	if ( ! neo_mem )
	{
		throw 123;
	}

	m_item = neo_mem;
	m_item_tot = neo_size;
}

void FloodStack::push (
	int	const	y,
	int	const	minx,
	int	const	maxx
	)
{
	if ( m_item_pos >= m_item_tot )
	{
		get_next_chunk ();
	}

	m_item[ m_item_pos ].m_y = y;
	m_item[ m_item_pos ].m_minx = minx;
	m_item[ m_item_pos ].m_maxx = maxx;

	m_item_pos++;
}

int FloodStack::pop (
	int	&y,
	int	&minx,
	int	&maxx
	)
{
	if ( m_item_pos < 1 )
	{
		return 1;
	}

	m_item_pos--;

	maxx = m_item[ m_item_pos ].m_maxx;
	minx = m_item[ m_item_pos ].m_minx;
	y = m_item[ m_item_pos ].m_y;

	return 0;
}



/// ----------------------------------------------------------------------------



static void flood_fill_internal (
	mtPixmap	* const	pixmap,
	mtPixmap	* const	im,
	int		const	x,
	int		const	y
	)
{
	FloodStack	stack;
	unsigned char	* const	al = pixy_pixmap_get_alpha ( im );

	if ( ! al )
	{
		return;
	}

	unsigned char	* pix = al + y * pixmap->width;
	int		minx = x;
	int		maxx = x;
	int		xp, yy = y, xp_min, xp_max;


	// Expand to the left if possible
	while ( minx > 0 )
	{
		if ( 1 != pix[ minx - 1 ] )
		{
			break;
		}

		minx--;
	}

	// Expand to the right if possible
	while ( maxx < (pixmap->width - 1) )
	{
		if ( 1 != pix[ maxx + 1 ] )
		{
			break;
		}

		maxx++;
	}

	// Initial scanline
	for ( xp = minx; xp <= maxx; xp++ )
	{
		pix[ xp ] = 255;
	}

	if ( yy >= 1 )
	{
		stack.push ( yy - 1, minx, maxx );
	}

	if ( (yy + 1) < pixmap->height )
	{
		stack.push ( yy + 1, minx, maxx );
	}

	while ( 0 == stack.pop ( yy, minx, maxx ) )
	{
		pix = al + yy * pixmap->width;
		int seg = 0;

		if ( 1 == pix[ minx ] )
		{
			// Expand to the left if possible
			while ( minx > 0 )
			{
				if ( 1 != pix[ minx - 1 ] )
				{
					break;
				}

				minx--;
			}
		}

		if ( 1 == pix[ maxx ] )
		{
			// Expand to the right if possible
			while ( maxx < (pixmap->width - 1) )
			{
				if ( 1 != pix[ maxx + 1 ] )
				{
					break;
				}

				maxx++;
			}
		}

		for ( xp = minx; xp <= maxx; xp++ )
		{
			if ( 1 == pix[ xp ] )
			{
				pix[ xp ] = 255;

				if ( 1 == seg )
				{
					xp_max = xp;
				}
				else
				{
					// New segment start
					seg = 1;
					xp_min = xp;
					xp_max = xp;
				}
			}
			else	// Pixel is 0 or 255 so ignore
			{
				if ( 1 == seg )
				{
					// Segment finish
					seg = 0;

					if ( yy >= 1 )
					{
						stack.push ( yy - 1,
							xp_min, xp_max );
					}

					if ( (yy + 1) < pixmap->height )
					{
						stack.push ( yy + 1,
							xp_min, xp_max );
					}
				}
			}
		}

		// Finish off final segment
		if ( seg == 1 )
		{
			if ( yy >= 1 )
			{
				stack.push ( yy - 1, xp_min, xp_max );
			}

			if ( (yy + 1) < pixmap->height )
			{
				stack.push ( yy + 1, xp_min, xp_max );
			}
		}
	}
}

static mtPixmap * flood_fill_prepare_alpha (
	mtPixmap	* const	pixmap,
	int		const	x,
	int		const	y
	)
{
	if (	! pixmap
		|| x < 0
		|| y < 0
		|| x >= pixmap->width
		|| y >= pixmap->height
		)
	{
		return NULL;
	}

	mtPixy::Pixmap alpha ( pixy_pixmap_new_alpha ( pixmap->width,
		pixmap->height ) );

	if ( ! alpha.get () )
	{
		return NULL;
	}

	unsigned char const * const can = pixmap->canvas;
	unsigned char * const alp = pixy_pixmap_get_alpha ( alpha.get() );

	if ( ! can || ! alp )
	{
		return NULL;
	}

	int	const	tot = pixmap->width * pixmap->height;
	int		i;

	if ( pixmap->bpp == PIXY_PIXMAP_BPP_INDEXED )
	{
		unsigned char const pix = can[ x + y * pixmap->width ];


		for ( i = 0; i < tot; i++ )
		{
			if ( can[i] == pix )
			{
				alp[i] = 1;
			}
			else
			{
				alp[i] = 0;
			}
		}
	}
	else if ( pixmap->bpp == PIXY_PIXMAP_BPP_RGB )
	{
		unsigned char const r = can[ 3 * (x + y * pixmap->width) + 0];
		unsigned char const g = can[ 3 * (x + y * pixmap->width) + 1];
		unsigned char const b = can[ 3 * (x + y * pixmap->width) + 2];


		for ( i = 0; i < tot; i++ )
		{
			if (	can[3 * i + 0] == r &&
				can[3 * i + 1] == g &&
				can[3 * i + 2] == b
				)
			{
				alp[i] = 1;
			}
			else
			{
				alp[i] = 0;
			}
		}
	}
	else
	{
		return NULL;
	}

	try
	{
		flood_fill_internal ( pixmap, alpha.get (), x, y );
	}
	catch ( ... )
	{
		return NULL;
	}

	// Make all 1's become 0 as they weren't filled
	for ( i = 0; i < tot; i++ )
	{
		if ( alp[i] == 1 )
		{
			alp[i] = 0;
		}
	}

	return alpha.release ();
}

int mtPixy::paint_flood_fill (
	mtPixmap * const pixmap,
	Brush		&brush,
	int	const	x,
	int	const	y
	)
{
	mtPixy::Pixmap const alpha ( flood_fill_prepare_alpha ( pixmap, x, y ));
	if ( ! alpha.get () )
	{
		return 1;
	}

	pixy_paint_flow ( alpha.get(), brush.get_flow() );

	return mtPixy::paste_alpha_pattern ( pixmap, alpha.get(), brush, 0, 0 );
}

int pixy_lasso (
	mtPixmap	* const	pixmap,
	int		const	x,
	int		const	y
	)
{
	mtPixy::Pixmap const alpha ( flood_fill_prepare_alpha ( pixmap, x, y ));
	if ( ! alpha.get () )
	{
		return 1;
	}

	int	const	pixtot = pixmap->width * pixmap->height;

	if ( ! pixmap->alpha )
	{
		if ( pixy_pixmap_create_alpha ( pixmap ) || ! pixmap->alpha )
		{
			return 1;
		}

		memset ( pixmap->alpha, 255, (size_t)pixtot );
	}

	unsigned char const * const src = pixy_pixmap_get_alpha ( alpha.get() );
	unsigned char * const	dest = pixmap->alpha;

	for ( int i = 0; i < pixtot; i++ )
	{
		if ( src[ i ] == 255 )
		{
			dest[ i ] = 0;
		}
	}

	return 0;
}

