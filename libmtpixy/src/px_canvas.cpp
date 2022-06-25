/*
	Copyright (C) 2021 Mark Tyler

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



class mtPixy::Canvas::Op
{
public:
	Op ();
	~Op ();

	void clear ();

/// ----------------------------------------------------------------------------

	cairo_t			* m_cr		= nullptr;
	cairo_surface_t		* m_surface	= nullptr;
	PangoFontDescription	* m_font_desc	= nullptr;
	PangoLayout		* m_layout	= nullptr;

	int			m_text_tight	= 0;
	double			m_text_h_justify = 0.0;
	double			m_text_v_justify = 0.0;

private:
	MTKIT_RULE_OF_FIVE( Op )
};



mtPixy::Canvas::Op::Op ()
{
}

mtPixy::Canvas::Op::~Op ()
{
	clear();
}

void mtPixy::Canvas::Op::clear ()
{
	if ( m_cr )
	{
		cairo_destroy ( m_cr );
		m_cr = nullptr;
	}

	if ( m_surface )
	{
		cairo_surface_destroy ( m_surface );
		m_surface = nullptr;
	}

	if ( m_font_desc )
	{
		pango_font_description_free ( m_font_desc );
		m_font_desc = nullptr;
	}

	if ( m_layout )
	{
		g_object_unref ( m_layout );
		m_layout = nullptr;
	}
}



/// ----------------------------------------------------------------------------



mtPixy::Canvas::Canvas ()
	:
	m_op		( new mtPixy::Canvas::Op() )
{
}

mtPixy::Canvas::~Canvas ()
{
}

int mtPixy::Canvas::init (
	int		const	type,
	char	const * const	filename,
	double		const	page_width,
	double		const	page_height
	) const
{
	if (	type < TYPE_MIN
		|| type > TYPE_MAX
		|| page_width < 1
		|| page_height < 1
		)
	{
		return 1;
	}

	if ( ! filename )
	{
		switch ( type )
		{
		case TYPE_PDF:
		case TYPE_SVG:
		case TYPE_PS:
		case TYPE_EPS:
			return 1;
		}
	}

	m_op->clear();

	switch ( type )
	{
	case TYPE_PIXMAP:
		m_op->m_surface = cairo_image_surface_create (
			CAIRO_FORMAT_RGB24, (int)page_width, (int)page_height );
		break;

	case TYPE_PDF:
		m_op->m_surface = cairo_pdf_surface_create ( filename,
			page_width, page_height );
		break;

	case TYPE_SVG:
		m_op->m_surface = cairo_svg_surface_create ( filename,
			page_width, page_height );
		break;

	case TYPE_PS:
		m_op->m_surface = cairo_ps_surface_create ( filename,
			page_width, page_height );
		break;

	case TYPE_EPS:
		m_op->m_surface = cairo_ps_surface_create ( filename,
			page_width, page_height );
		cairo_ps_surface_set_eps ( m_op->m_surface, 1 );
		break;
	}

	if ( cairo_surface_status ( m_op->m_surface ) != CAIRO_STATUS_SUCCESS )
	{
		std::cerr << "cairo_surface_status() error\n";
		m_op->clear();
		return 1;
	}

	m_op->m_cr = cairo_create ( m_op->m_surface );
	m_op->m_font_desc = pango_font_description_from_string ( "Sans" );
	m_op->m_layout = pango_cairo_create_layout ( m_op->m_cr );

	if ( ! m_op->m_cr || ! m_op->m_font_desc || ! m_op->m_layout )
	{
		std::cerr << "m_cr, m_font_desc, m_layout error\n";
		m_op->clear();
		return 1;
	}

	set_font_justify ( 0, 0.0, 0.0 );

	return 0;
}

void mtPixy::Canvas::set_font_name ( char const * const name ) const
{
	PangoFontDescription * desc = pango_font_description_from_string (name);

	if ( ! desc )
	{
		std::cerr << "Unable to set font to: " << name << "\n";
		return;
	}

	if ( m_op->m_font_desc )
	{
		pango_font_description_free ( m_op->m_font_desc );
	}

	m_op->m_font_desc = desc;
}

void mtPixy::Canvas::set_font_size ( double const size ) const
{
	pango_font_description_set_size ( m_op->m_font_desc,
		(gint)( size * PANGO_SCALE ) );
}

void mtPixy::Canvas::set_font_style ( int const style ) const
{
	PangoAttrList	* list = pango_attr_list_new ();
	PangoAttribute	* a;

	if ( style & FONT_STYLE_BOLD )
	{
		a = pango_attr_weight_new ( PANGO_WEIGHT_BOLD );
		pango_attr_list_insert ( list, a );
	}

	if ( style & FONT_STYLE_ITALIC )
	{
		a = pango_attr_style_new ( PANGO_STYLE_ITALIC );
		pango_attr_list_insert ( list, a );
	}

	int const underlines = style & FONT_STYLE_UNDERLINE_BITS;

	if ( underlines )
	{
		switch ( underlines )
		{
		case FONT_STYLE_UNDERLINE_DOUBLE:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_DOUBLE );
			break;

		case FONT_STYLE_UNDERLINE_WAVY:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_ERROR );
			break;

		default:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_SINGLE );
			break;
		}

		pango_attr_list_insert ( list, a );
	}

	if ( style & FONT_STYLE_STRIKETHROUGH )
	{
		a = pango_attr_strikethrough_new ( TRUE );
		pango_attr_list_insert ( list, a );
	}

	pango_layout_set_attributes ( m_op->m_layout, list );
	pango_attr_list_unref ( list );
}

void mtPixy::Canvas::set_font_justify ( 
	int	const	tight,
	double	const	h_justify,
	double	const	v_justify
	) const
{
	m_op->m_text_tight = tight;
	m_op->m_text_h_justify = h_justify;
	m_op->m_text_v_justify = v_justify;
}

void mtPixy::Canvas::set_color (
	double	const	r,
	double	const	g,
	double	const	b,
	double	const	a
	) const
{
	cairo_set_source_rgba ( m_op->m_cr, r, g, b, a );
}

void mtPixy::Canvas::set_stroke_width ( double const width ) const
{
	cairo_set_line_width ( m_op->m_cr, width );
}

void mtPixy::Canvas::new_path () const
{
	cairo_new_path ( m_op->m_cr );
}

void mtPixy::Canvas::move_to ( double const x, double const y ) const
{
	cairo_move_to ( m_op->m_cr, x, y );
}

void mtPixy::Canvas::line_to ( double x, double y ) const
{
	cairo_line_to ( m_op->m_cr, x, y );
}

void mtPixy::Canvas::close_path () const
{
	cairo_close_path ( m_op->m_cr );
}

void mtPixy::Canvas::draw_stroke () const
{
	cairo_stroke ( m_op->m_cr );
}

void mtPixy::Canvas::draw_stroke_preserve () const
{
	cairo_stroke_preserve ( m_op->m_cr );
}

void mtPixy::Canvas::draw_fill () const
{
	cairo_fill ( m_op->m_cr );
}

void mtPixy::Canvas::draw_fill_preserve () const
{
	cairo_fill_preserve ( m_op->m_cr );
}

void mtPixy::Canvas::draw_text (
	char	const * const	text,
	double		const	x,
	double		const	y
	) const
{
	if ( ! text )
	{
		return;
	}

	PangoRectangle rect;

	pango_layout_set_text ( m_op->m_layout, text, -1 );
	pango_layout_set_font_description ( m_op->m_layout, m_op->m_font_desc );

	if ( m_op->m_text_tight )
	{
		pango_layout_get_extents ( m_op->m_layout, &rect, NULL );
	}
	else
	{
		pango_layout_get_extents ( m_op->m_layout, NULL, &rect );
	}

	double const scale = PANGO_SCALE;
	double const rx = x - (rect.width * m_op->m_text_h_justify + rect.x) /
		scale;
	double const ry = y - (rect.height * m_op->m_text_v_justify + rect.y) /
		scale;

	move_to ( rx, ry );

	pango_cairo_update_layout ( m_op->m_cr, m_op->m_layout );
	pango_cairo_show_layout ( m_op->m_cr, m_op->m_layout );
}

void mtPixy::Canvas::draw_rectangle (
	double	const	x,
	double	const	y,
	double	const	w,
	double	const	h
	) const
{
	move_to ( x, y );
	line_to ( x+w, y );
	line_to ( x+w, y+h );
	line_to ( x, y+h );
	close_path ();

	draw_stroke ();
}

void mtPixy::Canvas::draw_circle (
	double	const	x,
	double	const	y,
	double	const	radius
	) const
{
	cairo_arc ( m_op->m_cr, x, y, radius, 0, 2*M_PI );
	draw_stroke ();
}

void mtPixy::Canvas::fill_rectangle (
	double	const	x,
	double	const	y,
	double	const	w,
	double	const	h
	) const
{
	move_to ( x, y );
	line_to ( x+w, y );
	line_to ( x+w, y+h );
	line_to ( x, y+h );
	close_path ();

	draw_fill ();
}

void mtPixy::Canvas::fill_circle (
	double	const	x,
	double	const	y,
	double	const	radius
	) const
{
	cairo_arc ( m_op->m_cr, x, y, radius, 0, 2*M_PI );
	cairo_fill ( m_op->m_cr );
}

int mtPixy::Canvas::save_png (
	char	const * const	filename,
	int		const	compress
	) const
{
	if ( ! filename )
	{
		return 1;
	}

	mtPixy::Pixmap pixmap ( render_pixmap () );

	if ( pixy_pixmap_save_png ( pixmap.get(), filename, compress ) )
	{
		return 1;
	}

	return 0;
}

mtPixmap * mtPixy::Canvas::render_pixmap () const
{
	return pixy_pixmap_from_cairo ( m_op->m_surface );
}

cairo_t * mtPixy::Canvas::get_cairo () const
{
	return m_op->m_cr;
}

cairo_surface_t * mtPixy::Canvas::get_surface () const
{
	return m_op->m_surface;
}

PangoFontDescription * mtPixy::Canvas::get_font_desc () const
{
	return m_op->m_font_desc;
}

PangoLayout * mtPixy::Canvas::get_layout () const
{
	return m_op->m_layout;
}

