/*
	Copyright (C) 2016 Mark Tyler

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



void mtPixyUI::File::set_tool_mode (
	ToolMode	const	m
	)
{
	if ( m == m_tool_mode )
	{
		return;
	}

	switch ( m_tool_mode )
	{
	case TOOL_MODE_PAINTING:
		paint_brush_finish ();
		break;

	default:
		break;
	}

	m_tool_mode = m;
}

mtPixyUI::File::ToolMode mtPixyUI::File::get_tool_mode () const
{
	return m_tool_mode;
}

int mtPixyUI::File::reset_tool_mode ()
{
	switch ( m_tool_mode )
	{
	case TOOL_MODE_PAINTING:
		set_tool_mode ( TOOL_MODE_PAINT );
		return 1;

	case TOOL_MODE_LINING:
		set_tool_mode ( TOOL_MODE_LINE );
		return 1;

	case TOOL_MODE_SELECTING_RECTANGLE:
	case TOOL_MODE_SELECTED_RECTANGLE:
	case TOOL_MODE_PASTE:
	case TOOL_MODE_PASTING:
		set_tool_mode ( TOOL_MODE_SELECT_RECTANGLE );
		return 1;

	case TOOL_MODE_SELECTING_POLYGON:
	case TOOL_MODE_SELECTED_POLYGON:
		set_tool_mode ( TOOL_MODE_SELECT_POLYGON );
		return 1;

	default:
		break;
	}

	return 0;
}

int mtPixyUI::File::paint_brush_start (
	int	const	x,
	int	const	y,
	int		&dirty_x,
	int		&dirty_y,
	int		&dirty_w,
	int		&dirty_h
	)
{
	m_tool_mode = TOOL_MODE_PAINTING;
	brush.set_space_mod ( 0 );

	int res = m_image->paint_brush ( brush, x, y, x, y, dirty_x, dirty_y,
		dirty_w, dirty_h );

	if ( 0 == res )
	{
		palette_mask.protect ( m_undo_stack.get_current_image (),
			m_image, dirty_x, dirty_y, dirty_w, dirty_h );
	}

	m_brush_x = x;
	m_brush_y = y;

	m_modified = 1;

	return res;
}

int mtPixyUI::File::paint_brush_to (
	int	const	x,
	int	const	y,
	int		&dirty_x,
	int		&dirty_y,
	int		&dirty_w,
	int		&dirty_h
	)
{
	if ( TOOL_MODE_PAINTING != m_tool_mode )
	{
		return paint_brush_start ( x, y, dirty_x, dirty_y, dirty_w,
			dirty_h );
	}

	if ( x == m_brush_x && y == m_brush_y )
	{
		return 1;
	}


	int res = m_image->paint_brush ( brush, m_brush_x, m_brush_y, x, y,
		dirty_x, dirty_y, dirty_w, dirty_h, true );


	if ( 0 == res )
	{
		palette_mask.protect ( m_undo_stack.get_current_image (),
			m_image, dirty_x, dirty_y, dirty_w, dirty_h );
	}

	m_brush_x = x;
	m_brush_y = y;

	return res;
}

int mtPixyUI::File::paint_brush_to (
	int	const	x,
	int	const	y
	)
{
	int		dx, dy, dw, dh;

	return paint_brush_to ( x, y, dx, dy, dw, dh );
}

int mtPixyUI::File::paint_brush_finish ()
{
	if ( TOOL_MODE_PAINTING == m_tool_mode )
	{
		m_tool_mode = TOOL_MODE_PAINT;

		m_undo_stack.add_next_step ( m_image );
	}

	return 0;
}

int mtPixyUI::File::paint_line (
	int	const	x1,
	int	const	y1,
	int	const	x2,
	int	const	y2
	)
{
	int		dx, dy, dw, dh;
	int	const	res = m_image->paint_brush ( brush, x1, y1, x2, y2,
				dx, dy, dw, dh );


	if ( 0 == res )
	{
		palette_mask.protect ( m_undo_stack.get_current_image (),
			m_image, dx, dy, dw, dh );

		m_modified = 1;

		m_undo_stack.add_next_step ( m_image );
	}

	return res;
}

int mtPixyUI::File::flood_fill (
	int	const	x,
	int	const	y
	)
{
	if ( palette_mask.is_masked ( m_image, x, y ) )
	{
		return 0;
	}

	if ( m_image->paint_flood_fill ( brush, x, y ) )
	{
		return 1;
	}

	m_undo_stack.add_next_step ( m_image );

	m_modified = 1;

	return 0;
}

int mtPixyUI::File::get_pixel_info (
	int		const	canvas_x,
	int		const	canvas_y,
	unsigned char		&pixel_red,
	unsigned char		&pixel_green,
	unsigned char		&pixel_blue,
	int			&pixel_index
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	unsigned char		* const	canvas = m_image->get_canvas ();
	int			const	imw = m_image->get_width ();
	int			const	imh = m_image->get_height ();
	mtPixy::Palette		* const pal = m_image->get_palette ();
	mtPixy::Color		* const col = pal->get_color ();

	if (	! canvas		||
		canvas_x < 0		||
		canvas_y < 0		||
		canvas_x >= imw		||
		canvas_y >= imh
		)
	{
		return 1;
	}

	int			const	bpp = m_image->get_canvas_bpp ();
	unsigned char	const * const	s = canvas + bpp *
						(canvas_x + canvas_y * imw);

	if ( bpp == 1 )
	{
		pixel_red	= col[ s[0] ].red;
		pixel_green	= col[ s[0] ].green;
		pixel_blue	= col[ s[0] ].blue;
		pixel_index	= s[0];
	}
	else if ( bpp == 3 )
	{
		pixel_red	= s[0];
		pixel_green	= s[1];
		pixel_blue	= s[2];
		pixel_index	= pal->get_color_index ( s[0], s[1], s[2] );
	}

	return 0;
}

int mtPixyUI::File::rectangle_fill ()
{
	int		x, y, w, h;

	rectangle_overlay.get_xywh ( x, y, w, h );

	if ( m_image->paint_rectangle ( brush, x, y, w, h ) )
	{
		return 1;
	}

	palette_mask.protect ( m_undo_stack.get_current_image (), m_image, x, y,
		w, h );

	m_modified = 1;

	m_undo_stack.add_next_step ( m_image );

	return 0;
}

int mtPixyUI::File::rectangle_outline ()
{
	int		x, y, w, h;

	rectangle_overlay.get_xywh ( x, y, w, h );

	ToolMode	const	old_tool_mode = m_tool_mode;
	int		const	x2 = x + w - 1;
	int		const	y2 = y + h - 1;
	int			dx, dy, dw, dh;

	paint_brush_start ( x, y, dx, dy, dw, dh );
	paint_brush_to ( x2, y, dx, dy, dw, dh );
	paint_brush_to ( x2, y2, dx, dy, dw, dh );
	paint_brush_to ( x, y2, dx, dy, dw, dh );
	paint_brush_to ( x, y, dx, dy, dw, dh );
	paint_brush_finish ();

	m_tool_mode = old_tool_mode;

	return 0;
}

int mtPixyUI::File::polygon_fill ()
{
	int		x, y, w, h;

	if ( m_image->paint_polygon ( brush, polygon_overlay, x, y, w, h ) )
	{
		return 1;
	}

	palette_mask.protect ( m_undo_stack.get_current_image (), m_image, x, y,
		w, h );

	m_modified = 1;

	m_undo_stack.add_next_step ( m_image );

	return 0;
}

int mtPixyUI::File::polygon_outline ()
{
	if ( polygon_overlay.m_point_total < 2 )
	{
		return 1;
	}

	ToolMode const	old_tool_mode = m_tool_mode;
	int		dx, dy, dw, dh;

	paint_brush_start ( polygon_overlay.m_x[0], polygon_overlay.m_y[0],
		dx, dy, dw, dh );

	for ( int i = 1; i < polygon_overlay.m_point_total; i++ )
	{
		paint_brush_to ( polygon_overlay.m_x[i], polygon_overlay.m_y[i],
			dx, dy, dw, dh );
	}

	paint_brush_to ( polygon_overlay.m_x[0], polygon_overlay.m_y[0], dx, dy,
		dw, dh );
	paint_brush_finish ();

	m_tool_mode = old_tool_mode;

	return 0;
}

int mtPixyUI::File::select_all ()
{
	if ( ! m_image )
	{
		return 1;
	}

	int xx, yy, ww, hh;

	rectangle_overlay.set_start ( 0, 0 );
	rectangle_overlay.set_end ( m_image->get_width () - 1,
		m_image->get_height () - 1, xx, yy, ww, hh );

	m_tool_mode = mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE;

	return 0;
}

int mtPixyUI::File::selection_copy (
	Clipboard	&clipboard
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	int		xx = 0, yy = 0, w = 1, h = 1;
	mtPixy::Image	* clp = NULL;

	switch ( m_tool_mode )
	{
	case TOOL_MODE_SELECTED_RECTANGLE:
		rectangle_overlay.get_xywh ( xx, yy, w, h );
		clp = m_image->resize ( xx, yy, w, h );
		break;

	case TOOL_MODE_SELECTED_POLYGON:
		clp = polygon_overlay.copy ( m_image, xx, yy, w, h );
		break;

	default:
		return 1;
	}

	if ( ! clp )
	{
		return 1;
	}

	if ( clipboard.set_image ( clp, xx, yy ) )
	{
		delete clp;
		return 1;
	}

	return 0;
}

int mtPixyUI::File::selection_lasso (
	Clipboard	&clipboard
	)
{
	int		x1, y1, x2, y2, xx = 0, yy = 0;

	switch ( m_tool_mode )
	{
	case TOOL_MODE_SELECTED_RECTANGLE:
		{
			rectangle_overlay.get_xy ( x1, y1, x2, y2 );
			xx = x1 - MIN ( x1, x2 );
			yy = y1 - MIN ( y1, y2 );
		}
		break;

	case TOOL_MODE_SELECTED_POLYGON:
		{
			polygon_overlay.get_xywh ( x1, y1, x2, y2 );
			xx = polygon_overlay.m_x[0] - x1;
			yy = polygon_overlay.m_y[0] - y1;
		}
		break;

	default:
		return 1;
	}

	if ( clipboard.lasso ( xx, yy ) )
	{
		return 1;
	}

	return 0;
}

int mtPixyUI::File::selection_fill ()
{
	switch ( m_tool_mode )
	{
	case TOOL_MODE_SELECTED_RECTANGLE:
		rectangle_fill ();
		break;

	case TOOL_MODE_SELECTED_POLYGON:
		polygon_fill ();
		break;

	default:
		return 1;
	}

	return 0;
}

int mtPixyUI::File::selection_outline ()
{
	switch ( m_tool_mode )
	{
	case TOOL_MODE_SELECTED_RECTANGLE:
		rectangle_outline ();
		break;

	case TOOL_MODE_SELECTED_POLYGON:
		polygon_outline ();
		break;

	default:
		return 1;
	}

	return 0;
}

int mtPixyUI::File::clipboard_rotate_clockwise (
	Clipboard	&clipboard
	)
{
	if ( clipboard.rotate_clockwise () )
	{
		return 1;
	}

	return rectangle_overlay.set_paste ( m_image, clipboard.get_image () );
}

int mtPixyUI::File::clipboard_rotate_anticlockwise (
	Clipboard	&clipboard
	)
{
	if ( clipboard.rotate_anticlockwise () )
	{
		return 1;
	}

	return rectangle_overlay.set_paste ( m_image, clipboard.get_image () );
}

int mtPixyUI::File::clipboard_render_text (
	Clipboard		&clipboard,
	char	const * const	txt,
	char	const * const	font_name,
	int		const	size,
	int		const	eff_bold,
	int		const	eff_italic,
	int		const	eff_underline,
	int		const	eff_strikethrough
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	mtPixy::Image * im = mtPixy::text_render_paste (
		m_image->get_type (), brush, txt, font_name, size,
		eff_bold, eff_italic,
		eff_underline ?
			mtPixy::Font::STYLE_UNDERLINE_SINGLE :
			mtPixy::Font::STYLE_UNDERLINE_NONE,
		eff_strikethrough );

	if ( ! im )
	{
		return 1;
	}

	if ( clipboard.set_image ( im, 0, 0, true ) )
	{
		delete im;
		return 1;
	}

	return 0;
}

