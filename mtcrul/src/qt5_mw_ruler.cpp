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

#include "qt5_mw.h"



#define LIST_COLUMN_ID		0
#define LIST_COLUMN_PROTECT	1
#define LIST_COLUMN_SHOW	2
#define LIST_COLUMN_LABEL	3

#define LIST_COLUMN_REF_ID	LIST_COLUMN_SHOW



int Frontend::add_ruler ( Crul::Ruler & ruler )
{
	try
	{
		int id = 1;

		if ( ruler_map.size () > 0 )
		{
			if ( ruler_map.rbegin () != ruler_map.rend () )
			{
				id = MAX( id, ruler_map.rbegin ()->first + 1 );
			}
		}

		ruler.set_id ( id );
		ruler_map.insert( std::pair<int, Crul::Ruler>(id, ruler) );
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}

Crul::Ruler * Frontend::get_ruler ( int const id )
{
	std::map<int, Crul::Ruler>::iterator const it = ruler_map.find ( id );

	return (it == ruler_map.end()) ? NULL : &it->second;
}

Crul::Ruler * Mainwindow::get_active_ruler ()
{
	int const row = m_ruler_table->currentRow ();

	if ( row < 0 )
	{
		return NULL;
	}

	QTableWidgetItem * twItem = m_ruler_table->item ( row,
		LIST_COLUMN_REF_ID );

	if ( twItem )
	{
		int const id = twItem->data( Qt::UserRole ).value<int>();
		return m_fe.get_ruler ( id );
	}

	return NULL;
}

void Mainwindow::press_view_show_rulers ()
{
	bool const on = act_view_show_rulers->isChecked ();

	m_fe.ruler_gl.set_show_lines ( on );
	populate_gl_rulers ();
	update_gl_view ();

	m_uprefs.set ( PREFS_VIEW_SHOW_RULERS, on ? 1 : 0 );
}

void Mainwindow::press_view_show_ruler_plane ()
{
	bool const on = act_view_show_ruler_plane->isChecked ();

	m_fe.ruler_gl.set_show_plane ( on );
	populate_gl_rulers ();
	update_gl_view ();

	m_uprefs.set ( PREFS_VIEW_SHOW_RULER_PLANE, on ? 1 : 0 );
}

void Mainwindow::press_ruler_swap_ab ()
{
	Crul::Ruler * const ruler = get_active_ruler ();
	if ( ! ruler )
	{
		return;
	}

	ruler->swap_ab ();

	update_ruler_info ();
	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::press_ruler_deselect ()
{
	m_ruler_table->clearSelection ();
	m_ruler_table->setCurrentCell ( -1, -1 );
	m_fe.ruler_gl.set_active_ruler_id ( 0 );

	update_ruler_info ();
	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::press_ruler_rgb_set ()
{
	Crul::Ruler * const ruler = get_active_ruler ();
	if ( ! ruler )
	{
		return;
	}

	Crul::Line const & line = ruler->get_line ();
	QColor const color = QColorDialog::getColor ( QColor (
		line.get_r_int (), line.get_g_int (), line.get_b_int ()  ),
		this, "Ruler Line Colour", QColorDialog::DontUseNativeDialog );

	if ( color.isValid () )
	{
		ruler->set_line_rgb ( color.red(), color.green(), color.blue());

		populate_gl_rulers ();
		update_ruler_info ();
		update_gl_view ();
	}
}

void Mainwindow::press_ruler_new ()
{
	DialogLabelEdit dialog ( this, "New Ruler", "" );

	std::string label;

	if ( dialog.get_text ( label ) )
	{
		mtGin::GL::Matrix4x4 camat;
		m_cloud_view_a->get_camera_matrix ( camat );

		mtGin::GL::Matrix3x3 const normal = camat.normal ();
		mtGin::GL::Array3x3_float const & d = normal.data ();

		double const len = 10.0;	// Distance away from camera
		double const lz = 5;		// Up/Down half length

		double const dx = len * (double)-d[0][2];
		double const dy = len * (double)-d[1][2];
		double const dz = len * (double)-d[2][2];

		Crul::Camera const & camera = m_cloud_view_a->get_camera();
		Crul::Ruler ruler;

		ruler.set_line (
			camera.get_x() + dx,
			camera.get_y() + dy,
			camera.get_z() + dz - lz,
			camera.get_x() + dx,
			camera.get_y() + dy,
			camera.get_z() + dz + lz
			);

		ruler.set_label ( label );

		if ( m_fe.add_ruler ( ruler ) )
		{
			QMessageBox::critical ( this, "Error",
				"Unable to add new ruler." );
		}
		else
		{
			populate_ruler_list ();

			int const row = m_ruler_table->rowCount () - 1;

			m_ruler_table->setCurrentCell ( row, 0 );

			populate_gl_rulers ();
			update_gl_view ();
		}
	}
}

void Mainwindow::press_ruler_copy ()
{
	Crul::Ruler const * const ruler = get_active_ruler ();
	if ( ! ruler )
	{
		return;
	}

	Crul::Ruler ruler_new = *ruler;

	if ( m_fe.add_ruler ( ruler_new ) )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to add new ruler." );
	}
	else
	{
		populate_ruler_list ();

		int const row = m_ruler_table->rowCount () - 1;

		m_ruler_table->setCurrentCell ( row, 0 );

		populate_gl_rulers ();
		update_gl_view ();
	}
}

void Mainwindow::press_ruler_delete ()
{
	Crul::Ruler * const ruler = get_active_ruler ();
	if ( ! ruler )
	{
		return;
	}

	int const res = QMessageBox::question ( this, "Question",
		"Do you really want to delete this ruler?",
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

	if ( res == QMessageBox::No )
	{
		return;
	}

	m_fe.ruler_map.erase ( ruler->get_id() );
	populate_ruler_list ();
	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::press_ruler_edit ()
{
	Crul::Ruler * const ruler = get_active_ruler ();
	if ( ! ruler )
	{
		return;
	}

	DialogLabelEdit dialog( this, "Edit Ruler", ruler->get_label().c_str());
	std::string new_label;

	if ( dialog.get_text ( new_label ) )
	{
		int const row = m_ruler_table->currentRow ();
		QTableWidgetItem * const twItem = m_ruler_table->item ( row,
			LIST_COLUMN_LABEL );

		if ( twItem )
		{
			ruler->set_label ( new_label );

			twItem->setText ( new_label.c_str () );
		}
	}
}

void Mainwindow::press_ruler_hide_all ()
{
	int const tot = m_ruler_table->rowCount ();

	for ( int i = 0; i < tot; i++ )
	{
		QTableWidgetItem * const twItem = m_ruler_table->item ( i,
			LIST_COLUMN_SHOW );

		if ( twItem )
		{
			twItem->setCheckState ( Qt::Unchecked );
		}
	}

	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::change_ruler_plane ( int const i )
{
	Crul::Ruler * const ruler = get_active_ruler ();

	if ( ! ruler )
	{
		return;
	}

	ruler->set_plane ( i );

	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::update_ruler_info ()
{
	Crul::Ruler * const ruler = get_active_ruler ();

	if ( ! ruler )
	{
		return;
	}

	Crul::Line const & line = ruler->get_line ();

	char buf[128];

	snprintf ( buf, sizeof(buf), "(%.2f, %.2f, %.2f)",
		(double)line.x1,
		(double)line.y1,
		(double)line.z1
		);
	m_label_rul_axyz->setText ( buf );

	snprintf ( buf, sizeof(buf), "(%.2f, %.2f, %.2f)",
		(double)line.x2,
		(double)line.y2,
		(double)line.z2
		);
	m_label_rul_bxyz->setText ( buf );

	snprintf ( buf, sizeof(buf), "%.2f", line.get_length () );
	m_label_rul_length->setText ( buf );

	double x, y, z;
	line.get_unit_vector ( x, y, z );

	snprintf ( buf, sizeof(buf), "(%.2f, %.2f, %.2f)", x, y, z );
	m_label_rul_unit_vec->setText ( buf );

	snprintf ( buf, sizeof(buf), "%.2f", line.get_angle_xy () );
	m_label_rul_angle_xy->setText ( buf );

	snprintf ( buf, sizeof(buf), "%.2f", line.get_angle_z () );
	m_label_rul_angle_z->setText ( buf );

	snprintf ( buf, sizeof(buf), "(%.2f, %.2f, %.2f)",
		(double)line.r, (double)line.g, (double)line.b );
	m_label_rul_rgb->setText ( buf );

	m_button_rul_plane->blockSignals ( true );
	m_button_rul_plane->setCurrentIndex ( ruler->get_plane() );
	m_button_rul_plane->blockSignals ( false );
}

void Mainwindow::press_ruler_list_row (
	int	const	ARG_UNUSED ( row ),
	int	const	ARG_UNUSED ( column ),
	int	const	ARG_UNUSED ( old_row ),
	int	const	ARG_UNUSED ( old_column )
	)
{
	Crul::Ruler * const ruler = get_active_ruler ();

	enable_ruler_buttons ( ruler );

	if ( ! ruler )
	{
		clear_ruler_info ();

		return;
	}

	update_ruler_info ();

	m_fe.ruler_gl.set_active_ruler_id ( ruler->get_id() );
	populate_gl_rulers ();
	update_gl_view ();
}

void Mainwindow::press_ruler_list_item_changed ( QTableWidgetItem * const item)
{
	if ( ! item )
	{
		return;
	}

	int const id = item->data( Qt::UserRole ).value<int>();
	Crul::Ruler * const ruler = m_fe.get_ruler ( id );
	if ( ! ruler )
	{
		return;
	}

	bool const on = item->checkState () == Qt::Checked ? true:false;

	switch ( item->column () )
	{
	case LIST_COLUMN_SHOW:
		ruler->set_visible ( on );
		populate_gl_rulers ();
		update_gl_view ();
		break;

	case LIST_COLUMN_PROTECT:
		ruler->set_read_only ( on );
		enable_ruler_buttons ( ruler );
		break;
	}
}

void Mainwindow::populate_ruler_list ()
{
	m_ruler_table->clearContents ();
	clear_ruler_info ();
	enable_ruler_buttons ( NULL );

	std::map<int, Crul::Ruler> const & map = m_fe.ruler_map;

	m_ruler_table->setRowCount ( (int)map.size () );

	int row = 0;

	m_ruler_table->blockSignals ( true );

	for ( auto && i : map )
	{
		QTableWidgetItem * twItem;
		int const id = i.second.get_id();

		twItem = new QTableWidgetItem;
		twItem->setText ( QString::number(id) );
		m_ruler_table->setItem ( row, LIST_COLUMN_ID, twItem );

		twItem = new QTableWidgetItem;
		twItem->setCheckState ( i.second.get_visible() ? Qt::Checked :
			Qt::Unchecked );
		m_ruler_table->setItem ( row, LIST_COLUMN_SHOW, twItem );
		twItem->setData ( Qt::UserRole, QVariant::fromValue (id) );

		twItem = new QTableWidgetItem;
		twItem->setCheckState ( i.second.get_read_only() ? Qt::Checked :
			Qt::Unchecked );
		m_ruler_table->setItem ( row, LIST_COLUMN_PROTECT, twItem );
		twItem->setData ( Qt::UserRole, QVariant::fromValue (id) );

		twItem = new QTableWidgetItem;
		twItem->setText ( i.second.get_label().c_str() );
		m_ruler_table->setItem ( row, LIST_COLUMN_LABEL, twItem );

		row++;
	}

	m_ruler_table->blockSignals ( false );

	// This hack ensures that all columns are right width (not just visible)
	m_ruler_table->setVisible ( false );
	m_ruler_table->resizeColumnsToContents ();
	m_ruler_table->setVisible ( true );

	m_fe.ruler_gl.set_active_ruler_id ( 0 );
}

void Mainwindow::clear_ruler_info ()
{
	m_label_rul_axyz->setText ( "" );
	m_label_rul_bxyz->setText ( "" );
	m_label_rul_length->setText ( "" );
	m_label_rul_unit_vec->setText ( "" );
	m_label_rul_angle_xy->setText ( "" );
	m_label_rul_angle_z->setText ( "" );
	m_label_rul_rgb->setText ( "" );
}

void Mainwindow::enable_ruler_buttons ( Crul::Ruler const * const rul )
{
	bool const on = rul ? !rul->get_read_only () : false;

	m_button_rul_rgb_set->setEnabled ( on );
	m_button_rul_plane->setEnabled ( on );
	m_button_rul_delete->setEnabled ( on );
//	m_button_rul_copy->setEnabled ( on );
	m_button_rul_edit->setEnabled ( on );
//	m_button_rul_hide_all->setEnabled ( on );
}

void Mainwindow::populate_gl_rulers ()
{
	m_fe.ruler_gl.populate ( m_fe.ruler_map,
		m_cloud_view_a->get_nudge () * 100 );
}

void Mainwindow::keypress_ruler (
	CloudView	* const	view,
	QKeyEvent	* const	event
	)
{
	Crul::Ruler * const ruler = get_active_ruler ();
	if ( ! ruler || ruler->get_read_only () || ! ruler->get_visible () )
	{
		return;
	}

	int mx = 0, my = 0;
	int const scale = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;
	double ruler_len_delta = 0;
	double const nudge = m_cloud_view_a->get_nudge ();

	switch ( event->key () )
	{
	case Qt::Key_PageUp:
		ruler_len_delta = scale * nudge;
		break;

	case Qt::Key_PageDown:
		ruler_len_delta = -scale * nudge;
		break;

	case Qt::Key_Up:	my = -1;	break;
	case Qt::Key_Down:	my = 1;		break;
	case Qt::Key_Left:	mx = -1;	break;
	case Qt::Key_Right:	mx = 1;		break;
	}

	if ( mx || my )
	{
		double const f = scale * nudge;
		double dxf = f, dyf = f, dzf = f;

		switch ( ruler->get_plane () )
		{
		case Crul::Ruler::PLANE_XY:	dzf = 0.0;	break;
		case Crul::Ruler::PLANE_XZ:	dyf = 0.0;	break;
		case Crul::Ruler::PLANE_YZ:	dxf = 0.0;	break;
		}

		mtGin::GL::Matrix4x4 camat;
		view->get_camera_matrix ( camat );

		mtGin::GL::Matrix3x3 const normal = camat.normal ();
		mtGin::GL::Array3x3_float const & d = normal.data ();

		double const dx = dxf * (my * (double)d[0][1] -
			mx * (double)d[0][0]);
		double const dy = dyf * (my * (double)d[1][1] -
			mx * (double)d[1][0]);
		double const dz = dzf * (my * (double)d[2][1] -
			mx * (double)d[2][0]);

		if ( dx !=0 || dy != 0 || dz != 0 )
		{
			Crul::Line const & line = ruler->get_line ();

			ruler->set_line (
				line.x1 - GLfloat(dx),
				line.y1 - GLfloat(dy),
				line.z1 - GLfloat(dz),
				line.x2, line.y2, line.z2 );

			update_ruler_info ();
			populate_gl_rulers ();
			update_gl_view ();
		}
	}
	else if ( ruler->change_length ( ruler_len_delta, nudge * 2.0 ) )
	{
		update_ruler_info ();
		populate_gl_rulers ();
		update_gl_view ();
	}
}

void Mainwindow::mouse_ruler (
	CloudView	* const view,
	QMouseEvent	* const	event,
	int		const	mx,
	int		const	my
	)
{
	Crul::Ruler * const ruler = get_active_ruler ();
	if (	! ruler
		|| (mx == 0 && my == 0)
		|| ruler->get_read_only ()
		|| ! ruler->get_visible ()
		)
	{
		return;
	}

	int const scale = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;
	double const nudge = view->get_nudge ();
	double const f = scale * nudge;
	double dxf = f, dyf = f, dzf = f;
	int const dim_a = (event->buttons () & Qt::LeftButton) ? 1 : 0;
	int const dim_b = (event->buttons () & Qt::RightButton) ? 1 : 0;

	switch ( ruler->get_plane () )
	{
	case Crul::Ruler::PLANE_XY:
		dxf *= dim_a;
		dyf *= dim_b;
		dzf = 0.0;
		break;

	case Crul::Ruler::PLANE_XZ:
		dxf *= dim_a;
		dyf = 0.0;
		dzf *= dim_b;
		break;

	case Crul::Ruler::PLANE_YZ:
		dxf = 0.0;
		dyf *= dim_a;
		dzf *= dim_b;
		break;
	}

	mtGin::GL::Matrix4x4 camat;
	view->get_camera_matrix ( camat );

	mtGin::GL::Matrix3x3 const normal = camat.normal ();
	mtGin::GL::Array3x3_float const & d = normal.data ();

	double const dx = dxf * (my * (double)d[0][1] - mx * (double)d[0][0]);
	double const dy = dyf * (my * (double)d[1][1] - mx * (double)d[1][0]);
	double const dz = dzf * (my * (double)d[2][1] - mx * (double)d[2][0]);

	if ( dx !=0 || dy != 0 || dz != 0 )
	{
		Crul::Line const & line = ruler->get_line ();

		ruler->set_line (
			line.x1 - GLfloat(dx),
			line.y1 - GLfloat(dy),
			line.z1 - GLfloat(dz),
			line.x2, line.y2, line.z2 );

		update_ruler_info ();
		populate_gl_rulers ();
		update_gl_view ();
	}
}

