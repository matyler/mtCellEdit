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

#include "qt5.h"



void Mainwindow::create_cline_files ()
{
	auto const & cline_files = backend.get_cline_files ();
	if ( cline_files.size () < 2 )
	{
		act_options_show_files->setEnabled ( false );
	}
	else
	{
		int const w = m_file_split->width();
		double const p = backend.mprefs.window_file_list_split;
		QList<int> const size_list = { (int)(w*p), (int)(w*(1-p)) };

		m_file_split->setSizes ( size_list );

		size_t const rowtot = cline_files.size();

		m_files_table->setRowCount ( (int)rowtot );

		for ( size_t i = 0; i < rowtot; i++ )
		{
			char const * const txt = strrchr ( cline_files[i],
				MTKIT_DIR_SEP );
			auto twItem = new QTableWidgetItem (
				mtQEX::qstringFromC ( txt ? (txt+1) :
					cline_files[i] ) );

			m_files_table->setItem ( (int)i, 0, twItem );
		}

		m_files_table->horizontalHeader ()->resizeSections (
			QHeaderView::ResizeToContents );

		act_options_show_files->setChecked ( true );
		m_files_table->setFocus ();
		m_files_table->setCurrentCell ( 0, 0 );
		connect ( m_files_table, &QTableWidget::currentCellChanged,
			[this, &cline_files]
			(	int const currentRow,
				int const ARG_UNUSED(currentColumn),
				int const ARG_UNUSED(previousRow),
				int const ARG_UNUSED(previousColumn)
				)
			{
				size_t const i = (size_t)currentRow;

				if ( i >= cline_files.size() )
				{
					return;		// Out of bounds
				}

				project_load ( cline_files[ i ] );
			} );
	}

	press_options_show_files ();
}

FileListKeyEater::FileListKeyEater ( Mainwindow & mw )
	:
	mainwindow	( mw )
{
}

bool FileListKeyEater::eventFilter (
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

bool FileListKeyEater::key_filter (
	QKeyEvent	* const	ev
	)
{
	switch ( ev->key () )
	{
	case Qt::Key_Escape:
		mainwindow.set_canvas_focus ();
		return true;

	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_PageUp:
	case Qt::Key_PageDown:
		return false;		// Widget handles these keys

	default:
		break;
	}

	return mainwindow.check_main_key_event ( ev );
}

