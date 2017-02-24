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



class CanvasAct
{
public:
	enum
	{
		MOUSE_PRESS	= 0,
		MOUSE_MOVE	= 2,

		BUTTON_LEFT	= 1,
		BUTTON_MIDDLE	= 2,
		BUTTON_RIGHT	= 3
	};

	CanvasAct ( Mainwindow &mw, QMouseEvent * ev, int caller, int zs );

	int stop () const { return m_stop; };
	int cx () const { return m_cx; };
	int cy () const { return m_cy; };
	void update_statusbar ();
	void update_canvas ();

	void action_pick_color ( int button );
	void action_paint ( int button );
	void action_line ( int button );
	void action_select_rectangle ( int button );
	void action_select_polygon ( int button );
	void action_paste ( int button );
	void action_flood_fill ( int button );

///	------------------------------------------------------------------------

private:
	Mainwindow	&mainwindow;

	mtPixyUI::File::ToolMode const m_toolmode;
	int	const	m_caller;
	int	const	m_key_ctrl;
	int	const	m_x;
	int	const	m_y;
	int		m_stop;
	int		m_cx;
	int		m_cy;
	unsigned char	m_cr, m_cg, m_cb;
	int		m_pixel_idx;
	int		m_dx, m_dy, m_dw, m_dh;
	mtPixy::Image	* m_im;
	int		m_right_but;
};



CanvasAct::CanvasAct (
	Mainwindow	&mw,
	QMouseEvent	* const	ev,
	int		const	caller,
	int		const	zs
	)
	:
	mainwindow	( mw )
	,m_toolmode	( mw.backend.file.get_tool_mode () )
	,m_caller	( caller )
	,m_key_ctrl	( (int)(ev->modifiers() & Qt::ControlModifier) )
	,m_x		( ev->x () )
	,m_y		( ev->y () )
	,m_stop		( 0 )
	,m_cx		( m_x )
	,m_cy		( m_y )
	,m_cr		( 0 )
	,m_cg		( 0 )
	,m_cb		( 0 )
	,m_pixel_idx	( -1 )
	,m_dx		( 0 )
	,m_dy		( 0 )
	,m_dw		( 0 )
	,m_dh		( 0 )
	,m_right_but	( (ev->buttons () & Qt::RightButton) ? 1 : 0 )
{
	m_im = mainwindow.backend.file.get_image ();
	if ( ! m_im )
	{
		m_stop = 1;
		return;
	}


	int	const	imw = m_im->get_width ();
	int	const	imh = m_im->get_height ();
	int		b = 0;


	// Calculate canvas X,Y
	if ( zs < 0 )
	{
		m_cx *= -zs;
		m_cy *= -zs;
	}
	else if ( zs > 0 )
	{
		m_cx /= zs;
		m_cy /= zs;
	}

	// Assert sanity
	m_cx = MAX ( 0, m_cx );
	m_cx = MIN ( imw - 1, m_cx );
	m_cy = MAX ( 0, m_cy );
	m_cy = MIN ( imh - 1, m_cy );

	if ( ev->buttons () & Qt::LeftButton )
	{
		b = BUTTON_LEFT;
	}
	else if ( ev->buttons () & Qt::RightButton )
	{
		b = BUTTON_RIGHT;
	}

	if ( mainwindow.backend.file.get_pixel_info ( m_cx, m_cy,
		m_cr, m_cg, m_cb, m_pixel_idx ) )
	{
		// Something unexpected has happened so bail out
		m_stop = 1;
		return;
	}

	mainwindow.m_cursor.set_xy ( m_cx, m_cy );

	if ( 0 == caller && m_key_ctrl && b > 0 )
	{
		action_pick_color ( b );
	}
	else switch ( m_toolmode )
	{
	case mtPixyUI::File::TOOL_MODE_PAINT:
	case mtPixyUI::File::TOOL_MODE_PAINTING:
		action_paint ( b );
		break;

	case mtPixyUI::File::TOOL_MODE_LINE:
	case mtPixyUI::File::TOOL_MODE_LINING:
		action_line ( b );
		break;

	case mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
		action_select_rectangle ( b );
		break;

	case mtPixyUI::File::TOOL_MODE_SELECT_POLYGON:
	case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
	case mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON:
		action_select_polygon ( b );
		break;

	case mtPixyUI::File::TOOL_MODE_PASTE:
	case mtPixyUI::File::TOOL_MODE_PASTING:
		action_paste ( b );
		break;

	case mtPixyUI::File::TOOL_MODE_FLOODFILL:
		action_flood_fill ( b );
		break;

	default:
		break;
	}
}

