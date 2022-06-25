/*
	Copyright (C) 2011-2022 Mark Tyler

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

#include "cui_sheet.h"



static unsigned char const header_col[][3] = {
	{ HEADER_R_BACK,	HEADER_G_BACK,		HEADER_B_BACK },
	{ HEADER_R_CURS_BACK,	HEADER_G_CURS_BACK,	HEADER_B_CURS_BACK },
	{ HEADER_R_TEXT,	HEADER_G_TEXT,		HEADER_B_TEXT },
	{ HEADER_R_CURS_TEXT,	HEADER_G_CURS_TEXT,	HEADER_B_CURS_TEXT }
	};

#define HCOL_BACK		0
#define HCOL_BACK_CURSOR	1
#define HCOL_TEXT		2
#define HCOL_TEXT_CURSOR	3



/// Abstract classes for all rendering engines ---------------------------------



class RowHeadBuf
{
public:
	RowHeadBuf (
		CuiRender	* const	cui,
		CedSheet	* const	sheet,
		int		const	row_start,
		int		const	y,
		int		const	h
		)
		:
		m_error		( ! sheet ),
		m_row_height	( cui->row_height() )
	{
		if ( m_error )
		{
			return;
		}

		m_cur1 = MIN ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
		m_cur2 = MAX ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );

		m_r1 = cui->row_from_y ( row_start, y );
		m_r2 = cui->row_from_y ( row_start, (y + h - 1) );

		m_ymod = y % m_row_height;

		m_txt[0] = 0;
	}

	inline int cursor_visible ( int const row ) const
	{
		return ( row >= cur1() && row <= cur2() );
	}

	inline int cursor_visible () const
	{
		return ( cur1() <= r2() && cur2() >= r1() );
	}

	inline char const * get_int_txt ( int const num )
	{
		snprintf ( m_txt, sizeof ( m_txt ), "%i", num );

		return m_txt;
	}

	inline int error() const	{ return m_error; }
	inline int row_height() const	{ return m_row_height; }
	inline int ymod() const		{ return m_ymod; }
	inline int r1() const		{ return m_r1; }
	inline int r2() const		{ return m_r2; }
	inline int cur1() const		{ return m_cur1; }
	inline int cur2() const		{ return m_cur2; }

	// Return Y pixel offset and Height pixels of cursor exposure
	// Caller must ensure cursor is visible first: cursor_visible()
	inline void cursor_expose_y_h ( int & cy, int & ch, int const h ) const
	{
		// Cursor must be clipped to r1 -> r2 to avoid int overflow
		int const cc_min = MAX( cur1(), r1() );
		int const cc_max = MIN( cur2(), r2() );

		cy = (cc_min - r1()) * row_height() - ymod();
		ch = (cc_max - cc_min + 1) * row_height();

		// Pixels must be clipped to exposure area
		if ( cy < 0 )
		{
			ch += cy;
			cy = 0;
		}

		if ( (cy + ch) > h )
		{
			ch = h - cy;
		}
	}

private:
	int	const	m_error;
	int	const	m_row_height;
	int		m_ymod;		// Pixels exposed in partial row
	int		m_r1, m_r2;	// Exposed rows: min, max
	int		m_cur1, m_cur2;	// Cursor columns: min, max
	char		m_txt[32];
};



class ColHeadBuf
{
public:
	ColHeadBuf (
		CuiRender	* const	cui,
		CedSheet	* const	sheet,
		int		const	col_start,
		int		const	x,
		int		const	w
		)
	{
		if ( ! sheet )
		{
			m_error = 1;
			return;
		}

		m_error = cui->init_column_array ( col_start, x, w, m_c1, m_c2,
			&m_col_x, &m_col_w );

		if ( m_error )
		{
			return;
		}

		m_cur1 = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
		m_cur2 = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );

		m_txt[0] = 0;
	}

	~ColHeadBuf ()
	{
		free ( m_col_x );
		free ( m_col_w );
	}

	inline int cursor_visible ( int const column ) const
	{
		return ( column >= cur1() && column <= cur2() );
	}

	inline int cursor_visible () const
	{
		return ( cur1() <= c2() && cur2() >= c1() );
	}

	inline char const * get_int_txt ( int const num )
	{
		snprintf ( m_txt, sizeof ( m_txt ), "%i", num );

		return m_txt;
	}

	// Caller must ensure c1 <= n <= c2
	inline int col_x(int n) const	{ return m_col_x[n - c1()]; }
	inline int col_w(int n) const	{ return m_col_w[n - c1()]; }
	inline int error() const	{ return m_error; }
	inline int c1() const		{ return m_c1; }
	inline int c2() const		{ return m_c2; }
	inline int cur1() const		{ return m_cur1; }
	inline int cur2() const		{ return m_cur2; }

	// Return X pixel offset and Width pixels of cursor exposure
	// Caller must ensure cursor is visible first: cursor_visible()
	inline void cursor_expose_x_w (
		int & cx, int & cw, int const x, int const w ) const
	{
		// Cursor must be clipped to c1 -> c2 to access array
		int const cc_min = MAX( cur1(), c1() );
		int const cc_max = MIN( cur2(), c2() );

		cx = col_x( cc_min ) - x;
		cw = col_x( cc_max ) + col_w( cc_max ) - col_x( cc_min );

		// Clip to exposure
		if ( cx < 0 )
		{
			cw += cx;
			cx = 0;
		}

		if ( (cx + cw) > w )
		{
			cw = w - cx;
		}
	}

private:
	int		m_error;
	int		* m_col_x	= nullptr;
	int		* m_col_w	= nullptr;
	int		m_c1, m_c2;	// Exposed rows: min, max
	int		m_cur1, m_cur2;	// Cursor columns: min, max
	char		m_txt[32];
};



/// CAIRO ----------------------------------------------------------------------



static void set_canvas_color (
	mtPixy::Canvas	& canvas,
	int	const	col
	)
{
	int const r = header_col[col][0];
	int const g = header_col[col][1];
	int const b = header_col[col][2];

	canvas.set_color ( r/255.0, g/255.0, b/255.0 );
}

int CuiRender::expose_row_header_cairo (
	int		const	row_start,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	mtPixy::Canvas canvas;

	if ( canvas.init ( mtPixy::Canvas::TYPE_PIXMAP, nullptr, w, h ) )
	{
		return 1;
	}

	RowHeadBuf rowbuf ( this, sheet(), row_start, y, h );

	if ( rowbuf.error() )
	{
		return 1;
	}

	set_canvas_color ( canvas, HCOL_BACK );
	canvas.fill_rectangle ( 0, 0, w, h );

	if ( rowbuf.cursor_visible() )
	{
		set_canvas_color ( canvas, HCOL_BACK_CURSOR );

		int cy, ch;
		rowbuf.cursor_expose_y_h ( cy, ch, h );

		canvas.fill_rectangle ( 0, cy, w, ch );
	}

	canvas.set_font_name ( font_name().c_str() );
	canvas.set_font_size ( font_size() * CUI_PANGO_SCALE );

	// Centralise text position
	int const mx = row_header_width() / 2;
	double const scale = PANGO_SCALE;
	PangoLayout * const layout = canvas.get_layout ();
	cairo_t * const cr = canvas.get_cairo ();
	PangoFontDescription * const font_desc = canvas.get_font_desc();

	int my = this->row_pad() - rowbuf.ymod();
	PangoRectangle logical;

	pango_layout_set_font_description ( layout, font_desc );

	for (	int i = rowbuf.r1();
		i <= rowbuf.r2();
		i++, my += rowbuf.row_height()
		)
	{
		if ( rowbuf.cursor_visible ( i ) )
		{
			set_canvas_color ( canvas, HCOL_TEXT_CURSOR );
		}
		else
		{
			set_canvas_color ( canvas, HCOL_TEXT );
		}

		char const * const txt = rowbuf.get_int_txt ( i );

		pango_layout_set_text ( layout, txt, -1 );
		pango_layout_get_extents ( layout, NULL, &logical);

		int const basel = PANGO_PIXELS ( pango_layout_get_baseline (
			layout ) );

		cairo_move_to ( cr,
			(int)(0.5 + mx - (logical.width*0.5 + logical.x)/scale),
			my + (this->baseline() - basel)
			);
		pango_cairo_update_layout ( cr, layout );
		pango_cairo_show_layout ( cr, layout );
	}

	unsigned char const * const rgb = cairo_image_surface_get_data (
		canvas.get_surface() );

	callback ( x, y, w, h, rgb, 4, callback_data );

	return 0;
}

int CuiRender::expose_column_header_cairo (
	int		const	col_start,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	mtPixy::Canvas canvas;

	if ( canvas.init ( mtPixy::Canvas::TYPE_PIXMAP, nullptr, w, h ) )
	{
		return 1;
	}

	ColHeadBuf colbuf ( this, sheet(), col_start, x, w );

	if ( colbuf.error() )
	{
		return 1;
	}

	set_canvas_color ( canvas, HCOL_BACK );
	canvas.fill_rectangle ( 0, 0, w, h );

	if ( colbuf.cursor_visible() )
	{
		set_canvas_color ( canvas, HCOL_BACK_CURSOR );

		int cx, cw;
		colbuf.cursor_expose_x_w ( cx, cw, x, w );

		canvas.fill_rectangle ( cx, 0, cw, h );
	}

	canvas.set_font_name ( font_name().c_str() );
	canvas.set_font_size ( font_size() * CUI_PANGO_SCALE );

	// Note: h == row_height() as per front end constraints
	int const my = this->row_pad();
	double const scale = PANGO_SCALE;
	PangoLayout * const layout = canvas.get_layout ();
	cairo_t * const cr = canvas.get_cairo ();
	PangoFontDescription * const font_desc = canvas.get_font_desc();

	int cell_x = colbuf.col_x( colbuf.c1() ) - x;
	PangoRectangle logical;

	pango_layout_set_font_description ( layout, font_desc );

	for ( int i = colbuf.c1(); i <= colbuf.c2(); i++ )
	{
		if ( colbuf.cursor_visible ( i ) )
		{
			set_canvas_color ( canvas, HCOL_TEXT_CURSOR );
		}
		else
		{
			set_canvas_color ( canvas, HCOL_TEXT );
		}

		int const cell_w = colbuf.col_w( i );
		char const * const txt = colbuf.get_int_txt ( i );

		// Stop text encroaching onto other columns
		cairo_rectangle ( cr, cell_x, 0, cell_w, h );
		cairo_clip ( cr );

		pango_layout_set_text ( layout, txt, -1 );
		pango_layout_get_extents ( layout, NULL, &logical);

		int const basel = PANGO_PIXELS ( pango_layout_get_baseline (
			layout ) );

		cairo_move_to ( cr,
			(int)(0.5 + cell_x + cell_w/2 -
				(logical.width*0.5 + logical.x)/scale),
			my + (this->baseline() - basel)
			);
		pango_cairo_update_layout ( cr, layout );
		pango_cairo_show_layout ( cr, layout );

		cairo_reset_clip ( cr );

		cell_x += cell_w;
	}

	unsigned char const * const rgb = cairo_image_surface_get_data (
		canvas.get_surface() );

	callback ( x, y, w, h, rgb, 4, callback_data );

	return 0;
}



/// PANGO ----------------------------------------------------------------------



class RGBbuffer
{
public:
	RGBbuffer ( int w, int h )
		:
		m_rgb ( (unsigned char *)calloc( (size_t)(w * h * 3), 1 ) )
	{
	}

	~RGBbuffer ()
	{
		free ( m_rgb );
	}

	inline unsigned char * rgb() const { return m_rgb; }

private:
	unsigned char * m_rgb;
};



namespace {
inline void fill_rgb (
	unsigned char		*& dest,
	int		const	w,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	for ( int i = 0; i < w; i++ )
	{
		*dest++ = r;
		*dest++ = g;
		*dest++ = b;
	}
}
} // namespace {



int CuiRender::expose_row_header_pango (
	int		const	row_start,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	RGBbuffer const buf ( w, h );

	if ( ! buf.rgb() )
	{
		return 1;
	}

	RowHeadBuf rowbuf ( this, sheet(), row_start, y, h );

	if ( rowbuf.error() )
	{
		return 1;
	}

	unsigned char * dest = buf.rgb();
	size_t	const	rowstride = (size_t)(w*3);

	fill_rgb ( dest, w*h,
		header_col[HCOL_BACK][0],
		header_col[HCOL_BACK][1],
		header_col[HCOL_BACK][2] );

	if ( rowbuf.cursor_visible() )
	{
		int yoff, tot;
		rowbuf.cursor_expose_y_h ( yoff, tot, h );

		dest = buf.rgb() + ((size_t)yoff) * rowstride;

		fill_rgb ( dest, w*tot,
			header_col[HCOL_BACK_CURSOR][0],
			header_col[HCOL_BACK_CURSOR][1],
			header_col[HCOL_BACK_CURSOR][2] );
	}

	font().set_row_pad ( row_pad() );
	font().set_style ( 0, 0, (mtPixy::Font::StyleUnderline)0, 0 );

	for ( int i = rowbuf.r1(); i <= rowbuf.r2(); i++ )
	{
		mtPixy::Pixmap const pixmap ( font().render_pixmap (
			rowbuf.get_int_txt ( i ), 0 ) );
		if ( ! pixmap.get() )
		{
			continue;
		}

		unsigned char const * const mem = pixy_pixmap_get_alpha (
			pixmap.get() );
		if ( ! mem )
		{
			continue;
		}

		unsigned char r, g, b;
		if ( rowbuf.cursor_visible ( i ) )
		{
			r = header_col[HCOL_TEXT_CURSOR][0];
			g = header_col[HCOL_TEXT_CURSOR][1];
			b = header_col[HCOL_TEXT_CURSOR][2];
		}
		else
		{
			r = header_col[HCOL_TEXT][0];
			g = header_col[HCOL_TEXT][1];
			b = header_col[HCOL_TEXT][2];
		}

		int const image_w = pixy_pixmap_get_width ( pixmap.get() );
		int const image_h = pixy_pixmap_get_height ( pixmap.get() );
		int const mx = (row_header_width() - image_w) / 2; // Centralise
		int const my = y_from_row ( row_start, i );

		cui_ren_mem_to_rgb ( mem, mx, my, image_w, image_h, 0, image_w,
			r, g, b, buf.rgb(), x, y, w, h );
	}

	callback ( x, y, w, h, buf.rgb(), 3, callback_data );

	return 0;
}

int CuiRender::expose_column_header_pango (
	int		const	col_start,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	RGBbuffer const buf ( w, h );

	if ( ! buf.rgb() )
	{
		return 1;
	}

	ColHeadBuf colbuf ( this, sheet(), col_start, x, w );

	if ( colbuf.error() )
	{
		return 1;
	}

	// We only need to render the top line of pixels - the others can be
	// memcpy'd.

	unsigned char * dest = buf.rgb();

	// Blank background
	fill_rgb ( dest, w,
		header_col[HCOL_BACK][0],
		header_col[HCOL_BACK][1],
		header_col[HCOL_BACK][2] );

	// Cursor background
	if ( colbuf.cursor_visible() )
	{
		int cx, cw;
		colbuf.cursor_expose_x_w ( cx, cw, x, w );

		dest = buf.rgb() + 3*cx;

		fill_rgb ( dest, cw,
			header_col[HCOL_BACK_CURSOR][0],
			header_col[HCOL_BACK_CURSOR][1],
			header_col[HCOL_BACK_CURSOR][2] );
	}

	{
		int const i = 3 * w;

		for ( int j = 1; j < h; j++ )
		{
			dest = buf.rgb() + i * j;
			memcpy ( dest, buf.rgb(), (size_t)i );
		}
	}

	font().set_style ( 0, 0, (mtPixy::Font::StyleUnderline)0, 0 );

	for ( int i = colbuf.c1(); i <= colbuf.c2(); i++ )
	{
		mtPixy::Pixmap const pixmap ( font().render_pixmap (
			colbuf.get_int_txt(i), 0 ) );
		if ( ! pixmap.get() )
		{
			continue;
		}

		unsigned char const * const mem = pixy_pixmap_get_alpha (
			pixmap.get());
		if ( ! mem )
		{
			continue;
		}

		unsigned char r, g, b;
		if ( colbuf.cursor_visible ( i ) )
		{
			r = header_col[HCOL_TEXT_CURSOR][0];
			g = header_col[HCOL_TEXT_CURSOR][1];
			b = header_col[HCOL_TEXT_CURSOR][2];
		}
		else
		{
			r = header_col[HCOL_TEXT][0];
			g = header_col[HCOL_TEXT][1];
			b = header_col[HCOL_TEXT][2];
		}

		// Get column X, Width
		int const cwidth = colbuf.col_w( i );
		int const image_w = pixy_pixmap_get_width ( pixmap.get() );
		int const image_h = pixy_pixmap_get_height ( pixmap.get() );
		int mx = colbuf.col_x( i );
		int mxo, mxw;

		if ( image_w > cwidth )
		{
			mxo = image_w - cwidth;
			mxw = cwidth;
		}
		else
		{
			mxo = 0;
			mxw = image_w;
			mx += (cwidth - image_w) / 2; // Right justify
		}

		cui_ren_mem_to_rgb ( mem, mx, 0, image_w, image_h, mxo,
			mxw, r, g, b, buf.rgb(), x, y, w, h );
	}

	callback ( x, y, w, h, buf.rgb(), 3, callback_data );

	return 0;
}

