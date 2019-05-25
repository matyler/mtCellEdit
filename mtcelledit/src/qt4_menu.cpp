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

#include "qt4.h"



void MainWindow::createMenus ()
{

/// FILE

	QAction * act_FileNew;
	QAction * act_FileOpen;
	QAction * act_FileImport;
	QAction * act_FileSave;
	QAction * act_FileSaveAs;

QEX_MENU ( FileNew, "New", "Ctrl+N", "document-new" )
QEX_MENU ( FileOpen, "Open ...", "Ctrl+O", "document-open" )
QEX_MENU ( FileImport, "Import", NULL, NULL )
QEX_MENU ( FileSave, "Save", "Ctrl+S", "document-save" )
QEX_MENU ( FileSaveAs, "Save As ...", "Shift+Ctrl+S", "document-save-as" )
QEX_MENU ( FileQuit, "Quit", "Ctrl+Q", "application-exit" )


	QSignalMapper * signalMapper = new QSignalMapper ( this );


	for ( int i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		act_FileRecent[i] = new QAction ( "", this );

		connect ( act_FileRecent[i], SIGNAL ( triggered () ),
			signalMapper, SLOT ( map () ) );
		signalMapper->setMapping ( act_FileRecent [ i ], i + 1 );

		if ( i < 10 )
		{
			act_FileRecent[i]->setShortcut (
				QString ( "Ctrl+%1" ).arg ( (i + 1) % 10 ) );
		}
	}

	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( press_FileRecent ( int ) ) );


	QMenu * menuFile = menuBar->addMenu ( "&File" );
	menuFile->setTearOffEnabled ( true );
	menuFile->addAction ( act_FileNew );
	menuFile->addAction ( act_FileOpen );
	menuFile->addSeparator ();
	menuFile->addAction ( act_FileImport );
	menuFile->addSeparator ();
	menuFile->addAction ( act_FileSave );
	menuFile->addAction ( act_FileSaveAs );
	act_FileRecentSeparator = menuFile->addSeparator ();

	for ( int i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		menuFile->addAction ( act_FileRecent[i] );
	}

	menuFile->addSeparator ();
	menuFile->addAction ( act_FileQuit );

/// EDIT

	QAction * act_EditCut;
	QAction * act_EditCopy;
	QAction * act_EditCopyVal;
	QAction * act_EditCopyOutput;
	QAction * act_EditTransformTrans;
	QAction * act_EditTransformFlipH;
	QAction * act_EditTransformFlipV;
	QAction * act_EditTransformRotClock;
	QAction * act_EditTransformRotAnti;
	QAction * act_EditPaste;
	QAction * act_EditPasteContent;
	QAction * act_EditPastePrefs;
	QAction * act_EditClear;
	QAction * act_EditClearContent;
	QAction * act_EditClearPrefs;
	QAction * act_EditFixYears;
	QAction * act_EditSelectAll;

