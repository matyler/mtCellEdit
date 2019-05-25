/*
	Copyright (C) 2016-2019 Mark Tyler

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
	m_x		( 0 ),
	m_y		( 0 ),
	m_path		( mtkit_file_home () ),
	m_text_paste	( false )
{
	m_path += "/.cache";
	mtkit_mkdir ( m_path.c_str () );

	m_path += "/" APP_NAME;
	mtkit_mkdir ( m_path.c_str () );

	m_path += "/clipboard_";
}

mtPixyUI::Clipboard::~Clipboard ()
{
}

std::string mtPixyUI::Clipboard::create_filename (
	int	const	n
	) const
{
	char	buf[16];

	snprintf ( buf, sizeof(buf), "%i", n );

	return m_path + buf + ".png";
}

int mtPixyUI::Clipboard::load (
	int	const	n
	)
{
	std::string filename = create_filename ( n );

	return set_image ( mtPixy::Image::load ( filename.c_str () ), 0, 0 );
}

int mtPixyUI::Clipboard::save (
	int	const	n
	)
{
	if ( ! m_img.get () )
	{
		return 1;
	}

	std::string filename = create_filename ( n );

	return m_img.get ()->save_png ( filename.c_str (), 5 );
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

	m_img.reset ( im );
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
	return m_img.get ();
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
	if ( ! m_img.get () )
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

	m_img.get ()->blit_rgb_alpha_blend ( pal, rgb, px, py, w, h, zs );
}

int mtPixyUI::Clipboard::paste (
	File		&file,
	int	const	x,
	int	const	y
	)
{
	mtPixy::Image	* dest = file.get_image ();

	if ( ! dest || ! m_img.get () )
	{
		return 1;
	}

	if ( dest->paste_alpha_blend ( m_img.get (), x, y ) )
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
	dirty_w = m_img.get ()->get_width ();
	dirty_h = m_img.get ()->get_height ();

	return 0;
}

int mtPixyUI::Clipboard::lasso (
	int	const	x,
	int	const	y
	)
{
	if ( ! m_img.get () )
	{
		return 1;
	}

	if ( m_img.get ()->lasso ( x, y ) )
	{
		return 1;
	}

	int		minx, miny;
	mtPixy::Image	* nim = m_img.get ()->resize_trim_by_alpha (minx, miny);

	if ( nim )
	{
		m_img.reset ( nim );

		m_x += minx;
		m_y += miny;
	}

	return 0;
}

int mtPixyUI::Clipboard::flip_vertical ()
{
	if ( m_img.get () )
	{
		return set_image ( m_img.get ()->flip_vertically (), m_x, m_y );
	}

	return 1;
}

int mtPixyUI::Clipboard::flip_horizontal ()
{
	if ( m_img.get () )
	{
		return set_image ( m_img.get ()->flip_horizontally(), m_x, m_y);
	}

	return 1;
}

int mtPixyUI::Clipboard::rotate_clockwise ()
{
	if ( m_img.get () )
	{
		return set_image ( m_img.get ()->rotate_clockwise (), m_x, m_y);
	}

	return 1;
}

int mtPixyUI::Clipboard::rotate_anticlockwise ()
{
	if ( m_img.get () )
	{
		return set_image ( m_img.get ()->rotate_anticlockwise (), m_x,
			m_y );
	}

	return 1;
}

bool mtPixyUI::Clipboard::is_text_paste () const
{
	return m_text_paste;
}

