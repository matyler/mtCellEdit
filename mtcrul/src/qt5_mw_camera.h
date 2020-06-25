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

#ifndef QT5_MW_CAMERA_H
#define QT5_MW_CAMERA_H

#include "qt5_mw.h"



class DialogLabelEdit : public QDialog
{
	Q_OBJECT

public:
	DialogLabelEdit (
		Mainwindow	* mw,
		char	const	* title,
		char	const	* label
		);

	int get_text ( std::string & text );	// 0=Cancelled 1=Confirmed

private:
	QLineEdit	* m_edit;
};



#endif		// QT5_MW_CAMERA_H

