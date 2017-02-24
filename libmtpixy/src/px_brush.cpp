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



mtPixy::Brush::Brush ()
	:
	m_shapes	(),
	m_patterns	(),
	m_shapes_palette(),
	m_patterns_palette (),
	m_shapes_palette_zoom ( 1 ),
	m_pattern_palette_zoom ( 1 ),
	m_color_a	( 0, 0, 0 ),
	m_color_b	( 0, 0, 0 ),
	m_index_a	( 0 ),
	m_index_b	( 0 ),
	m_shape_num	( 0 ),
	m_pattern_num	( 0 ),
	m_spacing	( 1 ),
	m_space_mod	( 0 ),
	m_flow		( FLOW_MAX )
{
	m_shape_mask = mtPixy::image_create ( mtPixy::Image::ALPHA,
		SHAPE_SIZE, SHAPE_SIZE );

	m_pattern_idx = mtPixy::image_create ( mtPixy::Image::INDEXED,
		PATTERN_SIZE, PATTERN_SIZE );

	m_pattern_rgb = mtPixy::image_create ( mtPixy::Image::RGB,
		PATTERN_SIZE, PATTERN_SIZE );
}

mtPixy::Brush::~Brush ()
{
	delete m_shapes;
	m_shapes = NULL;

	delete m_patterns;
	m_patterns = NULL;

	delete m_shape_mask;
	m_shape_mask = NULL;

	delete m_pattern_idx;
	m_pattern_idx = NULL;

	delete m_pattern_rgb;
	m_pattern_rgb = NULL;

	delete m_shapes_palette;
	m_shapes_palette = NULL;

	delete m_patterns_palette;
	m_patterns_palette = NULL;
}

int mtPixy::Brush::load_shapes (
	char	const * const	fn
	)
{
	mtPixy::Image	* ni;


	ni = mtPixy::image_load ( fn );
	if ( ! ni )
	{
		return 1;
	}

	if (	ni->get_type () != mtPixy::Image::INDEXED	||
		ni->get_width () % SHAPE_SIZE != 0		||
		ni->get_height () % SHAPE_SIZE != 0
		)
	{
		delete ni;
		return 1;
	}

	delete m_shapes;
	m_shapes = ni;
	m_shape_num = 0;

	rebuild_shape_mask ();

	return 0;
}

int mtPixy::Brush::load_patterns (
	char	const * const	fn
	)
{
	mtPixy::Image	* ni;


	ni = mtPixy::image_load ( fn );
	if ( ! ni )
	{
		return 1;
	}

	if (	ni->get_type () != mtPixy::Image::INDEXED	||
		ni->get_width () % PATTERN_SIZE != 0		||
		ni->get_height () % PATTERN_SIZE != 0
		)
	{
		delete ni;
		return 1;
	}

	delete m_patterns;
	m_patterns = ni;
	m_pattern_num = 0;

	rebuild_pattern_mask ();

	return 0;
}

void mtPixy::Brush::set_color_ab (
	unsigned char		const	idx_a,
	unsigned char		const	idx_b,
	mtPixy::Color	const * const	col
	)
{
	m_index_a = idx_a;
	m_color_a.red = col[idx_a].red;
	m_color_a.green = col[idx_a].green;
	m_color_a.blue = col[idx_a].blue;

	m_index_b = idx_b;
	m_color_b.red = col[idx_b].red;
	m_color_b.green = col[idx_b].green;
	m_color_b.blue = col[idx_b].blue;

	rebuild_pattern_mask ();
}

void mtPixy::Brush::set_color_a (
	unsigned char		const	idx,
	mtPixy::Color	const * const	col
	)
{
	m_index_a = idx;
	m_color_a.red = col[idx].red;
	m_color_a.green = col[idx].green;
	m_color_a.blue = col[idx].blue;

	rebuild_pattern_mask ();
}

