/*
	Copyright (C) 2018 Mark Tyler

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



#define PATTERNS_FILE	"/usr/share/mtpixy-qt4/patterns/default.png"
#define SHAPES_FILE	"/usr/share/mtpixy-qt4/shapes/default.png"



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

	m_image.reset ( mtPixy::Image::create ( mtPixy::Image::TYPE_RGB,
		IMAGE_WIDTH, IMAGE_HEIGHT ) );

	if ( ! m_image.get () )
	{
		std::cerr << "Unable to create image.\n";

		throw 123;
	}

	m_brush.reset ( new mtPixy::Brush () );

	if (	m_brush.get ()->load_shapes ( SHAPES_FILE )	||
		m_brush.get ()->load_patterns ( PATTERNS_FILE )
		)
	{
		std::cerr << "Unable to load shapes and patterns.\n";

		throw 123;
	}

	m_palette = m_image.get ()->get_palette ();
	m_palette->set_color_total ( 256 );

	m_color = m_palette->get_color ();

	for ( int i = 0; i < 256; i++ )
	{
		m_color[i].red	= (unsigned char)m_random.get_int ( 256 );
		m_color[i].green= (unsigned char)m_random.get_int ( 256 );
		m_color[i].blue	= (unsigned char)m_random.get_int ( 256 );
	}

	m_image.get ()->palette_sort ( 0, 255, mtPixy::Image::SORT_HUE, false );

	m_brush.get ()->set_shape ( 0 );
	m_brush.get ()->set_pattern ( 0 );
	m_brush.get ()->set_spacing ( 0 );
	m_brush.get ()->set_color_a ( 0, m_color );

	paint_rectangles ();

	if ( m_image.get ()->save_png ( filename.c_str (), 6 ) )
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
	mtPixy::Image * const image = m_image.get ();

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

		image->paint_rectangle ( brush, x, y, w, h );
	}
}

