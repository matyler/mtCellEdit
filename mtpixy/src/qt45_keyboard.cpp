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


keyPressEater::keyPressEater (
	Mainwindow	&mw
	)
	:
	mainwindow	( mw )
{
}

bool keyPressEater::eventFilter (
	QObject		* const obj,
	QEvent		* const ev
	)
{
	if (	ev->type () == QEvent::KeyPress	&&
		key_filter ( static_cast<QKeyEvent *>( ev ) )
		)
	{
		// Event handled
		return true;
	}

	// Standard event processing
	return QObject::eventFilter ( obj, ev );
}

bool Mainwindow::handle_zoom_keys (
	QKeyEvent	* const	ev
	)
{
	int const key_ctrl = (int)(ev->modifiers () & Qt::ControlModifier);


	switch ( ev->key () )
	{
	case Qt::Key_1:
		press_options_zoom_split_3 ();
		return true;

	case Qt::Key_2:
		press_options_zoom_split_100 ();
		return true;

	case Qt::Key_3:
		press_options_zoom_split_3200 ();
		return true;

	case Qt::Key_4:
		press_options_zoom_main_3 ();
		return true;

	case Qt::Key_5:
		press_options_zoom_main_100 ();
		return true;

	case Qt::Key_6:
		press_options_zoom_main_3200 ();
		return true;

	case Qt::Key_Plus:
	case Qt::Key_Equal:
		key_ctrl ?	press_options_zoom_split_in () :
				press_options_zoom_main_in ();
		return true;

	case Qt::Key_Minus:
		key_ctrl ?	press_options_zoom_split_out () :
				press_options_zoom_main_out ();
		return true;

	default:
		break;
	}

	return false;
}

bool keyPressEater::key_filter (
	QKeyEvent	* const	ev
	)
{
	int zs = mainwindow.get_last_zoom_scale ();

	if ( zs < 1 )
	{
		zs = 1;
	}

	int const key_shift = (int)(ev->modifiers () & Qt::ShiftModifier);
	int const key_ctrl = (int)(ev->modifiers () & Qt::ControlModifier);
	int const delta = key_shift ? mainwindow.prefs.getInt
			( PREFS_CURSOR_NUDGE_PIXELS ) : 1;
	QPoint	const	cpos = QCursor::pos ();
	mtPixyUI::File::ToolMode const toolmode = mainwindow.backend.file.
			get_tool_mode ();

	switch ( ev->key () )
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
		mainwindow.tool_action_key ();
		return true;

	case Qt::Key_Escape:
		switch ( toolmode )
		{
		case mtPixyUI::File::TOOL_MODE_LINING:
			mainwindow.set_tool_mode (
				mtPixyUI::File::TOOL_MODE_LINE );
			return true;

		case mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE:
		case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
			mainwindow.tool_action_recsel_clear ();
			return true;

		case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
		case mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON:
			mainwindow.tool_action_polysel_clear ();
			return true;

		case mtPixyUI::File::TOOL_MODE_PASTE:
		case mtPixyUI::File::TOOL_MODE_PASTING:
			mainwindow.tool_action_recsel_clear ();
			return true;

		default:
			break;
		}
		return true;

	case Qt::Key_Up:
		if ( toolmode == mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE )
		{
			mainwindow.tool_action_recsel_move (
				0, -1, key_shift, key_ctrl, delta );
		}
		else if ( toolmode == mtPixyUI::File::TOOL_MODE_PASTE )
		{
			mainwindow.tool_action_paste_move ( 0, -1, key_shift,
				delta );
		}
		else
		{
			QCursor::setPos ( cpos.x (), cpos.y () - delta * zs );
		}
		return true;

	case Qt::Key_Down:
		if ( toolmode == mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE )
		{
			mainwindow.tool_action_recsel_move (
				0, 1, key_shift, key_ctrl, delta );
		}
		else if ( toolmode == mtPixyUI::File::TOOL_MODE_PASTE )
		{
			mainwindow.tool_action_paste_move ( 0, 1, key_shift,
				delta );
		}
		else
		{
			QCursor::setPos ( cpos.x (), cpos.y () + delta * zs );
		}
		return true;

	case Qt::Key_Left:
		if ( toolmode == mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE )
		{
			mainwindow.tool_action_recsel_move (
				-1, 0, key_shift, key_ctrl, delta );
		}
		else if ( toolmode == mtPixyUI::File::TOOL_MODE_PASTE )
		{
			mainwindow.tool_action_paste_move ( -1, 0, key_shift,
				delta );
		}
		else
		{
			QCursor::setPos ( cpos.x () - delta * zs, cpos.y () );
		}
		return true;

	case Qt::Key_Right:
		if ( toolmode == mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE )
		{
			mainwindow.tool_action_recsel_move (
				1, 0, key_shift, key_ctrl, delta );
		}
		else if ( toolmode == mtPixyUI::File::TOOL_MODE_PASTE )
		{
			mainwindow.tool_action_paste_move ( 1, 0, key_shift,
				delta );
		}
		else
		{
			QCursor::setPos ( cpos.x () + delta * zs, cpos.y () );
		}
		return true;

	case Qt::Key_Q:
		mainwindow.press_file_quit ();
		return true;

	case Qt::Key_BracketLeft:
		key_ctrl ?	mainwindow.color_ab_delta ( -1, 'b' ) :
				mainwindow.color_ab_delta ( -1 );
		return true;

	case Qt::Key_BracketRight:
		key_ctrl ?	mainwindow.color_ab_delta ( 1, 'b' ) :
				mainwindow.color_ab_delta ( 1 );
		return true;

	case Qt::Key_Home:
		mainwindow.toggle_view_mode ();
		return true;

	case Qt::Key_End:
		mainwindow.press_options_pan_window ();
		return true;

	default:
		break;
	}

	if ( mainwindow.handle_zoom_keys ( ev ) )
	{
		return true;
	}

	return false;
}

