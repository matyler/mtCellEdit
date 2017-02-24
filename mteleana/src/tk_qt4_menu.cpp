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
	QAction		**	const	action,
	char	const	*	const	text,
	char	const	*	const	shortcut,
	char	const	*	const	icon
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

void MainWindow::pressFileSaveMap ()
{
	if ( ! election )
	{
		return;
	}

	QStringList	format_list;
	QString		last = mtQEX::qstringFromC ( prefs.getString (
				PREFS_LAST_MAP_FILE_NAME ) );
	int		format = prefs.getInt ( PREFS_LAST_MAP_FILE_FORMAT );


	format_list

#ifdef USE_CAIRO_EPS
		<< "EPS"
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
		<< "PDF"
#endif
#ifdef CAIRO_HAS_PNG_FUNCTIONS
		<< "PNG"
#endif
#ifdef CAIRO_HAS_PS_SURFACE
		<< "PS"
#endif
#ifdef CAIRO_HAS_SVG_SURFACE
		<< "SVG"
#endif
		;


	while ( 1 )
	{
		mtQEX::SaveFileDialog dialog ( this, "Save Map File",
			format_list, format, last.toUtf8 ().data () );

		if ( ! dialog.exec () )
		{
			break;
		}


		QStringList	fileList = dialog.selectedFiles ();
		QString		filename = fileList.at ( 0 );


		if ( ! filename.isEmpty () )
		{
			format = dialog.getFormat ();

			if ( election->savePolymap ( filename.toUtf8().data(),
				zoomMap, format, comboMapMode->currentIndex (),
				comboDiagramLeft->currentText().toUtf8().data()
				) )
			{
				QMessageBox::critical ( this, "Error",
					"Unable to save map." );

				continue;
			}

			prefs.set ( PREFS_LAST_MAP_FILE_NAME,
				filename.toUtf8 ().data () );
			prefs.set ( PREFS_LAST_MAP_FILE_FORMAT, format );

			break;
		}
	}
}

void MainWindow::pressFileQuit ()
{
	close ();
}

void MainWindow::pressHelpAboutQt ()
{
	QMessageBox::aboutQt ( this );
}

void MainWindow::pressHelpAbout ()
{
	QMessageBox::about ( this, "About",
		VERSION"\n"
	"\n"
	"Copyright (C) 2009-2016 Mark Tyler.\n"
	"\n"
	"This program is free software: you can redistribute it and/or modify "
	"it under the terms of the GNU General Public License as published by "
	"the Free Software Foundation, either version 3 of the License, or "
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful, "
	"but WITHOUT ANY WARRANTY; without even the implied warranty of "
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License "
	"along with this program.  If not, see http://www.gnu.org/licenses/\n"
		);
}

void MainWindow::createMenus ()
{
	QAction
			* actFileSaveMap,
			* actFileQuit,
			* actEditFind,
			* actHelpAboutQt,
			* actHelpAbout
			;


#define MENFU( A, B, C, D ) \
	menuInit ( &act ## A , B, C, D ); \
	connect ( act ## A, SIGNAL ( triggered () ), this, \
		SLOT ( press ## A () ) );


MENFU ( FileSaveMap,	"Save Map Image ...",	NULL,	NULL )
MENFU ( FileQuit,	"Quit",			"Ctrl+Q", "application-exit" )

MENFU ( EditFind,	"Find",			"Ctrl+F", "edit-find" )

MENFU ( HelpAboutQt,	"About Qt ...",		NULL, NULL )
MENFU ( HelpAbout,	"About ...",		"F1", "help-about" )


	QMenu * menuFile = menuBar->addMenu ( "&File" );
	menuFile->setTearOffEnabled ( true );
	menuFile->addAction ( actFileSaveMap );
	menuFile->addSeparator ();
	menuFile->addAction ( actFileQuit );

	QMenu * menuEdit = menuBar->addMenu ( "&Edit" );
	menuEdit->setTearOffEnabled ( true );

	menuEdit->addAction ( actEditFind );

	QMenu * menuHelp = menuBar->addMenu ( "&Help" );

	menuHelp->addAction ( actHelpAboutQt );
	menuHelp->addAction ( actHelpAbout );
}

