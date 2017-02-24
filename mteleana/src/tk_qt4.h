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



enum
{
	UI_TAB_MAP,
	UI_TAB_CARTOGRAM,
	UI_TAB_DIAGRAM,
	UI_TAB_FIND
};



class	MainWindow;
class	MapView;
class	MapCanvas;



extern MainWindow	* mainwindow;



namespace mtEleana
{
	void tableSetupDetails ( QTableWidget * table );
}



class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow ();
	~MainWindow ();

	void		polymapScroll ( int dx, int dy );
	cairo_t		* prepRender ( int px, int py, int pw, int ph );
	void		wheelZoom ( int xx, int yy, int delta );
	void		clickMap ( int xx, int yy );
	void		resetMapSize ();
	void		addFindRow (
				char const * seat_text,
				char const * cell_type,
				char const * cell_text,
				int seat_id
				);

private slots:
	void		pressFileSaveMap ();
	void		pressFileQuit ();

	void		pressEditFind ();

	void		pressHelpAboutQt ();
	void		pressHelpAbout ();

	void		pressFindButton ();
	void		findCellChanged (
				int currentRow,
				int currentColumn,
				int previousRow,
				int previousColumn
				);

	void		yearChanged ( int val );

	void		mapModeChanged ( int i );
	void		diagramLeftChanged ( int i );
	void		diagramRightChanged ( int i );

	void		zoomCartogramChanged ( int zoom );
	void		zoomDiagramChanged ( int zoom );
	void		zoomMapChanged ( int zoom );

private:
	void		createMenus ();
	void		menuInit (
				QAction ** action,
				char const * text,
				char const * shortcut,
				char const * icon
				);

	void		clearSeatTable ();
	void		setupSeatTable ( int row );

	void		mapRedraw ();	// Redraw map & cartogram
	void		moveMapFocus ( int row );
	void		diagramRedraw ();

	int		projectLoad ( char const * filename );

	void		purgeIndexLoaded ();
	void		purgeDataLoaded ();

	void		resetMapZoomPos ();

// -----------------------------------------------------------------------------

	QMenuBar	* menuBar;
	QComboBox	* comboYear;
	QLabel		* labelSeat;
	QTableWidget	* tableSeat;
	QTableWidget	* tableSummary;

	QTabWidget	* tabWidget;

	QComboBox	* comboMapMode;
	QSlider		* sliderMapZoom;
	MapView		* polymapScrollArea;
	MapCanvas	* mapCanvas;

	mtQEX::Image	* cartogramWidget;

	mtQEX::Image	* diagramWidget;
	QComboBox	* comboDiagramLeft;
	QComboBox	* comboDiagramRight;

	QLineEdit	* editFindText;
	QTableWidget	* findTable;

	double		zoomMap;
	eleanaIndex	* index;
	eleanaElection	* election;

	mtKit::Prefs	prefs;
};

class MapView : public QScrollArea
{
	Q_OBJECT

private:
	void		resizeEvent ( QResizeEvent * ev );
};

class MapCanvas : public QWidget
{
	Q_OBJECT

public:
	MapCanvas ();

private:
	void		paintEvent ( QPaintEvent * ev );
	void		mousePressEvent ( QMouseEvent * ev );
	void		mouseMoveEvent ( QMouseEvent * ev );
	void		mouseEventRouter ( QMouseEvent * ev, int caller );
	void		wheelEvent ( QWheelEvent * ev );

// -----------------------------------------------------------------------------

	int		oldMouseX;
	int		oldMouseY;
};

