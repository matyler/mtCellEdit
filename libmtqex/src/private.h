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

#ifdef U_TK_QT4
	#include "mtqex4.h"
#endif

#ifdef U_TK_QT5
	#include "mtqex5.h"
#endif



class privDialogText : public QDialog
{
	Q_OBJECT

public:
	privDialogText (
		int textType,		// 0 = Single line, 1 = Multi line
		QWidget * par,
		QString title,
		QString label,
		QString text,		// Initial text to populate dialog
		int maxLength = -1	// Maximum number of characters -1 = any
		);

	QString getText ();

private:
	QLineEdit	* textLineEdit;
	QTextEdit	* textEdit;
};

