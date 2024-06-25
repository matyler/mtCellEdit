/*
	Copyright (C) 2011-2024 Mark Tyler

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

#ifndef CUI_SHEET_H_
#define CUI_SHEET_H_



#include "mtpixy_cairo.h"
#include "cui.h"



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif


#ifdef __cplusplus
extern "C" {
#endif



// Cursor background on main sheet
#define CURSOR_R_MAIN	20
#define CURSOR_G_MAIN	60
#define CURSOR_B_MAIN	120

// Cursor border foreground on main sheet
#define CURSOR_R_BORDER	0
#define CURSOR_G_BORDER	150
#define CURSOR_B_BORDER	255

// Cursor text foreground on main sheet
#define CURSOR_R_TEXT	255
#define CURSOR_G_TEXT	255
#define CURSOR_B_TEXT	255

#define HEADER_R_BACK	200
#define HEADER_G_BACK	200
#define HEADER_B_BACK	190

#define HEADER_R_CURS_BACK	0
#define HEADER_G_CURS_BACK	0
#define HEADER_B_CURS_BACK	100

#define HEADER_R_TEXT	0
#define HEADER_G_TEXT	0
#define HEADER_B_TEXT	0

#define HEADER_R_CURS_TEXT	255
#define HEADER_G_CURS_TEXT	255
#define HEADER_B_CURS_TEXT	255

#define mm_2_PT(MM)	(MM * 72 / 25.4)



class CuiRenPage;
class PageColSize;
class SheetRenCairo;
class SheetRenCore;
class SheetRenPango;



void cui_ren_mem_to_rgb (
	unsigned char	const * mem,	// Rendered font memory (1bpp)
	int			mx,	// X - text render onto the RGB
	int			my,	// Y - text render onto the RGB
	int			mw,
	int			mh,	// mem geometry
	int			mxo,	// X origin
	int			mxw,	// X width to render
	unsigned char		mr,	// Font red
	unsigned char		mg,	// Font green
	unsigned char		mb,	// Font blue
	unsigned char	*	rgb,	// RGB destination (3bpp)
	int			x,
	int			y,	// RGB origin
	int			w,
	int			h	// RGB geometry
	);



class SheetRenCore
{
public:
	explicit SheetRenCore ( CuiRender * const render )
		:
		m_viewren	( render )
	{
	}

	virtual ~SheetRenCore ()
	{
		free_column_arrays ();
	}

	// Also initializes and populates the column arrays
	int init_expose (
		int	row_start,
		int	col_start,
		int	x,
		int	y,
		int	w,
		int	h
		);

	// Also initializes and populates the column arrays
	int init_expose_sheet ( int r1, int c1, int r2, int c2 );

	int render_cell_left (		// Render cells to the left which
					// intrude onto the render area.
		CedCell	const * cell,
		int		row,
		int		col
		);
	void prepare_ren_expansion (	// Expand the rendered text as required,
					// and then render.
		CedCell const * cell,
		int		row,
		int		col,
		int		col_exp
		);
	void justify_text_position ();

	int render_cell_text ( CedCell const * cell, int row, int col );
	int render_cell_background (
		CedCell const * cell,
		int		row,
		int		col
		);
	int render_cell_foreground (
		CedCell const * cell,
		int		row,
		int		col
		);

	// Implemented by Pango/Cairo backends
	virtual int be_ren_text_prepare ( CedCell const * cell ) = 0;
	virtual void be_ren_text (
		CedCell const * cell,
		int		row,
		int		col
		) = 0;
	virtual int be_ren_background (
		CedCell const * cell,
		int		col
		) = 0;
	virtual void be_ren_borders (
		CedCell const * cell,
		int		col,
		int		cursor
		) = 0;

	void get_cell_text_rgb (
		int		row,
		int		col,
		CedCell const * cell,
		unsigned char	color[3]
		) const;

	inline int has_cursor ( int const row, int const col ) const
	{
		if (	row >= m_cur_row1 &&
			row <= m_cur_row2 &&
			col >= m_cur_col1 &&
			col <= m_cur_col2
			)
		{
			return 1;
		}

		return 0;
	}

	inline int row_start() const	{ return m_row_start; }
	inline int col_start() const	{ return m_col_start; }
	inline CuiRender * viewren() const { return m_viewren; }
	inline int x() const		{ return m_x; }
	inline int y() const		{ return m_y; }
	inline int w() const		{ return m_w; }
	inline int h() const		{ return m_h; }
	inline int draw_cursor() const	{ return m_draw_cursor; }

	inline unsigned char cell_act_col ( int i ) const
	{
		return m_cell_act[i - m_c1];
	}
	inline int col_x_pix ( int i ) const { return m_col_x_px[i - m_c1]; }
	inline int col_w_pix ( int i ) const { return m_col_w_px[i - m_c1]; }

	inline void set_cell_act ( int const i ) const
	{
		m_cell_act[ i - m_c1 ] = 1;
	}

	inline void clear_cell_act () const
	{
		memset ( m_cell_act, 0, (size_t)(m_c2 - m_c1 + 1) );
	}

	inline int c1() const { return m_c1; }
	inline int c2() const { return m_c2; }
	inline int r1() const { return m_r1; }
	inline int r2() const { return m_r2; }

	inline void set_cell_text_justify( int j ) { m_cell_text_justify = j; }
	inline int cell_text_justify() const { return m_cell_text_justify; }

	inline void set_cell_width_px( int w ) { m_cell_width_px = w; }
	inline int cell_width_px() const { return m_cell_width_px; }
	inline void set_cell_x_px( int x ) { m_cell_x_px = x; }
	inline int cell_x_px() const { return m_cell_x_px; }
	inline void set_cell_y_px( int y ) { m_cell_y_px = y; }
	inline int cell_y_px() const { return m_cell_y_px; }

	inline void set_text_width_px( int w ) { m_text_width_px = w; }
	inline int text_width_px() const { return m_text_width_px; }
	inline void set_text_x_px( int x ) { m_text_x_px = x; }
	inline int text_x_px() const { return m_text_x_px; }
	inline void set_text_y_px( int y ) { m_text_y_px = y; }
	inline int text_y_px() const { return m_text_y_px; }

	inline int cur_row1() const { return m_cur_row1; }
	inline int cur_row2() const { return m_cur_row2; }
	inline int cur_col1() const { return m_cur_col1; }
	inline int cur_col2() const { return m_cur_col2; }

	inline int row_height() const { return m_viewren->row_height(); };

private:
	void free_column_arrays ();
	int init_column_arrays ( int c1, size_t tot );

/// ----------------------------------------------------------------------------

	unsigned char	* m_cell_act	= nullptr;
	int		* m_col_x_px	= nullptr;
	int		* m_col_w_px	= nullptr;

	int		m_row_start	= 0;
	int		m_col_start	= 0;
	CuiRender * const m_viewren;
	int		m_x		= 0;
	int		m_y		= 0;
	int		m_w		= 0;
	int		m_h		= 0;

	int		m_r1		= 0;
	int		m_r2		= 0;
	int		m_c1		= 0;
	int		m_c2		= 0;

	int		m_cell_text_justify = 0;
	int		m_text_width_px	= 0;
	int		m_text_x_px	= 0;
	int		m_text_y_px	= 0;
	int		m_cell_width_px	= 0;
	int		m_cell_x_px	= 0;
	int		m_cell_y_px	= 0;

	int		m_cur_row1	= 0;
	int		m_cur_row2	= 0;
	int		m_cur_col1	= 0;
	int		m_cur_col2	= 0;

	// Cursor overlap with visible area
	int		m_draw_cursor	= 0;
};



class SheetRenPango : public SheetRenCore
{
public:
	explicit SheetRenPango ( CuiRender * const render )
		:
		SheetRenCore ( render )
	{
	}

	// Conversion from core coords to clipped Pango bitmap coords
	void normalize_xw ();
	inline int pan_cwid() const { return m_pan_cwid; }
	inline int pan_cx() const { return m_pan_cx; }

	void render_border (
		int			type,
		int			col,
		unsigned char	const	rgb[3]
		);

	int render_expose ( CuiRenCB callback, void * callback_data );

	// Override core virtual funcs
	int be_ren_text_prepare ( CedCell const * cell )	override;
	void be_ren_text ( CedCell const * cell, int row, int col ) override;
	int be_ren_background ( CedCell const * cell, int col )	override;
	void be_ren_borders ( CedCell const * cell, int col, int cursor )
			override;

private:
	int		m_pan_cwid = 0;
	int		m_pan_cx = 0;

	mtPixy::Pixmap	m_canvas;
	mtPixy::Pixmap	m_text_pixmap;
};



class PageColSize
{
public:
	~PageColSize();

	int init ( CuiRender * viewren, int coltot );

	int page_cols_get ( int col, double visible_width ) const;
	int width_px () const { return m_width_px; }

	inline void set_colw ( int c, int w ) const { m_colw[c] = w; }
	inline CuiRender * viewren() const { return m_viewren; }

private:
	void clear();

/// ----------------------------------------------------------------------------

	int		* m_colx	= nullptr;
	int		* m_colw	= nullptr;

	int		m_width_px	= 0;
	int		m_cols		= 0;
	CuiRender	* m_viewren	= nullptr;
};



class SheetRenCairo : public SheetRenCore
{
public:
	explicit SheetRenCairo ( CuiRender * const render )
		:
		SheetRenCore ( render )
	{
	}

	enum
	{
		HEADFOOT_NONE,
		HEADFOOT_FILENAME_LONG,
		HEADFOOT_FILENAME_SHORT,
		HEADFOOT_SHEET_NAME,
		HEADFOOT_PAGE_NUM,
		HEADFOOT_DATE,
		HEADFOOT_DATETIME,

		HEADFOOT_TOTAL
	};


	void render_border (
		int type,
		int dx,
		int dy,
		int dw,
		int dh
		);

	int page_init ();	// Only needed for file exports

	void page_prepare_clean ();

	int page_recurse_rows ( mtTreeNode const * node );
	char const * page_setup_headfoot_text (
		int type,		// HEADFOOT_*
		char const * filename
		) const;

	void page_prepare_header_footer ();
	void page_setup_data_time_text ();

	void send_headfoot (
		char	const	* txt,
		int		y,
		double		xpos
		);

	int export_pdf_multi_paged (
		CuiRenPage const & page,
		char	const *	filename,
		char	const *	mem_filename,
		int		filetype
		);
	int export_single_page (
		char	const *	filename,
		int		filetype
		);

	int render_expose ( CuiRenCB callback, void * callback_data );

	// Override core virtual funcs
	int be_ren_text_prepare ( CedCell const * cell )	override;
	void be_ren_text ( CedCell const * cell, int row, int col ) override;
	int be_ren_background ( CedCell const * cell, int col )	override;
	void be_ren_borders ( CedCell const * cell, int col, int cursor )
			override;

/// ----------------------------------------------------------------------------

private:
	PageColSize	m_page_cols;

	int		m_filetype	= 0;

	int		m_page_width_px	= 0.0;
	int		m_page_height_px = 0.0;

	int		m_pgv_xl_px	= 0.0;
	int		m_pgv_xr_px	= 0.0;	// Visible left/right

	int		m_pgv_yh_px	= 0.0;
	int		m_pgv_ytop_px	= 0.0;	// Header/top Y

	int		m_pgv_yf_px	= 0.0;
	int		m_pgv_ybot_px	= 0.0;	// Footer/bottom Y

	int		m_rows		= 0;
	int		m_cols		= 0;	// Total in this sheet
	int		m_page_num	= 0;
	int		m_pages_total	= 0;
	int		m_rows_per_page	= 0;
	int		m_cols_per_page	= 0;

	char	const	* m_head_txt[3] = {nullptr};
	char	const	* m_foot_txt[3] = {nullptr};
	char		m_date_txt[128] = {0};
	char		m_datetime_txt[128] = {0};
	char		m_pagenum_txt[128] = {0};

	mtPixy::Canvas	m_canvas;
};



class CuiRenPage
{
public:
	explicit CuiRenPage ( mtKit::UserPrefs const & uprefs );

	inline int width_mm() const	{ return m_width_mm; }
	inline int height_mm() const	{ return m_height_mm; }
	inline int margin_x_mm() const	{ return m_margin_x_mm; }
	inline int margin_y_mm() const	{ return m_margin_y_mm; }
	inline int footer(int f) const	{ return m_footer[f]; }
	inline int header(int h) const	{ return m_header[h]; }

private:
	int		m_width_mm	= 0;
	int		m_height_mm	= 0;
	int		m_margin_x_mm	= 0;
	int		m_margin_y_mm	= 0;
	int		m_footer[3]	= { 0 };
	int		m_header[3]	= { 0 };
};



#ifdef __cplusplus
}
#endif



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// CUI_SHEET_H_

