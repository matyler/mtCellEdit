/*
	Copyright (C) 2024 Mark Tyler

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

#include "wasp_qt.h"



void MainWindow::create_statusbar ()
{
	QStatusBar * const sb = statusBar ();
	QWidget		* w;


	sb->setStyleSheet ( "QStatusBar::item { border: 0px}" );

///	LEFT

	m_statusbar_left = new QLabel ( "" );
	sb->addWidget ( m_statusbar_left );

	w = new QWidget;
	w->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Preferred );
	sb->addWidget ( w );

	m_statusbar_middle_left = new QLabel;
	sb->addWidget ( m_statusbar_middle_left );

///	RIGHT

	m_statusbar_middle_right = new QLabel;
	sb->addPermanentWidget ( m_statusbar_middle_right );

	w = new QWidget;
	w->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Preferred );
	sb->addPermanentWidget ( w );

	m_statusbar_right = new QLabel;
	sb->addPermanentWidget ( m_statusbar_right );
}

void MainWindow::update_statusbar ()
{
	char txt[128];

	snprintf ( txt, sizeof(txt), "Audio device: %i",
		m_mprefs.audio_output_device );
	m_statusbar_left->setText ( txt );

//	m_statusbar_middle_left->setText ( "" );
//	m_statusbar_middle_right->setText ( "" );

	snprintf( txt, sizeof(txt), "Hz: %f", m_project.get_wave_function_hz());
	m_statusbar_right->setText ( txt );
}

