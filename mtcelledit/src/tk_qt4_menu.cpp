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

#include "tk_qt4.h"



void MainWindow::menuInit (
	QAction		** const action,
	char	const	* const	text,
	char	const	* const	shortcut,
	char	const	* const	icon
	)
{
	if ( icon )
	{
		action[0] = new QAction ( QIcon::fromTheme ( icon ), text,
			this );
		action[0]->setIconVisibleInMenu ( true );
	}
	else
	{
		action[0] = new QAction ( text, this );
	}

	if ( shortcut )
	{
		action[0]->setShortcut ( QString ( shortcut ) );
	}
}

void MainWindow::createMenus ()
{
	QAction
			* actFileNew,
			* actFileOpen,
			* actFileImport,
			* actFileSave,
			* actFileSaveAs,

			* actEditCut,
			* actEditCopy,
			* actEditCopyVal,
			* actEditCopyOutput,
			* actEditTransformTrans,
			* actEditTransformFlipH,
			* actEditTransformFlipV,
			* actEditTransformRotClock,
			* actEditTransformRotAnti,
			* actEditPaste,
			* actEditPasteContent,
			* actEditPastePrefs,
			* actEditClear,
			* actEditClearContent,
			* actEditClearPrefs,
			* actEditFixYears,
			* actEditSelectAll,

			* actSheetNew,
			* actSheetDuplicate,
			* actSheetRename,
			* actSheetDelete,
			* actSheetExport,
			* actSheetExportOutput,
			* actSheetPrevious,
			* actSheetNext,
			* actSheetRecalcBook,
			* actSheetRecalc,

			* actRowInsert,
			* actRowInsertPasteHeight,
			* actRowDelete,
			* actRowSort,

			* actColumnInsert,
			* actColumnInsertPasteWidth,
			* actColumnDelete,
			* actColumnSort,
			* actColumnSetWidth,
			* actColumnSetWidthAuto,

			* actOptionsFullScreen,
			* actOptionsFind,
			* actOptionsGraph,
			* actOptionsView,
			* actOptionsEditCell,
			* actOptionsCellPrefs,
			* actOptionsBookPrefs,
			* actOptionsProgramPrefs,

			* actOptionsTextStyle_none,
			* actOptionsTextStyle_bold,
			* actOptionsTextStyle_italic,
			* actOptionsTextStyle_underline,
			* actOptionsTextStyle_underline_double,
			* actOptionsTextStyle_underline_wavy,
			* actOptionsTextStyle_strikethrough,

			* actOptionsBackgroundColor,
			* actOptionsForegroundColor,
			* actOptionsBorderColor,
			* actOptionsBorder [ BORDER_MENU_TOTAL ],
			* actOptionsHelp,
			* actOptionsAboutQt,
			* actOptionsAbout,

			* actGraphNew
			;


#define MENFU( A, B, C, D ) \
	menuInit ( &act ## A , B, C, D ); \
	connect ( act ## A, SIGNAL ( triggered () ), this, \
		SLOT ( press ## A () ) );


MENFU ( FileNew,		"New",			"Ctrl+N",	"document-new" )
MENFU ( FileOpen,		"Open ...",		"Ctrl+O",	"document-open" )
MENFU ( FileImport,		"Import",		NULL,		NULL )
MENFU ( FileSave,		"Save",			"Ctrl+S",	"document-save" )
MENFU ( FileSaveAs,		"Save As ...",		"Shift+Ctrl+S",	"document-save-as" )
MENFU ( FileQuit,		"Quit",			"Ctrl+Q",	"application-exit" )

MENFU ( EditUndo,		"Undo",			"Ctrl+Z",	"edit-undo" )
MENFU ( EditRedo,		"Redo",			"Ctrl+Y",	"edit-redo" )
MENFU ( EditCut,		"Cut",			"Ctrl+X",	"edit-cut" )
MENFU ( EditCopy,		"Copy",			"Ctrl+C",	"edit-copy" )
MENFU ( EditCopyVal,		"Copy As Values",	"Shift+Ctrl+C",	NULL )
MENFU ( EditCopyOutput,		"Copy As Output",	NULL,		NULL )
MENFU ( EditTransformTrans,	"Transpose",		NULL,		NULL )
MENFU ( EditTransformFlipH,	"Flip Horizontally",	NULL,		NULL )
MENFU ( EditTransformFlipV,	"Flip Vertically",	NULL,		NULL )
MENFU ( EditTransformRotClock,	"Rotate Clockwise",	NULL,		NULL )
MENFU ( EditTransformRotAnti,	"Rotate Anticlockwise",	NULL,		NULL )
MENFU ( EditUseSystemClipboard,	"Use System Clipboard",	NULL,		NULL )
MENFU ( EditPaste,		"Paste",		"Ctrl+V",	"edit-paste" )
MENFU ( EditPasteContent,	"Paste Content",	"Ctrl+F7",	NULL )
MENFU ( EditPastePrefs,		"Paste Preferences",	"F7",		NULL )
MENFU ( EditClear,		"Clear",		"Delete",	"edit-clear" )
MENFU ( EditClearContent,	"Clear Content",	"Backspace",	NULL )
MENFU ( EditClearPrefs,		"Clear Preferences",	"Ctrl+Backspace", NULL )
MENFU ( EditFixYears,		"Fix 2-digit Years",	NULL,		NULL )
MENFU ( EditSelectAll,		"Select All",		"Ctrl+A",	"edit-select-all" )

MENFU ( SheetNew,		"New",			"Ctrl+T",	"document-new" )
MENFU ( SheetDuplicate,		"Duplicate",		"Shift+Ctrl+T",	NULL )
MENFU ( SheetRename,		"Rename ...",		"Ctrl+F2",	NULL )
MENFU ( SheetDelete,		"Delete",		"Shift+Ctrl+W",	"edit-delete" )
MENFU ( SheetExport,		"Export ...",		NULL,		NULL )
MENFU ( SheetExportOutput,	"Export Output ...",	NULL,		NULL )
MENFU ( SheetFreezePanes,	"Freeze Panes",		NULL,		NULL )
MENFU ( SheetLock,		"Lock",			"F12",		NULL )
MENFU ( SheetPrevious,		"Previous",		"Ctrl+Page Up",	NULL )
MENFU ( SheetNext,		"Next",			"Ctrl+Page Down", NULL )
MENFU ( SheetRecalcBook,	"Recalculate Book",	"F4",		NULL )
MENFU ( SheetRecalc,		"Recalculate",		"F5",		"view-refresh" )

MENFU ( RowInsert,		"Insert",			"Shift+Insert",	"list-add" )
MENFU ( RowInsertPasteHeight,	"Insert Paste Height",		NULL,		NULL )
MENFU ( RowDelete,		"Delete",			"Shift+Delete",	"list-remove" )
MENFU ( RowSort,		"Sort ...",			NULL,		"view-sort-ascending" )

MENFU ( ColumnInsert,		"Insert",			"Ctrl+Insert",	"list-add" )
MENFU ( ColumnInsertPasteWidth, "Insert Paste Width",		NULL,		NULL )
MENFU ( ColumnDelete,		"Delete",			"Ctrl+Delete",	"list-remove" )
MENFU ( ColumnSort,		"Sort ...",			NULL,		"view-sort-ascending" )
MENFU ( ColumnSetWidth,		"Set Width ...",		"F6",		NULL )
MENFU ( ColumnSetWidthAuto,	"Set Width Automatically",	"Ctrl+F6",	NULL )

MENFU ( OptionsFullScreen,	"Full Screen",			"F11",		"view-fullscreen" )
MENFU ( OptionsFind,		"Find ...",			"Ctrl+F",	"edit-find" )
MENFU ( OptionsGraph,		"Graph ...",			"Ctrl+G",	NULL )
MENFU ( OptionsView,		"View ...",			NULL,		NULL )
MENFU ( OptionsEditCell,	"Edit Cell",			"F2",		NULL )
MENFU ( OptionsCellPrefs,	"Cell Preferences ...",		"F3",		NULL )
MENFU ( OptionsBookPrefs,	"Book Preferences ...",		"Ctrl+F3",	NULL )
MENFU ( OptionsProgramPrefs,	"Program Preferences ...",	"Ctrl+P",	"preferences-other" )

MENFU ( OptionsBackgroundColor,	"Background Colour ...",	"F8",		NULL )
MENFU ( OptionsForegroundColor,	"Foreground Colour ...",	"F9",		NULL )
MENFU ( OptionsBorderColor,	"Border Colour ...",		NULL,		NULL )
MENFU ( OptionsHelp,		"Help ...",			"F1",		"help-contents" )
MENFU ( OptionsAboutQt,		"About Qt ...",			NULL,		NULL )
MENFU ( OptionsAbout,		"About ...",			NULL,		"help-about" )

MENFU ( FindWildcards,		"? * Wildcards",		NULL,		NULL )
MENFU ( FindCase,		"Case Sensitive",		NULL,		NULL )
MENFU ( FindValue,		"Match Value",			NULL,		NULL )
MENFU ( FindSheets,		"All Sheets",			NULL,		NULL )

MENFU ( GraphNew,		"New",				NULL,		"document-new" )
MENFU ( GraphDuplicate,		"Duplicate",			NULL,		NULL )
MENFU ( GraphRename,		"Rename ...",			NULL,		NULL )
MENFU ( GraphDelete,		"Delete ...",			NULL,		"edit-delete" )
MENFU ( GraphRedraw,		"Redraw",			"Ctrl+F5",	"view-refresh" )
MENFU ( GraphExport,		"Export ...",			NULL,		NULL )
MENFU ( GraphSClipboard,	"Sheet Selection to Clipboard",	NULL,		NULL )


	QSignalMapper * signalMapper = new QSignalMapper ( this );
	int		i;


	for ( i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		actFileRecent[i] = new QAction ( "", this );

		connect ( actFileRecent[i], SIGNAL ( triggered () ),
			signalMapper, SLOT ( map () ) );
		signalMapper->setMapping ( actFileRecent [ i ], i + 1 );

		if ( i < 10 )
		{
			actFileRecent[i]->setShortcut (
				QString ( "Ctrl+%1" ).arg ( (i + 1) % 10 ) );
		}
	}

	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( pressFileRecent ( int ) ) );


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

	for ( i = 0; i < BORDER_MENU_TOTAL; i++ )
	{
		actOptionsBorder[i] = new QAction ( bp[i].text, this );

		connect ( actOptionsBorder[i], SIGNAL ( triggered () ),
			signalMapper, SLOT ( map () ) );
		signalMapper->setMapping ( actOptionsBorder[i], bp[i].num );
	}

	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( pressOptionsBorder ( int ) ) );



#define MENTS( A, B, C, D, E ) \
	menuInit ( &actOptionsTextStyle_ ## A , B, C, D ); \
	connect ( actOptionsTextStyle_ ## A, SIGNAL ( triggered () ), signalMapper, \
		SLOT ( map () ) ); \
	signalMapper->setMapping ( actOptionsTextStyle_ ## A, E );



	signalMapper = new QSignalMapper ( this );

MENTS ( none,		"None",			NULL,		NULL,		CED_TEXT_STYLE_CLEAR )
MENTS ( bold,		"Bold",			"Ctrl+B",	"format-text-bold", CED_TEXT_STYLE_BOLD )
MENTS ( italic,		"Italic",		"Ctrl+I",	"format-text-italic", CED_TEXT_STYLE_ITALIC )
MENTS ( underline,	"Underline",		"Ctrl+U",	"format-text-underline", CED_TEXT_STYLE_UNDERLINE_SINGLE )
MENTS ( underline_double, "Underline Double",	NULL,		NULL,		CED_TEXT_STYLE_UNDERLINE_DOUBLE )
MENTS ( underline_wavy, "Underline Wavy",	NULL,		NULL,		CED_TEXT_STYLE_UNDERLINE_WAVY )
MENTS ( strikethrough,	"Strikethrough",	NULL,		NULL,		CED_TEXT_STYLE_STRIKETHROUGH )

	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( pressOptionsTextStyle ( int ) ) );




	actEditUseSystemClipboard->setCheckable ( true );
	actEditUseSystemClipboard->setChecked ( pprfs->getInt (
		GUI_INIFILE_CLIPBOARD_USE_SYSTEM ) == 0 ? false : true );

	actFindWildcards->setCheckable ( true );
	actFindWildcards->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_WILDCARDS ) == 0 ? false : true );

	actFindCase->setCheckable ( true );
	actFindCase->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_CASE_SENSITIVE ) == 0 ? false : true );

	actFindValue->setCheckable ( true );
	actFindValue->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_VALUE ) == 0 ? false : true );

	actFindSheets->setCheckable ( true );
	actFindSheets->setChecked ( pprfs->getInt (
		GUI_INIFILE_FIND_ALL_SHEETS ) == 0 ? false : true );


	QMenu * menuFile = menuBar->addMenu ( "&File" );
	menuFile->setTearOffEnabled ( true );
	menuFile->addAction ( actFileNew );
	menuFile->addAction ( actFileOpen );
	menuFile->addSeparator ();
	menuFile->addAction ( actFileImport );
	menuFile->addSeparator ();
	menuFile->addAction ( actFileSave );
	menuFile->addAction ( actFileSaveAs );
	actFileRecentSeparator = menuFile->addSeparator ();

	for ( i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		menuFile->addAction ( actFileRecent[i] );
	}

	menuFile->addSeparator ();
	menuFile->addAction ( actFileQuit );

	QMenu * menuEdit = menuBar->addMenu ( "&Edit" );
	menuEdit->setTearOffEnabled ( true );

	menuEdit->addAction ( actEditUndo );
	menuEdit->addAction ( actEditRedo );
	menuEdit->addSeparator ();
	menuEdit->addAction ( actEditCut );
	menuEdit->addAction ( actEditCopy );
	menuEdit->addAction ( actEditCopyVal );
	menuEdit->addAction ( actEditCopyOutput );
	menuEdit->addSeparator ();

	QMenu * menuEditTransform = menuEdit->addMenu ( "Transform Clipboard" );
	menuEditTransform->setTearOffEnabled ( true );
	menuEditTransform->addAction ( actEditTransformTrans );

	menuEditTransform->addAction ( actEditTransformFlipH );
	menuEditTransform->addAction ( actEditTransformFlipV );
	menuEditTransform->addAction ( actEditTransformRotClock );
	menuEditTransform->addAction ( actEditTransformRotAnti );

	menuEdit->addAction ( actEditUseSystemClipboard );

	menuEdit->addSeparator ();
	menuEdit->addAction ( actEditPaste );
	menuEdit->addAction ( actEditPasteContent );
	menuEdit->addAction ( actEditPastePrefs );
	menuEdit->addSeparator ();
	menuEdit->addAction ( actEditClear );
	menuEdit->addAction ( actEditClearContent );
	menuEdit->addAction ( actEditClearPrefs );
	menuEdit->addSeparator ();
	menuEdit->addAction ( actEditFixYears );
	menuEdit->addAction ( actEditSelectAll );

	QMenu * menuSheet = menuBar->addMenu ( "&Sheet" );
	menuSheet->setTearOffEnabled ( true );
	menuSheet->addAction ( actSheetNew );
	menuSheet->addAction ( actSheetDuplicate );
	menuSheet->addSeparator ();
	menuSheet->addAction ( actSheetRename );
	menuSheet->addAction ( actSheetDelete );
	menuSheet->addSeparator ();
	menuSheet->addAction ( actSheetExport );
	menuSheet->addAction ( actSheetExportOutput );
	menuSheet->addSeparator ();
	menuSheet->addAction ( actSheetFreezePanes );
	menuSheet->addAction ( actSheetLock );
	menuSheet->addSeparator ();
	menuSheet->addAction ( actSheetPrevious );
	menuSheet->addAction ( actSheetNext );
	menuSheet->addSeparator ();
	menuSheet->addAction ( actSheetRecalcBook );
	menuSheet->addAction ( actSheetRecalc );

	QMenu * menuRow = menuBar->addMenu ( "&Row" );
	menuRow->setTearOffEnabled ( true );
	menuRow->addAction ( actRowInsert );
	menuRow->addAction ( actRowInsertPasteHeight );
	menuRow->addSeparator ();
	menuRow->addAction ( actRowDelete );
	menuRow->addSeparator ();
	menuRow->addAction ( actRowSort );

	QMenu * menuColumn = menuBar->addMenu ( "&Column" );
	menuColumn->setTearOffEnabled ( true );
	menuColumn->addAction ( actColumnInsert );
	menuColumn->addAction ( actColumnInsertPasteWidth );
	menuColumn->addSeparator ();
	menuColumn->addAction ( actColumnDelete );
	menuColumn->addSeparator ();
	menuColumn->addAction ( actColumnSort );
	menuColumn->addSeparator ();
	menuColumn->addAction ( actColumnSetWidth );
	menuColumn->addAction ( actColumnSetWidthAuto );

	QMenu * menuOptions = menuBar->addMenu ( "&Options" );
	menuOptions->setTearOffEnabled ( true );
	menuOptions->addAction ( actOptionsFullScreen );
	menuOptions->addAction ( actOptionsFind );
	menuOptions->addAction ( actOptionsGraph );
	menuOptions->addAction ( actOptionsView );
	menuOptions->addSeparator ();
	menuOptions->addAction ( actOptionsEditCell );
	menuOptions->addAction ( actOptionsCellPrefs );
	menuOptions->addAction ( actOptionsBookPrefs );
	menuOptions->addAction ( actOptionsProgramPrefs );
	menuOptions->addSeparator ();

	QMenu * menuOptionTextStyle = menuOptions->addMenu ( "Text Style" );
	menuOptionTextStyle->setTearOffEnabled ( true );
	menuOptionTextStyle->addAction ( actOptionsTextStyle_none );
	menuOptionTextStyle->addSeparator ();
	menuOptionTextStyle->addAction ( actOptionsTextStyle_bold );
	menuOptionTextStyle->addAction ( actOptionsTextStyle_italic );
	menuOptionTextStyle->addAction ( actOptionsTextStyle_underline );
	menuOptionTextStyle->addAction ( actOptionsTextStyle_underline_double );
	menuOptionTextStyle->addAction ( actOptionsTextStyle_underline_wavy );
	menuOptionTextStyle->addAction ( actOptionsTextStyle_strikethrough );

	menuOptions->addAction ( actOptionsBackgroundColor );
	menuOptions->addAction ( actOptionsForegroundColor );
	menuOptions->addAction ( actOptionsBorderColor );

	QMenu * menuOptionBorderType = menuOptions->addMenu ( "Border Type" );
	menuOptionBorderType->setTearOffEnabled ( true );
	menuOptionBorderType->addAction ( actOptionsBorder [ 0 ] );
	menuOptionBorderType->addSeparator ();
	menuOptionBorderType->addAction ( actOptionsBorder [ 1 ] );
	menuOptionBorderType->addAction ( actOptionsBorder [ 2 ] );
	menuOptionBorderType->addAction ( actOptionsBorder [ 3 ] );
	menuOptionBorderType->addSeparator ();
	menuOptionBorderType->addAction ( actOptionsBorder [ 4 ] );
	menuOptionBorderType->addAction ( actOptionsBorder [ 5 ] );
	menuOptionBorderType->addAction ( actOptionsBorder [ 6 ] );
	menuOptionBorderType->addSeparator ();

	QMenu * menuBorderTypeH = menuOptionBorderType->addMenu ("Horizontal" );
	menuBorderTypeH->setTearOffEnabled ( true );
	menuBorderTypeH->addAction ( actOptionsBorder [ 7 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 8 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 9 ] );
	menuBorderTypeH->addSeparator ();
	menuBorderTypeH->addAction ( actOptionsBorder [ 10 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 11 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 12 ] );
	menuBorderTypeH->addSeparator ();
	menuBorderTypeH->addAction ( actOptionsBorder [ 13 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 14 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 15 ] );
	menuBorderTypeH->addSeparator ();
	menuBorderTypeH->addAction ( actOptionsBorder [ 16 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 17 ] );
	menuBorderTypeH->addAction ( actOptionsBorder [ 18 ] );

	QMenu * menuBorderTypeV = menuOptionBorderType->addMenu ( "Vertical" );
	menuBorderTypeV->setTearOffEnabled ( true );
	menuBorderTypeV->addAction ( actOptionsBorder [ 19 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 20 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 21 ] );
	menuBorderTypeV->addSeparator ();
	menuBorderTypeV->addAction ( actOptionsBorder [ 22 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 23 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 24 ] );
	menuBorderTypeV->addSeparator ();
	menuBorderTypeV->addAction ( actOptionsBorder [ 25 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 26 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 27 ] );
	menuBorderTypeV->addSeparator ();
	menuBorderTypeV->addAction ( actOptionsBorder [ 28 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 29 ] );
	menuBorderTypeV->addAction ( actOptionsBorder [ 30 ] );

	QMenu * menuHelp = menuBar->addMenu ( "&Help" );
	menuHelp->setTearOffEnabled ( true );
	menuHelp->addAction ( actOptionsHelp );
	menuHelp->addAction ( actOptionsAboutQt );
	menuHelp->addAction ( actOptionsAbout );

	QMenu * menuFind = findMenuBar->addMenu ( "Options" );
	menuFind->addAction ( actFindWildcards );
	menuFind->addAction ( actFindCase );
	menuFind->addAction ( actFindValue );
	menuFind->addAction ( actFindSheets );

	QMenu * menuGraph = graphMenuBar->addMenu ( "&Graph" );
	menuGraph->setTearOffEnabled ( true );
	menuGraph->addAction ( actGraphNew );
	menuGraph->addAction ( actGraphDuplicate );
	menuGraph->addSeparator ();
	menuGraph->addAction ( actGraphRename );
	menuGraph->addAction ( actGraphDelete );
	menuGraph->addSeparator ();
	menuGraph->addAction ( actGraphRedraw );
	menuGraph->addAction ( actGraphExport );
	menuGraph->addAction ( actGraphSClipboard );
}