QEX_MENU ( EditUndo, "Undo", "Ctrl+Z", "edit-undo" )
QEX_MENU ( EditRedo, "Redo", "Ctrl+Y", "edit-redo" )
QEX_MENU ( EditCut, "Cut", "Ctrl+X", "edit-cut" )
QEX_MENU ( EditCopy, "Copy", "Ctrl+C", "edit-copy" )
QEX_MENU ( EditCopyVal, "Copy As Values", "Shift+Ctrl+C", NULL )
QEX_MENU ( EditCopyOutput, "Copy As Output", NULL, NULL )
QEX_MENU ( EditTransformTrans, "Transpose", NULL, NULL )
QEX_MENU ( EditTransformFlipH, "Flip Horizontally", NULL, NULL )
QEX_MENU ( EditTransformFlipV, "Flip Vertically", NULL, NULL )
QEX_MENU ( EditTransformRotClock, "Rotate Clockwise", NULL, NULL )
QEX_MENU ( EditTransformRotAnti, "Rotate Anticlockwise", NULL, NULL )
QEX_MENU ( EditUseSystemClipboard, "Use System Clipboard", NULL, NULL )
QEX_MENU ( EditPaste, "Paste", "Ctrl+V", "edit-paste" )
QEX_MENU ( EditPasteContent, "Paste Content", "Ctrl+F7",	NULL )
QEX_MENU ( EditPastePrefs, "Paste Preferences", "F7", NULL )
QEX_MENU ( EditClear, "Clear", "Delete", "edit-clear" )
QEX_MENU ( EditClearContent, "Clear Content", "Backspace", NULL )
QEX_MENU ( EditClearPrefs, "Clear Preferences", "Ctrl+Backspace", NULL )
QEX_MENU ( EditFixYears, "Fix 2-digit Years", NULL, NULL )
QEX_MENU ( EditSelectAll, "Select All", "Ctrl+A", "edit-select-all" )

	QMenu * menuEdit = menuBar->addMenu ( "&Edit" );
	menuEdit->setTearOffEnabled ( true );

	menuEdit->addAction ( act_EditUndo );
	menuEdit->addAction ( act_EditRedo );
	menuEdit->addSeparator ();
	menuEdit->addAction ( act_EditCut );
	menuEdit->addAction ( act_EditCopy );
	menuEdit->addAction ( act_EditCopyVal );
	menuEdit->addAction ( act_EditCopyOutput );
	menuEdit->addSeparator ();

	QMenu * menuEditTransform = menuEdit->addMenu ( "Transform Clipboard" );
	menuEditTransform->setTearOffEnabled ( true );
	menuEditTransform->addAction ( act_EditTransformTrans );

	menuEditTransform->addAction ( act_EditTransformFlipH );
	menuEditTransform->addAction ( act_EditTransformFlipV );
	menuEditTransform->addAction ( act_EditTransformRotClock );
	menuEditTransform->addAction ( act_EditTransformRotAnti );

	menuEdit->addAction ( act_EditUseSystemClipboard );

	menuEdit->addSeparator ();
	menuEdit->addAction ( act_EditPaste );
	menuEdit->addAction ( act_EditPasteContent );
	menuEdit->addAction ( act_EditPastePrefs );
	menuEdit->addSeparator ();
	menuEdit->addAction ( act_EditClear );
	menuEdit->addAction ( act_EditClearContent );
	menuEdit->addAction ( act_EditClearPrefs );
	menuEdit->addSeparator ();
	menuEdit->addAction ( act_EditFixYears );
	menuEdit->addAction ( act_EditSelectAll );

	act_EditUseSystemClipboard->setCheckable ( true );
	act_EditUseSystemClipboard->setChecked ( pprfs->getInt (
		GUI_INIFILE_CLIPBOARD_USE_SYSTEM ) == 0 ? false : true );


/// SHEET

	QAction * act_SheetNew;
	QAction * act_SheetDuplicate;
	QAction * act_SheetRename;
	QAction * act_SheetDelete;
	QAction * act_SheetExport;
	QAction * act_SheetExportOutput;
	QAction * act_SheetPrevious;
	QAction * act_SheetNext;
	QAction * act_SheetRecalcBook;
	QAction * act_SheetRecalc;

QEX_MENU ( SheetNew, "New", "Ctrl+T", "document-new" )
QEX_MENU ( SheetDuplicate, "Duplicate", "Shift+Ctrl+T", NULL )
QEX_MENU ( SheetRename, "Rename ...", "Ctrl+F2", NULL )
QEX_MENU ( SheetDelete, "Delete", "Shift+Ctrl+W", "edit-delete" )
QEX_MENU ( SheetExport, "Export ...", NULL, NULL )
QEX_MENU ( SheetExportOutput, "Export Output ...", NULL, NULL )
QEX_MENU ( SheetFreezePanes, "Freeze Panes", NULL, NULL )
QEX_MENU ( SheetLock, "Lock", "F12", NULL )
QEX_MENU ( SheetPrevious, "Previous", "Ctrl+Page Up", NULL )
QEX_MENU ( SheetNext, "Next", "Ctrl+Page Down", NULL )
QEX_MENU ( SheetRecalcBook, "Recalculate Book", "F4", NULL )
QEX_MENU ( SheetRecalc, "Recalculate", "F5", "view-refresh" )

	QMenu * menuSheet = menuBar->addMenu ( "&Sheet" );
	menuSheet->setTearOffEnabled ( true );
	menuSheet->addAction ( act_SheetNew );
	menuSheet->addAction ( act_SheetDuplicate );
	menuSheet->addSeparator ();
	menuSheet->addAction ( act_SheetRename );
	menuSheet->addAction ( act_SheetDelete );
	menuSheet->addSeparator ();
	menuSheet->addAction ( act_SheetExport );
	menuSheet->addAction ( act_SheetExportOutput );
	menuSheet->addSeparator ();
	menuSheet->addAction ( act_SheetFreezePanes );
	menuSheet->addAction ( act_SheetLock );
	menuSheet->addSeparator ();
	menuSheet->addAction ( act_SheetPrevious );
	menuSheet->addAction ( act_SheetNext );
	menuSheet->addSeparator ();
	menuSheet->addAction ( act_SheetRecalcBook );
	menuSheet->addAction ( act_SheetRecalc );

