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



int SheetRenCairo::export_pdf_multi_paged (
	CuiRenPage	const	& page,
	char	const	* const	filename,
	char	const	* const	mem_filename,
	int		const	filetype
	)
{
	if ( ! filename || ! viewren()->sheet() )
	{
		return 1;
	}

	m_filetype = filetype;

	if ( page_init () )
	{
		return 1;
	}

	m_page_width_px = (int)(mm_2_PT ( page.width_mm() ) + 0.5);
	m_page_height_px = (int)(mm_2_PT ( page.height_mm() ) + 0.5);

	if ( m_canvas.init ( mtPixy::Canvas::TYPE_PDF, filename,
		m_page_width_px, m_page_height_px ) )
	{
		return 1;
	}

	page_prepare_clean ();

	if (	! viewren()->sheet()->rows ||
		! viewren()->sheet()->rows->root
		)
	{
		return 0;
	}

	page_setup_data_time_text ();

	for ( int i = 0; i < 3; i++ )
	{
		m_head_txt[i] = page_setup_headfoot_text ( page.header(i),
			mem_filename );

		m_foot_txt[i] = page_setup_headfoot_text ( page.footer(i),
			mem_filename );
	}

	m_pgv_xl_px	= (int)(mm_2_PT ( page.margin_x_mm() ) + 0.5);
	m_pgv_xr_px	= m_page_width_px - m_pgv_xl_px;
	m_pgv_yh_px	= (int)(mm_2_PT ( page.margin_y_mm() ) + 0.5);
	m_pgv_ytop_px	= m_pgv_yh_px + viewren()->row_height() * 2;
	m_pgv_yf_px	= m_page_height_px - m_pgv_yh_px;
	m_pgv_ybot_px	= m_page_height_px - m_pgv_ytop_px;

	m_rows_per_page = (m_page_height_px - 2*m_pgv_ytop_px) /
		viewren()->row_height();

	if ( m_rows_per_page < 1 )
	{
		m_rows_per_page = 1;
	}

	// Calculate how many pages across are required
	int pages_across = 0;

	for ( int i = 1; i <= m_cols; i += m_cols_per_page )
	{
		m_cols_per_page = m_page_cols.page_cols_get ( i-1,
			(m_pgv_xr_px - m_pgv_xl_px) );
		pages_across ++;
	}

	if ( pages_across < 1 )
	{
		pages_across = 1;
	}

	int pages_down = (m_rows + m_rows_per_page - 1) / m_rows_per_page;

	if ( pages_down < 1 )
	{
		pages_down = 1;
	}

	m_pages_total = pages_down * pages_across;
	m_page_num = 1;

	cairo_t * const cr = m_canvas.get_cairo();

	for ( int r = 1; r <= m_rows; r += m_rows_per_page )
	{
		for ( int c = 1; c <= m_cols; c += m_cols_per_page )
		{
			snprintf ( m_pagenum_txt, sizeof(m_pagenum_txt),
				"%i / %i", m_page_num, m_pages_total );

			m_cols_per_page = m_page_cols.page_cols_get ( c - 1,
				(m_pgv_xr_px - m_pgv_xl_px) );
			page_prepare_header_footer ();

			if ( init_expose_sheet ( r, c,
				r + m_rows_per_page - 1,
				c + m_cols_per_page - 1 ) )
			{
				return 1;
			}

			// Last, rightmost visible column
			int const lc = MIN( m_cols, c + m_cols_per_page - 1 );

			double const cx = m_pgv_xl_px;
			double const cy = m_pgv_ytop_px;
			double const cw = col_x_pix(lc) + col_w_pix(lc);
			double const ch = m_pgv_ybot_px - m_pgv_ytop_px;

			cairo_save ( cr );
			cairo_rectangle ( cr, cx, cy, cw, ch );
			cairo_clip ( cr );

			page_recurse_rows ( viewren()->sheet()->rows->root );

			// Output page to PDF file
			cairo_show_page ( cr );
			cairo_restore ( cr );

			m_page_num ++;
			if ( m_page_num <= m_pages_total )
			{
				page_prepare_clean ();
			}
		}
	}

	return 0;
}

