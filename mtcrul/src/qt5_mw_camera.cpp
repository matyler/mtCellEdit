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

#include "qt5_mw_camera.h"



#define LIST_COLUMN_ID		0
#define LIST_COLUMN_PROTECT	1
#define LIST_COLUMN_LABEL	2

#define LIST_COLUMN_REF_ID	LIST_COLUMN_PROTECT



int Frontend::add_camera ( Crul::Camera & camera )
{
	try
	{
		int id = 1;

		if ( camera_map.size () > 0 )
		{
			if ( camera_map.rbegin () != camera_map.rend () )
			{
				id = MAX( id, camera_map.rbegin ()->first + 1 );
			}
		}

		camera.set_id ( id );
		camera_map.insert( std::pair<int, Crul::Camera>(id, camera) );
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}

Crul::Camera * Frontend::get_camera ( int const id )
{
	std::map<int, Crul::Camera>::iterator const it = camera_map.find ( id );

	return (it == camera_map.end()) ? NULL : &it->second;
}

Crul::Camera * Mainwindow::get_active_camera ()
{
	int const row = m_camera_table->currentRow ();

	if ( row < 0 )
	{
		return NULL;
	}

	QTableWidgetItem * twItem = m_camera_table->item ( row,
		LIST_COLUMN_REF_ID );

	if ( twItem )
	{
		int const id = twItem->data( Qt::UserRole ).value<int>();
		return m_fe.get_camera ( id );
	}

	return NULL;
}

void Mainwindow::press_camera_new ()
{
	DialogLabelEdit dialog ( this, "New Camera", "" );

	std::string label;

	if ( dialog.get_text ( label ) )
	{
		Crul::Camera camera;
		m_cloud_view_a->store_camera ( &camera );
		camera.set_label ( label );

		if ( m_fe.add_camera ( camera ) )
		{
			QMessageBox::critical ( this, "Error",
				"Unable to add new camera." );
		}
		else
		{
			populate_camera_list ();

			int const row = m_camera_table->rowCount () - 1;

			m_camera_table->setCurrentCell ( row, 0 );
		}
	}
}

void Mainwindow::press_camera_delete ()
{
	Crul::Camera * const camera = get_active_camera ();
	if ( ! camera )
	{
		return;
	}

	int const res = QMessageBox::question ( this, "Question",
		"Do you really want to delete this camera?",
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

	if ( res == QMessageBox::No )
	{
		return;
	}

	m_fe.camera_map.erase ( camera->get_id () );

	int const row = m_camera_table->currentRow ();
	m_camera_table->removeRow ( row );
	int const new_row = MIN(row,(m_camera_table->rowCount() - 1));

	m_camera_table->setCurrentCell ( new_row, 0 );
}

void Mainwindow::press_camera_edit ()
{
	Crul::Camera * const camera = get_active_camera ();
	if ( ! camera )
	{
		return;
	}

	DialogLabelEdit dialog ( this, "Edit Camera", camera->get_label().
		c_str() );

	std::string new_label;

	if ( dialog.get_text ( new_label ) )
	{
		int const row = m_camera_table->currentRow ();
		QTableWidgetItem * const twItem = m_camera_table->item ( row,
			LIST_COLUMN_LABEL );

		if ( twItem )
		{
			camera->set_label ( new_label );

			twItem->setText ( new_label.c_str () );
		}
	}
}

void Mainwindow::press_camera_a_xyz ()
{
	m_cloud_view_a->store_camera ( get_active_camera () );
}

void Mainwindow::press_camera_b_xyz ()
{
	m_cloud_view_b->store_camera ( get_active_camera () );
}

void Mainwindow::press_camera_list_row (
	int	const	row,
	int	const	ARG_UNUSED ( column ),
	int	const	ARG_UNUSED ( old_row ),
	int	const	ARG_UNUSED ( old_column )
	)
{
	if ( row < 0 )
	{
		return;
	}

	QTableWidgetItem * twItem = m_camera_table->item ( row,
		LIST_COLUMN_REF_ID );
	int id = 0;

	if ( twItem )
	{
		id = twItem->data( Qt::UserRole ).value<int>();
	}

	Crul::Camera const * const camera = m_fe.get_camera ( id );
	if ( camera )
	{
		if ( QGuiApplication::queryKeyboardModifiers().
			testFlag(Qt::ShiftModifier) )
		{
			m_cloud_view_b->set_camera ( camera );
		}
		else
		{
			m_cloud_view_a->set_camera ( camera );
		}
	}

	enable_camera_buttons ( camera );
}

void Mainwindow::press_camera_list_item_changed ( QTableWidgetItem * const item)
{
	if ( ! item )
	{
		return;
	}

	if ( item->column () == LIST_COLUMN_PROTECT )
	{
		// Checkbox has been changed for the protected read only state

		int const id = item->data( Qt::UserRole ).value<int>();
		bool const ro = item->checkState () == Qt::Checked ? true:false;

		Crul::Camera * const cam = m_fe.get_camera ( id );

		if ( cam )
		{
			cam->set_read_only ( ro );
		}

		enable_camera_buttons ( cam );
	}
}

void Mainwindow::populate_camera_list ()
{
	m_camera_table->clearContents ();

	std::map<int, Crul::Camera> const & map = m_fe.camera_map;

	m_camera_table->setRowCount ( (int)map.size () );

	int row = 0;

	m_camera_table->blockSignals ( true );

	for ( auto && i : map )
	{
		QTableWidgetItem * twItem;
		int const id = i.second.get_id();

		twItem = new QTableWidgetItem;
		twItem->setText ( QString::number(id) );
		m_camera_table->setItem ( row, LIST_COLUMN_ID, twItem );

		twItem = new QTableWidgetItem;
		twItem->setCheckState ( i.second.get_read_only() ? Qt::Checked :
			Qt::Unchecked );
		m_camera_table->setItem ( row, LIST_COLUMN_PROTECT, twItem );
		twItem->setData ( Qt::UserRole, QVariant::fromValue (id) );

		// Stop first 2 items from being unprotected
		if ( row < 2 )
		{
			twItem->setFlags ( twItem->flags() &
				(~ Qt::ItemIsEnabled) );
		}

		twItem = new QTableWidgetItem;
		twItem->setText ( i.second.get_label().c_str() );
		m_camera_table->setItem ( row, LIST_COLUMN_LABEL, twItem );

		row++;
	}

	m_camera_table->blockSignals ( false );

	// This hack ensures that all columns are right width (not just visible)
	m_camera_table->setVisible ( false );
	m_camera_table->resizeColumnsToContents ();
	m_camera_table->setVisible ( true );
}

void Mainwindow::enable_camera_buttons ( Crul::Camera const * const cam )
{
	bool const on = cam ? !cam->get_read_only () : false;

	m_button_cam_delete->setEnabled ( on );
	m_button_cam_edit->setEnabled ( on );
	m_button_cam_a_xyz->setEnabled ( on );
	m_button_cam_b_xyz->setEnabled ( on );
}

DialogLabelEdit::DialogLabelEdit (
	Mainwindow	* const	mw,
	char	const * const	title,
	char	const * const	label
	)
	:
	QDialog		( mw ),
	m_edit		( NULL )
{
	setWindowTitle ( title );

	QVBoxLayout * vbox = new QVBoxLayout;
	setLayout ( vbox );

	QWidget * w = new QWidget;
	vbox->addWidget ( w );
	QHBoxLayout * hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	hbox->addWidget ( new QLabel ( "Label: " ) );

	m_edit = new QLineEdit;
	hbox->addWidget ( m_edit );
	m_edit->setMaxLength ( Crul::LABEL_LENGTH_MAX );
	m_edit->setText ( label );

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( accepted () ), this,
		SLOT ( accept () ) );
	connect ( button_box, SIGNAL ( rejected () ), this,
		SLOT ( reject () ) );

	exec ();
}

int DialogLabelEdit::get_text ( std::string & text )
{
	if ( result () == QDialog::Rejected )
	{
		return 0;
	}

	text = m_edit->text ().toUtf8 ().data ();

	return 1;
}

