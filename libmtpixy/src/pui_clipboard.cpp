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

	return set_pixmap ( pixy_pixmap_load ( filename.c_str(), NULL ), 0, 0 );
}

int mtPixyUI::Clipboard::save (
	int	const	n
	) const
{
	std::string filename = create_filename ( n );

	return pixy_pixmap_save_png ( get_pixmap(), filename.c_str (), 5 );
}

int mtPixyUI::Clipboard::set_pixmap (
	mtPixmap	* const	im,
	int		const	x,
	int		const	y,
	bool		const	txt
	)
{
	if ( ! im )
	{
		return 1;
	}

	m_pixmap.reset ( im );
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

void mtPixyUI::Clipboard::render (
	mtPalette	const * const	pal,
	mtPixy::RecSelOverlay	const	&ovl,
	unsigned char		* const	rgb,
	int			const	x,
	int			const	y,
	int			const	w,
	int			const	h,
	int			const	zs
	) const
{
	mtPixmap const * const pixmap = get_pixmap ();
	if ( ! pixmap )
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

	pixy_pixmap_blit_rgb_alpha_blend ( pixmap, pal, rgb, px, py, w, h, zs );
}

int mtPixyUI::Clipboard::paste (
	File	const	&file,
	int	const	x,
	int	const	y
	) const
{
	mtPixmap	* dest = file.get_pixmap ();
	mtPixmap const * const pixmap = get_pixmap ();

	if ( ! dest || ! pixmap )
	{
		return 1;
	}

	if ( pixy_pixmap_paste_alpha_blend ( dest, pixmap, x, y ) )
	{
		return 1;
	}

	return 0;
}

int mtPixyUI::Clipboard::paste (
	File	const	&file,
	int	const	x,
	int	const	y,
	int		&dirty_x,
	int		&dirty_y,
	int		&dirty_w,
	int		&dirty_h
	) const
{
	if ( paste ( file, x, y ) )
	{
		return 1;
	}

	mtPixmap const * const pixmap = get_pixmap ();

	dirty_x = x;
	dirty_y = y;
	dirty_w = pixmap->width;
	dirty_h = pixmap->height;

	return 0;
}

int mtPixyUI::Clipboard::lasso (
	int	const	x,
	int	const	y
	)
{
	mtPixmap * const pixmap = get_pixmap ();

	if ( ! pixmap )
	{
		return 1;
	}

	if ( pixy_lasso ( pixmap, x, y ) )
	{
		return 1;
	}

	int	minx, miny;
	mtPixmap * nim = pixy_pixmap_resize_trim_by_alpha(pixmap, &minx, &miny);

	if ( nim )
	{
		m_pixmap.reset ( nim );

		m_x += minx;
		m_y += miny;
	}

	return 0;
}

int mtPixyUI::Clipboard::flip_vertical ()
{
	mtPixmap const * const pixmap = get_pixmap ();

	if ( pixmap )
	{
		return set_pixmap ( pixy_pixmap_flip_vertically ( pixmap ), m_x,
			m_y );
	}

	return 1;
}

int mtPixyUI::Clipboard::flip_horizontal ()
{
	mtPixmap const * const pixmap = get_pixmap ();

	if ( pixmap )
	{
		return set_pixmap ( pixy_pixmap_flip_horizontally ( pixmap ),
			m_x, m_y );
	}

	return 1;
}

int mtPixyUI::Clipboard::rotate_clockwise ()
{
	mtPixmap const * const pixmap = get_pixmap ();

	if ( pixmap )
	{
		return set_pixmap ( pixy_pixmap_rotate_clockwise ( pixmap ),
			m_x, m_y );
	}

	return 1;
}

int mtPixyUI::Clipboard::rotate_anticlockwise ()
{
	mtPixmap const * const pixmap = get_pixmap ();

	if ( pixmap )
	{
		return set_pixmap ( pixy_pixmap_rotate_anticlockwise ( pixmap ),
			m_x, m_y );
	}

	return 1;
}

bool mtPixyUI::Clipboard::is_text_paste () const
{
	return m_text_paste;
}

