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



// Convert mtCellEdit packed text style into mtPixy::Font style
static void font_set_attr (
	mtPixy::Font * const	font,
	int		const	input
	)
{
	int		bo=0, it=0, st=0;
	mtPixy::Font::StyleUnderline un = mtPixy::Font::STYLE_UNDERLINE_NONE;


	if ( CED_TEXT_STYLE_IS_BOLD ( input ) )
	{
		bo = 1;
	}

	if ( CED_TEXT_STYLE_IS_ITALIC ( input ) )
	{
		it = 1;
	}

	if ( CED_TEXT_STYLE_IS_UNDERLINE ( input ) )
	{
		switch ( input & CED_TEXT_STYLE_UNDERLINE_ANY )
		{
		case CED_TEXT_STYLE_UNDERLINE_DOUBLE:
			un = mtPixy::Font::STYLE_UNDERLINE_DOUBLE;
			break;

		case CED_TEXT_STYLE_UNDERLINE_WAVY:
			un = mtPixy::Font::STYLE_UNDERLINE_WAVY;
			break;

		default:
			un = mtPixy::Font::STYLE_UNDERLINE_SINGLE;
			break;
		}
	}

	if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( input ) )
	{
		st = 1;
	}

	font->set_style ( bo, it, un, st );
}

// Create an mtPixmap for this cell text +
// set mx position based on justification
int SheetRenPango::be_ren_text_prepare ( CedCell const * const cell )
{
	char	cbuf[ 2000 ];
	int	justify;

	if ( ced_cell_create_output ( cell, &justify, cbuf, sizeof(cbuf) ) )
	{
		return 1;
	}

	set_cell_text_justify ( justify );

	int	const	text_style = cell->prefs ? cell->prefs->text_style : 0;

	font_set_attr ( &viewren()->font(), text_style );

	m_text_pixmap.reset ( viewren()->font().render_pixmap ( cbuf,
		CUI_CELL_MAX_PIXELS ) );
	if ( ! m_text_pixmap.get() )
	{
		return 1;
	}

	if ( ! pixy_pixmap_get_alpha ( m_text_pixmap.get() ) )
	{
		m_text_pixmap.reset ( nullptr );

		return 1;
	}

	set_text_width_px ( pixy_pixmap_get_width ( m_text_pixmap.get() ) );

	justify_text_position ();

	return 0;
}

void SheetRenPango::be_ren_text (
	CedCell const * const	cell,
	int		const	row,
	int		const	col
	)
{
	unsigned char	color[3];
	get_cell_text_rgb ( row, col, cell, color );

	mtPixmap const * const pixmap = m_text_pixmap.get();

	int pos_x = 0;			// X start on text pixmap
	int pos_w = text_width_px();	// Width of text pixmap to paint
	int mx = text_x_px();		// X start on destination

	int const text_right = mx + pos_w;
	int const cell_right = cell_x_px() + cell_width_px();

	if ( text_right > cell_right )
	{
		// Clip right edge
		pos_w -= text_right - cell_right;
	}

	if ( mx < cell_x_px() )
	{
		// Clip left edge
		pos_x = cell_x_px() - mx;
		pos_w -= pos_x;
		mx = cell_x_px();
	}

	cui_ren_mem_to_rgb ( pixy_pixmap_get_alpha ( pixmap ), mx, 0,
		pixy_pixmap_get_width ( pixmap ),
		pixy_pixmap_get_height ( pixmap ),
		pos_x, pos_w, color[0], color[1], color[2],
		pixy_pixmap_get_canvas ( m_canvas.get() ), x(), 0, w(),
		row_height() );
}

static void border_line (
	unsigned char	*	dest,
	int		const	stride,
	int			tot,
	unsigned char	const	rgb[3]
	)
{
	for ( ; tot>0; tot--, dest += stride )
	{
		dest[0] = rgb[0];
		dest[1] = rgb[1];
		dest[2] = rgb[2];
	}
}

