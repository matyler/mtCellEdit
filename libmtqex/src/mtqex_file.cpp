/*
	Copyright (C) 2013-2016 Mark Tyler

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
	QString		s;
	int		i;


	setWindowTitle ( title );
	setAcceptMode ( QFileDialog::AcceptSave );
	setOptions ( QFileDialog::DontUseNativeDialog );

	if ( filename )
	{
		selectFile ( mtQEX::qstringFromC ( filename ) );
	}

	if ( formatList.count () > 0 )
	{
		QLayout		* l = layout ();
		QGridLayout	* grid = dynamic_cast<QGridLayout *>( l );
		QWidget		* w;
		QHBoxLayout	* row;



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

		for ( i = 0; i < formatList.count (); i++ )
		{
			s = formatList.at ( i );

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

