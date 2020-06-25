/*
	Copyright (C) 2013-2019 Mark Tyler

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

#include "private.h"



mtQEX::SaveFileDialog::SaveFileDialog (
	QWidget		* const	par,
	QString		const	title,
	QStringList	const	formatList,
	int		const	format,
	char	const	* const	filename
	)
	:
	QFileDialog	( par ),
	comboFormat	()
{
	setWindowTitle ( title );
	setAcceptMode ( QFileDialog::AcceptSave );
	setOptions ( QFileDialog::DontUseNativeDialog );

	if ( filename )
	{
		selectFile ( mtQEX::qstringFromC ( filename ) );
	}

	QLayout * const l = layout ();

	if ( l && formatList.count () > 0 )
	{
		QGridLayout	* grid = dynamic_cast<QGridLayout *>( l );
		QHBoxLayout	* row;
		QWidget		* w;

		w = new QWidget ();

		if ( grid )
		{
			grid->addWidget ( w, grid->rowCount (), 0, 1,
				grid->columnCount () );
		}
		else
		{
			// If all else fails just add it!
			l->addWidget ( w );
		}

		row = new QHBoxLayout;
		row->setMargin ( 0 );
		w->setLayout ( row );

		QLabel * label = new QLabel ( "File Format:" );
		row->addWidget ( label );

		comboFormat = new QComboBox;
		row->addWidget ( comboFormat );

		w = new QWidget;
		w->setSizePolicy ( QSizePolicy ( QSizePolicy::MinimumExpanding,
			QSizePolicy::Preferred ) );
		row->addWidget ( w );

		for ( int i = 0; i < formatList.count (); i++ )
		{
			QString s = formatList.at ( i );

			comboFormat->addItem ( s );
		}

		comboFormat->setCurrentIndex ( format );
	}
}

int mtQEX::SaveFileDialog::getFormat ()
{
	if ( comboFormat )
	{
		return comboFormat->currentIndex ();
	}

	return 0;
}

int mtQEX::message_file_overwrite (
	QWidget	* const	parent,
	QString	const	&filename
	)
{
	if ( 0 == mtkit_file_readable ( filename.toUtf8 ().data () ) )
	{
		// No file so caller can save to this filename
		return 0;
	}

	int const res = QMessageBox::warning ( parent, "Warning",
		QString ( "%1 already exists. Do you want to replace it?" ).
			arg ( QString ( filename ) ),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

	if ( res == QMessageBox::Yes )
	{
		// Permission from user to overwrite
		return 0;
	}

	// No or cancel, so caller cannot overwrite the existing file
	return 1;
}

void mtQEX::set_multi_directories ( QFileDialog &dialog )
{
	dialog.setOption ( QFileDialog::DontUseNativeDialog );
	dialog.setOption ( QFileDialog::ShowDirsOnly );
	dialog.setFileMode ( QFileDialog::Directory );

	QListView * l = dialog.findChild<QListView *> ( "listView" );
	if ( l )
	{
		l->setSelectionMode ( QAbstractItemView::MultiSelection );
	}

	QTreeView * t = dialog.findChild<QTreeView *>();
	if ( t )
	{
		t->setSelectionMode ( QAbstractItemView::MultiSelection );
	}
}

QString mtQEX::get_filename ( QFileDialog &dialog )
{
	QStringList const files = dialog.selectedFiles ();

	if ( files.size () < 1 )
	{
		return "";
	}

	return files.at (0);
}

