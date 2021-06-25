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



mtPixy::Brush::Brush ()
	:
	m_shape_mask	( pixy_pixmap_new_alpha ( SHAPE_SIZE, SHAPE_SIZE ) ),
	m_pattern_idx	( pixy_pixmap_new_indexed( PATTERN_SIZE, PATTERN_SIZE)),
	m_pattern_rgb	( pixy_pixmap_new_rgb ( PATTERN_SIZE, PATTERN_SIZE ) )
{
}

mtPixy::Brush::~Brush ()
{
}

int mtPixy::Brush::load_shapes (
	char	const * const	fn
	)
{
	mtPixy::Pixmap npix ( pixy_pixmap_load ( fn, nullptr ) );
	if ( ! npix.get() )
	{
		return 1;
	}

	if (	npix.get()->bpp != PIXY_PIXMAP_BPP_INDEXED	||
		npix.get()->width % SHAPE_SIZE != 0		||
		npix.get()->height % SHAPE_SIZE != 0
		)
	{
		return 1;
	}

	m_shapes.reset ( npix.release() );
	m_shape_num = 0;

	rebuild_shape_mask ();

	return 0;
}

int mtPixy::Brush::load_patterns (
	char	const * const	fn
	)
{
	mtPixy::Pixmap npix ( pixy_pixmap_load ( fn, nullptr ) );
	if ( ! npix.get() )
	{
		return 1;
	}

	if (	npix.get()->bpp != PIXY_PIXMAP_BPP_INDEXED	||
		npix.get()->width % PATTERN_SIZE != 0		||
		npix.get()->height % PATTERN_SIZE != 0
		)
	{
		return 1;
	}

	m_patterns.reset ( npix.release() );
	m_pattern_num = 0;

	rebuild_pattern_mask ();

	return 0;
}

