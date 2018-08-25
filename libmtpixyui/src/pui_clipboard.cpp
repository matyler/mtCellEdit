/*
	Copyright (C) 2016-2018 Mark Tyler

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



mtPixyUI::Clipboard::Clipboard ()
	:
	m_image		( NULL ),
	m_x		( 0 ),
	m_y		( 0 ),
	m_filename	( NULL ),
	m_text_paste	( false )
{
	mtString * mts = mtkit_string_new ( mtkit_file_home () );

	if ( ! mts )
	{
		return;
	}

	mtkit_string_append ( mts, "/.cache" );
	mkdir ( mtkit_string_get_buf ( mts ), S_IRWXU | S_IRWXG | S_IRWXO );

	mtkit_string_append ( mts, "/" APP_NAME );
	mkdir ( mtkit_string_get_buf ( mts ), S_IRWXU | S_IRWXG | S_IRWXO );

	mtkit_string_append ( mts, "/clipboard_" );

	m_filename = mtkit_string_destroy_get_buf ( mts );
	mts = NULL;
}

mtPixyUI::Clipboard::~Clipboard ()
{
	delete m_image;
	m_image = NULL;

	free ( m_filename );
	m_filename = NULL;
}

char * mtPixyUI::Clipboard::create_filename (
	int	const	n
	)
{
	char	buf[16];

	snprintf ( buf, sizeof(buf), "%i", n );

	return mtkit_string_join ( m_filename, buf, ".png", NULL );
}

int mtPixyUI::Clipboard::load (
	int	const	n
	)
{
	char * filename = create_filename ( n );

	mtPixy::Image * im = mtPixy::Image::load ( filename );

	free ( filename );
	filename = NULL;

	if ( ! im )
	{
		return 1;
	}

	if ( set_image ( im, 0, 0 ) )
	{
		delete im;
		im = NULL;

		return 1;
	}

	return 0;
}

int mtPixyUI::Clipboard::save (
	int	const	n
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	char * filename = create_filename ( n );

	int const res = m_image->save_png ( filename, 5 );

	free ( filename );
	filename = NULL;

	return res;
}

int mtPixyUI::Clipboard::set_image (
	mtPixy::Image	* const	im,
	int		const	x,
	int		const	y,
	bool		const	txt
	)
{
	if ( ! im )
	{
		return 1;
	}

	delete m_image;
	m_image = im;
	m_text_paste = txt;

	m_x = x;
	m_y = y;

	return 0;
}

void mtPixyUI::Clipboard::get_xy (
	int		&x,
	int		&y
	) const
{
	x = m_x;
	y = m_y;
}

mtPixy::Image * mtPixyUI::Clipboard::get_image ()
{
	return m_image;
}

void mtPixyUI::Clipboard::render (
	mtPixy::Color	const * const	pal,
	mtPixy::RecSelOverlay	const	&ovl,
	unsigned char		* const	rgb,
	int			const	x,
	int			const	y,
	int			const	w,
	int			const	h,
	int			const	zs
	)
{
	if ( ! m_image )
	{
		return;
	}

	int px = ovl.get_x1 ();
	int py = ovl.get_y1 ();

	if ( zs < 0 )
	{
		px /= -zs;
		py /= -zs;
	}
	else if ( zs > 1 )
	{
		px *= zs;
		py *= zs;
	}

	px -= x;
	py -= y;

	m_image->blit_rgb_alpha_blend ( pal, rgb, px, py, w, h, zs );
}

int mtPixyUI::Clipboard::paste (
	File		&file,
	int	const	x,
	int	const	y
	)
{
	mtPixy::Image	* dest = file.get_image ();

	if ( ! dest || ! m_image )
	{
		return 1;
	}

	if ( dest->paste_alpha_blend ( m_image, x, y ) )
	{
		return 1;
	}

	return 0;
}

int mtPixyUI::Clipboard::paste (
	File		&file,
	int	const	x,
	int	const	y,
	int		&dirty_x,
	int		&dirty_y,
	int		&dirty_w,
	int		&dirty_h
	)
{
	if ( paste ( file, x, y ) )
	{
		return 1;
	}

	dirty_x = x;
	dirty_y = y;
	dirty_w = m_image->get_width ();
	dirty_h = m_image->get_height ();

	return 0;
}

int mtPixyUI::Clipboard::lasso (
	int	const	x,
	int	const	y
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	if ( m_image->lasso ( x, y ) )
	{
		return 1;
	}

	int		minx, miny;
	mtPixy::Image	* nim = m_image->resize_trim_by_alpha ( minx, miny );

	if ( nim )
	{
		delete m_image;
		m_image = nim;

		m_x += minx;
		m_y += miny;
	}

	return 0;
}

int mtPixyUI::Clipboard::flip_vertical ()
{
	if ( m_image )
	{
		return set_image ( m_image->flip_vertically (), m_x, m_y );
	}

	return 1;
}

int mtPixyUI::Clipboard::flip_horizontal ()
{
	if ( m_image )
	{
		return set_image ( m_image->flip_horizontally (), m_x, m_y );
	}

	return 1;
}

int mtPixyUI::Clipboard::rotate_clockwise ()
{
	if ( m_image )
	{
		return set_image ( m_image->rotate_clockwise (), m_x, m_y );
	}

	return 1;
}

int mtPixyUI::Clipboard::rotate_anticlockwise ()
{
	if ( m_image )
	{
		return set_image ( m_image->rotate_anticlockwise (), m_x, m_y );
	}

	return 1;
}

bool mtPixyUI::Clipboard::is_text_paste () const
{
	return m_text_paste;
}