int SheetRenCairo::export_single_page (
	char	const * const	filename,
	int		const	filetype
	)
{
	if ( ! filename || ! viewren()->sheet() )
	{
		return 1;
	}

	m_filetype = filetype;

	if ( page_init () )
	{
		return 1;
	}

	if ( init_expose_sheet ( 1, 1, m_rows, m_cols ) )
	{
		return 1;
	}

	m_cols_per_page = m_cols;

	m_pgv_xr_px = m_page_width_px;
	m_pgv_yf_px = m_pgv_ybot_px = m_page_height_px;

	int	res = 1;

	// Create new surface based on new geometry and target file
	switch ( m_filetype )
	{
	case CUI_SHEET_EXPORT_EPS:
		res = m_canvas.init ( mtPixy::Canvas::TYPE_EPS, filename,
			m_page_width_px, m_page_height_px );
		break;

	case CUI_SHEET_EXPORT_PDF:
		res = m_canvas.init ( mtPixy::Canvas::TYPE_PDF, filename,
			m_page_width_px, m_page_height_px );
		break;

	case CUI_SHEET_EXPORT_PNG:
		res = m_canvas.init ( mtPixy::Canvas::TYPE_PIXMAP, filename,
			m_page_width_px, m_page_height_px );
		break;

	case CUI_SHEET_EXPORT_PS:
		res = m_canvas.init ( mtPixy::Canvas::TYPE_PS, filename,
			m_page_width_px, m_page_height_px );
		break;

	case CUI_SHEET_EXPORT_SVG:
		res = m_canvas.init ( mtPixy::Canvas::TYPE_SVG, filename,
			m_page_width_px, m_page_height_px );
		break;
	}

	if ( res )
	{
		return res;
	}

	page_prepare_clean ();

	if ( viewren()->sheet()->rows && viewren()->sheet()->rows->root )
	{
		res = page_recurse_rows ( viewren()->sheet()->rows->root );
	}
	else
	{
		res = 0;
	}

	if ( m_filetype == CUI_SHEET_EXPORT_PNG && ! res )
	{
		if ( m_canvas.save_png ( filename, 5 ) )
		{
			res = 1;
		}
	}

	return res;
}

int SheetRenCairo::render_expose (
	CuiRenCB const	callback,
	void	* const	callback_data
	)
{
	if ( m_canvas.init ( mtPixy::Canvas::TYPE_PIXMAP, nullptr, w(), h() ) )
	{
		return 1;
	}

	// Used to clear whole area & draw cursor by page_prepare_clean ();
	m_page_width_px = w();
	m_page_height_px = h();

	// Needed to adjust cell contents to correct pixel position
	m_pgv_xl_px	= 0;
	m_pgv_ytop_px	= -(y() % viewren()->row_height());

	page_prepare_clean ();

	if ( viewren()->sheet()->rows && viewren()->sheet()->rows->root )
	{
		if ( page_recurse_rows ( viewren()->sheet()->rows->root ) )
		{
			return 1;
		}
	}

	unsigned char const * const rgb = cairo_image_surface_get_data (
		m_canvas.get_surface() );

	callback ( x(), y(), w(), h(), rgb, 4, callback_data );

	return 0;
}

int SheetRenCairo::be_ren_background (
	CedCell	const * const	cell,
	int		const	col
	)
{
	int const r = pixy_int_2_red ( cell->prefs->color_background );
	int const g = pixy_int_2_green ( cell->prefs->color_background );
	int const b = pixy_int_2_blue ( cell->prefs->color_background );

	double const dx = m_pgv_xl_px + col_x_pix( col ) - x();
	double const dw = col_w_pix( col );
	double const dh = viewren()->row_height();

	// Output a coloured rectangle for this cell
	m_canvas.set_color ( r/255.0, g/255.0, b/255.0 );
	m_canvas.fill_rectangle ( dx, cell_y_px(), dw, dh );

	return 0;
}

