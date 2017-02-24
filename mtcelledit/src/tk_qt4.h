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
class	CedView;
class	CedViewArea;
class	MyLineEdit;
class	SortDialog;
class	SwatchDialog;



enum
{
	VIEW_ROW_HEADER		= 0,
	VIEW_ROW_PANE_TOP	= 1,
	VIEW_ROW_FREEZE		= 2,
	VIEW_ROW_PANE_BOTTOM	= 3,
	VIEW_ROW_SCROLL		= 4,

	VIEW_ROW_TOTAL		= 5
};

enum
{
	VIEW_COLUMN_HEADER	= 0,
	VIEW_COLUMN_PANE_LEFT	= 1,
	VIEW_COLUMN_FREEZE	= 2,
	VIEW_COLUMN_PANE_RIGHT	= 3,
	VIEW_COLUMN_SCROLL	= 4,

	VIEW_COLUMN_TOTAL	= 5
};

enum
{
	TAB_FIND		= 0,
	TAB_GRAPH		= 1,
	TAB_VIEW		= 2
};



#define BORDER_MENU_TOTAL	31



extern MainWindow	* mainwindow;



class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow ( Backend * be, QApplication &app );
	~MainWindow ();

	int projectSetFont (
		char	const * name,
		int		sz
		);
		// 0 = success
		// 1 = failure with message box presented to user

	void projectGetSheetGeometry ( int * r, int * c );
	CedSheet * projectGetSheet ();	// Get from render
	CedSheet * projectSetSheet ();	// Get from book, set render
	CuiFile * projectGetCedFile ();
	CuiRender * projectGetRender ();
	int projectReportUpdates ( int error );

	void insertCellText ( char const * text );

	int addFindRow (
		CedSheet * sheet,
		CedCell	const * cell,
		int r,
		int c
		);

	int isEditingCellText ();
	int isMainView ( CedView * v );

	void setCursorRange (
		int r1,
		int c1,
		int r2,
		int c2,
		int cursor_visible,
		int force_update,
		int follow_all		// Force cursor visibility everywhere
		);

	void setChangesFlag ();

	void updateChangesChores (
		int new_geometry,
		int block_sheet_recalcs
		);

	void updateGraph ( char const * graph_name );
	void updateRecentFiles ();
	void updateView ();
	void updateViewConfig ();

public slots:
	void pressGraphRedraw ();

protected:
	void closeEvent ( QCloseEvent * ev );
	void keyPressEvent ( QKeyEvent * ev );

private slots:
	void graphChanged ( int i );
	void findCellChanged (
		int currentRow,
		int currentColumn,
		int previousRow,
		int previousColumn
		);
	void quicksumChanged ( int i );
	void sheetChanged ( int i );

	void pressUpdateCellRef ();
	void pressStopCellRef ();
	void pressTabCellRef ( int t );
	void pressFocusOutCellRef ();
	void pressUpdateCellText ();
	void pressStopCellText ();
	void pressTabCellText ( int t );
	void pressArrowCellText ( int r );

	void pressFileNew ();
	void pressFileOpen ();
	void pressFileImport ();
	void pressFileSave ();
	void pressFileSaveAs ();
	void pressFileRecent ( int i );
	void pressFileQuit ();

	void pressEditUndo ();
	void pressEditRedo ();
	void pressEditCut ();
	void pressEditCopy ();
	void pressEditCopyVal ();
	void pressEditCopyOutput ();
	void pressEditTransformTrans ();
	void pressEditTransformFlipH ();
	void pressEditTransformFlipV ();
	void pressEditTransformRotClock ();
	void pressEditTransformRotAnti ();
	void pressEditUseSystemClipboard ();
	void pressEditPaste ();
	void pressEditPasteContent ();
	void pressEditPastePrefs ();
	void pressEditClear ();
	void pressEditClearContent ();
	void pressEditClearPrefs ();
	void pressEditFixYears ();
	void pressEditSelectAll ();

	void pressSheetNew ();
	void pressSheetDuplicate ();
	void pressSheetRename ();
	void pressSheetDelete ();
	void pressSheetExport ();
	void pressSheetExportOutput ();
	void pressSheetFreezePanes ();
	void pressSheetLock ();
	void pressSheetPrevious ();
	void pressSheetNext ();
	void pressSheetRecalcBook ();
	void pressSheetRecalc ();

	void pressRowInsert ();
	void pressRowInsertPasteHeight ();
	void pressRowDelete ();
	void pressRowSort ();

	void pressColumnInsert ();
	void pressColumnInsertPasteWidth ();
	void pressColumnDelete ();
	void pressColumnSort ();
	void pressColumnSetWidth ();
	void pressColumnSetWidthAuto ();

	void pressOptionsFullScreen ();
	void pressOptionsFind ();
	void pressOptionsGraph ();
	void pressOptionsView ();
	void pressOptionsEditCell ();
	void pressOptionsCellPrefs ();
	void pressOptionsBookPrefs ();
	void pressOptionsProgramPrefs ();
	void pressOptionsTextStyle ( int i );
	void pressOptionsBackgroundColor ();
	void pressOptionsForegroundColor ();
	void pressOptionsBorderColor ();
	void pressOptionsBorder ( int i );
	void pressOptionsHelp ();
	void pressOptionsAboutQt ();
	void pressOptionsAbout ();

	void pressFind ();
	void pressFindWildcards ();
	void pressFindCase ();
	void pressFindValue ();
	void pressFindSheets ();

	void pressGraphNew ();
	void pressGraphDuplicate ();
	void pressGraphRename ();
	void pressGraphDelete ();
	void pressGraphExport ();
	void pressGraphSClipboard ();