void SheetRenPango::render_border (
	int		const	type,
	int		const	col,
	unsigned char	const	rgb[3]
	)
{
	int		co[3], offset;
	int	const	hori[3] = {
			(type >> CED_CELL_BORDER_TOP_SHIFT) &
				CED_CELL_BORDER_MASK,
			(type >> CED_CELL_BORDER_MIDDLE_SHIFT) &
				CED_CELL_BORDER_MASK,
			(type >> CED_CELL_BORDER_BOTTOM_SHIFT) &
				CED_CELL_BORDER_MASK
			};
	int	const	vert[3] = {
			(type >> CED_CELL_BORDER_LEFT_SHIFT) &
				CED_CELL_BORDER_MASK,
			(type >> CED_CELL_BORDER_CENTER_SHIFT) &
				CED_CELL_BORDER_MASK,
			(type >> CED_CELL_BORDER_RIGHT_SHIFT) &
				CED_CELL_BORDER_MASK
			};

	co[0] = 0;
	co[1] = (row_height() >> 1) - 1;
	co[2] = row_height() - 3;

	int const stride = 3 * w();
	unsigned char * const dst = pixy_pixmap_get_canvas ( m_canvas.get() );

	set_cell_x_px ( col_x_pix( col ) );
	set_cell_width_px ( col_w_pix( col ) );
	normalize_xw ();

	for ( int i = 0; i < 3; i++ )
	{
		unsigned char * dest = dst + 3 * pan_cx() + co[i]*stride;

		if ( hori[i] == CED_CELL_BORDER_THIN )
		{
			border_line ( dest + i * stride, 3, pan_cwid(), rgb );
		}
		else if ( hori[i] == CED_CELL_BORDER_THICK )
		{
			border_line( dest + (i/2) * stride, 3, pan_cwid(), rgb);
			border_line( dest + (i/2 + 1) * stride, 3, pan_cwid(),
				rgb );
		}
		else if ( hori[i] == CED_CELL_BORDER_DOUBLE )
		{
			border_line ( dest, 3, pan_cwid(), rgb );
			border_line ( dest + 2 * stride, 3, pan_cwid(), rgb );
		}
	}

	co[0] = col_x_pix( col );
	co[1] = co[0] + col_w_pix( col ) / 2 - 1;
	co[2] = co[0] + col_w_pix( col ) - 3;

	for ( int i = 0; i < 3; i++ )
	{
		unsigned char * const dest = dst + 3 * (co[i] - x());

		if ( vert[i] == CED_CELL_BORDER_THIN )
		{
			if (	(co[i] + i) >= x() &&
				(co[i] + i) < (x() + w())
				)
			{
				border_line( dest + i*3, stride, row_height(),
					rgb );
			}
		}
		else if ( vert[i] == CED_CELL_BORDER_THICK )
		{
			offset = i / 2;

			if (	(co[i] + offset) >= x() &&
				(co[i] + offset) < (x() + w())
				)
			{
				border_line( dest + 3 * offset, stride,
					row_height(), rgb );
			}

			if (	(co[i] + offset + 1) >= x() &&
				(co[i] + offset + 1) < (x() + w())
				)
			{
				border_line ( dest + 3 * ( offset + 1), stride,
					row_height(), rgb );
			}
		}
		else if ( vert[i] == CED_CELL_BORDER_DOUBLE )
		{
			if (	co[i] >= x() &&
				co[i] < (x() + w())
				)
			{
				border_line( dest, stride, row_height(), rgb);
			}

			if (	(co[i] + 2) >= x() &&
				(co[i] + 2) < (x() + w())
				)
			{
				border_line ( dest + 6, stride, row_height(),
					rgb );
			}
		}
	}
}

void cui_ren_mem_to_rgb (
	unsigned char	const * const	mem,	// Rendered font memory (1bpp)
	int		const	mx,		// X - text render onto the RGB
	int		const	my,		// Y - text render onto the RGB
	int		const	mw,
	int		const	mh,		// mem geometry
	int		const	mxo,		// X origin
	int		const	mxw,		// X width to render
	unsigned char	const	mr,		// Font red
	unsigned char	const	mg,		// Font green
	unsigned char	const	mb,		// Font blue
	unsigned char	* const	rgb,		// RGB destination (3bpp)
	int		const	x,
	int		const	y,		// RGB origin
	int		const	w,
	int		const	h		// RGB geometry
	)
{
	int		x1 = mx,
			y1 = my,
			xw = mxw,
			yh = mh,
			mx1 = 0,
			my1 = 0;


	if ( x1 < x )
	{
		xw -= (x - x1);
		mx1 = (x - x1);
		x1 = x;
	}

	if ( (x1 + xw) > (x + w) )
	{
		xw = (x + w) - x1;
	}

	if ( y1 < y )
	{
		yh -= (y - y1);
		my1 = (y - y1);
		y1 = y;
	}

	if ( (y1 + yh) > (y + h) )
	{
		yh = (y + h) - y1;
	}

	for ( int j = 0; j < yh; j++ )
	{
		unsigned char const * src = mem + mw * ( j + my1 ) + mx1 + mxo;
		unsigned char * dest = rgb + 3 * ( w * (j + y1 - y) + (x1 - x));

		for ( int i = 0; i < xw; i++ )
		{
			unsigned char const b = *src++;

			if ( b )
			{
				dest[0] = (unsigned char)( ( b * mr +
					(255 - b) * dest[0] ) / 255 );

				dest[1] = (unsigned char)( ( b * mg +
					(255 - b) * dest[1] ) / 255 );

				dest[2] = (unsigned char)( ( b * mb +
					(255 - b) * dest[2] ) / 255 );
			}

			dest += 3;
		}
	}
}

