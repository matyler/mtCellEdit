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



mtPixy::Image * mtPixy::text_render_preview (
	Image::Type		const	type,
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

	Image * im = font.render_image ( utf8, 0 );
	if ( ! im )
	{
		return NULL;
	}

	if ( im->create_rgb_canvas () )
	{
		delete im;
		return NULL;
	}

	unsigned char * alp = im->get_alpha ();
	unsigned char * can = im->get_canvas ();
	size_t	const	pixtot = (size_t)( im->get_width() * im->get_height() );

	if ( ! alp || ! can )
	{
		delete im;
		return NULL;
	}

	memset ( can, 255, 3 * pixtot );

	switch ( type )
	{
	case Image::INDEXED:
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

	case Image::RGB:
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

	im->destroy_alpha ();

	return im;
}

mtPixy::Image * mtPixy::text_render_paste (
	Image::Type		const	type,
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

	Image * im = font.render_image ( utf8, 0 );
	if ( ! im )
	{
		return NULL;
	}

	switch ( type )
	{
	case Image::INDEXED:
		if ( im->create_indexed_canvas () )
		{
			delete im;
			return NULL;
		}
		break;

	case Image::RGB:
		if ( im->create_rgb_canvas () )
		{
			delete im;
			return NULL;
		}
		break;

	default:
		break;
	}

	im->paint_canvas_rectangle ( bru, 0, 0, im->get_width (),
		im->get_height () );

	return im;
}

