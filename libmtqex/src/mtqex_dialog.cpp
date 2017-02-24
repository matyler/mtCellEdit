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



QString mtQEX::dialogTextLine (
	QWidget		* const	par,
	QString		const	title,
	QString		const	label,
	QString		const	text,
	int		const	maxLength
	)
{
	privDialogText	dialog ( 0, par, title, label, text, maxLength );


	if ( dialog.exec () == QDialog::Accepted )
	{
		return dialog.getText ();
	}

	return QString ();
}

QString mtQEX::dialogText (
	QWidget		* const	par,
	QString		const	title,
	QString		const	label,
	QString		const	text
	)
{
	privDialogText	dialog ( 1, par, title, label, text );


	if ( dialog.exec () == QDialog::Accepted )
	{
		return dialog.getText ();
	}

	return QString ();
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

