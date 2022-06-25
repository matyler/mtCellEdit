/*
	Copyright (C) 2008-2021 Mark Tyler

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

#include <librsvg/rsvg.h>

#include "private.h"



struct mtPixy::SVG::Op
{
public:
	Op ();
	~Op ();

	void clear_all ();
	void clear_cairo ();

	int create_cairo ( int width, int height );

/// ----------------------------------------------------------------------------

	cairo_t		* cr		= nullptr;
	RsvgHandle	* rsvg		= nullptr;
	cairo_surface_t * surface	= nullptr;

private:

	MTKIT_RULE_OF_FIVE( Op )
};



mtPixy::SVG::Op::Op ()
{
}

mtPixy::SVG::Op::~Op ()
{
	clear_all ();
}

void mtPixy::SVG::Op::clear_all ()
{
	if ( rsvg )
	{
		g_object_unref ( rsvg );
		rsvg = NULL;
	}

	clear_cairo ();
}

void mtPixy::SVG::Op::clear_cairo ()
{
	if ( cr )
	{
		cairo_destroy ( cr );
		cr = NULL;
	}

	if ( surface )
	{
		cairo_surface_destroy ( surface );
		surface = NULL;
	}
}

int mtPixy::SVG::Op::create_cairo (
	int	const	width,
	int	const	height
	)
{
	clear_cairo ();

	surface = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32, width,
		height );

	if ( ! surface )
	{
		return 1;
	}

	cr = cairo_create ( surface );
	if ( ! cr )
	{
		clear_cairo ();
		return 1;
	}

	return 0;
}



/// mtPixy::SVG	----------------------------------------------------------------



mtPixy::SVG::SVG ()
	:
	m_op	( new mtPixy::SVG::Op )
{
}

mtPixy::SVG::~SVG ()
{
}

int mtPixy::SVG::load ( char const * const filename )
{
	if ( ! filename )
	{
		return 1;
	}

	m_op->clear_all ();
	m_op->rsvg = rsvg_handle_new_from_file ( filename, NULL );

	if ( ! m_op->rsvg )
	{
		return 1;
	}

	return 0;
}

int mtPixy::SVG::load_pixmap (
	mtPixy::Pixmap		& pixmap,
	char	const * const	filename,
	int		const	width,
	int		const	height
	)
{
	mtPixy::SVG	svg;

	svg.load ( filename );
	svg.render ( width, height );
	svg.create_pixmap ( pixmap );

	return (pixmap.get()) ? 1 : 0;
}

unsigned char * mtPixy::SVG::render (
	int	const	width,
	int	const	height
	)
{
	if ( m_op->create_cairo ( width, height ) )
	{
		return nullptr;
	}

	RsvgDimensionData dimensions = { 0, 0, 0.0, 0.0 };
	rsvg_handle_get_dimensions ( m_op->rsvg, &dimensions );

	double const scale_width = ((double)width) / dimensions.width;
	double const scale_height =((double)height) / dimensions.height;

	cairo_scale ( m_op->cr, scale_width, scale_height );

	rsvg_handle_render_cairo ( m_op->rsvg, m_op->cr );

	return cairo_image_surface_get_data ( m_op->surface );
}

int mtPixy::SVG::create_pixmap ( Pixmap & pixmap )
{
	pixmap.reset ( pixy_pixmap_from_cairo ( m_op->surface ) );

	return ( pixmap.get () ) ? 0 : 1;
}

void mtPixy::SVG::render_free ()
{
	m_op->clear_cairo ();
}



/// ----------------------------------------------------------------------------



void mtPixy::Canvas::draw_svg ( SVG const & svg ) const
{
	RsvgHandle * const rsvg = svg.get_op()->rsvg;
	cairo_t * const cr = get_cairo ();

	if ( rsvg && cr )
	{
		rsvg_handle_render_cairo ( rsvg, cr );
	}
}
