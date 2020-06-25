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

#include "qt5_mw_update.h"



void ThreadPtsExtent::run ()
{
	m_extents = m_cpts->extents_calculate ();
}

static void print_label (
	QLabel	* const	label,
	double	const	min,
	double const	max
	)
{
	char buf[128];

	snprintf ( buf, sizeof(buf), "%.8g -> %.8g", min, max );
	label->setText ( buf );
}

void Mainwindow::update_info ()
{
	char		buf[256];
	char	const	* cp;
	std::string	st;

	st = m_fe.crul_db.get_filename ();
	cp = strrchr ( st.c_str (), MTKIT_DIR_SEP );

	if ( ! cp )
	{
		m_info_file->setText ( mtQEX::qstringFromC ( st.c_str () ) );
	}
	else
	{
		m_info_file->setText ( mtQEX::qstringFromC ( cp + 1 ) );
	}

	snprintf ( buf, sizeof(buf), "%i", (int)m_fe.cloud.size () );
	mtkit_strtothou ( buf, buf, (int)sizeof(buf), ',', '-', '.', 3, 0 );
	m_info_points->setText ( buf );

	Crul::CloudPTS	* const	cpts = m_fe.cloud.get_cpts ();
	Crul::PtsExtent	const *	extents = cpts->extents_ready ();

	if ( ! extents )
	{
		mtQEX::BusyDialog dialog ( this, "Calculating extents." );
		ThreadPtsExtent work ( cpts );

		work.start ();
		dialog.wait_for_thread ( work );

		extents = work.get_extents ();
	}

	if ( ! extents )
	{
		update_extents_clear ();
	}
	else
	{
		print_label ( m_info_x, extents->x_min, extents->x_max );
		print_label ( m_info_y, extents->y_min, extents->y_max );
		print_label ( m_info_z, extents->z_min, extents->z_max );
		print_label ( m_info_r, extents->r_min, extents->r_max );
		print_label ( m_info_g, extents->g_min, extents->g_max );
		print_label ( m_info_b, extents->b_min, extents->b_max );
	}
}

void Mainwindow::update_extents_clear ()
{
	m_info_x->setText ( "" );
	m_info_y->setText ( "" );
	m_info_z->setText ( "" );
	m_info_r->setText ( "" );
	m_info_g->setText ( "" );
	m_info_b->setText ( "" );
}

void Mainwindow::update_gl_view ()
{
	m_cloud_view_a->update ();
	m_cloud_view_b->update ();
}

void Mainwindow::set_slider_angle (
	QSlider	* const	slider,
	double	const	angle
	)
{
	slider->blockSignals ( true );
	slider->setValue ( (int)round (angle) );
	slider->blockSignals ( false );
}

void Mainwindow::set_xrotation_a ( double const angle )
{
	set_slider_angle ( m_slider_ax, angle );
}

void Mainwindow::set_zrotation_a ( double const angle )
{
	set_slider_angle ( m_slider_az, angle );
}

void Mainwindow::set_xrotation_b ( double const angle )
{
	set_slider_angle ( m_slider_bx, angle );
}

void Mainwindow::set_zrotation_b ( double const angle )
{
	set_slider_angle ( m_slider_bz, angle );
}

void Mainwindow::update_view_a ()
{
	m_cloud_view_a->update ();
}

void Mainwindow::update_view_b ()
{
	m_cloud_view_b->update ();
}

void Mainwindow::set_nudge ( int const i )
{
	int const num = mtkit_int_bound ( i, Crul::VIEW_NUDGE_MIN,
		Crul::VIEW_NUDGE_MAX );

	m_prefs.set ( PREFS_VIEW_NUDGE_SIZE, num );

	char buf[128];
	double const val = pow ( 2, num );
	snprintf ( buf, sizeof(buf), "%i = %.8g", num, val );
	m_label_nudge->setText ( buf );
	m_slider_nudge->setValue ( i );

	m_cloud_view_a->set_nudge ( val );
	m_cloud_view_b->set_nudge ( val );

	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::set_point_size ( int const i )
{
	int const num = mtkit_int_bound ( i, Crul::VIEW_POINT_SIZE_MIN,
		Crul::VIEW_POINT_SIZE_MAX );

	m_prefs.set ( PREFS_GL_POINT_SIZE, (double)num );

	char buf[128];
	snprintf ( buf, sizeof(buf), "%i", num );
	m_label_pt_size->setText ( buf );
	m_slider_pt_size->setValue ( i );

	m_fe.cloud_gl.set_point_size ( (double)num );

	update_gl_view ();
}

void Mainwindow::set_line_butt_size ( int const i )
{
	int const num = mtkit_int_bound ( i, Crul::VIEW_LINE_BUTT_SIZE_MIN,
		Crul::VIEW_LINE_BUTT_SIZE_MAX );

	m_prefs.set ( PREFS_GL_LINE_BUTT_SIZE, (double)num );

	char buf[128];
	double const val = pow ( 2, i );
	snprintf ( buf, sizeof(buf), "%i = %.8g", num, val );
	m_label_line_butt_size->setText ( buf );
	m_slider_line_butt_size->setValue ( i );

	m_fe.ruler_gl.set_line_butt_size ( val );

	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::set_line_thickness ( int const i )
{
	int const num = mtkit_int_bound ( i, Crul::VIEW_LINE_THICKNESS_MIN,
		Crul::VIEW_LINE_THICKNESS_MAX );

	m_prefs.set ( PREFS_GL_LINE_THICKNESS, (double)num );

	char buf[128];
	snprintf ( buf, sizeof(buf), "%i", num );
	m_label_line_thickness->setText ( buf );
	m_slider_line_thickness->setValue ( i );

	m_fe.ruler_gl.set_line_thickness ( num );

	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::update_ruler ()
{
	update_ruler_info ();
	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::update_view_show_items ()
{
	act_view_show_antialiasing->setChecked ( m_prefs.getInt
		( PREFS_VIEW_SHOW_ANTIALIASING ) );

	act_view_show_crosshair->setChecked ( m_prefs.getInt
		( PREFS_VIEW_SHOW_CROSSHAIR ) );

	act_view_show_statusbar->setChecked ( m_prefs.getInt
		( PREFS_VIEW_SHOW_STATUSBAR ) );

	act_view_show_rulers->setChecked ( m_prefs.getInt
		( PREFS_VIEW_SHOW_RULERS ) );

	act_view_show_ruler_plane->setChecked ( m_prefs.getInt
		( PREFS_VIEW_SHOW_RULER_PLANE ) );

	act_view_show_cloud->setChecked ( m_prefs.getInt
		( PREFS_VIEW_SHOW_CLOUD ) );

	act_view_show_model->setChecked ( m_prefs.getInt
		( PREFS_VIEW_SHOW_MODEL ) );

	press_view_show_antialiasing ();
	press_view_show_crosshair ();
	press_view_show_statusbar ();
	press_view_show_rulers ();
	press_view_show_ruler_plane ();
	press_view_show_cloud ();
	press_view_show_model ();
}