private:
	void coreRecalcBook ();
	void coreRecalcSheet ();

	int clipboardClearSelection ( int mode );
	void clipboardCopyRouter ( int mode );
	int clipboardCopySelection ( int mode );
	void clipboardFlushInternal ();
	int clipboardGetMtcelledit ( QClipboard * clipboard );
	int clipboardObtainPaste ();
	int clipboardPasteAtCursor ( int mode );
	int clipboardReadSystem ();
	void clipboardSetOwner ();
	void clipboardTransform ( int mode );

	void createMenus ();
	void createQuicksum ();

	QStringList getFileExportTypes ();
	int getSelectionPosition (	// Get position & obtain paste
		CedSheet * sheet,	// All args optional
		int * row,
		int * col
		);
		// 0 = Success

	void menuInit (
		QAction ** action,
		char const * text,
		char const * shortcut,
		char const * icon
		);

	int okToLoseChanges ();
			// 0 = Changes and user doesn't consent to losing them
			// 1 = No changes or user consents to losing changes

	void projectClearAll ();
	void projectGraphStoreChanges ();
	void projectGraphRedraw ();

			// 0 = success
			// 1 = failure with message box presented to user
	int projectLoad ( char const * filename );
	int projectSave ( char const * filename, int format );
	int projectImport ( char const * filename );

	void projectRegister ();
	int projectRenameSheet ( CedSheet * sheet, QString new_name );
	int projectRenameGraph ( QString newName );
	void projectSetSheetGeometry ();

	void reportLargeTSV ();

	void setCellFromInput ( int rowd, int cold );

	void updateEntryCellref ();
	void updateEntryCelltext ();
	void updateMainArea ();
	void updateMenus ();
	void updateQuicksumLabel ();
	void updateRecalcBook ();
	void updateRender ();
	void updateSheetSelector ();
	void updateTitleBar ();

// -----------------------------------------------------------------------------

	Backend		* const backend;
	mtKit::Prefs	* const pprfs;

	QAction
			* actFileRecentSeparator,
			* actFileRecent [ RECENT_MENU_TOTAL ],
			* actFileQuit,

			* actEditUndo,
			* actEditRedo,

			* actSheetFreezePanes,
			* actSheetLock,

			* actEditUseSystemClipboard,

			* actFindWildcards,
			* actFindCase,
			* actFindValue,
			* actFindSheets,

			* actGraphDuplicate,
			* actGraphRename,
			* actGraphDelete,
			* actGraphRedraw,
			* actGraphExport,
			* actGraphSClipboard
			;

	QWidget		* rightSplit;
	QTabWidget	* tabWidget;
	QMenuBar
			* findMenuBar,
			* graphMenuBar,
			* menuBar
			;
	QTextEdit	* graphTextEdit;

	mtQEX::ButtonMenu * buttonGraph,
			* buttonQuicksum,
			* buttonSheet;

	QLineEdit	* editCellref,
			* editCelltext,
			* editFindText
			;
	QTableWidget	* findTable;
	QLabel		* labelQuicksum;

	mtQEX::Image	* graphWidget;

	CedView		* viewMain,
			* viewTab;

	CuiFile		* cedFile;
	CuiClip		* cedClipboard;
	CuiRender	crendr;

	int		sheetRows,
			sheetCols,
			memChanged,
			lastExportSheetType,
			lastExportGraphType
			;
};

