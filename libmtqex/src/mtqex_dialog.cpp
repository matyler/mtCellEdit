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



int mtQEX::dialogTextLine (
	QWidget		* const	par,
	QString		const	title,
	QString		const	label,
	QString		const	text,
	int		const	maxLength,
	QString			&result
	)
{
	privDialogText	dialog ( 0, par, title, label, text, maxLength );


	if ( dialog.exec () == QDialog::Accepted )
	{
		result = dialog.getText ();
		return 0;
	}

	return 1;
}

int mtQEX::dialogText (
	QWidget		* const	par,
	QString		const	title,
	QString		const	label,
	QString		const	text,
	QString			&result
	)
{
	privDialogText	dialog ( 1, par, title, label, text );


	if ( dialog.exec () == QDialog::Accepted )
	{
		result = dialog.getText ();
		return 0;
	}

	return 1;
}

void mtQEX::set_minimum_width (
	QLineEdit	* const	edit,
	int		const	length
	)
{
	if ( ! edit )
	{
		return;
	}

	QString txt;

	int const len = MIN ( length, 100 );

	for ( int i = 0; i < len; i++ )
	{
		txt.append ('0');
	}

	QFontMetrics fm = edit->fontMetrics ();
	edit->setMinimumSize ( fm.boundingRect (txt).width (), 0 );
}

privDialogText::privDialogText (
	int		const	textType,
	QWidget		* const	par,
	QString		const	title,
	QString		const	label,
	QString		const	text,
	int		const	maxLength
	)
	:
	QDialog		( par ),
	textLineEdit	(),
	textEdit	()
{
	QVBoxLayout	* vLayout;
	QDialogButtonBox * buttonBox;


	vLayout = new QVBoxLayout;

	vLayout->addWidget ( new QLabel ( label ) );

	if ( textType == 0 )
	{
		textLineEdit = new QLineEdit ( text );
		vLayout->addWidget ( textLineEdit );

		if ( maxLength > 0 )
		{
			textLineEdit->setMaxLength ( maxLength );
		}

		mtQEX::set_minimum_width ( textLineEdit, 25 );
	}
	else
	{
		textEdit = new QTextEdit ();
		textEdit->setPlainText ( text );
		vLayout->addWidget ( textEdit );
	}

	buttonBox = new QDialogButtonBox (
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
	vLayout->addWidget ( buttonBox );

	connect ( buttonBox, SIGNAL ( accepted () ), this, SLOT ( accept () ) );
	connect ( buttonBox, SIGNAL ( rejected () ), this, SLOT ( reject () ) );

	setLayout ( vLayout );

	setWindowTitle ( title );
}

QString privDialogText::getText ()
{
	if ( textLineEdit )
	{
		return textLineEdit->text ();
	}

	if ( textEdit )
	{
		return textEdit->toPlainText ();
	}

	return QString ();
}

