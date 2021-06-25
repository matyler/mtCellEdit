/*
	Copyright (C) 2016-2020 Mark Tyler

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



mtPixmap * mtPixy::text_render_preview_pixmap (
	int			const	bpp,
	char		const * const	utf8,
	char		const * const	font_name,
	int			const	size,
	int			const	bold,
	int			const	italics,
	Font::StyleUnderline	const	underline,
	int			const	strikethrough
	)
{
	Font		font ( font_name, size );

	font.set_style ( bold, italics, underline, strikethrough );

	mtPixmap * const im = font.render_pixmap ( utf8, 0 );
	if ( ! im )
	{
		return NULL;
	}

	mtPixy::Pixmap pixmap ( im );	// Auto cleanup on leaving scope

	if ( pixy_pixmap_create_rgb_canvas ( im ) )
	{
		return NULL;
	}

	unsigned char * alp = im->alpha;
	unsigned char * can = im->canvas;
	size_t	const	pixtot = (size_t)( im->width * im->height );

	if ( ! alp || ! can )
	{
		return NULL;
	}

	memset ( can, 255, 3 * pixtot );

	switch ( bpp )
	{
	case PIXY_PIXMAP_BPP_INDEXED:
		for ( size_t i = 0; i < pixtot; i++ )
		{
			if ( alp[i] > 127 )
			{
				can[ 3*i + 0 ] = 0;
				can[ 3*i + 1 ] = 0;
				can[ 3*i + 2 ] = 0;
			}
		}
		break;

	case PIXY_PIXMAP_BPP_RGB:
		for ( size_t i = 0; i < pixtot; i++ )
		{
			can[ 3*i + 0 ] = (unsigned char)(255 - alp[ i ]);
			can[ 3*i + 1 ] = (unsigned char)(255 - alp[ i ]);
			can[ 3*i + 2 ] = (unsigned char)(255 - alp[ i ]);
		}
		break;

	default:
		break;
	}

	pixy_pixmap_destroy_alpha ( im );

	return pixmap.release();
}

mtPixmap * mtPixy::text_render_paste (
	int			const	bpp,
	Brush				&bru,
	char		const * const	utf8,
	char		const * const	font_name,
	int			const	size,
	int			const	bold,
	int			const	italics,
	Font::StyleUnderline	const	underline,
	int			const	strikethrough
	)
{
	Font		font ( font_name, size );

	font.set_style ( bold, italics, underline, strikethrough );

	mtPixy::Pixmap pixmap ( font.render_pixmap ( utf8, 0 ) );
	if ( ! pixmap.get() )
	{
		return NULL;
	}

	switch ( bpp )
	{
	case PIXY_PIXMAP_BPP_INDEXED:
		if ( pixy_pixmap_create_indexed_canvas ( pixmap.get() ) )
		{
			return NULL;
		}
		break;

	case PIXY_PIXMAP_BPP_RGB:
		if ( pixy_pixmap_create_rgb_canvas ( pixmap.get() ) )
		{
			return NULL;
		}
		break;

	default:
		break;
	}

	bru.paint_canvas_rectangle ( pixmap.get(), 0, 0,
		pixy_pixmap_get_width ( pixmap.get() ),
		pixy_pixmap_get_height ( pixmap.get() ) );

	return pixmap.release();
}

