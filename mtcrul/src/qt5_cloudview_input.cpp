/*
	Copyright (C) 2020 Mark Tyler

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
		double const nudge = 1.0/8.0;

		set_xrotation ( m_camera.get_rotX() + scale * nudge * my / 4.0);
		set_zrotation ( m_camera.get_rotZ() - scale * nudge * mx / 4.0);
	}
	else
	{
		double const nudge = m_view_nudge;
		QMatrix4x4 camera;
		get_camera_matrix ( camera );
		QMatrix3x3 const normal = camera.normalMatrix ();
		float const * const d = normal.data ();

		double const dx = -mx * (double)d[0] + my * (double)d[1];
		double const dy = -mx * (double)d[3] + my * (double)d[4];
		double const dz = -mx * (double)d[6] + my * (double)d[7];

		m_camera.set_x ( m_camera.get_x() + scale * nudge * dx );
		m_camera.set_y ( m_camera.get_y() + scale * nudge * dy );
		m_camera.set_z ( m_camera.get_z() + scale * nudge * dz );

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
		double const nudge = 1.0/8.0;

		switch ( event->key () )
		{
		case Qt::Key_Up:
			set_xrotation ( m_camera.get_rotX() + scale * nudge );
			break;

		case Qt::Key_Down:
			set_xrotation ( m_camera.get_rotX() - scale * nudge );
			break;

		case Qt::Key_Left:
			set_zrotation ( m_camera.get_rotZ() - scale * nudge );
			break;

		case Qt::Key_Right:
			set_zrotation ( m_camera.get_rotZ() + scale * nudge );
			break;

		default:
			return;
		}

		return;
	}

	QMatrix4x4 camera;
	get_camera_matrix ( camera );

	QMatrix3x3 const normal = camera.normalMatrix ();
	float const * const d = normal.data ();

	double dx = 0.0, dy = 0.0, dz = 0.0;

	switch ( event->key () )
	{
	case Qt::Key_PageUp:
		dx = -d[2];
		dy = -d[5];
		dz = -d[8];
		break;

	case Qt::Key_PageDown:
		dx = d[2];
		dy = d[5];
		dz = d[8];
		break;

	case Qt::Key_Up:
		dx = d[1];
		dy = d[4];
		dz = d[7];
		break;

	case Qt::Key_Down:
		dx = -d[1];
		dy = -d[4];
		dz = -d[7];
		break;

	case Qt::Key_Left:
		dx = -d[0];
		dy = -d[3];
		dz = -d[6];
		break;

	case Qt::Key_Right:
		dx = d[0];
		dy = d[3];
		dz = d[6];
		break;

	default:
		// Nothing to do so let the base class handle this event instead
		QWidget::keyPressEvent ( event );
	}

	m_camera.set_x ( m_camera.get_x() + scale * m_view_nudge * dx );
	m_camera.set_y ( m_camera.get_y() + scale * m_view_nudge * dy );
	m_camera.set_z ( m_camera.get_z() + scale * m_view_nudge * dz );

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

