/*
	Copyright (C) 2011-2021 Mark Tyler

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



PageColSize::~PageColSize()
{
	clear();
}

void PageColSize::clear()
{
	free ( m_colx );
	m_colx = NULL;

	free ( m_colw );
	m_colw = NULL;
}

int PageColSize::page_cols_get (
	int	const	col,
	double	const	visible_width
	) const
{
	int		i;

	for ( i = col; i < m_cols; i++ )
	{
		double const cw = m_colx[i] - m_colx[col] + m_colw[i];

		if ( cw > visible_width )
		{
			break;
		}
	}

	if ( i == col )
	{
		// This single column is greater than the page width

		return 1;
	}

	return (i - col);
}

static int col_width_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	if (	cell->prefs &&
		cell->prefs->width > 0
		)
	{
		auto * const state = static_cast<PageColSize *>(user_data);

		state->set_colw ( col - 1, cell->prefs->width *
			state->viewren()->font_width() );
	}

	return 0;		// continue
}

int PageColSize::init (
	CuiRender	* const	viewren,
	int		const	coltot
	)
{
	// Create and populate array of column widths/x coords
	clear();

	m_colw = (int *)calloc ( (size_t)coltot, sizeof( m_colw[0] ) );
	m_colx = (int *)calloc ( (size_t)coltot, sizeof( m_colx[0] ) );
	m_cols = coltot;
	m_viewren = viewren;

	if ( ! m_colw || ! m_colx )
	{
		return 1;
	}

	for ( int i = 0; i < m_cols; i++ )
	{
		m_colw[i] = CUI_DEFAULT_CELLWIDTH_CHARS *
			viewren->font_width();
	}

	if ( ced_sheet_scan_area ( viewren->sheet(), 0, 1, 1, coltot,
		col_width_cb, this ) )
	{
		return 1;
	}

	int c = 0;
	for ( int i = 0; i < m_cols; i++ )
	{
		m_colx[i] = c;

		c += m_colw[i];
	}

	m_width_px = c;

	return 0;
}

CuiRenPage::CuiRenPage ( mtKit::UserPrefs const & uprefs )
{
	m_width_mm = uprefs.get_int ( CUI_INIFILE_PAGE_WIDTH );
	m_height_mm = uprefs.get_int ( CUI_INIFILE_PAGE_HEIGHT );
	m_margin_x_mm = uprefs.get_int ( CUI_INIFILE_PAGE_MARGIN_X );
	m_margin_y_mm = uprefs.get_int ( CUI_INIFILE_PAGE_MARGIN_Y );
	m_footer[0] = uprefs.get_int ( CUI_INIFILE_PAGE_FOOTER_LEFT );
	m_footer[1] = uprefs.get_int ( CUI_INIFILE_PAGE_FOOTER_CENTRE );
	m_footer[2] = uprefs.get_int ( CUI_INIFILE_PAGE_FOOTER_RIGHT );
	m_header[0] = uprefs.get_int ( CUI_INIFILE_PAGE_HEADER_LEFT );
	m_header[1] = uprefs.get_int ( CUI_INIFILE_PAGE_HEADER_CENTRE );
	m_header[2] = uprefs.get_int ( CUI_INIFILE_PAGE_HEADER_RIGHT );
}



/// ----------------------------------------------------------------------------



int CuiRender::export_output (
	mtKit::UserPrefs const	& uprefs,
	char	const	* const	filename,
	char	const	*	gui_filename,
	int		const	filetype
	)
{
	// Subroutines check args

	switch ( filetype )
	{
	case CUI_SHEET_EXPORT_TSV:
		return ced_sheet_save ( sheet(), filename,
			CED_FILE_TYPE_OUTPUT_TSV );

	case CUI_SHEET_EXPORT_TSV_QUOTED:
		return ced_sheet_save ( sheet(), filename,
			CED_FILE_TYPE_OUTPUT_TSV_QUOTED );

	case CUI_SHEET_EXPORT_HTML:
		return ced_sheet_save ( sheet(), filename,
			CED_FILE_TYPE_OUTPUT_HTML );
	}

	if ( ! gui_filename )
	{
		gui_filename = "";
	}

	SheetRenCairo	kr ( this );

	if ( filetype == CUI_SHEET_EXPORT_PDF_PAGED )
	{
		CuiRenPage	page ( uprefs );

		return kr.export_pdf_multi_paged ( page, filename, gui_filename,
			filetype );
	}

	return kr.export_single_page ( filename, filetype );
}

int CuiRender::set_backend_pango (
	char	const * const	font_name,
	int		const	font_size
	)
{
	m_backend = RENDERER_PANGO;
	m_font.set_font ( font_name, font_size );
	m_font_height = m_font.get_height();
	m_font_width = m_font.get_width();
	m_font_size = m_font.get_size();
	m_baseline = m_font.get_baseline();

	update_font_calcs ();

	return 0;
}

int CuiRender::set_backend_cairo (
	char	const * const	font_name,
	int		const	font_size
	)
{
	if ( ! font_name )
	{
		return 1;
	}

	m_backend = RENDERER_CAIRO;
	m_font_name = font_name;
	m_font_size = font_size;

	mtPixy::Canvas canvas;

	if ( canvas.init ( mtPixy::Canvas::TYPE_PIXMAP, nullptr, 1, 1 ) )
	{
		std::cerr << "set_backend_cairo: unable to canvas.init()\n";
		return 1;
	}

	PangoLayout * const layout = canvas.get_layout ();
	PangoFontDescription * const font_desc = canvas.get_font_desc ();

	pango_layout_set_text ( layout, "0123456789", -1 );
	pango_font_description_set_weight ( font_desc, PANGO_WEIGHT_NORMAL );
	pango_font_description_set_size ( font_desc,
		(gint)( ((double)font_size) * PANGO_SCALE * CUI_PANGO_SCALE ) );
	pango_layout_set_font_description ( layout, font_desc );

	PangoRectangle logical;
	pango_layout_get_extents ( layout, NULL, &logical );
	pango_extents_to_pixels ( NULL, &logical );

	m_font_width = logical.width / 10;
	m_font_height = logical.height;
	m_baseline = PANGO_PIXELS( pango_layout_get_baseline( layout ));

	update_font_calcs ();

	return 0;
}

std::string const & CuiRender::font_name () const
{
	if ( renderer() == RENDERER_PANGO )
	{
		return m_font.get_name();
	}

	return m_font_name;
}

int CuiRender::expose_sheet (
	int		const	row_start,
	int		const	col_start,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	if ( renderer() == RENDERER_CAIRO )
	{
		SheetRenCairo cren ( this );

		if ( cren.init_expose ( row_start, col_start, x, y, w, h ) )
		{
			return 1;
		}

		return cren.render_expose ( callback, callback_data );
	}
	else if ( renderer() == RENDERER_PANGO )
	{
		SheetRenPango pren ( this );

		if ( pren.init_expose ( row_start, col_start, x, y, w, h ) )
		{
			return 1;
		}

		return pren.render_expose ( callback, callback_data );
	}

	return 1;
}

int CuiRender::expose_row_header (
	int	const	row_start,
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h,
	CuiRenCB const	callback,
	void	* const	callback_data
	)
{
	if ( renderer() == RENDERER_CAIRO )
	{
		return expose_row_header_cairo ( row_start, x, y, w, h,
			callback, callback_data );
	}
	else if ( renderer() == RENDERER_PANGO )
	{
		return expose_row_header_pango ( row_start, x, y, w, h,
			callback, callback_data );
	}

	return 1;
}

int CuiRender::expose_column_header (
	int	const	col_start,
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h,
	CuiRenCB const	callback,
	void	* const	callback_data
	)
{
	if ( renderer() == RENDERER_CAIRO )
	{
		return expose_column_header_cairo ( col_start, x, y, w, h,
			callback, callback_data );
	}
	else if ( renderer() == RENDERER_PANGO )
	{
		return expose_column_header_pango ( col_start, x, y, w, h,
			callback, callback_data );
	}

	return 1;
}

int CuiRender::cell_width ( int const column ) const
{
	CedCell const * const cell = ced_sheet_get_cell ( sheet(), 0, column );

	if ( ! cell )
	{
		return default_cell_width();
	}

	int cellw;

	if ( cell->prefs )
	{
		cellw = cell->prefs->width * font_width();
	}
	else
	{
		cellw = 0;
	}

	if ( cellw == 0 )
	{
		return default_cell_width();
	}
	else if	( cellw < 0 )
	{
		return 0;
	}

	return cellw;
}

