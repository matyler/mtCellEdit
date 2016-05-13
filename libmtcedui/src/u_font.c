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



struct mtFont
{
	int		height;		// Glyph height in pixels
	int		width;		// Glyph width in pixels
	int		baseline;
	int		bold;

	PangoFontMap	* font_map;
	PangoContext	* context;
	PangoLayout	* layout;

	PangoFontDescription * font_desc;
};

typedef struct
{
	// Used on input, i.e. Set/Created/Destroyed by calling program,
	// read by library

	mtFont		* const	font;		// Prepared font to use
	char	const * const	utf8;		// Prepared string to use

	int		const	max_width;	// Maximum pixel width
						// (0 = no limit)

	int		width;
	int		height;		// Pixel geometry of the output
	int		row_pad;

} mtFontRender;



static int cui_font_set_size (
	mtFont	* const	font,
	int	const	size
	)
{
	if ( ! font || size < 1 )
	{
		return 1;
	}


	PangoRectangle	logical;


	pango_layout_set_text ( font->layout, "01234567890", -1 );
	pango_font_description_set_weight ( font->font_desc,
		PANGO_WEIGHT_NORMAL );
	pango_font_description_set_size ( font->font_desc,
		(gint)( ( (double)size ) * PANGO_SCALE ) );
	pango_layout_set_font_description( font->layout, font->font_desc );

	pango_layout_get_extents ( font->layout, NULL, &logical );
	pango_extents_to_pixels ( NULL, &logical );

	font->width = logical.width / 11;
	font->height = logical.height;
	font->baseline = PANGO_PIXELS( pango_layout_get_baseline (
		font->layout ) );

	return 0;
}

mtFont * cui_font_new_pango (
	char	const	* const	name,
	int		const	size
	)
{
	mtFont		* font;


	font = calloc ( sizeof ( mtFont ), 1 );
	if ( ! font )
	{
		return NULL;
	}

	font->font_desc = pango_font_description_from_string ( name );
	font->font_map = pango_ft2_font_map_new ();
	font->context = pango_font_map_create_context ( font->font_map );
	font->layout = pango_layout_new ( font->context );

	cui_font_set_size ( font, size );

	return font;
}

int cui_font_destroy (
	mtFont	* const	font
	)
{
	if ( ! font )
	{
		return 1;
	}

	if ( font->layout )
	{
		g_object_unref ( font->layout );
		font->layout = NULL;
	}

	if ( font->context )
	{
		g_object_unref ( font->context );
		font->context = NULL;
	}

	if ( font->font_map )
	{
		g_object_unref ( font->font_map );
		font->font_map = NULL;
	}

	if ( font->font_desc )
	{
		pango_font_description_free ( font->font_desc );
		font->font_desc = NULL;
	}

	free ( font );

	return 0;
}

static unsigned char * mt_font_render_pango (
	mtFontRender	* const	ren
	)
{
	FT_Bitmap	bitmap;
	int		x=0, y=0, baseline=0;
	PangoRectangle	logical_rect;


	memset ( &bitmap, 0, sizeof ( bitmap ) );

	pango_layout_set_text ( ren->font->layout, ren->utf8, -1 );
	pango_layout_set_font_description ( ren->font->layout,
		ren->font->font_desc );
	pango_layout_get_extents ( ren->font->layout, NULL, &logical_rect );
	pango_extents_to_pixels ( NULL, &logical_rect );

	if ( ren->max_width > 0 )
	{
		ren->width = MIN ( logical_rect.width, ren->max_width );
	}
	else
	{
		ren->width = logical_rect.width;
	}

	ren->height = ren->font->height + 2 * ren->row_pad;
	baseline = PANGO_PIXELS ( pango_layout_get_baseline (
		ren->font->layout ) );

	if ( ren->height > 0 && ren->width > 0 )
	{
		bitmap.buffer = calloc( (size_t)ren->height,(size_t)ren->width);

		if ( ! bitmap.buffer )
		{
			return NULL;
		}

		bitmap.rows = (unsigned int)ren->height;
		bitmap.width = (unsigned int)ren->width;
		bitmap.pitch = ren->width;
		bitmap.num_grays = 256;
		bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;

		y = ren->font->baseline - baseline + ren->row_pad;

		pango_ft2_render_layout ( &bitmap, ren->font->layout, x, y );
	}

	return bitmap.buffer;
}

mtImage * cui_font_render (
	mtFont		* const	font,
	char	const	* const	utf8,
	int		const	max_width,
	int		const	bold,
	int		const	row_pad
	)
{
	if ( ! font || ! utf8 )
	{
		return NULL;
	}


	mtFontRender	ren = { font, utf8, max_width, 0, 0, row_pad };
	mtImage		* image = NULL;
	unsigned char	* mem = NULL;


	if ( bold != font->bold )
	{
		cui_font_set_attr ( font->layout, bold );
		font->bold = bold;
	}

	mem = mt_font_render_pango ( &ren );
	if ( ! mem )
	{
		return NULL;
	}

	image = mtkit_image_new_data ( ren.width, ren.height, NULL, mem );
	if ( ! image )
	{
		free ( mem );
		return NULL;
	}

	return image;
}

int cui_font_get_height (
	mtFont	* const	font
	)
{
	if ( font )
	{
		return font->height;
	}

	return 0;
}

int cui_font_get_width (
	mtFont	* const	font
	)
{
	if ( font )
	{
		return font->width;
	}

	return 0;
}

void cui_font_set_attr (
	PangoLayout	* const	layout,
	int		const	attr_val	// CED_TEXT_STYLE_* bit fields
	)
{
	PangoAttrList	* list;
	PangoAttribute	* a;


	list = pango_attr_list_new ();

	if ( CED_TEXT_STYLE_IS_BOLD ( attr_val ) )
	{
		a = pango_attr_weight_new ( PANGO_WEIGHT_BOLD );
		pango_attr_list_insert ( list, a );
	}

	if ( CED_TEXT_STYLE_IS_ITALIC ( attr_val ) )
	{
		a = pango_attr_style_new ( PANGO_STYLE_ITALIC );
		pango_attr_list_insert ( list, a );
	}

	if ( CED_TEXT_STYLE_IS_UNDERLINE ( attr_val ) )
	{
		switch ( attr_val & CED_TEXT_STYLE_UNDERLINE_ANY )
		{
		case CED_TEXT_STYLE_UNDERLINE_DOUBLE:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_DOUBLE );
			break;

		case CED_TEXT_STYLE_UNDERLINE_WAVY:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_ERROR );
			break;

		default:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_SINGLE );
			break;
		}

		pango_attr_list_insert ( list, a );
	}

	if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( attr_val ) )
	{
		a = pango_attr_strikethrough_new ( TRUE );
		pango_attr_list_insert ( list, a );
	}

	pango_layout_set_attributes ( layout, list );
	pango_attr_list_unref ( list );
}