void mtPixy::Brush::set_color_b (
	unsigned char		const	idx,
	mtPixy::Color	const * const	col
	)
{
	m_index_b = idx;
	m_color_b.red = col[idx].red;
	m_color_b.green = col[idx].green;
	m_color_b.blue = col[idx].blue;

	rebuild_pattern_mask ();
}

int mtPixy::Brush::set_shape (
	int	const	num
	)
{
	if ( ! m_shapes )
	{
		return 1;
	}


	int	const	totx = m_shapes->get_width() / SHAPE_SIZE;
	int	const	toty = m_shapes->get_height() / SHAPE_SIZE;
	int	const	mx = totx * toty - 1;


	m_shape_num = MAX ( 0, num );
	m_shape_num = MIN ( m_shape_num, mx );

	rebuild_shape_mask ();

	return 0;
}

int mtPixy::Brush::set_shape (
	int	const	x,
	int	const	y
	)
{
	if ( ! m_shapes )
	{
		return 1;
	}


	int	const	totx = m_shapes->get_width() / SHAPE_SIZE;
	int	const	nx = ((x / m_shapes_palette_zoom) - SHAPE_PAD / 2 ) /
				(SHAPE_SIZE + SHAPE_PAD);
	int	const	ny = ((y / m_shapes_palette_zoom) - SHAPE_PAD / 2 ) /
				(SHAPE_SIZE + SHAPE_PAD);
	int	const	n = nx + ny * totx;


	return set_shape ( n );
}

int mtPixy::Brush::set_pattern (
	int	const	num
	)
{
	if ( ! m_patterns )
	{
		return 1;
	}

	int	const	totx = m_patterns->get_width() / PATTERN_SIZE;
	int	const	toty = m_patterns->get_height() / PATTERN_SIZE;
	int	const	mx = totx * toty - 1;

	m_pattern_num = MAX ( 0, num );
	m_pattern_num = MIN ( m_pattern_num, mx );

	rebuild_pattern_mask ();

	return 0;
}

int mtPixy::Brush::set_pattern (
	int	const	x,
	int	const	y
	)
{
	if ( ! m_patterns )
	{
		return 1;
	}


	int	const	totx = m_patterns->get_width() / PATTERN_SIZE;
	int	const	nx = ((x / m_pattern_palette_zoom) - SHAPE_PAD / 2 ) /
				(3 * PATTERN_SIZE + PATTERN_PAD);
	int	const	ny = ((y / m_pattern_palette_zoom) - SHAPE_PAD / 2 ) /
				(3 * PATTERN_SIZE + PATTERN_PAD);
	int	const	n = nx + ny * totx;


	return set_pattern ( n );
}

int mtPixy::Brush::get_spacing () const
{
	return m_spacing;
}

void mtPixy::Brush::set_spacing (
	int	const	n
	)
{
	m_spacing = MAX ( SPACING_MIN, MIN ( SPACING_MAX, n ) );
}

int mtPixy::Brush::get_space_mod () const
{
	return m_space_mod;
}

void mtPixy::Brush::set_space_mod (
	int	const	n
	)
{
	m_space_mod = MAX ( SPACING_MIN, MIN ( SPACING_MAX - 1, n ) );
}

int mtPixy::Brush::get_flow () const
{
	return m_flow;
}

void mtPixy::Brush::set_flow (
	int	const	n
	)
{
	m_flow = MAX ( FLOW_MIN, MIN ( FLOW_MAX, n ) );
}

static void idx_rgb_blit (
	unsigned char * const	dest,
	int		const	drow,
	unsigned char * const	src,
	int		const	srow,
	int		const	pxtot,
	int		const	scale
	)
{
	int		a, x, y;
	unsigned char	* s, * d;


	for ( a = 0; a < scale * scale; a++ )
	{
		for ( y = 0; y < pxtot; y++ )
		{
			s = src + srow * y;
			d = dest + drow * y;
			d += (a % scale) * pxtot * 3;
			d += (a / scale) * drow * pxtot;

			for ( x = 0; x < pxtot; x++ )
			{
				if ( 1 == *s++ )
				{
					*d++ = 255;
					*d++ = 255;
					*d++ = 255;
				}
				else
				{
					*d++ = 0;
					*d++ = 0;
					*d++ = 0;
				}
			}
		}
	}
}

