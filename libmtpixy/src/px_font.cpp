/*
	Copyright (C) 2004-2016 Mark Tyler

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

#include <pango/pangoft2.h>



namespace mtPixy
{

class FontData
{
public:
	FontData ( char	const * name );
	~FontData ();

/// ----------------------------------------------------------------------

	PangoFontDescription * m_font_desc;
	PangoFontMap	* m_font_map;
	PangoContext	* m_context;
	PangoLayout	* m_layout;
private:
};

}



mtPixy::FontData::FontData (
	char	const	* const	name
	)
	:
	m_font_desc	( pango_font_description_from_string ( name ) ),
	m_font_map	( pango_ft2_font_map_new () ),
	m_context	( pango_font_map_create_context ( m_font_map ) ),
	m_layout	( pango_layout_new ( m_context ) )
{
}

mtPixy::FontData::~FontData ()
{
	if ( m_layout )
	{
		g_object_unref ( m_layout );
		m_layout = NULL;
	}

	if ( m_context )
	{
		g_object_unref ( m_context );
		m_context = NULL;
	}

	if ( m_font_map )
	{
		g_object_unref ( m_font_map );
		m_font_map = NULL;
	}

	if ( m_font_desc )
	{
		pango_font_description_free ( m_font_desc );
		m_font_desc = NULL;
	}
}

mtPixy::Font::Font (
	char	const	* const	name,
	int		const	size
	)
	:
	m_height	( 0 ),
	m_width		( 0 ),
	m_baseline	( 0 ),
	m_style_bold	( 0 ),
	m_style_italics	( 0 ),
	m_style_underline ( STYLE_UNDERLINE_NONE ),
	m_style_strikethrough ( 0 ),
	m_style_row_pad	( 0 )
{
	m_font_data = new FontData ( name );

	set_size ( size );
}

mtPixy::Font::~Font ()
{
	delete m_font_data;
	m_font_data = NULL;
}

int mtPixy::Font::set_size (
	int	const	size
	)
{
	if ( size < 1 || ! m_font_data )
	{
		return 1;
	}


	PangoRectangle	logical;


	pango_layout_set_text ( m_font_data->m_layout, "01234567890", -1 );
	pango_font_description_set_weight ( m_font_data->m_font_desc,
		PANGO_WEIGHT_NORMAL );
	pango_font_description_set_size ( m_font_data->m_font_desc,
		(gint)( ( (double)size ) * PANGO_SCALE ) );
	pango_layout_set_font_description( m_font_data->m_layout,
		m_font_data->m_font_desc );

	pango_layout_get_extents ( m_font_data->m_layout, NULL, &logical );
	pango_extents_to_pixels ( NULL, &logical );

	m_width = logical.width / 11;
	m_height = logical.height;
	m_baseline = PANGO_PIXELS ( pango_layout_get_baseline (
		m_font_data->m_layout ) );

	return 0;
}

void mtPixy::Font::set_style (
	int		const	bold,
	int		const	italics,
	StyleUnderline	const	underline,
	int		const	strikethrough
	)
{
	if (	m_style_bold != bold		||
		m_style_italics != italics	||
		m_style_underline != underline	||
		m_style_strikethrough != strikethrough
		)
	{
		m_style_bold = bold;
		m_style_italics = italics;
		m_style_underline = underline;
		m_style_strikethrough = strikethrough;

		set_style ();
	}
}

void mtPixy::Font::set_row_pad (
	int	const	row_pad
	)
{
	m_style_row_pad = row_pad;
}

mtPixy::Image * mtPixy::Font::render_image (
	char	const	* const	utf8,
	int		const	max_width
	)
{
	if ( ! utf8 || ! m_font_data )
	{
		return NULL;
	}


	mtPixy::Image	* image = NULL;
	unsigned char	* mem = NULL;
	FT_Bitmap	bitmap;
	int		w=0, h=0,x=0, y=0, basel=0;
	PangoRectangle	logical_rect;


	memset ( &bitmap, 0, sizeof ( bitmap ) );

	pango_layout_set_text ( m_font_data->m_layout, utf8, -1 );
	pango_layout_set_font_description ( m_font_data->m_layout,
		m_font_data->m_font_desc );
	pango_layout_get_extents( m_font_data->m_layout, NULL, &logical_rect );
	pango_extents_to_pixels ( NULL, &logical_rect );

	if ( max_width > 0 )
	{
		w = MIN ( logical_rect.width, max_width );
	}
	else
	{
		w = logical_rect.width;
	}

	h = m_height + 2 * m_style_row_pad;
	basel = PANGO_PIXELS ( pango_layout_get_baseline (
		m_font_data->m_layout ) );

	if ( h > 0 && w > 0 )
	{
		bitmap.buffer = (unsigned char *)calloc( (size_t)h, (size_t)w);

		if ( ! bitmap.buffer )
		{
			return NULL;
		}

		bitmap.rows = (unsigned int)h;
		bitmap.width = (unsigned int)w;
		bitmap.pitch = w;
		bitmap.num_grays = 256;
		bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;

		y = m_baseline - basel + m_style_row_pad;

		pango_ft2_render_layout( &bitmap, m_font_data->m_layout, x, y );
	}

	mem = bitmap.buffer;
	if ( ! mem )
	{
		return NULL;
	}

	image = mtPixy::image_from_data( mtPixy::Image::ALPHA, w, h, NULL, mem);
	if ( ! image )
	{
		free ( mem );
		return NULL;
	}

	return image;
}

void mtPixy::Font::set_style ()
{
	PangoAttrList	* list;
	PangoAttribute	* a;


	list = pango_attr_list_new ();

	if ( m_style_bold )
	{
		a = pango_attr_weight_new ( PANGO_WEIGHT_BOLD );
		pango_attr_list_insert ( list, a );
	}

	if ( m_style_italics )
	{
		a = pango_attr_style_new ( PANGO_STYLE_ITALIC );
		pango_attr_list_insert ( list, a );
	}

	if ( m_style_underline )
	{
		a = NULL;

		switch ( m_style_underline )
		{
		case STYLE_UNDERLINE_SINGLE:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_SINGLE );
			break;

		case STYLE_UNDERLINE_DOUBLE:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_DOUBLE );
			break;

		case STYLE_UNDERLINE_WAVY:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_ERROR );
			break;

		default:
			// Ignore rubbish
			break;
		}

		if ( a )
		{
			pango_attr_list_insert ( list, a );
		}
	}

	if ( m_style_strikethrough )
	{
		a = pango_attr_strikethrough_new ( TRUE );
		pango_attr_list_insert ( list, a );
	}

	pango_layout_set_attributes ( m_font_data->m_layout, list );
	pango_attr_list_unref ( list );
}

int mtPixy::Font::get_width () const
{
	return m_width;
}

int mtPixy::Font::get_height () const
{
	return m_height;
}

