/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include "qt4.h"



groupList::groupList (
	mtKit::Prefs		&preferences,
	QString		const	&title,
	QWidget		* const	parent
	)
	:
	QGroupBox	( title, parent ),
	prefs		( preferences ),
	m_title		( title ),
	m_id		( 0 ),
	m_table		( NULL )
{
	setAcceptDrops ( true );

	QVBoxLayout * gb_layv = new QVBoxLayout ( this );

	QWidget * w = new QWidget;
	gb_layv->addWidget ( w );

	QHBoxLayout * gb_layh = new QHBoxLayout ( w );

	QPushButton * button;

	button = new QPushButton ( QIcon::fromTheme ( "list-add" ), "Add" );
	gb_layh->addWidget ( button );
	connect( button, SIGNAL(clicked()), this, SLOT( press_add () ) );

	button = new QPushButton ( QIcon::fromTheme ("list-remove"), "Remove" );
	gb_layh->addWidget ( button );
	connect( button, SIGNAL(clicked()), this, SLOT( press_remove() ));

	m_table = new QTableWidget;
	gb_layv->addWidget ( m_table );

	m_table->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_table->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_table->setColumnCount ( 2 );
	m_table->setShowGrid ( false );
	m_table->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
	m_table->horizontalHeader ()->hide ();
	m_table->horizontalHeader ()->setStretchLastSection ( true );
	m_table->setHorizontalScrollMode ( QAbstractItemView::ScrollPerPixel );
}

groupList::~groupList ()
{
}

int groupList::add_filename ( char const * const filename )
{
	if ( 0 == mtkit_file_readable ( filename ) )
	{
		// Not a file, doesn't exist or unreadable
		fprintf ( stderr, "Not a file; doesn't exist; unreadable: "
			"'%s'\n", filename );

		return 1;
	}

	try
	{
		m_file_map.insert ( m_id, filename );
	}
	catch ( ... )
	{
		return 1;
	}

	int	const	row = m_table->rowCount ();

	m_table->setRowCount ( row + 1 );

	QTableWidgetItem * twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( filename ).toUtf8 () );
	m_table->setItem ( row, 0, twItem );
	twItem->setData ( Qt::UserRole, QVariant::fromValue ( m_id ) );

	m_id++;

	return 0;
}

void groupList::resize_columns ()
{
	// This hack ensures that all columns are right width (not just visible)
	m_table->setVisible ( false );
	m_table->resizeColumnsToContents ();
	m_table->setVisible ( true );
}

void groupList::dragEnterEvent ( QDragEnterEvent * const event )
{
	if ( event->mimeData ()->hasUrls () )
	{
		event->acceptProposedAction ();
	}
}

void groupList::dropEvent ( QDropEvent * const event )
{
	QList<QUrl> const list = event->mimeData ()->urls ();
	int dropped = 0;

	for ( int i = 0; i < list.size (); i++ )
	{
		if ( add_filename ( (char const *)list.at ( i ).toLocalFile ().
			toUtf8 ().data () ) )
		{
			dropped = 1;
			continue;
		}
	}

	resize_columns ();

	if ( dropped )
	{
		QMessageBox::critical ( this, "Warning",
			"One or more of the files were invalid and were "
			"ignored." );
	}

	event->acceptProposedAction ();

	update_title ();
}

void groupList::press_add ()
{
	char const * last_dir = prefs.getString ( PREFS_LAST_DIRECTORY );

	if ( ! last_dir || last_dir[0] == 0 )
	{
		last_dir = mtkit_file_home ();
	}

	QStringList const files = QFileDialog::getOpenFileNames ( this,
		"Add File(s)", mtQEX::qstringFromC ( last_dir ), NULL, NULL,
		QFileDialog::DontUseNativeDialog );

	if ( files.size () < 1 )
	{
		return;
	}

	for ( int i = 0; i < files.size (); i++ )
	{
		add_filename ( files.at ( i ).toUtf8 ().data () );
	}

	resize_columns ();

	prefs.set ( PREFS_LAST_DIRECTORY, files.at (0).toUtf8 ().data () );

	update_title ();
}

void groupList::press_remove ()
{
	QList<QTableWidgetItem *> const list = m_table->selectedItems ();

	for ( int i = 0; i < list.size (); i++ )
	{
		QTableWidgetItem * const twItem = list.at ( i );
		if ( ! twItem )
		{
			continue;
		}

		int const id = twItem->data( Qt::UserRole ).value<int>();

		m_file_map.remove ( id );

		m_table->removeRow ( twItem->row () );
	}

	update_title ();
}

void groupList::update_title ()
{
	m_table->selectAll ();

	int const tot = m_table->rowCount ();

	setTitle ( m_title + " (" + QString::number ( tot ) + ")" );
}

int groupList::get_filenames ( QList<std::string> &list )
{
	try
	{
		QList<QTableWidgetItem *> const sel = m_table->selectedItems ();

		for ( int i = 0; i < sel.size (); i++ )
		{
			QTableWidgetItem * const twItem = sel.at ( i );
			if ( ! twItem )
			{
				continue;
			}

			int const id = twItem->data( Qt::UserRole).value<int>();

			std::string const file = m_file_map.value ( id, "" );

			list.append ( file );
		}
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

