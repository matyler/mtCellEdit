/*
	Copyright (C) 2013-2020 Mark Tyler

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

#include "qt5.h"



TableAnalysis::TableAnalysis (
	CedSheet	* const	sheet,
	std::string	const	& path,
	MainWindow		& mw,
	QVBoxLayout	* const	vbox
	)
	:
	mainwindow	( mw ),
	m_sort_order	( Qt::AscendingOrder ),
	m_sort_column	( RAFT_COL_BYTES ),
	m_row_total	( 0 ),
	m_path		( path ),
	m_sheet		( sheet )
{
	ced_sheet_get_geometry ( sheet, &m_row_total, NULL );

	setSelectionMode ( QAbstractItemView::SingleSelection );
	setSelectionBehavior ( QAbstractItemView::SelectRows );
	setEditTriggers ( QAbstractItemView::NoEditTriggers );
	setColumnCount ( RAFT_COL_TOTAL - 1 );

	QStringList	column_labels;
	column_labels
		<< "Name"
		<< "Files"
		<< "%"
		<< "Bytes"
		<< "MB"
		<< "%"
		<< "Subdirs"
		<< "Other"
		;
	setHorizontalHeaderLabels ( column_labels );

	QTableWidgetItem * twItem = horizontalHeaderItem ( 0 );
	if ( twItem )
	{
		twItem->setTextAlignment ( Qt::AlignLeft | Qt::AlignVCenter );
	}

	for ( int i = 1; i < (RAFT_COL_TOTAL - 1); i++ )
	{
		twItem = horizontalHeaderItem ( i );
		if ( twItem )
		{
			twItem->setTextAlignment ( Qt::AlignRight |
				Qt::AlignVCenter );
		}
	}

	horizontalHeader ()->setSortIndicatorShown ( true );
	sort_table ( m_sort_column, m_sort_order );

	verticalHeader ()->hide ();

	setShowGrid ( false );

	connect ( horizontalHeader (), SIGNAL ( sectionClicked ( int ) ),
		this, SLOT ( table_header_click ( int ) ) );

	connect ( this, SIGNAL ( cellActivated ( int, int ) ),
		this, SLOT ( table_cell_activated ( int, int ) ) );

	horizontalHeader ()->resizeSections ( QHeaderView::ResizeToContents );

	// Add some padding to columns
	for ( int i = 0; i < (RAFT_COL_TOTAL - 1); i++ )
	{
		int const w = horizontalHeader ()->sectionSize ( i );
		horizontalHeader ()->resizeSection ( i, w + 8 );
	}

	QLineEdit * const edit = new QLineEdit ( mtQEX::qstringFromC (
		path.c_str() ) );
	edit->setReadOnly ( true );

	vbox->addWidget ( edit );
	vbox->addWidget ( this );
}

TableAnalysis::~TableAnalysis ()
{
	ced_sheet_destroy ( m_sheet );
	m_sheet = NULL;
}

void TableAnalysis::copy_to_clipboard ()
{
	char * txt = raft_get_clipboard ( m_sheet );

	if ( txt )
	{
		QApplication::clipboard ()->setText ( txt );

		free ( txt );
		txt = NULL;
	}
}

void TableAnalysis::sort_table (
	int		const	column,
	Qt::SortOrder	const	direction
	)
{
	if ( column == m_sort_column )
	{
		// Reverse the current sort direction
		if ( m_sort_order == Qt::DescendingOrder )
		{
			m_sort_order = Qt::AscendingOrder;
		}
		else
		{
			m_sort_order = Qt::DescendingOrder;
		}
	}
	else
	{
		m_sort_order = direction;
	}

	m_sort_column = column;

	horizontalHeader()->setSortIndicator((m_sort_column - 1), m_sort_order);

	int cols[] = { 1, 0 }, order = 0;
	if ( m_sort_order == Qt::DescendingOrder )
	{
		order = CED_SORT_MODE_DESCENDING;
	}

	cols[0] = m_sort_column;
	ced_sheet_sort_rows ( m_sheet, 1, 0, cols, order, NULL );

	setCurrentItem ( NULL );	// Stops double selection of 0,0
	clearContents ();
	setRowCount ( m_row_total );

	for ( int r = 1; r <= m_row_total; r++ )
	{
		for ( int c = 1; c < RAFT_COL_TOTAL; c++ )
		{
			CedCell * cell = ced_sheet_get_cell ( m_sheet, r, c );
			if ( ! cell )
			{
				// Should never happen
				continue;
			}

			char buf[2000];
			if ( ced_cell_create_output ( cell, NULL, buf,
				sizeof(buf) ) )
			{
				continue;
			}

			QTableWidgetItem * const twitem = new QTableWidgetItem;
			twitem->setText ( mtQEX::qstringFromC ( buf ) );

			if ( c > 1 )
			{
				twitem->setTextAlignment ( Qt::AlignRight |
					Qt::AlignVCenter );
			}

			setItem ( r - 1, c - 1, twitem );
		}
	}

	setCurrentCell ( 0, 0 );
}

void TableAnalysis::table_header_click (
	int	const	index
	)
{
	sort_table ( index + 1, Qt::DescendingOrder );
}

void TableAnalysis::table_cell_activated (
	int	const	row,
	int	const	ARG_UNUSED ( column )
	)
{
	std::string new_path = raft_path_merge ( m_path, m_sheet, row + 1 );
	mainwindow.analyse ( new_path.c_str() );
}

