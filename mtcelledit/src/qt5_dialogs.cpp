/*
	Copyright (C) 2013-2021 Mark Tyler

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

#include "qt5.h"



void MainWindow::press_HelpAboutQt ()
{
	QMessageBox::aboutQt ( this );
}

void MainWindow::press_HelpAbout ()
{
	mtQEX::DialogAbout dialog ( this, VERSION );

	dialog.add_info ( "About",
		VERSION"\n"
	"\n"
	"Copyright (C) " MT_COPYRIGHT_YEARS " Mark Tyler.\n"
	"\n"
	"https://www.marktyler.org/\n"
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

	dialog.exec ();
}

void MainWindow::press_HelpHelp ()
{
	char const * program = mprefs.help_browser.c_str();

	if ( ! program[0] )
	{
		program = getenv ( "BROWSER" );
	}

	if ( ! program || ! program[0] )
	{
		program = "xdg-open";
	}

	std::string path;
	mtKit::get_data_dir ( path, mprefs.help_file.c_str() );
	char const * const html = path.c_str ();

/*
	if ( ! mtkit_file_readable ( html ) )
	{
		QMessageBox::critical ( this, "Error",
			"I am unable to find the documentation.  "
			"You need to set the correct location in the "
			"Preferences." );

		return;
	}
*/

	QStringList list;


	list << mtQEX::qstringFromC ( html );

	if ( ! QProcess::startDetached ( mtQEX::qstringFromC ( program ),
		list ) )
	{
		QMessageBox::critical ( this, "Error",
			"There was a problem running the HTML browser.  "
			"You need to set the correct program name in the "
			"Preferences window." );

		return;
	}
}

void MainWindow::press_OptionsFullScreen ()
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

int MainWindow::okToLoseChanges ()
{
	projectGraphStoreChanges ();

	if ( memChanged == 0 )
	{
		return 1;
	}


	QMessageBox	mb ( this );


	mb.setIcon ( QMessageBox::Warning );
	mb.setWindowTitle ( "Warning" );
	mb.setText ( "This project contains changes that have not been "
		"saved.  Do you really want to lose these changes?" );
	mb.addButton ( "Cancel", QMessageBox::RejectRole );
	mb.addButton ( "Lose Changes", QMessageBox::AcceptRole );

	if ( mb.exec () == QDialog::Rejected )
	{
		return 0;
	}

	return 1;
}

SwatchDialog::SwatchDialog (
	QString		const	title,
	int		const	font_size
	)
{
	setWindowTitle ( title );

	QVBoxLayout * const layv = new QVBoxLayout;
	layv->setContentsMargins ( 0, 0, 0, 0 );
	layv->setSpacing ( 0 );
	setLayout ( layv );

	QGridLayout * const grid = new QGridLayout;
	layv->addLayout ( grid );

	mtQEX::ArrowFilter arrowKeyFilter ( grid );
	int const swatch_size = 2 * font_size;

	for ( int r = 0, idx = 0; r < COLOR_SWATCH_ROWS; r++ )
	{
		for ( int c = 0; c < COLOR_SWATCH_COLS; c++, idx++ )
		{
			QPushButton * const button = new QPushButton;
			grid->addWidget ( button, r, c, 1, 1 );

			connect ( button, &QPushButton::clicked, [idx,this]
			{
				pressButton ( idx );
			} );

			int const irgb = be_color_swatch_get ( idx );
			int const ir = pixy_int_2_red ( irgb );
			int const ig = pixy_int_2_green ( irgb );
			int const ib = pixy_int_2_blue ( irgb );

			QPixmap pixmap ( swatch_size, swatch_size );
			pixmap.fill ( QColor ( ir, ig, ib ) );

			button->setIcon ( QIcon ( pixmap ) );
			button->setIconSize ( QSize ( swatch_size,
				swatch_size ) );

			button->installEventFilter ( &arrowKeyFilter );
		}
	}

	QPushButton * const button = new QPushButton ( "Cancel" );
	layv->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressCancel () ) );

	/*
	This hack approximates the centre of the dialog to the mouse.  I don't
	show the dialog first to get the real height/width as this means the
	dialog can be moved partially off screen which is undesirable.
	*/

	QPoint ps = QCursor::pos ();

	ps.setX ( ps.x() - width () / 4 );
	ps.setY ( ps.y() - height () / 4 );
	move ( ps );

	exec ();
}

int SwatchDialog::getColor ()
{
	return color;
}

void SwatchDialog::pressButton (
	int	const	i
	)
{
	color = be_color_swatch_get ( i );
	close ();
}

void SwatchDialog::pressCancel ()
{
	close ();
}