void CanvasAct::update_statusbar ()
{
	if ( m_pixel_idx < 0 )
	{
		mainwindow.set_statusbar_cursor (
			QString ( "%1,%2 {%3,%4,%5}" )
			.arg ( m_cx ).arg ( m_cy )
			.arg ( m_cr ).arg ( m_cg ).arg ( m_cb )
			);
	}
	else
	{
		mainwindow.set_statusbar_cursor (
			QString ( "%1,%2 [%3] = {%4,%5,%6}" )
			.arg ( m_cx ).arg ( m_cy ).arg ( m_pixel_idx )
			.arg ( m_cr ).arg ( m_cg ).arg ( m_cb )
			);
	}
}

void CanvasAct::update_canvas ()
{
	mainwindow.update_canvas ( m_dx, m_dy, m_dw, m_dh );
}

void CanvasAct::action_pick_color (
	int	const	button
	)
{
	int		res = 0;
	unsigned char	idx = 0;


	// Convert RGB into palette index or append as new
	if ( m_pixel_idx < 0 )
	{
		res = mainwindow.backend.file.palette_append (
			m_cr, m_cg, m_cb );

		if ( res >= 0 )
		{
			// New colour appended so new undo step
			idx = (unsigned char)res;
			res = 1;
		}
		else
		{
			// Failed to add colour so do nothing
		}
	}
	else
	{
		idx = (unsigned char)m_pixel_idx;
	}

	if ( res >= 0 )
	{
		mtPixy::Palette	* const pal = m_im->get_palette ();
		mtPixy::Color	* const col = pal->get_color ();
		int updt = Mainwindow::UPDATE_PALETTE
				| Mainwindow::UPDATE_TOOLBAR;


		if ( button == BUTTON_LEFT )
		{
			mainwindow.backend.file.brush.set_color_a( idx, col );
		}
		else if ( button == BUTTON_RIGHT )
		{
			mainwindow.backend.file.brush.set_color_b( idx, col );
		}

		if ( 1 == res )
		{
			updt |= Mainwindow::UPDATE_TITLEBAR
				| Mainwindow::UPDATE_STATUS_UNDO
				| Mainwindow::UPDATE_MENUS;
		}

		mainwindow.update_ui ( updt );
	}
}

void CanvasAct::action_paint (
	int	const	button
	)
{
	if ( m_key_ctrl || button != BUTTON_LEFT )
	{
		return;
	}

	if ( MOUSE_PRESS == m_caller )
	{
		// Mouse left button click
		mainwindow.tool_action_paint_start ( m_cx, m_cy );
	}
	else if ( MOUSE_MOVE == m_caller )
	{
		// Mouse movement left button down
		mainwindow.tool_action_paint_to ( m_cx, m_cy );
	}
}

void CanvasAct::action_line (
	int	const	button
	)
{
	if ( button == BUTTON_RIGHT )
	{
		if ( m_toolmode == mtPixyUI::File::TOOL_MODE_LINING )
		{
			mainwindow.tool_action_line_finish ();
		}
	}
	else if (	button == BUTTON_LEFT	&&
			! m_key_ctrl		&&
			MOUSE_PRESS == m_caller
			)
	{
		if ( m_toolmode == mtPixyUI::File::TOOL_MODE_LINE )
		{
			mainwindow.tool_action_line_start ( m_cx, m_cy );
		}
		else
		{
			mainwindow.tool_action_line_to ( m_cx, m_cy );
		}
	}
	else if ( m_toolmode == mtPixyUI::File::TOOL_MODE_LINING )
	{
		// No button presses or actions so update screen with redraw
		mainwindow.line_overlay.set_end ( m_cx, m_cy, m_dx, m_dy,
			m_dw, m_dh );
		update_canvas ();
	}
}

