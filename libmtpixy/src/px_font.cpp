/*
	Copyright (C) 2004-2023 Mark Tyler

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
	FontData ();
	~FontData ();

	int set_name ( char const * name );

/// ----------------------------------------------------------------------

	PangoFontDescription * m_font_desc	= nullptr;
	PangoFontMap	* m_font_map		= nullptr;
	PangoContext	* m_context		= nullptr;
	PangoLayout	* m_layout		= nullptr;
private:
};

}



mtPixy::FontData::FontData ()
	:
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

int mtPixy::FontData::set_name ( char const * const name )
{
	PangoFontDescription * const new_font_desc =
		pango_font_description_from_string ( name );

	if ( new_font_desc )
	{
		if ( m_font_desc )
		{
			pango_font_description_free ( m_font_desc );
		}

		m_font_desc = new_font_desc;
	}
	else
	{
		std::cerr << "set_name: bad font name: '" << name << "'\n";
		return 1;
	}

	return 0;
}



/// ----------------------------------------------------------------------------



mtPixy::Font::Font (
	char	const	* const	name,
	int		const	size
	)
	:
	m_font_data	( new FontData )
{
	if ( set_font ( name, size ) )
	{
		set_font ( "Sans", 12 );
	}
}

mtPixy::Font::~Font ()
{
}

int mtPixy::Font::set_font (
	char	const	* const	name,
	int		const	size
	)
{
	if ( size < 1 || ! name )
	{
		return 1;
	}

	m_font_data->set_name ( name );

	PangoRectangle	logical, ink;

	pango_layout_set_text ( m_font_data->m_layout, "0123456789", -1 );
	pango_font_description_set_weight ( m_font_data->m_font_desc,
		PANGO_WEIGHT_NORMAL );
	pango_font_description_set_size ( m_font_data->m_font_desc,
		(gint)( ( (double)size ) * PANGO_SCALE ) );
	pango_layout_set_font_description( m_font_data->m_layout,
		m_font_data->m_font_desc );

	pango_layout_get_extents ( m_font_data->m_layout, &ink, &logical );
	pango_extents_to_pixels ( NULL, &logical );

	// NOTE: pango_extents_to_pixels doesn't also work exactly for all fonts
	int const yextra = ink.y < 0 ? PANGO_PIXELS (-ink.y) : 0;

	m_name = name;
	m_size = size;
	m_width = logical.width / 10;
	m_height = logical.height + yextra;
	m_baseline = PANGO_PIXELS ( pango_layout_get_baseline (
		m_font_data->m_layout ) ) + yextra;

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

mtPixmap * mtPixy::Font::render_pixmap (
	char	const	* const	utf8,
	int		const	max_width
	)
{
	if ( ! utf8 || ! m_font_data )
	{
		return NULL;
	}


	mtPixmap	* image = NULL;
	unsigned char	* mem = NULL;
	FT_Bitmap	bitmap;
	int		w=0, h=0,basel=0;
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

	h = m_height + 2 * m_row_pad;
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

		int y = m_baseline - basel + m_row_pad;

		pango_ft2_render_layout( &bitmap, m_font_data->m_layout, 0, y );
	}

	mem = bitmap.buffer;
	if ( ! mem )
	{
		return NULL;
	}

	image = pixy_pixmap_from_data ( PIXY_PIXMAP_BPP_ALPHA_ONLY, w, h,
		NULL, mem );
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

