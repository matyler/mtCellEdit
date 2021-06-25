/*
	Copyright (C) 2020-2021 Mark Tyler

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

#include "qt5_cloudview.h"



void CloudView::mousePressEvent ( QMouseEvent * const event )
{
	m_last_pos = event->pos ();
	setFocus ( Qt::OtherFocusReason );
}

void CloudView::mouse_camera (
	QMouseEvent	* const	event,
	int		const	mx,
	int		const	my
	)
{
	int const scale = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;

	if ( (event->buttons () & Qt::RightButton) )
	{
		double const nudge = scale / 8.0 / 4.0;

		set_xrotation ( m_camera.get_rot_x() + nudge * my );
		set_zrotation ( m_camera.get_rot_z() - nudge * mx );
	}
	else
	{
		double const nudge = m_view_nudge * scale;

		m_camera.move ( -mx * nudge, my * nudge, 0 );

		emit camera_changed ();
	}
}

void CloudView::mouseMoveEvent ( QMouseEvent * const event )
{
	int const mx = event->x () - m_last_pos.x ();
	int const my = event->y () - m_last_pos.y ();

	setFocus ( Qt::OtherFocusReason );

	switch ( m_mode )
	{
	case MODE_CAMERA:
		mouse_camera ( event, mx, my );
		break;

	case MODE_RULER:
		emit mouse_ruler ( this, event, mx, my );
		break;
	}

	m_last_pos = event->pos ();
}

void CloudView::keypress_camera ( QKeyEvent * const event )
{
	int const scale = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;

	if ( (event->modifiers() & Qt::ControlModifier) )
	{
		double const nudge = (double)scale / 8.0;

		switch ( event->key () )
		{
		case Qt::Key_Up:
			set_xrotation ( m_camera.get_rot_x() + nudge );
			break;

		case Qt::Key_Down:
			set_xrotation ( m_camera.get_rot_x() - nudge );
			break;

		case Qt::Key_Left:
			set_zrotation ( m_camera.get_rot_z() - nudge );
			break;

		case Qt::Key_Right:
			set_zrotation ( m_camera.get_rot_z() + nudge );
			break;
		}

		return;
	}

	double const nudge = scale * m_view_nudge;

	switch ( event->key () )
	{
	case Qt::Key_PageUp:
		m_camera.move ( 0, 0, nudge );
		break;

	case Qt::Key_PageDown:
		m_camera.move ( 0, 0, -nudge );
		break;

	case Qt::Key_Up:
		m_camera.move ( 0, nudge, 0 );
		break;

	case Qt::Key_Down:
		m_camera.move ( 0, -nudge, 0 );
		break;

	case Qt::Key_Left:
		m_camera.move ( -nudge, 0, 0 );
		break;

	case Qt::Key_Right:
		m_camera.move ( nudge, 0, 0 );
		break;

	default:
		// Nothing to do so let the base class handle this event instead
		QWidget::keyPressEvent ( event );
		return;
	}

	emit camera_changed ();
}

void CloudView::keyPressEvent ( QKeyEvent * const event )
{
	switch ( m_mode )
	{
	case MODE_CAMERA:
		keypress_camera ( event );
		break;

	case MODE_RULER:
		emit keypress_ruler ( this, event );
		break;
	}
}