/// ROW

	QAction * act_RowInsert;
	QAction * act_RowInsertPasteHeight;
	QAction * act_RowDelete;
	QAction * act_RowSort;

QEX_MENU ( RowInsert, "Insert", "Shift+Insert",	"list-add" )
QEX_MENU ( RowInsertPasteHeight, "Insert Paste Height", NULL, NULL )
QEX_MENU ( RowDelete, "Delete", "Shift+Delete", "list-remove" )
QEX_MENU ( RowSort, "Sort ...", NULL, "view-sort-ascending" )

	QMenu * menuRow = menuBar->addMenu ( "&Row" );
	menuRow->setTearOffEnabled ( true );
	menuRow->addAction ( act_RowInsert );
	menuRow->addAction ( act_RowInsertPasteHeight );
	menuRow->addSeparator ();
	menuRow->addAction ( act_RowDelete );
	menuRow->addSeparator ();
	menuRow->addAction ( act_RowSort );

/// COLUMN

	QAction * act_ColumnInsert;
	QAction * act_ColumnInsertPasteWidth;
	QAction * act_ColumnDelete;
	QAction * act_ColumnSort;
	QAction * act_ColumnSetWidth;
	QAction * act_ColumnSetWidthAuto;

QEX_MENU ( ColumnInsert, "Insert", "Ctrl+Insert", "list-add" )
QEX_MENU ( ColumnInsertPasteWidth, "Insert Paste Width", NULL, NULL )
QEX_MENU ( ColumnDelete, "Delete", "Ctrl+Delete", "list-remove" )
QEX_MENU ( ColumnSort, "Sort ...", NULL, "view-sort-ascending" )
QEX_MENU ( ColumnSetWidth, "Set Width ...", "F6", NULL )
QEX_MENU ( ColumnSetWidthAuto, "Set Width Automatically", "Ctrl+F6", NULL )

	QMenu * menuColumn = menuBar->addMenu ( "&Column" );
	menuColumn->setTearOffEnabled ( true );
	menuColumn->addAction ( act_ColumnInsert );
	menuColumn->addAction ( act_ColumnInsertPasteWidth );
	menuColumn->addSeparator ();
	menuColumn->addAction ( act_ColumnDelete );
	menuColumn->addSeparator ();
	menuColumn->addAction ( act_ColumnSort );
	menuColumn->addSeparator ();
	menuColumn->addAction ( act_ColumnSetWidth );
	menuColumn->addAction ( act_ColumnSetWidthAuto );

/// OPTIONS

	QAction * act_OptionsFullScreen;
	QAction * act_OptionsFind;
	QAction * act_OptionsGraph;
	QAction * act_OptionsView;
	QAction * act_OptionsEditCell;
	QAction * act_OptionsCellPrefs;
	QAction * act_OptionsBookPrefs;
	QAction * act_OptionsProgramPrefs;
	QAction * act_OptionsTextStyle_none;
	QAction * act_OptionsTextStyle_bold;
	QAction * act_OptionsTextStyle_italic;
	QAction * act_OptionsTextStyle_underline;
	QAction * act_OptionsTextStyle_underline_double;
	QAction * act_OptionsTextStyle_underline_wavy;
	QAction * act_OptionsTextStyle_strikethrough;
	QAction * act_OptionsBackgroundColor;
	QAction * act_OptionsForegroundColor;
	QAction * act_OptionsBorderColor;
	QAction * act_OptionsBorder [ BORDER_MENU_TOTAL ];