int SheetRenPango::be_ren_background (
	CedCell	const * const	cell,
	int		const	col
	)
{
	unsigned char color[3];

	color[0] = (unsigned char)pixy_int_2_red (
		cell->prefs->color_background );

	color[1] = (unsigned char)pixy_int_2_green (
		cell->prefs->color_background );

	color[2] = (unsigned char)pixy_int_2_blue (
		cell->prefs->color_background );

	set_cell_x_px ( col_x_pix( col ) );
	set_cell_width_px ( col_w_pix( col ) );
	normalize_xw ();

	unsigned char * dest = pixy_pixmap_get_canvas ( m_canvas.get() ) +
		3 * pan_cx();

	for ( int tot = pan_cwid(); tot > 0; tot-- )
	{
		*dest ++ = color[0];
		*dest ++ = color[1];
		*dest ++ = color[2];
	}

	return 0;			// Continue
}

void SheetRenPango::normalize_xw ()
{
	m_pan_cx = cell_x_px() - x();
	m_pan_cwid = cell_width_px();

	if ( m_pan_cx < 0 )
	{
		m_pan_cwid += m_pan_cx;
		m_pan_cx = 0;
	}

	if ( (m_pan_cx + m_pan_cwid) > w() )
	{
		m_pan_cwid = w() - m_pan_cx;
	}
}

void SheetRenPango::be_ren_borders (
	CedCell	const * const	cell,
	int		const	col,	// Might be > c2(), always >= c1()
	int		const	cursor
	)
{
	unsigned char rgb[3];

	if ( cursor )
	{
		rgb[0] = CURSOR_R_BORDER;
		rgb[1] = CURSOR_G_BORDER;
		rgb[2] = CURSOR_B_BORDER;
	}
	else
	{
		rgb[0] = (unsigned char)pixy_int_2_red (
			cell->prefs->border_color );

		rgb[1] = (unsigned char)pixy_int_2_green (
			cell->prefs->border_color );

		rgb[2] = (unsigned char)pixy_int_2_blue (
			cell->prefs->border_color );
	}

	render_border ( cell->prefs->border_type, col, rgb );
}

int SheetRenPango::render_expose (
	CuiRenCB const	callback,
	void	* const	callback_data
	)
{
	int my = viewren()->y_from_row ( row_start(), r1() );

	m_canvas.reset ( pixy_pixmap_new_rgb ( w(), row_height() ) );
	if ( ! m_canvas.get() )
	{
		return 1;
	}

	int cur_x = 0, cur_w = 0;

	if ( draw_cursor() )
	{
		// Get the overlap between the redraw area and the cursor area
		int const ovl_c1 = MAX ( cur_col1(), c1() );
		int const ovl_c2 = MIN ( cur_col2(), c2() );

		// Get cursor start X and Width
		cur_x = col_x_pix( ovl_c1 );
		cur_w = col_x_pix( ovl_c2 ) + col_w_pix( ovl_c2 ) - cur_x;

		if ( cur_x < x() )
		{
			cur_w -= (x() - cur_x);
			cur_x  = x();
		}

		if ( (cur_x + cur_w) > (x() + w()) )
		{
			cur_w = (x() + w()) - cur_x;
		}
	}

	unsigned char * const rgb = pixy_pixmap_get_canvas ( m_canvas.get() );

	for ( int row = r1(); row <= r2(); row++, my += row_height() )
	{
		// Clear row scanline
		memset ( rgb, 255, (size_t)(w() * 3) );

		// Clear active cell flags
		clear_cell_act ();

		// Draw cursor scanline
		if (	draw_cursor() &&
			row >= cur_row1() &&
			row <= cur_row2()
			)
		{
			unsigned char * dest = rgb + 3*(cur_x - x());

			for ( int c = 0; c < cur_w; c++ )
			{
				*dest++ = CURSOR_R_MAIN;
				*dest++ = CURSOR_G_MAIN;
				*dest++ = CURSOR_B_MAIN;
			}
		}

		// Render coloured backgrounds for exposed cells
		ced_sheet_scan_area ( viewren()->sheet(), row, c1(), 1,
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

		// Duplicate scanline to whole row
		for ( int c = 1; c < row_height(); c++ )
		{
			memcpy ( rgb + w() * 3 * c, rgb, (size_t)(w() * 3) );
		}

		if (	cell_act_col(c1()) == 0 &&
			c1() > 1
			)
		{
			/*
			Find the first active cell to the left of the first
			exposed cell just in case it trails into view. Check
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
		of exposed area/ Check only up to 100 columns to the right for
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

		callback ( x(), my, w(), row_height(), rgb, 3, callback_data );
	}

	return 0;
}