class CedView : public QWidget
{
	Q_OBJECT

public:
	CedView		();

	void ensureVisible ( CedSheet * sheet, int row, int column );

	int getBell ();
	int getHScrollValue ();
	int getVScrollValue ();
	int getVisibleRows ( CuiRender * crender = NULL );
	int getPageColumnsLeft ( int column );
	int getPageColumnsRight ( int column );

	bool hasFocus ();

	void reconfigure ();	// Update pane view, scrollbars etc.

	void redrawArea ( // Update all visible areas of this sheet
		CedSheet * sheet,
		int nr1,
		int nc1,
		int nr2,
		int nc2
		);

	void updateGeometry ();
	void updateRedraw ();

	void setBell ();
	void setFocus ();
	void setScrollbars ();

private slots:
	void vscrollAction ( int action );
	void vscrollMove ( int value );

	void hscrollAction ( int action );
	void hscrollMove ( int value );

private:
	void setPanes ();
	void setRowHeaderWidth ();

	void wheelEvent ( QWheelEvent * ev );

///-----------------------------------------------------------------------------

	CedViewArea	* area [ CEDVIEW_AREA_TOTAL ];

	QGridLayout	* layout;

	QScrollBar	* hscroll,
			* vscroll;

	int		bellState[2],
			hscrollLast,
			vscrollLast
			;
};

class CedViewArea : public QWidget
{
	Q_OBJECT

public:
	CedViewArea ( CedView * cv, int id );

protected:
	bool event ( QEvent * ev );	// Catch tabs

private:
	void paintEvent ( QPaintEvent * ev );

	void mousePressEvent ( QMouseEvent * ev );
	void mouseMoveEvent ( QMouseEvent * ev );
	void mouseEventRouter ( QMouseEvent * ev, int caller );

///-----------------------------------------------------------------------------

	CedView		* cedview;
	int		areaID;
};

class MyLineEdit : public QLineEdit
{
	Q_OBJECT

signals:
	void pressUpdateKey ();		// Return or Enter keys
	void pressStopKey ();		// Escape
	void pressTabKey ( int t );	// Tab = 1 Ctrl+Tab = -1
	void pressArrowKey ( int r );	// Up = -1, down = 1
	void pressFocusOut ();

protected:
	bool event ( QEvent * ev );
	void focusOutEvent ( QFocusEvent * e );
};

class SortDialog : public QDialog
{
	Q_OBJECT

public:
	SortDialog (
		int axis,		// 0 = Rows, 1 = Columns
		QWidget * par = NULL
		);

private slots:
	void pressClose ();
	void pressMoveDown ();
	void pressMoveUp ();
	void pressSort ();

	void spinChanged ( int i );
	void directionChanged ( int i );
	void caseChanged ( int i );
	void tableRowClick (
		int currentRow,
		int currentColumn,
		int previousRow,
		int previousColumn
		);

private:

	void setRowValues ( int row );
	int getTableRow ();
	void swapRows ( int delta );
			// Swap table rows, update widget, cursor to new row

///-----------------------------------------------------------------------------

	QTableWidget	* tableWidget;
	QSpinBox	* spinBoxWidget;
	QCheckBox	* checkDirection;
	QCheckBox	* checkCase;
};

class SwatchDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SwatchDialog ( QString title, mtKit::Prefs * prefs );

	int getColor ();

private slots:
	void pressButton ( int i );
	void pressCancel ();

private:
	int		color;
};

