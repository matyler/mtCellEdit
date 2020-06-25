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

#include "qt5_mw.h"



CloudView * Mainwindow::get_active_view ()
{
	if ( m_cloud_view_b && m_cloud_view_b->hasFocus () )
	{
		return m_cloud_view_b;
	}

	return m_cloud_view_a;
}

void Mainwindow::press_view_show_antialiasing ()
{
	bool const on = act_view_show_antialiasing->isChecked ();

	m_cloud_view_a->set_antialiasing ( on );
	m_cloud_view_b->set_antialiasing ( on );

	m_prefs.set ( PREFS_VIEW_SHOW_ANTIALIASING, on ? 1 : 0 );
}

void Mainwindow::press_view_show_crosshair ()
{
	bool const on = act_view_show_crosshair->isChecked ();

	m_cloud_view_a->show_crosshair ( on );
	m_cloud_view_b->show_crosshair ( on );

	m_prefs.set ( PREFS_VIEW_SHOW_CROSSHAIR, on ? 1 : 0 );
}

void Mainwindow::press_view_show_statusbar ()
{
	bool const on = act_view_show_statusbar->isChecked ();

	m_cloud_view_a->show_statusbar ( on );
	m_cloud_view_b->show_statusbar ( on );

	m_prefs.set ( PREFS_VIEW_SHOW_STATUSBAR, on ? 1 : 0 );
}

void Mainwindow::press_view_show_cloud ()
{
	bool const on = act_view_show_cloud->isChecked ();

	m_cloud_view_a->show_cloud ( on );
	m_cloud_view_b->show_cloud ( on );

	m_prefs.set ( PREFS_VIEW_SHOW_CLOUD, on ? 1 : 0 );
}

void Mainwindow::press_view_show_model ()
{
	bool const on = act_view_show_model->isChecked ();

	m_cloud_view_a->show_model ( on );
	m_cloud_view_b->show_model ( on );

	m_prefs.set ( PREFS_VIEW_SHOW_MODEL, on ? 1 : 0 );
}

void Mainwindow::press_view_select_a ()
{
	m_cloud_view_a->setFocus ( Qt::OtherFocusReason );
}

void Mainwindow::press_view_select_b ()
{
	m_cloud_view_b->setFocus ( Qt::OtherFocusReason );
}

void Mainwindow::set_view_split_on ( bool const on )
{
	if ( on )
	{
		m_cloud_view_b->show ();
		act_view_split_switch->setEnabled ( true );
	}
	else
	{
		m_cloud_view_b->hide ();
		act_view_split_switch->setEnabled ( false );
	}
}

void Mainwindow::press_view_split ()
{
	set_view_split_on ( act_view_split->isChecked () );
}

void Mainwindow::set_view_split_vert ( bool const vert )
{
	if ( vert )
	{
		m_split_view->setOrientation ( Qt::Vertical );
	}
	else
	{
		m_split_view->setOrientation ( Qt::Horizontal );
	}
}

void Mainwindow::press_view_split_switch ()
{
	set_view_split_vert ( m_split_view->orientation () != Qt::Vertical );
}

void Mainwindow::press_view_reset_camera ()
{
	CloudView * view = get_active_view ();

	if ( view )
	{
		view->reset_camera ();
	}
}

void Mainwindow::press_view_spin_180 ()
{
	CloudView * view = get_active_view ();

	if ( view )
	{
		view->spin_180 ();
		view->update ();
	}
}

void Mainwindow::press_view_xyz_snap_nudge ()
{
	m_cloud_view_a->set_xyz_snap_nudge ();
	m_cloud_view_b->set_xyz_snap_nudge ();
}

void Mainwindow::press_view_clone_ab ()
{
	m_cloud_view_b->clone_camera ( m_cloud_view_a );
}

void Mainwindow::press_view_clone_ba ()
{
	m_cloud_view_a->clone_camera ( m_cloud_view_b );
}

static void move_slider (
	QSlider	* const	slider,
	int	const	delta
	)
{
	slider->setValue ( slider->value () + delta );
}

void Mainwindow::press_view_nudge_up ()
{
	move_slider ( m_slider_nudge, 1 );
}

void Mainwindow::press_view_nudge_down ()
{
	move_slider ( m_slider_nudge, -1 );
}

void Mainwindow::set_view_resolution ( int const type )
{
	char const * const name = m_fe.crul_db.get_cache_name ( type );

	if ( ! name )
	{
		QMessageBox::critical ( this, "Error", "Bad resolution type" );
		return;
	}

	load_cloud_from_db ( type );
	update_info ();

	m_info_resolution->setText ( name );
}

void Mainwindow::press_view_res_low ()
{
	set_view_resolution ( Crul::DB::CACHE_TYPE_LOW );
}

void Mainwindow::press_view_res_medium ()
{
	set_view_resolution ( Crul::DB::CACHE_TYPE_MEDIUM );
}

void Mainwindow::press_view_res_high ()
{
	set_view_resolution ( Crul::DB::CACHE_TYPE_HIGH );
}

