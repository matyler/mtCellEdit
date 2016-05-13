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



void MainWindow :: pressOptionsAboutQt ()
{
	QMessageBox::aboutQt ( this );
}

void MainWindow :: pressOptionsAbout ()
{
	QMessageBox :: about ( this, tr ( "About" ),
		tr ( VERSION"\n"
	"\n"
	"Copyright (C) 2008-2016 Mark Tyler.\n"
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
		) );
}

void MainWindow :: pressOptionsFullScreen ()
{
	if ( isFullScreen () )
	{
		showNormal ();
	}
	else
	{
		showFullScreen ();
	}
}

void MainWindow :: pressOptionsHelp ()
{
	char	const	* program;
	char	const	* html;


	program = prefs_get_string ( GUI_INIFILE_HELP_BROWSER );

	if ( ! program[0] )
	{
		program = getenv ( "BROWSER" );
	}

	if ( ! program || ! program[0] )
	{
		program = "firefox";
	}

	html = prefs_get_string ( GUI_INIFILE_HELP_FILE );

	if ( ! mtkit_file_readable ( html ) )
	{
		QMessageBox :: critical ( this, tr ( "Error" ),
			tr ("I am unable to find the documentation.  "
			"You need to set the correct location in the "
			"Preferences." ) );

		return;
	}


	QStringList list;


	list << mtQEX::qstringFromC ( html );

	if ( ! QProcess::startDetached ( mtQEX::qstringFromC ( program ),
		list ) )
	{
		QMessageBox :: critical ( this, tr ( "Error" ),
			tr ("There was a problem running the HTML browser.  "
			"You need to set the correct program name in the "
			"Preferences window." ) );

		return;
	}
}

int MainWindow :: okToLoseChanges ()
{
	projectGraphStoreChanges ();

	if ( memChanged == 0 )
	{
		return 1;
	}


	QMessageBox	mb ( this );


	mb.setIcon ( QMessageBox::Warning );
	mb.setWindowTitle ( "Warning" );
	mb.setText ( tr ( "This project contains changes that have not been "
		"saved.  Do you really want to lose these changes?" ) );
	mb.addButton ( "Cancel", QMessageBox::RejectRole );
	mb.addButton ( "Lose Changes", QMessageBox::AcceptRole );

	if ( mb.exec () == QDialog::Rejected )
	{
		return 0;
	}

	return 1;
}

SwatchDialog :: SwatchDialog ( QString const title )
	:
	color		( -1 )
{
	int		r,
			c,
			idx,
			irgb,
			ir,
			ig,
			ib;
	QPushButton	* button;
	QGridLayout	* grid;
	QSignalMapper	* signalMapper = new QSignalMapper ( this );


	setWindowTitle ( title );

	QVBoxLayout * layout = new QVBoxLayout;
	layout->setMargin ( 0 );
	layout->setSpacing ( 0 );
	setLayout ( layout );

	grid = new QGridLayout;
	layout->addLayout ( grid );

	qexArrowFilter * arrowKeyFilter = new qexArrowFilter ( grid );

	for ( r = 0, idx = 0; r < COLOR_SWATCH_ROWS; r++ )
	{
		for ( c = 0; c < COLOR_SWATCH_COLS; c++, idx++ )
		{
			button = new QPushButton;
			grid->addWidget ( button, r, c, 1, 1 );

			connect ( button, SIGNAL ( clicked () ),
				signalMapper, SLOT ( map () ) );

			signalMapper->setMapping ( button, idx );

			irgb = be_color_swatch_get ( idx );
			ir = MTKIT_INT_2_R ( irgb );
			ig = MTKIT_INT_2_G ( irgb );
			ib = MTKIT_INT_2_B ( irgb );

			QPixmap pixmap ( COLOR_SWATCH_SIZE, COLOR_SWATCH_SIZE );
			pixmap.fill ( QColor ( ir, ig, ib ) );

			button->setIcon ( QIcon ( pixmap ) );
			button->setIconSize ( QSize ( COLOR_SWATCH_SIZE,
				COLOR_SWATCH_SIZE ) );

			button->installEventFilter ( arrowKeyFilter );
		}
	}

	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
	     this, SLOT ( pressButton ( int ) ) );

	button = new QPushButton ( tr ( "Cancel" ) );
	layout->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressCancel () ) );


	/*
	This hack approximates the centre of the dialog to the mouse.  I don't
	show the dialog first to get the real height/width as this means the
	dialog can be moved partially off screen which is undesirable.
	*/

	QPoint pos = QCursor::pos ();

	pos.setX ( pos.x() - width () / 4 );
	pos.setY ( pos.y() - height () / 4 );
	move ( pos );

	exec ();
}

int SwatchDialog :: getColor ()
{
	return color;
}

void SwatchDialog :: pressButton (
	int	const	i
	)
{
	color = be_color_swatch_get ( i );
	close ();
}

void SwatchDialog :: pressCancel ()
{
	close ();
}

