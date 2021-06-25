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

#include <unistd.h>



#ifdef U_TK_QT5
	#include "mtqex5.h"
#endif

#ifdef U_TK_QT6
	#include "mtqex6.h"
#endif



class privDialogText;
class UPrefsWindow;



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



class UPrefsWindow : public QDialog
{
	Q_OBJECT

public:
	UPrefsWindow (
		QWidget * parent,
		mtKit::UserPrefs & prefs,
		QString title
		);
	~UPrefsWindow ();

private:
	void pressButtonEdit ();
	void populateTable ();
	void update_table_status_value ( int row );
	std::string get_key ( int row, int * type = nullptr ) const;

/// ----------------------------------------------------------------------------

	QLineEdit	* m_filter_edit		= nullptr;
	QLineEdit	* m_info_edit		= nullptr;
	QTableWidget	* m_table_widget	= nullptr;
	QPushButton	* m_button_reset	= nullptr;
	QPushButton	* m_button_edit		= nullptr;

	mtKit::UserPrefs & m_prefs;
};