QEX_MENU ( OptionsFullScreen, "Full Screen", "F11", "view-fullscreen" )
QEX_MENU ( OptionsFind, "Find ...", "Ctrl+F", "edit-find" )
QEX_MENU ( OptionsGraph, "Graph ...", "Ctrl+G", NULL )
QEX_MENU ( OptionsView, "View ...", NULL, NULL )
QEX_MENU ( OptionsEditCell, "Edit Cell", "F2", NULL )
QEX_MENU ( OptionsCellPrefs, "Cell Preferences ...", "F3", NULL )
QEX_MENU ( OptionsBookPrefs, "Book Preferences ...", "Ctrl+F3", NULL )
QEX_MENU ( OptionsProgramPrefs,	"Program Preferences ...", "Ctrl+P", "preferences-other" )
QEX_MENU ( OptionsBackgroundColor, "Background Colour ...", "F8", NULL )
QEX_MENU ( OptionsForegroundColor, "Foreground Colour ...", "F9", NULL )
QEX_MENU ( OptionsBorderColor, "Border Colour ...", NULL, NULL )


typedef struct
{
	char	const	* text;
	int		num;
} menuPair;


	menuPair	const bp [ BORDER_MENU_TOTAL ] = {
			{ "None",		CUI_CELLBORD_NONE },
			{ "Thin Outside",	CUI_CELLBORD_OUT_THIN },
			{ "Thick Outside",	CUI_CELLBORD_OUT_THICK },
			{ "Double Outside",	CUI_CELLBORD_OUT_DOUBLE },
			{ "Thin Top && Bottom",	CUI_CELLBORD_TB_THIN },
			{ "Thick Top && Bottom",CUI_CELLBORD_TB_THICK },
			{ "Double Top && Bottom",CUI_CELLBORD_TB_DOUBLE },

			{ "Clear Top",		CUI_CELLBORD_H_CLEAR_TOP },
			{ "Clear Middle",	CUI_CELLBORD_H_CLEAR_MID },
			{ "Clear Bottom",	CUI_CELLBORD_H_CLEAR_BOT },
			{ "Thin Top",		CUI_CELLBORD_H_THIN_TOP },
			{ "Thin Middle",	CUI_CELLBORD_H_THIN_MID },
			{ "Thin Bottom",	CUI_CELLBORD_H_THIN_BOT },
			{ "Thick Top",		CUI_CELLBORD_H_THICK_TOP },
			{ "Thick Middle",	CUI_CELLBORD_H_THICK_MID },
			{ "Thick Bottom",	CUI_CELLBORD_H_THICK_BOT },
			{ "Double Top",		CUI_CELLBORD_H_DOUB_TOP },
			{ "Double Middle",	CUI_CELLBORD_H_DOUB_MID },
			{ "Double Bottom",	CUI_CELLBORD_H_DOUB_BOT },

			{ "Clear Left",		CUI_CELLBORD_V_CLEAR_L },
			{ "Clear Centre",	CUI_CELLBORD_V_CLEAR_C },
			{ "Clear Right",	CUI_CELLBORD_V_CLEAR_R },
			{ "Thin Left",		CUI_CELLBORD_V_THIN_L },
			{ "Thin Centre",	CUI_CELLBORD_V_THIN_C },
			{ "Thin Right",		CUI_CELLBORD_V_THIN_R },
			{ "Thick Left",		CUI_CELLBORD_V_THICK_L },
			{ "Thick Centre",	CUI_CELLBORD_V_THICK_C },
			{ "Thick Right",	CUI_CELLBORD_V_THICK_R },
			{ "Double Left",	CUI_CELLBORD_V_DOUB_L },
			{ "Double Centre",	CUI_CELLBORD_V_DOUB_C },
			{ "Double Right",	CUI_CELLBORD_V_DOUB_R }
			};


	signalMapper = new QSignalMapper ( this );

	for ( int i = 0; i < BORDER_MENU_TOTAL; i++ )
	{
		act_OptionsBorder[i] = new QAction ( bp[i].text, this );

		connect ( act_OptionsBorder[i], SIGNAL ( triggered () ),
			signalMapper, SLOT ( map () ) );
		signalMapper->setMapping ( act_OptionsBorder[i], bp[i].num );
	}

	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( press_OptionsBorder ( int ) ) );