static mtPixy::Image * enlarge_image (
	int		const	zoom,
	mtPixy::Image	* const	i,
	int		const	w,
	int		const	h
	)
{
	if ( zoom <= 1 )
	{
		return i;
	}


	mtPixy::Image	* iz;


	iz = i->scale ( w * zoom, h * zoom, mtPixy::Image::BLOCKY );
	delete i;

	return iz;
}

int mtPixy::Brush::rebuild_shapes_palette (
	int	const	zoom
	)
{
	if ( ! m_shapes || zoom < 1 )
	{
		return 1;
	}


	int	const	sw = m_shapes->get_width ();
	int	const	sh = m_shapes->get_height ();
	int	const	swtot = sw / SHAPE_SIZE;
	int	const	shtot = sh / SHAPE_SIZE;
	int	const	stot = swtot * shtot;
	int	const	iw = swtot * (SHAPE_SIZE + SHAPE_PAD);
	int	const	ih = shtot * (SHAPE_SIZE + SHAPE_PAD);
	mtPixy::Image	* i;


	i = mtPixy::image_create ( mtPixy::Image::RGB, iw, ih );
	if ( ! i )
	{
		return 1;
	}


	unsigned char * const	src = m_shapes->get_canvas ();
	unsigned char * const	dest = i->get_canvas ();
	unsigned char		* s, * d;
	int			a;


	if ( ! src || ! dest )
	{
		delete i;
		return 1;
	}

	for ( a = 0; a < stot; a++ )
	{
		s = src;
		s += SHAPE_SIZE * (a % swtot);
		s += SHAPE_SIZE * (a / swtot) * sw;

		d = dest + 3 * (SHAPE_PAD / 2) + 3 * (SHAPE_PAD / 2) * iw;
		d += 3 * (SHAPE_SIZE + SHAPE_PAD) * (a % swtot);
		d += 3 * (SHAPE_SIZE + SHAPE_PAD) * (a / swtot) * iw;

		idx_rgb_blit ( d, 3 * iw, s, sw, SHAPE_SIZE, 1 );
	}

	i = enlarge_image ( zoom, i, iw, ih );
	if ( ! i )
	{
		return 1;
	}

	delete m_shapes_palette;
	m_shapes_palette = i;
	m_shapes_palette_zoom = zoom;

	return 0;
}

int mtPixy::Brush::rebuild_patterns_palette (
	int	const	zoom
	)
{
	if ( ! m_patterns || zoom < 1 )
	{
		return 1;
	}

	int	const	sw = m_patterns->get_width ();
	int	const	sh = m_patterns->get_height ();
	int	const	swtot = sw / PATTERN_SIZE;
	int	const	shtot = sh / PATTERN_SIZE;
	int	const	stot = swtot * shtot;
	int	const	iw = swtot * (3 * PATTERN_SIZE + PATTERN_PAD);
	int	const	ih = shtot * (3 * PATTERN_SIZE + PATTERN_PAD);
	mtPixy::Image	* i;


	i = mtPixy::image_create ( mtPixy::Image::RGB, iw, ih );
	if ( ! i )
	{
		return 1;
	}


	unsigned char * const	src = m_patterns->get_canvas ();
	unsigned char * const	dest = i->get_canvas ();
	unsigned char		* s, * d;
	int			a;


	if ( ! src || ! dest )
	{
		delete i;
		return 1;
	}

	for ( a = 0; a < stot; a++ )
	{
		s = src;
		s += PATTERN_SIZE * (a % swtot);
		s += PATTERN_SIZE * (a / swtot) * sw;

		d = dest + 3 * (PATTERN_PAD / 2) + 3 * (PATTERN_PAD / 2) * iw;
		d += 3 * (3 * PATTERN_SIZE + PATTERN_PAD) * (a % swtot);
		d += 3 * (3 * PATTERN_SIZE + PATTERN_PAD) * (a / swtot) * iw;

		idx_rgb_blit ( d, 3 * iw, s, sw, PATTERN_SIZE, 3 );
	}

	i = enlarge_image ( zoom, i, iw, ih );
	if ( ! i )
	{
		return 1;
	}

	delete m_patterns_palette;
	m_patterns_palette = i;
	m_pattern_palette_zoom = zoom;

	return 0;
}

