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

#ifndef MTPIXY_CAIRO_H_
#define MTPIXY_CAIRO_H_

#include <pango/pango.h>
#include <cairo.h>

#include <mtkit.h>
#include <mtpixy.h>



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API

namespace mtPixy
{

class Canvas;


class Canvas
{
public:
	Canvas ();
	~Canvas ();

	enum
	{
		TYPE_MIN		= 1,

		TYPE_PIXMAP		= 1,
		TYPE_PDF		= 2,
		TYPE_SVG		= 3,
		TYPE_EPS		= 4,
		TYPE_PS			= 5,

		TYPE_MAX		= 5,

		FONT_STYLE_NORMAL		= 0,
		FONT_STYLE_BOLD			= 1,
		FONT_STYLE_ITALIC		= 2,
		FONT_STYLE_UNDERLINE_SINGLE	= 4,
		FONT_STYLE_UNDERLINE_DOUBLE	= 8,
		FONT_STYLE_UNDERLINE_WAVY	= 12,
		FONT_STYLE_UNDERLINE_BITS	= 12,
		FONT_STYLE_STRIKETHROUGH	= 16
	};

	int init (
		int type,
		char const * filename,	// PDF, SVG, EPS, PS
		double page_width,
		double page_height
		) const;

	// Only use the functions below after a successful init()

	void set_font_name ( char const * name ) const;
	void set_font_size ( double size ) const;
	void set_font_style ( int style ) const;	// FONT_STYLE_* | ...
	void set_font_justify ( 
		int tight,		// 1=Extent by ink 0=Logical
		double h_justify,	// 0.0=Left 0.5=Centre 1.0=Right
		double v_justify	// 0.0=Bottom 0.5=Centre 1.0=Top
		) const;

	void set_color ( double r, double g, double b, double a=1.0 ) const;
	void set_stroke_width ( double width ) const;

	void new_path () const;
	void move_to ( double x, double y ) const;
	void line_to ( double x, double y ) const;
	void close_path () const;
	void draw_stroke () const;
	void draw_stroke_preserve () const;	// Preserve path
	void draw_fill () const;
	void draw_fill_preserve () const;	// Preserve path

	void draw_text ( char const * text, double x, double y ) const;
	void draw_svg ( SVG const & svg ) const;
	void draw_rectangle ( double x, double y, double w, double h ) const;
	void draw_circle (
		double x,
		double y,
		double radius
		) const;

	void fill_rectangle ( double x, double y, double w, double h ) const;
	void fill_circle (
		double x,
		double y,
		double radius
		) const;

	int save_png ( char const * filename, int compress ) const; // 0..9
	mtPixmap * render_pixmap () const;

	cairo_t * get_cairo () const;
	cairo_surface_t * get_surface () const;
	PangoFontDescription * get_font_desc () const;
	PangoLayout * get_layout () const;

private:
	class Op;
	std::unique_ptr<Op> m_op;

	MTKIT_RULE_OF_FIVE( Canvas )
};

}	// namespace mtPixy




#endif		// C++ API



#endif		// MTPIXY_CAIRO_H_