#define MENTS( A, B, C, D, E ) \
	act_OptionsTextStyle_ ## A = mtQEX::menu_init ( this, B, C, D ); \
	connect ( act_OptionsTextStyle_ ## A, SIGNAL ( triggered () ), \
		signalMapper, SLOT ( map () ) ); \
	signalMapper->setMapping ( act_OptionsTextStyle_ ## A, E );



	signalMapper = new QSignalMapper ( this );

MENTS ( none, "None", NULL, NULL, CED_TEXT_STYLE_CLEAR )
MENTS ( bold, "Bold", "Ctrl+B",	"format-text-bold", CED_TEXT_STYLE_BOLD )
MENTS ( italic, "Italic", "Ctrl+I", "format-text-italic", CED_TEXT_STYLE_ITALIC )
MENTS ( underline, "Underline", "Ctrl+U", "format-text-underline", CED_TEXT_STYLE_UNDERLINE_SINGLE )
MENTS ( underline_double, "Underline Double", NULL, NULL, CED_TEXT_STYLE_UNDERLINE_DOUBLE )
MENTS ( underline_wavy, "Underline Wavy", NULL, NULL, CED_TEXT_STYLE_UNDERLINE_WAVY )
MENTS ( strikethrough,	"Strikethrough", NULL, NULL, CED_TEXT_STYLE_STRIKETHROUGH )

	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( press_OptionsTextStyle ( int ) ) );


	QMenu * menuOptions = menuBar->addMenu ( "&Options" );
	menuOptions->setTearOffEnabled ( true );
	menuOptions->addAction ( act_OptionsFullScreen );
	menuOptions->addAction ( act_OptionsFind );
	menuOptions->addAction ( act_OptionsGraph );
	menuOptions->addAction ( act_OptionsView );
	menuOptions->addSeparator ();
	menuOptions->addAction ( act_OptionsEditCell );
	menuOptions->addAction ( act_OptionsCellPrefs );
	menuOptions->addAction ( act_OptionsBookPrefs );
	menuOptions->addAction ( act_OptionsProgramPrefs );
	menuOptions->addSeparator ();

	QMenu * menuOptionTextStyle = menuOptions->addMenu ( "Text Style" );
	menuOptionTextStyle->setTearOffEnabled ( true );
	menuOptionTextStyle->addAction ( act_OptionsTextStyle_none );
	menuOptionTextStyle->addSeparator ();
	menuOptionTextStyle->addAction ( act_OptionsTextStyle_bold );
	menuOptionTextStyle->addAction ( act_OptionsTextStyle_italic );
	menuOptionTextStyle->addAction ( act_OptionsTextStyle_underline );
	menuOptionTextStyle->addAction ( act_OptionsTextStyle_underline_double );
	menuOptionTextStyle->addAction ( act_OptionsTextStyle_underline_wavy );
	menuOptionTextStyle->addAction ( act_OptionsTextStyle_strikethrough );

	menuOptions->addAction ( act_OptionsBackgroundColor );
	menuOptions->addAction ( act_OptionsForegroundColor );
	menuOptions->addAction ( act_OptionsBorderColor );

	QMenu * menuOptionBorderType = menuOptions->addMenu ( "Border Type" );
	menuOptionBorderType->setTearOffEnabled ( true );
	menuOptionBorderType->addAction ( act_OptionsBorder [ 0 ] );
	menuOptionBorderType->addSeparator ();
	menuOptionBorderType->addAction ( act_OptionsBorder [ 1 ] );
	menuOptionBorderType->addAction ( act_OptionsBorder [ 2 ] );
	menuOptionBorderType->addAction ( act_OptionsBorder [ 3 ] );
	menuOptionBorderType->addSeparator ();
	menuOptionBorderType->addAction ( act_OptionsBorder [ 4 ] );
	menuOptionBorderType->addAction ( act_OptionsBorder [ 5 ] );
	menuOptionBorderType->addAction ( act_OptionsBorder [ 6 ] );
	menuOptionBorderType->addSeparator ();

	QMenu * menuBorderTypeH = menuOptionBorderType->addMenu ("Horizontal" );
	menuBorderTypeH->setTearOffEnabled ( true );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 7 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 8 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 9 ] );
	menuBorderTypeH->addSeparator ();
	menuBorderTypeH->addAction ( act_OptionsBorder [ 10 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 11 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 12 ] );
	menuBorderTypeH->addSeparator ();
	menuBorderTypeH->addAction ( act_OptionsBorder [ 13 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 14 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 15 ] );
	menuBorderTypeH->addSeparator ();
	menuBorderTypeH->addAction ( act_OptionsBorder [ 16 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 17 ] );
	menuBorderTypeH->addAction ( act_OptionsBorder [ 18 ] );

	QMenu * menuBorderTypeV = menuOptionBorderType->addMenu ( "Vertical" );
	menuBorderTypeV->setTearOffEnabled ( true );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 19 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 20 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 21 ] );
	menuBorderTypeV->addSeparator ();
	menuBorderTypeV->addAction ( act_OptionsBorder [ 22 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 23 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 24 ] );
	menuBorderTypeV->addSeparator ();
	menuBorderTypeV->addAction ( act_OptionsBorder [ 25 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 26 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 27 ] );
	menuBorderTypeV->addSeparator ();
	menuBorderTypeV->addAction ( act_OptionsBorder [ 28 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 29 ] );
	menuBorderTypeV->addAction ( act_OptionsBorder [ 30 ] );