void SheetRenCairo::render_border (
	int	const	type,
	int	const	dx,
	int	const	dy,
	int	const	dw,
	int	const	dh
	)
{
	cairo_t * const cr = m_canvas.get_cairo ();
	double		co[3];
	int	const	hori[3] = {
		(type >> CED_CELL_BORDER_TOP_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_MIDDLE_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_BOTTOM_SHIFT) & CED_CELL_BORDER_MASK
			};
	int	const	vert[3] = {
		(type >> CED_CELL_BORDER_LEFT_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_CENTER_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_RIGHT_SHIFT) & CED_CELL_BORDER_MASK
			};

	co[0] = dy;		// Top		-0.0	-0.0	+0.0,+2.0=1.0
	co[1] = dy + dh / 2;	// Middle	-0.5	-1.0	-1.5,+0.5=-0.5
	co[2] = dy + dh;	// Bottom	-1.0	-2.0	-3.0,-1.0=-2.0

	// Make pixmap rendering less blurry
	cairo_set_antialias ( cr, CAIRO_ANTIALIAS_NONE );

	for ( int i = 0; i < 3; i++ )
	{
		if ( hori[i] == CED_CELL_BORDER_THIN )
		{
			double const y = co[i] - ( (double)i / 2 );

			m_canvas.fill_rectangle ( dx, y, dw, 1 );
		}
		else if ( hori[i] == CED_CELL_BORDER_THICK )
		{
			double const y = co[i] - i;

			m_canvas.fill_rectangle ( dx, y, dw, 2 );
		}
		else if ( hori[i] == CED_CELL_BORDER_DOUBLE )
		{
			double const y = co[i] - i*1.5 + 1;

			m_canvas.fill_rectangle ( dx, y-1, dw, 1 );
			m_canvas.fill_rectangle ( dx, y+1, dw, 1 );
		}
	}

	co[0] = dx;		// Left		-0.0	-0.0	+0.0,+2.0=1.0
	co[1] = dx + dw / 2;	// Middle	-0.5	-1.0	-1.5,+0.5=-0.5
	co[2] = dx + dw;	// Right	-1.0	-2.0	-3.0,-1.0=-2.0

	for ( int i = 0; i < 3; i++ )
	{
		if ( vert[i] == CED_CELL_BORDER_THIN )
		{
			double const x = co[i] - ( (double)i / 2 );

			m_canvas.fill_rectangle ( x, dy, 1, dh );
		}
		else if ( vert[i] == CED_CELL_BORDER_THICK )
		{
			double const x = co[i] - i;

			m_canvas.fill_rectangle ( x, dy, 2, dh );
		}
		else if ( vert[i] == CED_CELL_BORDER_DOUBLE )
		{
			double const x = co[i] - i*1.5 + 1;

			m_canvas.fill_rectangle ( x-1, dy, 1, dh );
			m_canvas.fill_rectangle ( x+1, dy, 1, dh );
		}
	}

	cairo_set_antialias ( cr, CAIRO_ANTIALIAS_DEFAULT );
}

static void cui_font_set_attr (
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
		a = NULL;

		switch ( attr_val & CED_TEXT_STYLE_UNDERLINE_ANY )
		{
		case CED_TEXT_STYLE_UNDERLINE_SINGLE:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_SINGLE );
			break;

		case CED_TEXT_STYLE_UNDERLINE_DOUBLE:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_DOUBLE );
			break;

		case CED_TEXT_STYLE_UNDERLINE_WAVY:
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

	if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( attr_val ) )
	{
		a = pango_attr_strikethrough_new ( TRUE );
		pango_attr_list_insert ( list, a );
	}

	pango_layout_set_attributes ( layout, list );
	pango_attr_list_unref ( list );
}

void SheetRenCairo::be_ren_borders (
	CedCell	const * const	cell,
	int		const	col,	// Might be > c2(), always >= c1()
	int		const	cursor
	)
{
	int r, g, b;
	if ( cursor )
	{
		r = CURSOR_R_BORDER;
		g = CURSOR_G_BORDER;
		b = CURSOR_B_BORDER;
	}
	else
	{
		r = pixy_int_2_red ( cell->prefs->border_color );
		g = pixy_int_2_green ( cell->prefs->border_color );
		b = pixy_int_2_blue ( cell->prefs->border_color );
	}

	cairo_t * const	cr = m_canvas.get_cairo ();

	cairo_set_line_width ( cr, 1 );
	cairo_set_source_rgb ( cr, r / 255.0, g / 255.0, b / 255.0 );

	int const dx = m_pgv_xl_px + col_x_pix( col ) - x();
	render_border( cell->prefs->border_type, dx, cell_y_px(),
		col_w_pix( col ), viewren()->row_height() );
}

int SheetRenCairo::be_ren_text_prepare ( CedCell const * const cell )
{
	char	cbuf[ 2000 ];
	int	justify;

	if ( ced_cell_create_output ( cell, &justify, cbuf, sizeof(cbuf) ) )
	{
		return 1;
	}

	set_cell_text_justify ( justify );

	PangoLayout * const layout = m_canvas.get_layout ();
	PangoFontDescription * const font_desc = m_canvas.get_font_desc();
	int	const	text_style = cell->prefs ? cell->prefs->text_style : 0;

	pango_layout_set_text ( layout, cbuf, -1 );

	cui_font_set_attr ( layout, text_style );

	pango_layout_set_font_description ( layout, font_desc );

	PangoRectangle logical;
	pango_layout_get_extents ( layout, NULL, &logical);
	pango_extents_to_pixels ( NULL, &logical );

	int const basel = PANGO_PIXELS ( pango_layout_get_baseline ( layout ) );
	set_text_y_px ( cell_y_px() + viewren()->row_pad() +
		(viewren()->baseline() - basel) );
	set_text_width_px ( logical.width );

	justify_text_position ();

	return 0;
}