void mtPixy::Brush::rebuild_shape_mask ()
{
	if ( ! m_shape_mask || ! m_shapes )
	{
		return;
	}


	unsigned char	* src, * dest, * s, * d;
	int		x, y, sw, dw;


	src = m_shapes->get_canvas ();
	dest = m_shape_mask->get_alpha ();

	if ( src && dest )
	{
		sw = m_shapes->get_width ();
		dw = m_shape_mask->get_width ();

		src += SHAPE_SIZE * (m_shape_num % (sw / SHAPE_SIZE));
		src += SHAPE_SIZE * (m_shape_num / (sw / SHAPE_SIZE)) * sw;

		for ( y = 0; y < SHAPE_SIZE; y++ )
		{
			s = src + sw * y;
			d = dest + dw * y;

			for ( x = 0; x < SHAPE_SIZE; x++ )
			{
				if ( 1 == *s++ )
				{
					*d++ = 255;
				}
				else
				{
					*d++ = 0;
				}
			}
		}
	}
}

void mtPixy::Brush::rebuild_pattern_mask ()
{
	if ( ! m_pattern_idx || ! m_pattern_rgb || ! m_patterns )
	{
		return;
	}


	unsigned char	* src, * dest, * s, * d;
	int		x, y, sw, dw;


	src = m_patterns->get_canvas ();
	dest = m_pattern_idx->get_canvas ();

	if ( src && dest )
	{
		sw = m_patterns->get_width ();
		dw = m_pattern_idx->get_width ();

		src += PATTERN_SIZE * (m_pattern_num % (sw / PATTERN_SIZE));
		src += PATTERN_SIZE * (m_pattern_num / (sw / PATTERN_SIZE)) *sw;

		for ( y = 0; y < PATTERN_SIZE; y++ )
		{
			s = src + sw * y;
			d = dest + dw * y;

			for ( x = 0; x < PATTERN_SIZE; x++ )
			{
				if ( 1 == *s++ )
				{
					*d++ = m_index_b;
				}
				else
				{
					*d++ = m_index_a;
				}
			}
		}
	}

	src = m_patterns->get_canvas ();
	dest = m_pattern_rgb->get_canvas ();

	if ( src && dest )
	{
		sw = m_patterns->get_width ();
		dw = m_pattern_rgb->get_width ();

		src += PATTERN_SIZE * (m_pattern_num % (sw / PATTERN_SIZE));
		src += PATTERN_SIZE * (m_pattern_num / (sw / PATTERN_SIZE)) *sw;

		for ( y = 0; y < PATTERN_SIZE; y++ )
		{
			s = src + sw * y;
			d = dest + dw * y * 3;

			for ( x = 0; x < PATTERN_SIZE; x++ )
			{
				if ( 1 == *s++ )
				{
					*d++ = m_color_b.red;
					*d++ = m_color_b.green;
					*d++ = m_color_b.blue;
				}
				else
				{
					*d++ = m_color_a.red;
					*d++ = m_color_a.green;
					*d++ = m_color_a.blue;
				}
			}
		}
	}
}

