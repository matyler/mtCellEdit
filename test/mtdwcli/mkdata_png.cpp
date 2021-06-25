/*
	Copyright (C) 2018-2020 Mark Tyler

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

#include "mkdata.h"



#define PATTERNS_FILE	"/usr/share/mtpixy-qt5/patterns/default.png"
#define SHAPES_FILE	"/usr/share/mtpixy-qt5/shapes/default.png"



CreatePNG::CreatePNG ( char const * const path, mtKit::Random & random )
	:
	m_random	( random ),
	m_palette	(),
	m_color		()
{
	std::string	filename;

	filename += path;
	filename += MTKIT_DIR_SEP;
	filename += "bottle.png";

	m_pixmap.reset ( pixy_pixmap_new_rgb ( IMAGE_WIDTH, IMAGE_HEIGHT ) );

	if ( ! m_pixmap.get () )
	{
		std::cerr << "Unable to create pixmap.\n";

		throw 123;
	}

	m_brush.reset ( new mtPixy::Brush () );

	if (	m_brush->load_shapes ( SHAPES_FILE )	||
		m_brush->load_patterns ( PATTERNS_FILE )
		)
	{
		std::cerr << "Unable to load shapes and patterns.\n";

		throw 123;
	}

	m_palette = pixy_pixmap_get_palette ( m_pixmap.get () );
	pixy_palette_set_size ( m_palette, 256 );

	m_color = &m_palette->color[0];

	for ( int i = 0; i < 256; i++ )
	{
		m_color[i].red	= (unsigned char)m_random.get_int ( 256 );
		m_color[i].green= (unsigned char)m_random.get_int ( 256 );
		m_color[i].blue	= (unsigned char)m_random.get_int ( 256 );
	}

	pixy_pixmap_palette_sort ( m_pixmap.get (), 0, 255,
		PIXY_PALETTE_SORT_HUE, 0 );

	m_brush->set_shape ( 0 );
	m_brush->set_pattern ( 0 );
	m_brush->set_spacing ( 0 );
	m_brush->set_color_a ( 0, m_color );

	paint_rectangles ();

	if ( pixy_pixmap_save_png ( m_pixmap.get (), filename.c_str (), 6 ) )
	{
		std::cerr << "Unable to save image to file " << filename <<"\n";
	}
}

CreatePNG::~CreatePNG ()
{
}

void CreatePNG::paint_rectangles ()
{
	int const rw = IMAGE_WIDTH / 8;
	int const rh = IMAGE_HEIGHT / 8;
	int const tiw = IMAGE_WIDTH + rw;
	int const tih = IMAGE_HEIGHT + rh;

	mtPixy::Brush & brush = *m_brush.get ();

	for ( int i = 0; i < 1000; i++ )
	{
		brush.set_color_a ( (unsigned char)m_random.get_int ( 256 ),
			m_color );

		brush.set_color_b ( (unsigned char)m_random.get_int ( 256 ),
			m_color );

		brush.set_pattern ( m_random.get_int ( 100 ) );

		int const x = m_random.get_int ( tiw ) - rw / 2;
		int const y = m_random.get_int ( tih ) - rh / 2;
		int const w = m_random.get_int ( rw );
		int const h = m_random.get_int ( rh );

		brush.paint_canvas_rectangle ( m_pixmap.get(), x, y, w, h );
	}
}