void CanvasAct::action_select_rectangle (
	int	const	button
	)
{
	if ( button == BUTTON_LEFT && ! m_key_ctrl )
	{
		if ( m_toolmode == mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE )
		{
			mainwindow.tool_action_recsel_start ( m_cx, m_cy );
		}
		else if ( m_toolmode ==
			mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE )
		{
			mainwindow.tool_action_recsel_to ( m_cx, m_cy );
		}
		else if ( m_toolmode ==
			mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE )
		{
			mainwindow.tool_action_recsel_corner( m_cx, m_cy );
		}
	}
	else if ( button == BUTTON_RIGHT && ! m_key_ctrl )
	{
		mainwindow.tool_action_recsel_clear ();
	}
}

void CanvasAct::action_select_polygon (
	int	const	button
	)
{
	if ( button == BUTTON_RIGHT )
	{
		if ( m_toolmode == mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON )
		{
			mainwindow.tool_action_polysel_finish ();
		}
	}
	else if (	button == BUTTON_LEFT	&&
			! m_key_ctrl		&&
			MOUSE_PRESS == m_caller
			)
	{
		if ( m_toolmode == mtPixyUI::File::TOOL_MODE_SELECT_POLYGON )
		{
			mainwindow.tool_action_polysel_start ( m_cx, m_cy );
		}
		else if ( m_toolmode ==
			mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON )
		{
			mainwindow.tool_action_polysel_to ( m_cx, m_cy );
		}
	}
	else if ( m_toolmode == mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON )
	{
		// No button presses or actions so update screen with redraw
		mainwindow.backend.file.polygon_overlay.set_end ( m_cx, m_cy,
			m_dx, m_dy, m_dw, m_dh );
		update_canvas ();
	}
}

void CanvasAct::action_paste (
	int	const	button
	)
{
	if ( button == BUTTON_LEFT )
	{
		if (	MOUSE_PRESS == m_caller &&
			m_toolmode == mtPixyUI::File::TOOL_MODE_PASTE )
		{
			mainwindow.tool_action_paste_drag_start ( m_cx, m_cy );
		}
		else if ( MOUSE_PRESS != m_caller &&
			m_toolmode == mtPixyUI::File::TOOL_MODE_PASTING )
		{
			mainwindow.tool_action_paste_drag_to ( m_cx, m_cy );
		}
	}

	if ( m_toolmode == mtPixyUI::File::TOOL_MODE_PASTING )
	{
		if ( m_right_but )
		{
			mainwindow.tool_action_paste_commit ();
		}
		else
		{
			mainwindow.tool_action_paste_set_undo ();
		}
	}
}

void CanvasAct::action_flood_fill (
	int	const	button
	)
{
	if (	MOUSE_PRESS == m_caller		&&
		button == BUTTON_LEFT		&&
		! m_key_ctrl
		)
	{
		mainwindow.tool_action_flood_fill ( m_cx, m_cy );
	}
}

void CanvasView::mouseEventRouter (
	QMouseEvent	* const	ev,
	int		const	caller
	)
{
	CanvasAct	canact ( mainwindow, ev, caller, get_zoom_scale () );


	if ( canact.stop () )
	{
		return;
	}

	if ( ev->buttons () & Qt::MiddleButton )
	{
		m_zoom_cx = canact.cx ();
		m_zoom_cy = canact.cy ();
	}

	canact.update_statusbar ();

	mainwindow.set_last_zoom_scale ( get_zoom_scale () );
}

void CanvasView::mousePressEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, CanvasAct::MOUSE_PRESS );
}

void CanvasView::mouseReleaseEvent (
	QMouseEvent	* const	ev
	)
{
	if ( ev->button () & Qt::LeftButton )
	{
		mtPixyUI::File::ToolMode const toolmode = mainwindow.backend.
			file.get_tool_mode ();


		switch ( toolmode )
		{
		case mtPixyUI::File::TOOL_MODE_PAINT:
		case mtPixyUI::File::TOOL_MODE_PAINTING:
			mainwindow.tool_action_paint_finish ();
			break;

		case mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE:
			mainwindow.tool_action_recsel_finish ();
			break;

		case mtPixyUI::File::TOOL_MODE_PASTING:
			mainwindow.tool_action_paste_finish ();
			break;

		default:
			break;
		}
	}
}

void CanvasView::mouseMoveEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, CanvasAct::MOUSE_MOVE );
}

void CanvasView::leaveEvent (
	QEvent		* const	ARG_UNUSED ( ev )
	)
{
	mainwindow.set_statusbar_cursor ( "" );
}

