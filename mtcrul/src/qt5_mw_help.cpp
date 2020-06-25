/*
	Copyright (C) 2020 Mark Tyler

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

#include "qt5_mw_help.h"



void Mainwindow::press_help_about_qt ()
{
	QMessageBox::aboutQt ( this );
}

void Mainwindow::press_help_about ()
{
	DialogAbout ( this );
}

DialogAbout::DialogAbout ( Mainwindow * const mw )
	:
	QDialog		( mw )
{
	setWindowTitle ( "About" );
	setModal ( true );

	resize ( 600, 600 );

	QVBoxLayout * vbox_main = new QVBoxLayout;
	setLayout ( vbox_main );

	QTextEdit	* text;

	QTabWidget * tabWidget = new QTabWidget;
	vbox_main->addWidget ( tabWidget );

	text = new QTextEdit;
	tabWidget->addTab ( text, "General" );
	text->setAcceptRichText ( false );
	text->setReadOnly ( true );
	text->setText (
	"mtCrul - Mark Tyler's point Cloud RULer\n"
	"\n"
	"A tool for exploring and measuring point clouds.\n"
	"\n"
	"https://www.marktyler.org/"
		);

	text = new QTextEdit;
	tabWidget->addTab ( text, "Workflow" );
	text->setAcceptRichText ( false );
	text->setReadOnly ( true );
	text->setText (
	"1. Create a new database somewhere on disk: File->Open\n"
	"\n"
	"2. Load in the PTS point cloud data: Edit->Import PTS"
	"\n"
	"3. Load in the model data: Edit->Import Model"
		);

	text = new QTextEdit;
	tabWidget->addTab ( text, "Camera Mode" );
	text->setAcceptRichText ( false );
	text->setReadOnly ( true );
	text->setText (
	"--------\n"
	"Keyboard\n"
	"--------\n"
	"\n"
	"Arrow keys: Move the camera in that direction by the nudge value.\n"
	"\n"
	"Ctrl+Arrow keys: Change the camera orientation.\n"
	"\n"
	"Page Up/Down: Move the camera forwards and backwards by the nudge "
		"value.\n"
	"\n"
	"Shift key multiplies the nudge by 10.\n"
	"\n"
	"-----\n"
	"Mouse\n"
	"-----\n"
	"\n"
	"Left click + drag: Move the camera in that direction by the nudge "
		"value.\n"
	"\n"
	"Right click + drag: Change the camera orientation.\n"
	"\n"
	"Shift key multiplies the nudge by 10.\n"
	"\n"
		);


	text = new QTextEdit;
	tabWidget->addTab ( text, "Ruler Mode" );
	text->setAcceptRichText ( false );
	text->setReadOnly ( true );
	text->setText (
	"--------\n"
	"Keyboard\n"
	"--------\n"
	"\n"
	"Arrow keys: Move point A in that direction by the nudge value.\n"
	"\n"
	"Page Up/Down: Extend or shrink the ruler point B by the nudge value.\n"
	"\n"
	"Shift key multiplies the nudge by 10.\n"
	"\n"
	"-----\n"
	"Mouse\n"
	"-----\n"
	"\n"
	"Left click + drag: Move point A in 1st dimension by the nudge value."
		"\n"
	"\n"
	"Right click + drag: Move point A in 2nd dimension by the nudge value."
		"\n"
	"\n"
	"Shift key multiplies the nudge by 10.\n"
	"\n"
		);

	text = new QTextEdit;
	tabWidget->addTab ( text, "License" );
	text->setAcceptRichText ( false );
	text->setReadOnly ( true );
	text->setText (
		VERSION"\n"
	"\n"
	"Copyright (C) " MT_COPYRIGHT_YEARS " Mark Tyler.\n"
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

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	vbox_main->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	show ();
	exec ();
}