int Mainwindow::get_last_zoom_scale ()
{
	return m_last_zoom_scale;
}

void Mainwindow::set_last_zoom_scale (
	int	const	zs
	)
{
	m_last_zoom_scale = zs;
}

void Mainwindow::color_ab_delta (
	int	const	d,
	char	const	c
	)
{
	mtPixy::Image	* im = backend.file.get_image ();
	if ( ! im )
	{
		return;
	}

	mtPixy::Color	* col = im->get_palette ()->get_color ();
	int	const	idx = c=='a' ? backend.file.brush.get_color_a_index ():
				backend.file.brush.get_color_b_index ();
	int	const	tot = im->get_palette ()->get_color_total ();
	int	const	nex = idx + d;

	if ( nex < 0 || nex >= tot )
	{
		return;
	}

	if ( c == 'a' )
	{
		backend.file.brush.set_color_a ( (unsigned char)nex, col );
	}
	else
	{
		backend.file.brush.set_color_b ( (unsigned char)nex, col );
	}

	update_ui ( UPDATE_TOOLBAR | UPDATE_PALETTE );
}


void Mainwindow::toggle_view_mode ()
{
	if ( m_view_mode_flags & VIEW_MODE )
	{
		// Switch view mode OFF

		if ( m_view_mode_flags & VIEW_MODE_STATUSBAR )
		{
			statusBar ()->setVisible ( true );
		}

		if ( m_view_mode_flags & VIEW_MODE_PALETTE )
		{
			m_palette_dock->setVisible ( true );
		}

		if ( m_view_mode_flags & VIEW_MODE_TOOLBAR )
		{
			m_toolbar->setVisible ( true );
		}

		menuBar ()->setVisible ( true );

		m_view_mode_flags = 0;
	}
	else
	{
		// Switch view mode ON

		m_view_mode_flags = VIEW_MODE;

		if ( statusBar ()->isVisible () )
		{
			statusBar ()->setVisible ( false );
			m_view_mode_flags |= VIEW_MODE_STATUSBAR;
		}

		if ( m_palette_dock->isVisible () )
		{
			m_palette_dock->setVisible ( false );
			m_view_mode_flags |= VIEW_MODE_PALETTE;
		}

		if ( m_toolbar->isVisible () )
		{
			m_toolbar->setVisible ( false );
			m_view_mode_flags |= VIEW_MODE_TOOLBAR;
		}

		menuBar ()->setVisible ( false );
	}
}

