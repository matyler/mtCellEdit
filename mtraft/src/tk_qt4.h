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
	#include <mtqex4.h>
#endif

#ifdef U_TK_QT5
	#include <mtqex5.h>
#endif



#include "be.h"



class	MainWindow;
class	tableAnalysis;



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	enum Limits
	{
		MAX_TABS = 10
	};

	MainWindow	( char const * scan_directory );
	~MainWindow	();

	void		doAnalysis ( char const * path );



	busyState	busy;

protected:
	void		closeEvent ( QCloseEvent * ev );

private slots:
	void		pressButtonQuit ();
	void		pressButtonCopy ();
	void		pressTabNext ();
	void		pressTabPrevious ();

private:
	mtKit::Prefs	prefs;

	QProgressBar	* progressBar;
	QTabWidget	* tabWidget;
	QPushButton	* buttonCopy;
	tableAnalysis	* table [ MAX_TABS ];
};

class tableAnalysis : public QTableWidget
{
	Q_OBJECT

public:
	tableAnalysis	(
			CedSheet	* sheetData,
			char	const	* pathAnalysed,
			MainWindow	* mainwin,
			QVBoxLayout	* layout
			);
	~tableAnalysis	();

	void		copyToClipboard ();

private slots:
	void		tableCellActivated ( int row, int column );
	void		tableHeaderClick ( int index );

private:
	void		sortTable ( int column, Qt::SortOrder direction );
				// -column = Flip or descending


	Qt::SortOrder	sortDirection;

	int		sortColumn;
	int		sheetRowTotal;
	char		* path;
	CedSheet	* sheet;
	MainWindow	* mainWindow;
};

