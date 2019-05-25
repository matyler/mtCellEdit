/*
	Copyright (C) 2013-2018 Mark Tyler

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



#include "static.h"
#include "be.h"



#define TABLE_ID "table_id"



class MainWindow;
class TableAnalysis;



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow ();
	~MainWindow ();

	void analyse ( char const * path );

protected:
	virtual void closeEvent ( QCloseEvent * ev );

private slots:
	void press_menu_open ();
	void press_menu_close ();
	void press_menu_next ();
	void press_menu_previous ();
	void press_menu_copy ();
	void press_menu_quit ();

	void press_help_about_qt ();
	void press_help_about ();

private:
	void create_menu ();

/// ----------------------------------------------------------------------------

	mtKit::Prefs	prefs;

	int		m_tab_id;
	QTabWidget	* m_tab_widget;

	QMap<int, TableAnalysis*> m_table_map;
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

