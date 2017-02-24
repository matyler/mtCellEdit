/*
	Copyright (C) 2016-2017 Mark Tyler

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

#include "qt45.h"



Cursor::Cursor (
	Mainwindow	&mw
	)
	:
	mainwindow	( mw ),
	m_x		( 0 ),
	m_y		( 0 ),
	m_on_screen	( false )
{
}

void Cursor::set_xy (
	int	const	xx,
	int	const	yy
	)
{
	bool painting = false;

	if ( mainwindow.backend.file.get_tool_mode () ==
		mtPixyUI::File::TOOL_MODE_PAINT )
	{
		painting = true;
	}

	bool moved = false;
	if ( m_x != xx || m_y != yy )
	{
		moved = true;
	}

	int	const	ox = m_x;
	int	const	oy = m_y;

	m_x = xx;
	m_y = yy;

	if (	m_on_screen &&
		( moved || ! painting )
		)
	{
		m_on_screen = false;

		int	const	bs = mtPixy::Brush::SHAPE_SIZE;

		mainwindow.update_canvas ( ox - bs/2, oy - bs/2, bs, bs );
	}

	if ( painting && ! m_on_screen )
	{
		m_on_screen = true;
		redraw ();
	}
}

void Cursor::redraw () const
{
	if ( m_on_screen )
	{
		int	const	bs = mtPixy::Brush::SHAPE_SIZE;

		mainwindow.update_canvas ( m_x - bs/2, m_y - bs/2, bs, bs );
	}
}

void Mainwindow::tool_action_paint_start (
	int	const	cx,
	int	const	cy
	)
{
	int		dx, dy, dw, dh;


	if ( 0 == backend.file.paint_brush_start ( cx, cy, dx, dy, dw, dh ) )
	{
		update_canvas ( dx, dy, dw, dh );
	}
}

void Mainwindow::tool_action_paint_to (
	int	const	cx,
	int	const	cy
	)
{
	int		dx, dy, dw, dh;


	if ( 0 == backend.file.paint_brush_to ( cx, cy, dx, dy, dw, dh ) )
	{
		update_canvas ( dx, dy, dw, dh );
	}
}

void Mainwindow::tool_action_paint_finish ()
{
	backend.file.paint_brush_finish ();
	update_ui ( UPDATE_TITLEBAR | UPDATE_STATUS_UNDO | UPDATE_MENUS );
}

void Mainwindow::tool_action_line_start (
	int	const	cx,
	int	const	cy
	)
{
	line_overlay.set_start ( cx, cy );
	update_canvas ( cx, cy, 1, 1 );
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_LINING, UPDATE_NONE );
}

void Mainwindow::tool_action_line_to (
	int	const	cx,
	int	const	cy
	)
{
	int	const	x1 = line_overlay.get_x1 ();
	int	const	y1 = line_overlay.get_y1 ();

	line_overlay.set_start ( cx, cy );

	backend.file.paint_line ( x1, y1, cx, cy );
	update_ui ( UPDATE_ALL );
}

void Mainwindow::tool_action_line_finish ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_LINE, UPDATE_CANVAS );
}

void Mainwindow::tool_action_recsel_start (
	int	const	cx,
	int	const	cy
	)
{
	backend.file.rectangle_overlay.set_start ( cx, cy );
	update_canvas ( cx, cy, 1, 1 );

	update_ui ( UPDATE_STATUS_SELECTION );
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE );
}

void Mainwindow::tool_action_recsel_to (
	int	const	cx,
	int	const	cy
	)
{
	int		dx, dy, dw, dh;


	backend.file.rectangle_overlay.set_end ( cx, cy, dx, dy, dw, dh );
	update_canvas ( dx, dy, dw, dh );
	update_ui ( UPDATE_STATUS_SELECTION );
}

void Mainwindow::tool_action_recsel_corner (
	int	const	cx,
	int	const	cy
	)
{
	int		dx, dy, dw, dh;


	backend.file.rectangle_overlay.set_corner ( cx, cy, dx, dy, dw, dh );
	update_canvas ( dx, dy, dw, dh );
	update_ui ( UPDATE_STATUS_SELECTION );
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE );
}

void Mainwindow::tool_action_recsel_finish ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE );
}

void Mainwindow::tool_action_recsel_clear ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE );
}

void Mainwindow::tool_action_recsel_move (
	int	const	xdir,
	int	const	ydir,
	int	const	key_shift,
	int	const	key_ctrl,
	int	const	delta
	)
{
	int		dx, dy, dw, dh;
	int	const	dd = key_shift ? delta : 1;


	mtPixy::Image	* const im = backend.file.get_image ();
	if ( ! im )
	{
		return;
	}


	int	const	max_x = im->get_width () - 1;
	int	const	max_y = im->get_height () - 1;


	if ( key_ctrl )
	{
		backend.file.rectangle_overlay.move_selection_end ( xdir * dd,
			ydir * dd, max_x, max_y, dx, dy, dw, dh );
	}
	else
	{
		backend.file.rectangle_overlay.move_selection ( xdir * dd,
			ydir * dd, max_x, max_y, dx, dy, dw, dh );
	}

	update_canvas ( dx, dy, dw, dh );
	update_ui ( UPDATE_STATUS_SELECTION );
}

void Mainwindow::tool_action_polysel_start (
	int	const	cx,
	int	const	cy
	)
{
	backend.file.polygon_overlay.clear ();
	backend.file.polygon_overlay.set_start ( cx, cy );

	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON,
		UPDATE_NONE );
	update_canvas ( cx, cy, 1, 1 );
}

void Mainwindow::tool_action_polysel_to (
	int	const	cx,
	int	const	cy
	)
{
	if ( backend.file.polygon_overlay.add () )
	{
		tool_action_polysel_finish ();
		return;
	}

	backend.file.polygon_overlay.set_start ( cx, cy );
	update_ui ( UPDATE_CANVAS );
}

void Mainwindow::tool_action_polysel_finish ()
{
	backend.file.polygon_overlay.add ();

	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON );
}

void Mainwindow::tool_action_polysel_clear ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECT_POLYGON );
}

void Mainwindow::tool_action_paste_drag_start (
	int	const	cx,
	int	const	cy
	)
{
	int		x1, y1, x2, y2;

	backend.file.rectangle_overlay.get_xy ( x1, y1, x2, y2 );

	if ( cx < x1 || cx > x2 || cy < y1 || cy > y2 )
	{
		return;
	}

	m_paste_committed = 0;

	set_tool_mode ( mtPixyUI::File::TOOL_MODE_PASTING, UPDATE_NONE );

	m_paste_drag_x = cx - x1;
	m_paste_drag_y = cy - y1;
}

void Mainwindow::tool_action_paste_drag_to (
	int	const	cx,
	int	const	cy
	)
{
	int		dx, dy, dw, dh;

	if ( 0 == backend.file.rectangle_overlay.move_paste (
		cx - m_paste_drag_x, cy - m_paste_drag_y,
		backend.file.get_image (), backend.clipboard.get_image (),
		dx, dy, dw, dh ) )
	{
		return;
	}

	update_canvas ( dx, dy, dw, dh );
	update_ui ( UPDATE_STATUS_SELECTION );
}

void Mainwindow::tool_action_paste_move (
	int	const	xdir,
	int	const	ydir,
	int	const	key_shift,
	int	const	delta
	)
{
	int		x1, y1, x2, y2;
	int		dx, dy, dw, dh;
	int	const	dd = key_shift ? delta : 1;


	backend.file.rectangle_overlay.get_xy ( x1, y1, x2, y2 );

	x1 += xdir * dd;
	y1 += ydir * dd;

	if ( 0 == backend.file.rectangle_overlay.move_paste ( x1, y1,
		backend.file.get_image (), backend.clipboard.get_image (),
		dx, dy, dw, dh ) )
	{
		return;
	}

	update_canvas ( dx, dy, dw, dh );
	update_ui ( UPDATE_STATUS_SELECTION );
}

void Mainwindow::tool_action_paste_commit ()
{
	int		x1, y1, x2, y2;
	int		dx, dy, dw, dh;

	backend.file.rectangle_overlay.get_xy ( x1, y1, x2, y2 );

	if ( backend.clipboard.paste ( backend.file, x1, y1, dx, dy, dw, dh ) )
	{
		return;
	}

	update_canvas ( dx, dy, dw, dh );

	m_paste_committed = 1;
}

void Mainwindow::tool_action_paste_set_undo ()
{
	if ( m_paste_committed )
	{
		m_paste_committed = 0;
		backend.file.commit_undo_step ();
		update_ui( UPDATE_STATUS_UNDO | UPDATE_TITLEBAR | UPDATE_MENUS);
	}
}

void Mainwindow::tool_action_paste_finish ()
{
	// This function also commits the undo step
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_PASTE );
}

void Mainwindow::tool_action_flood_fill (
	int	const	cx,
	int	const	cy
	)
{
	operation_update ( backend.file.flood_fill ( cx, cy ), "Flood Fill",
		UPDATE_ALL_IMAGE );
}

void Mainwindow::tool_action_key ()
{
	mtPixyUI::File::ToolMode const toolmode = backend.file.get_tool_mode ();


	switch ( toolmode )
	{
	case mtPixyUI::File::TOOL_MODE_PAINT:
		tool_action_paint_start ( m_cursor.x(), m_cursor.y() );
		tool_action_paint_finish ();
		break;

	case mtPixyUI::File::TOOL_MODE_PAINTING:
		tool_action_paint_finish ();
		break;

	case mtPixyUI::File::TOOL_MODE_LINE:
		tool_action_line_start ( m_cursor.x(), m_cursor.y() );
		break;

	case mtPixyUI::File::TOOL_MODE_LINING:
		tool_action_line_to ( m_cursor.x(), m_cursor.y() );
		break;

	case mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE:
		tool_action_recsel_start ( m_cursor.x(), m_cursor.y() );
		tool_action_recsel_finish ();
		break;

	case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
		tool_action_recsel_corner ( m_cursor.x(), m_cursor.y() );
		tool_action_recsel_finish ();
		break;

	case mtPixyUI::File::TOOL_MODE_SELECT_POLYGON:
		tool_action_polysel_start ( m_cursor.x(), m_cursor.y() );
		break;

	case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
		tool_action_polysel_to ( m_cursor.x(), m_cursor.y() );
		break;

	case mtPixyUI::File::TOOL_MODE_PASTE:
	case mtPixyUI::File::TOOL_MODE_PASTING:
		tool_action_paste_commit ();
		tool_action_paste_finish ();
		break;

	case mtPixyUI::File::TOOL_MODE_FLOODFILL:
		tool_action_flood_fill ( m_cursor.x(), m_cursor.y() );
		break;

	default:
		break;
	}
}