mtPixy::Image * mtPixy::Brush::build_color_swatch (
	int	const	zoom
	)
{
	mtPixy::Image	* i;
	int	const	w = zoom * 3 * mtPixy::Brush::PATTERN_SIZE;


	i = mtPixy::image_create ( mtPixy::Image::RGB, w, w );
	if ( ! i )
	{
		return NULL;
	}

	int			p;
	int		const	tot = w * (w / 2);
	unsigned char		* dest = i->get_canvas ();

	for ( p = 0; p < tot; p++ )
	{
		*dest++ = m_color_a.red;
		*dest++ = m_color_a.green;
		*dest++ = m_color_a.blue;
	}

	for ( p = 0; p < tot; p++ )
	{
		*dest++ = m_color_b.red;
		*dest++ = m_color_b.green;
		*dest++ = m_color_b.blue;
	}

	return i;
}

mtPixy::Image * mtPixy::Brush::build_shape_swatch (
	int	const	zoom
	)
{
	mtPixy::Image	* i;


	i = mtPixy::image_create ( mtPixy::Image::RGB, SHAPE_SIZE, SHAPE_SIZE );
	if ( ! i )
	{
		return NULL;
	}


	int		const	sw = m_shapes->get_width ();
	int		const	swtot = sw / SHAPE_SIZE;
	unsigned char * const	src = m_shapes->get_canvas ();
	unsigned char * const	dest = i->get_canvas ();
	unsigned char		* s;


	if ( ! src || ! dest )
	{
		delete i;
		return NULL;
	}

	s = src;
	s += SHAPE_SIZE * (m_shape_num % swtot);
	s += SHAPE_SIZE * (m_shape_num / swtot) * sw;

	idx_rgb_blit ( dest, 3 * SHAPE_SIZE, s, sw, SHAPE_SIZE, 1 );

	return enlarge_image ( zoom, i, i->get_width (), i->get_height () );
}

static mtPixy::Image * prepare_pattern (
	mtPixy::Image * const	patterns,
	int		const	pattern_num
	)
{
	mtPixy::Image	* i;
	int	const	w = mtPixy::Brush::PATTERN_SIZE;


	i = mtPixy::image_create ( mtPixy::Image::RGB, 3 * w, 3 * w );
	if ( ! i )
	{
		return NULL;
	}


	int		const	sw = patterns->get_width ();
	int		const	swtot = sw / w;
	unsigned char * const	src = patterns->get_canvas ();
	unsigned char * const	dest = i->get_canvas ();
	unsigned char		* s;


	if ( ! src || ! dest )
	{
		delete i;
		return NULL;
	}

	s = src;
	s += w * (pattern_num % swtot);
	s += w * (pattern_num / swtot) * sw;

	idx_rgb_blit ( dest, 9 * w, s, sw, w, 3 );

	return i;
}

mtPixy::Image * mtPixy::Brush::build_pattern_swatch (
	int	const	zoom
	)
{
	mtPixy::Image	* i = prepare_pattern ( m_patterns, m_pattern_num );


	if ( ! i )
	{
		return NULL;
	}

	return enlarge_image ( zoom, i, i->get_width (), i->get_height () );
}

mtPixy::Image * mtPixy::Brush::build_preview_swatch (
	int	const	zoom
	)
{
	mtPixy::Image	* i;
	int	const	w = zoom * 3 * mtPixy::Brush::PATTERN_SIZE;


	i = mtPixy::image_create ( mtPixy::Image::RGB, w, w );
	if ( ! i )
	{
		return NULL;
	}

	i->paint_canvas_rectangle ( *this, 0, 0, w, w );

	return i;
}

mtPixy::Image * mtPixy::Brush::get_shape_mask ()
{
	return m_shape_mask;
}

mtPixy::Image * mtPixy::Brush::get_pattern_idx ()
{
	return m_pattern_idx;
}

mtPixy::Image * mtPixy::Brush::get_pattern_rgb ()
{
	return m_pattern_rgb;
}

mtPixy::Image * mtPixy::Brush::get_shapes_palette ()
{
	return m_shapes_palette;
}

mtPixy::Image * mtPixy::Brush::get_patterns_palette ()
{
	return m_patterns_palette;
}

