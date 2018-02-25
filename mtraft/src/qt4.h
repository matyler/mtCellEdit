/*
	Copyright (C) 2013-2017 Mark Tyler

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
	#include <mtqex4.h>
#endif

#ifdef U_TK_QT5
	#include <mtqex5.h>
#endif



#include "be.h"



class MainWindow;
class TableAnalysis;



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow ();
	~MainWindow ();

	void analyse ( char const * path );

	BusyState busy;

protected:
	virtual void closeEvent ( QCloseEvent * ev );

private slots:
	void press_button_quit ();
	void press_button_copy ();
	void press_tab_next ();
	void press_tab_previous ();

private:
	mtKit::Prefs	prefs;

	QProgressBar	* m_progress;
	QTabWidget	* m_tab_widget;
	QPushButton	* m_button_copy;

	TableAnalysis	* m_table [ MAX_TABS ];		// Owned
};



class TableAnalysis : public QTableWidget
{
	Q_OBJECT

public:
	TableAnalysis (
		CedSheet * sheet,
		char const * path,
		MainWindow &mw,
		QVBoxLayout * vbox
		);
	~TableAnalysis ();

	void copy_to_clipboard ();

private slots:
	void table_cell_activated ( int row, int column );
	void table_header_click ( int index );

private:
	void sort_table ( int column, Qt::SortOrder direction );
			// -column = Flip or descending

	MainWindow	&mainwindow;

	Qt::SortOrder	m_sort_order;
	int		m_sort_column;
	int		m_row_total;

	char		* m_path;			// Owned
	CedSheet	* m_sheet;			// Owned
};