void SheetRenCairo::be_ren_text (
	CedCell const * const	cell,
	int		const	row,
	int		const	col
	)
{
	unsigned char	color[3];
	get_cell_text_rgb ( row, col, cell, color );

	cairo_t * const cr = m_canvas.get_cairo ();
	PangoLayout * const layout = m_canvas.get_layout ();

	cairo_save ( cr );

	// Clipping
	int const xo = m_pgv_xl_px - x();
	int const cx = cell_x_px() + xo;

	cairo_rectangle ( cr, cx, cell_y_px(), cell_width_px(),
		viewren()->row_height() );
	cairo_clip ( cr );

	// Text
	cairo_set_source_rgb ( cr, color[0] / 255.0, color[1] / 255.0,
		color[2] / 255.0 );
	cairo_move_to ( cr, text_x_px() + xo, text_y_px() );
	pango_cairo_update_layout ( cr, layout );
	pango_cairo_show_layout ( cr, layout );

	cairo_restore ( cr );
}

int SheetRenCairo::page_recurse_rows ( mtTreeNode const * const node )
{
	int const row = (int)(intptr_t)node->key;

	if ( node->left && row > r1() )
	{
		page_recurse_rows ( node->left );
	}

	if (	row >= r1() &&
		row <= r2()
		)
	{
		set_cell_y_px ( m_pgv_ytop_px + (row - r1()) *
			viewren()->row_height() );

		// Clear active cell flags
		clear_cell_act ();

		ced_sheet_scan_area( viewren()->sheet(), row, c1(), 1,
			c2() - c1() + 1, [](
				CedSheet *,
				CedCell	* const	cell,
				int	const	r,
				int	const	c,
				void	* const	user_data
				)
			{
				auto * const s = static_cast<SheetRenCore *>(
					user_data);

				return s->render_cell_background ( cell, r, c );
			},
			this );

		if (	cell_act_col(c1()) == 0 &&
			c1() > 1
			)
		{
			/*
			Find the first active cell to the left of the first
			exposed cell just in case it trails into view/ Check
			only up to 100 columns to the left for the sake of
			sanity.
			*/

			ced_sheet_scan_area_backwards ( viewren()->sheet(), row,
				c1() - 1, 1, 100, [](
					CedSheet *,
					CedCell	* const	cell,
					int	const	r,
					int	const	c,
					void	* const	user_data
					)
				{
					auto * const s = static_cast
						<SheetRenCore *>(user_data);

					return s->render_cell_left(cell, r, c );
				},
				this );
		}

		/*
		Render text for exposed cells + check first active cell to right
		of exposed area. Check only up to 100 columns to the right for
		the sake of sanity.
		*/
		ced_sheet_scan_area ( viewren()->sheet(), row, c1(), 1,
			c2() - c1() + 100, [](
				CedSheet *,
				CedCell	* const	cell,
				int	const	r,
				int	const	c,
				void	* const	user_data
				)
			{
				auto * const s = static_cast<SheetRenCore *>(
					user_data);

				return s->render_cell_foreground ( cell, r, c );
			},
			this );
	}

	if ( node->right && row < r2() )
	{
		page_recurse_rows( node->right );
	}

	return 0;
}

int SheetRenCairo::page_init ()
{
	if ( ced_sheet_get_geometry ( viewren()->sheet(), &m_rows, &m_cols ) )
	{
		return -1;
	}

	if (	m_rows > CUI_SHEET_MAX_ROW	||
		m_cols > CUI_SHEET_MAX_COL	||
		(m_rows * m_cols) > CUI_SHEET_MAX_AREA
		)
	{
		std::cerr << "Sheet is too large to export\n";
		return 1;
	}

	if ( m_rows < 1 )
	{
		m_rows = 1;
	}

	if ( m_cols < 1 )
	{
		m_cols = 1;
	}

	m_canvas.init ( mtPixy::Canvas::TYPE_PIXMAP, nullptr, 1, 1 );
	m_canvas.set_font_name ( viewren()->font_name().c_str() );

	if ( m_page_cols.init ( viewren(), m_cols ) )
	{
		return 1;
	}

	m_page_width_px = m_page_cols.width_px ();
	m_page_height_px = m_rows * viewren()->row_height();

	if ( m_page_width_px <= 0 )
	{
		m_page_width_px = 1;
	}

	if ( m_page_height_px <= 0 )
	{
		m_page_height_px = 1;
	}

	return 0;
}