/// HELP

	QAction * act_HelpHelp;
	QAction * act_HelpAboutQt;
	QAction * act_HelpAbout;

QEX_MENU ( HelpHelp, "Help ...", "F1", "help-contents" )
QEX_MENU ( HelpAboutQt, "About Qt ...", NULL, NULL )
QEX_MENU ( HelpAbout, "About ...", NULL, "help-about" )

	QMenu * menuHelp = menuBar->addMenu ( "&Help" );
	menuHelp->setTearOffEnabled ( true );
	menuHelp->addAction ( act_HelpHelp );
	menuHelp->addAction ( act_HelpAboutQt );
	menuHelp->addAction ( act_HelpAbout );

/// FIND

QEX_MENU ( FindWildcards, "? * Wildcards", NULL, NULL )
QEX_MENU ( FindCase, "Case Sensitive", NULL, NULL )
QEX_MENU ( FindValue, "Match Value", NULL, NULL )
QEX_MENU ( FindSheets, "All Sheets", NULL, NULL )

	act_FindWildcards->setCheckable ( true );
	act_FindWildcards->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_WILDCARDS ) == 0 ? false : true );

	act_FindCase->setCheckable ( true );
	act_FindCase->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_CASE_SENSITIVE ) == 0 ? false : true );

	act_FindValue->setCheckable ( true );
	act_FindValue->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_VALUE ) == 0 ? false : true );

	act_FindSheets->setCheckable ( true );
	act_FindSheets->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_ALL_SHEETS ) == 0 ? false : true );

	QMenu * menuFind = findMenuBar->addMenu ( "Options" );
	menuFind->addAction ( act_FindWildcards );
	menuFind->addAction ( act_FindCase );
	menuFind->addAction ( act_FindValue );
	menuFind->addAction ( act_FindSheets );

/// GRAPH

	QAction * act_GraphNew;

QEX_MENU ( GraphNew, "New", NULL, "document-new" )
QEX_MENU ( GraphDuplicate, "Duplicate", NULL, NULL )
QEX_MENU ( GraphRename, "Rename ...", NULL, NULL )
QEX_MENU ( GraphDelete, "Delete ...", NULL, "edit-delete" )
QEX_MENU ( GraphRedraw, "Redraw", "Ctrl+F5", "view-refresh" )
QEX_MENU ( GraphExport, "Export ...", NULL, NULL )
QEX_MENU ( GraphSClipboard, "Sheet Selection to Clipboard", NULL, NULL )

	QMenu * menuGraph = graphMenuBar->addMenu ( "&Graph" );
	menuGraph->setTearOffEnabled ( true );
	menuGraph->addAction ( act_GraphNew );
	menuGraph->addAction ( act_GraphDuplicate );
	menuGraph->addSeparator ();
	menuGraph->addAction ( act_GraphRename );
	menuGraph->addAction ( act_GraphDelete );
	menuGraph->addSeparator ();
	menuGraph->addAction ( act_GraphRedraw );
	menuGraph->addAction ( act_GraphExport );
	menuGraph->addAction ( act_GraphSClipboard );
}

