/*
	Copyright (C) 2016-2017 Mark Tyler

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
	FloodStack ( int * err );
	~FloodStack ();

	// push, pop return 1 on failure, 0 on success
	int push ( int y, int minx, int maxx );
	int pop ( int * y, int * minx, int * maxx );

private:
	enum Limits
	{
		MEM_SLICE = 3000
	};

	int		* m_mem;
	size_t		m_mem_size;	// Starts at MEM_SLICE, then grows
	size_t		m_position;	// Next free slot
};



FloodStack::FloodStack (
	int	* const	err
	)
	:
	m_mem		(),
	m_mem_size	( MEM_SLICE ),
	m_position	( 0 )
{
	m_mem = (int *)malloc ( m_mem_size * sizeof(*m_mem) );
	if ( ! m_mem && err )
	{
		err[0] = 1;
	}
}

FloodStack::~FloodStack ()
{
	free ( m_mem );
}

int FloodStack::push (
	int	const	y,
	int	const	minx,
	int	const	maxx
	)
{
	if ( (m_position + 2) >= m_mem_size )
	{
		size_t	const	neo_size = m_mem_size + MEM_SLICE;
		int		* neo_mem;


		neo_mem = (int *)realloc ( m_mem, neo_size * sizeof(*m_mem) );
		if ( ! neo_mem )
		{
			return 1;
		}

		m_mem = neo_mem;
		m_mem_size = neo_size;
	}

	m_mem[ m_position++ ] = y;
	m_mem[ m_position++ ] = minx;
	m_mem[ m_position++ ] = maxx;

	return 0;
}

int FloodStack::pop (
	int	* const	y,
	int	* const	minx,
	int	* const	maxx
	)
{
	if ( m_position < 3 )
	{
		return 1;
	}

	maxx[0] = m_mem[ --m_position ];
	minx[0] = m_mem[ --m_position ];
	y[0] = m_mem[ --m_position ];

	return 0;
}

mtPixy::Image * mtPixy::Image::flood_fill_prepare_alpha (
	int	const	x,
	int	const	y
	)
{
	if ( x < 0 || y < 0 || x >= m_width || y >= m_height )
	{
		return NULL;
	}

	Image * ial = mtPixy::image_create ( ALPHA, m_width, m_height );
	if ( ! ial )
	{
		return NULL;
	}

	unsigned char	const * const	can = m_canvas;
	unsigned char		* const	alp = ial->m_alpha;

	if ( ! can || ! alp )
	{
		delete ial;
		return NULL;
	}

	int	const	tot = m_width * m_height;
	int		i;

	if ( m_type == INDEXED )
	{
		unsigned char const pix = can[ x + y * m_width ];


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
	else if ( m_type == RGB )
	{
		unsigned char const r = can[ 3 * (x + y * m_width) + 0 ];
		unsigned char const g = can[ 3 * (x + y * m_width) + 1 ];
		unsigned char const b = can[ 3 * (x + y * m_width) + 2 ];


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
		delete ial;
		return NULL;
	}

	if ( flood_fill_internal ( ial, x, y ) )
	{
		delete ial;
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

	return ial;
}

int mtPixy::Image::paint_flood_fill (
	Brush		&bru,
	int	const	x,
	int	const	y
	)
{
	Image * ial = flood_fill_prepare_alpha ( x, y );
	if ( ! ial )
	{
		return 1;
	}

	ial->paint_flow ( bru );

	int res = paste_alpha_pattern ( ial, bru, 0, 0 );
	delete ial;

	return res;
}

int mtPixy::Image::lasso (
	int	const	x,
	int	const	y
	)
{
	Image * alp = flood_fill_prepare_alpha ( x, y );
	if ( ! alp )
	{
		return 1;
	}

	int	const	pixtot = m_width * m_height;

	if ( ! m_alpha )
	{
		if ( create_alpha () || ! m_alpha )
		{
			delete alp;
			return 1;
		}

		memset ( m_alpha, 255, (size_t)pixtot );
	}

	for ( int i = 0; i < pixtot; i++ )
	{
		if ( alp->m_alpha[ i ] == 255 )
		{
			m_alpha[ i ] = 0;
		}
	}

	delete alp;

	return 0;
}

int mtPixy::Image::flood_fill_internal (
	Image	* const	im,
	int	const	x,
	int	const	y
	)
{
	int		err = 0;
	FloodStack	stack ( &err );
	unsigned char	* const	al = im->m_alpha;


	if ( ! al || err )
	{
		return 1;
	}


	unsigned char * pix = al + y * m_width;
	int		minx = x;
	int		maxx = x;
	int		xp, yy = y, seg, xp_min, xp_max;


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
	while ( maxx < (m_width - 1) )
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

	if ( (yy - 1) >= 0 )
	{
		if ( stack.push ( yy - 1, minx, maxx ) )
		{
			return 1;
		}
	}

	if ( (yy + 1) < m_height )
	{
		if ( stack.push ( yy + 1, minx, maxx ) )
		{
			return 1;
		}
	}

	while ( 0 == stack.pop ( &yy, &minx, &maxx ) )
	{
		pix = al + yy * m_width;
		seg = 0;

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
			while ( maxx < (m_width - 1) )
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

					if ( (yy - 1) >= 0 )
					{
						if ( stack.push ( yy - 1,
							xp_min, xp_max ) )
						{
							return 1;
						}
					}

					if ( (yy + 1) < m_height )
					{
						if ( stack.push ( yy + 1,
							xp_min, xp_max ) )
						{
							return 1;
						}
					}
				}
			}
		}

		// Finish off final segment
		if ( seg == 1 )
		{
			if ( (yy - 1) >= 0 )
			{
				if ( stack.push ( yy - 1, xp_min, xp_max ) )
				{
					return 1;
				}
			}

			if ( (yy + 1) < m_height )
			{
				if ( stack.push ( yy + 1, xp_min, xp_max ) )
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