void SheetRenCairo::page_prepare_clean ()
{
	m_canvas.set_font_name ( viewren()->font_name().c_str() );
	m_canvas.set_font_style ( 0 );
	m_canvas.set_font_size ( viewren()->font_size() * CUI_PANGO_SCALE );
	m_canvas.set_stroke_width ( 0 );

	// Clear background to white
	m_canvas.set_color ( 1.0, 1.0, 1.0 );
	m_canvas.fill_rectangle ( 0, 0, m_page_width_px, m_page_height_px );

	if ( draw_cursor() )
	{
		int const tr1 = MAX( r1(), cur_row1() );
		int const tr2 = MIN( r2(), cur_row2() );
		int const tc1 = MAX( c1(), cur_col1() );
		int const tc2 = MIN( c2(), cur_col2() );

		double const x = m_pgv_xl_px + col_x_pix( tc1 ) - this->x();
		double const w = col_x_pix( tc2 ) - col_x_pix( tc1 ) +
			col_w_pix( tc2 );

		double const y = m_pgv_ytop_px + viewren()->row_height() *
			(tr1 - r1());
		double const h = viewren()->row_height() * (tr2 - tr1 + 1);

		m_canvas.set_color (
			CURSOR_R_MAIN / 255.0,
			CURSOR_G_MAIN / 255.0,
			CURSOR_B_MAIN / 255.0 );
		m_canvas.fill_rectangle ( x, y, w, h );
	}
}

char const * SheetRenCairo::page_setup_headfoot_text (
	int		const	type,
	char	const	* const	filename
	) const
{
	switch ( type )
	{
	case HEADFOOT_FILENAME_SHORT:
	{
		char const * const st = strrchr ( filename, MTKIT_DIR_SEP );
		if ( st )
		{
			return (st + 1);
		}
	}
		// FALL THROUGH

	case HEADFOOT_FILENAME_LONG:
		return filename;

	case HEADFOOT_SHEET_NAME:
		if (	viewren()->sheet()->book_tnode &&
			viewren()->sheet()->book_tnode->key
			)
		{
			return (char const*)viewren()->sheet()->book_tnode->key;
		}

		return "?";

	case HEADFOOT_PAGE_NUM:
		return m_pagenum_txt;

	case HEADFOOT_DATE:
		return m_date_txt;

	case HEADFOOT_DATETIME:
		return m_datetime_txt;
	}

	return NULL;
}

void SheetRenCairo::send_headfoot (
	char	const	* const	txt,
	int		const	y,
	double		const	xpos
	)
{
	if ( ! txt )
	{
		return;
	}

	cairo_t * const cr = m_canvas.get_cairo ();
	PangoLayout * const layout = m_canvas.get_layout ();
	PangoFontDescription * const font_desc = m_canvas.get_font_desc ();

	// Get extents
	pango_layout_set_text ( layout, txt, -1 );
	pango_layout_set_font_description ( layout, font_desc );

	PangoRectangle logical;
	pango_layout_get_pixel_extents ( layout, NULL, &logical );

	double const x = (1 - xpos)*(m_pgv_xl_px) + xpos *
		(m_pgv_xr_px - logical.width );

	// Render
	cairo_move_to ( cr, x, y );
	pango_cairo_update_layout ( cr, layout );
	pango_cairo_show_layout ( cr, layout );
}

void SheetRenCairo::page_prepare_header_footer ()
{
	m_canvas.set_color ( 0, 0, 0 );

	int y = m_pgv_yh_px + viewren()->row_pad();

	send_headfoot ( m_head_txt[0], y, 0.0 );
	send_headfoot ( m_head_txt[1], y, 0.5 );
	send_headfoot ( m_head_txt[2], y, 1.0 );

	y = m_pgv_yf_px - viewren()->row_height() + viewren()->row_pad();

	send_headfoot ( m_foot_txt[0], y, 0.0 );
	send_headfoot ( m_foot_txt[1], y, 0.5 );
	send_headfoot ( m_foot_txt[2], y, 1.0 );
}

void SheetRenCairo::page_setup_data_time_text ()
{
	time_t		const	now = time ( nullptr );
	struct tm const * const	now_tm = localtime ( &now );

	if ( 0 == strftime ( m_date_txt, sizeof(m_date_txt), "%F", now_tm ) )
	{
		m_date_txt[0] = 0;
	}

	if ( 0 == strftime ( m_datetime_txt, sizeof(m_datetime_txt), "%F %T",
		now_tm ) )
	{
		m_datetime_txt[0] = 0;
	}
}