void mtPixy::Brush::set_color_ab (
	unsigned char		const	idx_a,
	unsigned char		const	idx_b,
	mtColor		const * const	col
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
	mtColor		const * const	col
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
	mtColor		const * const	col
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
	if ( ! m_shapes.get() )
	{
		return 1;
	}

	int	const	totx = pixy_pixmap_get_width ( m_shapes.get() ) /
				SHAPE_SIZE;
	int	const	toty = pixy_pixmap_get_height ( m_shapes.get() ) /
				SHAPE_SIZE;
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
	if ( ! m_shapes.get() )
	{
		return 1;
	}

	int	const	totx = pixy_pixmap_get_width ( m_shapes.get() ) /
				SHAPE_SIZE;
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
	if ( ! m_patterns.get() )
	{
		return 1;
	}

	int	const	totx = pixy_pixmap_get_width ( m_patterns.get() ) /
				PATTERN_SIZE;
	int	const	toty = pixy_pixmap_get_height ( m_patterns.get() ) /
				PATTERN_SIZE;
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
	if ( ! m_patterns.get() )
	{
		return 1;
	}

	int	const	totx = pixy_pixmap_get_width ( m_patterns.get()) /
				PATTERN_SIZE;
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
	unsigned char		* const	dest,
	int			const	drow,
	unsigned char	const * const	src,
	int			const	srow,
	int			const	pxtot,
	int			const	scale
	)
{
	for ( int a = 0; a < scale * scale; a++ )
	{
		for ( int y = 0; y < pxtot; y++ )
		{
			unsigned char const * s = src + srow * y;
			unsigned char * d = dest + drow * y +
				(a % scale) * pxtot * 3 +
				(a / scale) * drow * pxtot;

			for ( int x = 0; x < pxtot; x++ )
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

static void enlarge_image (
	int	const	zoom,
	mtPixy::Pixmap	&i,
	int	const	w,
	int	const	h
	)
{
	if ( zoom <= 1 )
	{
		return;
	}

	i.reset ( pixy_pixmap_scale ( i.get(), w * zoom, h * zoom,
		PIXY_SCALE_BLOCKY ) );
}

int mtPixy::Brush::rebuild_shapes_palette (
	int	const	zoom
	)
{
	if ( zoom < 1 )
	{
		return 1;
	}

	int	const	sw = pixy_pixmap_get_width ( m_shapes.get() );
	int	const	sh = pixy_pixmap_get_height ( m_shapes.get() );
	int	const	swtot = sw / SHAPE_SIZE;
	int	const	shtot = sh / SHAPE_SIZE;
	int	const	stot = swtot * shtot;
	int	const	iw = swtot * (SHAPE_SIZE + SHAPE_PAD);
	int	const	ih = shtot * (SHAPE_SIZE + SHAPE_PAD);

	mtPixy::Pixmap	i ( pixy_pixmap_new_rgb ( iw, ih ) );
	unsigned char const * const src =pixy_pixmap_get_canvas(m_shapes.get());
	unsigned char * const	dest = pixy_pixmap_get_canvas ( i.get() );

	if ( ! src || ! dest )
	{
		return 1;
	}

	for ( int a = 0; a < stot; a++ )
	{
		unsigned char const * const s = src +
			SHAPE_SIZE * (a % swtot) +
			SHAPE_SIZE * (a / swtot) * sw;

		unsigned char * const d = dest +
			3 * (SHAPE_PAD / 2) +
			3 * (SHAPE_PAD / 2) * iw +
			3 * (SHAPE_SIZE + SHAPE_PAD) * (a % swtot) +
			3 * (SHAPE_SIZE + SHAPE_PAD) * (a / swtot) * iw;

		idx_rgb_blit ( d, 3 * iw, s, sw, SHAPE_SIZE, 1 );
	}

	enlarge_image ( zoom, i, iw, ih );
	if ( ! i.get() )
	{
		return 1;
	}

	m_shapes_palette.reset ( i.release() );
	m_shapes_palette_zoom = zoom;

	return 0;
}

int mtPixy::Brush::rebuild_patterns_palette (
	int	const	zoom
	)
{
	if ( zoom < 1 )
	{
		return 1;
	}

	int	const	sw = pixy_pixmap_get_width ( m_patterns.get() );
	int	const	sh = pixy_pixmap_get_height ( m_patterns.get() );
	int	const	swtot = sw / PATTERN_SIZE;
	int	const	shtot = sh / PATTERN_SIZE;
	int	const	stot = swtot * shtot;
	int	const	iw = swtot * (3 * PATTERN_SIZE + PATTERN_PAD);
	int	const	ih = shtot * (3 * PATTERN_SIZE + PATTERN_PAD);

	mtPixy::Pixmap	i ( pixy_pixmap_new_rgb ( iw, ih ) );
	unsigned char const * const src = pixy_pixmap_get_canvas (
		m_patterns.get() );
	unsigned char * const	dest = pixy_pixmap_get_canvas ( i.get() );

	if ( ! src || ! dest )
	{
		return 1;
	}

	for ( int a = 0; a < stot; a++ )
	{
		unsigned char const * const s = src
			+ PATTERN_SIZE * (a % swtot)
			+ PATTERN_SIZE * (a / swtot) * sw;

		unsigned char * d = dest +
			3 * (PATTERN_PAD / 2) +
			3 * (PATTERN_PAD / 2) * iw +
			3 * (3 * PATTERN_SIZE + PATTERN_PAD) * (a % swtot) +
			3 * (3 * PATTERN_SIZE + PATTERN_PAD) * (a / swtot) * iw;

		idx_rgb_blit ( d, 3 * iw, s, sw, PATTERN_SIZE, 3 );
	}

	enlarge_image ( zoom, i, iw, ih );
	if ( ! i.get() )
	{
		return 1;
	}

	m_patterns_palette.reset ( i.release() );
	m_pattern_palette_zoom = zoom;

	return 0;
}

void mtPixy::Brush::rebuild_shape_mask ()
{
	unsigned char const * src = pixy_pixmap_get_canvas (
		m_shapes.get () );

	unsigned char * const dest = pixy_pixmap_get_alpha (
		m_shape_mask.get () );

	if ( src && dest )
	{
		int const sw = pixy_pixmap_get_width ( m_shapes.get () );
		int const dw = pixy_pixmap_get_width ( m_shape_mask.get () );

		src += SHAPE_SIZE * (m_shape_num % (sw / SHAPE_SIZE));
		src += SHAPE_SIZE * (m_shape_num / (sw / SHAPE_SIZE)) * sw;

		for ( int y = 0; y < SHAPE_SIZE; y++ )
		{
			unsigned char const * s = src + sw * y;
			unsigned char * d = dest + dw * y;

			for ( int x = 0; x < SHAPE_SIZE; x++ )
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
	unsigned char const * src = pixy_pixmap_get_canvas (
		m_patterns.get ());

	unsigned char * dest = pixy_pixmap_get_canvas (
		m_pattern_idx.get () );

	if ( src && dest )
	{
		int const sw = pixy_pixmap_get_width ( m_patterns.get () );
		int const dw = pixy_pixmap_get_width ( m_pattern_idx.get () );

		src += PATTERN_SIZE * (m_pattern_num % (sw / PATTERN_SIZE));
		src += PATTERN_SIZE * (m_pattern_num / (sw / PATTERN_SIZE)) *sw;

		for ( int y = 0; y < PATTERN_SIZE; y++ )
		{
			unsigned char const	* s = src + sw * y;
			unsigned char		* d = dest + dw * y;

			for ( int x = 0; x < PATTERN_SIZE; x++ )
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

	src = pixy_pixmap_get_canvas ( m_patterns.get() );
	dest = pixy_pixmap_get_canvas ( m_pattern_rgb.get() );

	if ( src && dest )
	{
		int const sw = pixy_pixmap_get_width ( m_patterns.get() );
		int const dw = pixy_pixmap_get_width ( m_pattern_rgb.get() );

		src += PATTERN_SIZE * (m_pattern_num % (sw / PATTERN_SIZE));
		src += PATTERN_SIZE * (m_pattern_num / (sw / PATTERN_SIZE)) *sw;

		for ( int y = 0; y < PATTERN_SIZE; y++ )
		{
			unsigned char const	* s = src + sw * y;
			unsigned char		* d = dest + dw * y * 3;

			for ( int x = 0; x < PATTERN_SIZE; x++ )
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

mtPixmap * mtPixy::Brush::build_color_swatch (
	int	const	zoom
	)
{
	int	const	w = zoom * 3 * PATTERN_SIZE;
	mtPixmap * const i = pixy_pixmap_new_rgb ( w, w );

	if ( ! i )
	{
		return NULL;
	}

	int		const	tot = w * (w / 2);
	unsigned char		* dest = pixy_pixmap_get_canvas ( i );

	for ( int p = 0; p < tot; p++ )
	{
		*dest++ = m_color_a.red;
		*dest++ = m_color_a.green;
		*dest++ = m_color_a.blue;
	}

	for ( int p = 0; p < tot; p++ )
	{
		*dest++ = m_color_b.red;
		*dest++ = m_color_b.green;
		*dest++ = m_color_b.blue;
	}

	return i;
}

mtPixmap * mtPixy::Brush::build_shape_swatch (
	int	const	zoom
	)
{
	mtPixy::Pixmap	 i ( pixy_pixmap_new_rgb ( SHAPE_SIZE, SHAPE_SIZE ) );
	int		const	sw = pixy_pixmap_get_width ( m_shapes.get() );
	int		const	swtot = sw / SHAPE_SIZE;
	unsigned char const * const src =pixy_pixmap_get_canvas(m_shapes.get());
	unsigned char * const	dest = pixy_pixmap_get_canvas ( i.get() );

	if ( ! src || ! dest )
	{
		return NULL;
	}

	unsigned char const * const s = src + SHAPE_SIZE * (m_shape_num % swtot)
		+ SHAPE_SIZE * (m_shape_num / swtot) * sw;

	idx_rgb_blit ( dest, 3 * SHAPE_SIZE, s, sw, SHAPE_SIZE, 1 );

	enlarge_image ( zoom, i, pixy_pixmap_get_width (i.get()),
		pixy_pixmap_get_width (i.get()) );

	return i.release();
}

static mtPixmap * prepare_pattern (
	mtPixmap	* const	patterns,
	int		const	pattern_num
	)
{
	int		const	w = mtPixy::Brush::PATTERN_SIZE;
	mtPixy::Pixmap		i ( pixy_pixmap_new_rgb ( 3 * w, 3 * w ) );
	int		const	sw = pixy_pixmap_get_width ( patterns );
	int		const	swtot = sw / w;
	unsigned char const * const src = pixy_pixmap_get_canvas ( patterns );
	unsigned char * const	dest = pixy_pixmap_get_canvas ( i.get() );

	if ( ! src || ! dest )
	{
		return NULL;
	}

	unsigned char const * const s = src + w * (pattern_num % swtot)
		+ w * (pattern_num / swtot) * sw;

	idx_rgb_blit ( dest, 9 * w, s, sw, w, 3 );

	return i.release();
}

mtPixmap * mtPixy::Brush::build_pattern_swatch (
	int	const	zoom
	)
{
	mtPixy::Pixmap i ( prepare_pattern ( m_patterns.get(), m_pattern_num ));
	if ( ! i.get() )
	{
		return NULL;
	}

	enlarge_image ( zoom, i, pixy_pixmap_get_width (i.get()),
		pixy_pixmap_get_height (i.get()) );

	return i.release();
}

mtPixmap * mtPixy::Brush::build_preview_swatch (
	int	const	zoom
	)
{
	int	const	w = zoom * 3 * PATTERN_SIZE;
	mtPixmap * const i = pixy_pixmap_new_rgb ( w, w );

	if ( ! i )
	{
		return NULL;
	}

	paint_canvas_rectangle ( i, 0, 0, w, w );

	return i;
}

mtPixmap * mtPixy::Brush::get_shape_mask ()
{
	return m_shape_mask.get();
}

mtPixmap * mtPixy::Brush::get_pattern_idx ()
{
	return m_pattern_idx.get();
}

mtPixmap * mtPixy::Brush::get_pattern_rgb ()
{
	return m_pattern_rgb.get();
}

mtPixmap * mtPixy::Brush::get_shapes_palette ()
{
	return m_shapes_palette.get();
}

mtPixmap * mtPixy::Brush::get_patterns_palette ()
{
	return m_patterns_palette.get();
}

mtColor mtPixy::Brush::get_color_a () const
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
	unsigned char const * const alpha = pixy_pixmap_get_alpha (
		m_shape_mask.get() );

	unsigned char const * const canvas = pixy_pixmap_get_canvas (
		m_pattern_rgb.get() );

	if ( ! alpha || ! canvas )
	{
		return;
	}

	int	const	bw = pixy_pixmap_get_width ( m_shape_mask.get () );
	int	const	bh = pixy_pixmap_get_height ( m_shape_mask.get () );
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