mtPixy::Color mtPixy::Brush::get_color_a () const
{
	return m_color_a;
}

unsigned char mtPixy::Brush::get_color_a_index () const
{
	return m_index_a;
}

unsigned char mtPixy::Brush::get_color_b_index () const
{
	return m_index_b;
}

void mtPixy::Brush::render_cursor (
	int		const	cx,
	int		const	cy,
	unsigned char	const	opacity,
	unsigned char	* const	dest,
	int		const	ox,
	int		const	oy,
	int		const	w,
	int		const	h,
	int		const	zs
	)
{
	if ( ! m_shape_mask || ! m_pattern_rgb )
	{
		return;
	}

	unsigned char	* alpha = m_shape_mask->get_alpha ();
	unsigned char	* canvas = m_pattern_rgb->get_canvas ();

	if ( ! alpha || ! canvas )
	{
		return;
	}

	int	const	bw = m_shape_mask->get_width ();
	int	const	bh = m_shape_mask->get_height ();
	int	const	x = zs < 0 ?
				((cx - bw/2) / -zs) - ox :
				((cx - bw/2) * zs) - ox;
	int	const	y = zs < 0 ?
				((cy - bh/2) / -zs) - oy :
				((cy - bh/2) * zs) - oy;
	int	const	dx1 = x < 0 ? 0 : x;
	int	const	dy1 = y < 0 ? 0 : y;
	int	const	dx2 = zs < 0 ?
				MIN ( w, x + bw / -zs ) :
				MIN ( w, x + bw * zs );
	int	const	dy2 = zs < 0 ?
				MIN ( h, y + bh / -zs ) :
				MIN ( h, y + bh * zs );
	int	const	sox = x < 0 ? -x : 0;
	int	const	soy = y < 0 ? -y : 0;

	unsigned char	const	* s;
	unsigned char	const	* a;
	unsigned char		* d;


	if ( dx1 >= dx2 || dy1 >= dy2 )
	{
		return;
	}

	if ( zs < 0 )
	{
		for ( int dy = dy1; dy < dy2; dy++ )
		{
			s = canvas + 24 * ( ((dy + oy) * -zs) % 8 );

			a = alpha + ((dy - dy1 + soy) * -zs) * bw + sox * -zs;
			d = dest + 3 * (dy * w + dx1);

			for ( int dx = dx1; dx < dx2; dx++ )
			{
				if ( a[0] > 0 )
				{
					unsigned char const * so = s + 3 *
						( ((dx + ox) * -zs) % 8 );

					int const p = opacity;
					int const pp = 255 - p;

					d[0] = (unsigned char)
						((p * so[0] + pp * d[0]) / 255);
					d[1] = (unsigned char)
						((p * so[1] + pp * d[1]) / 255);
					d[2] = (unsigned char)
						((p * so[2] + pp * d[2]) / 255);

				}

				a += -zs;
				d += 3;
			}
		}
	}
	else if ( zs > 0 )
	{
		for ( int dy = dy1; dy < dy2; dy++ )
		{
			s = canvas + 24 * ( ((dy + oy) / zs) % 8 );
			a = alpha + ((dy - dy1 + soy) / zs) * bw;
			d = dest + 3 * (dy * w + dx1);

			for ( int dx = dx1; dx < dx2; dx++ )
			{
				int const of = (dx - dx1 + sox) / zs;

				if ( a[ of ] > 0 )
				{
					unsigned char const * so = s + 3 *
						( ((dx + ox) / zs) % 8 );

					int const p = opacity;
					int const pp = 255 - p;

					d[0] = (unsigned char)
						((p * so[0] + pp * d[0]) / 255);
					d[1] = (unsigned char)
						((p * so[1] + pp * d[1]) / 255);
					d[2] = (unsigned char)
						((p * so[2] + pp * d[2]) / 255);
				}

				d += 3;
			}
		}
	}
}

