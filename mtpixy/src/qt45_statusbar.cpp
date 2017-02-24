/*
	Copyright (C) 2016 Mark Tyler

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

#include "qt45.h"



void Mainwindow::set_statusbar_geometry (
	QString	const	txt
	)
{
	m_statusbar_geometry->setText ( txt );
}

void Mainwindow::set_statusbar_cursor (
	QString	const	txt
	)
{
	m_statusbar_cursor->setText ( txt );
}

void Mainwindow::set_statusbar_selection (
	QString	const	txt
	)
{
	m_statusbar_selection->setText ( txt );
}

void Mainwindow::set_statusbar_undo (
	QString	const	txt
	)
{
	m_statusbar_undo->setText ( txt );
}

static void print_zoom_info (
	char	* const	buf,
	size_t	const	buflen,
	int	const	zs,
	int	const	zp
	)
{
	mtkit_strnncat ( buf, "   ", buflen );

	size_t l = strlen ( buf );

	if ( zs < 0 )
	{
		snprintf ( buf + l, buflen - l, "%i:1 (%i%%)", -zs, zp );
	}
	else	// zs >= 0
	{
		snprintf ( buf + l, buflen - l, "1:%i (%i%%)", zs, zp );
	}
}

void Mainwindow::update_statusbar_geometry ()
{
	mtPixy::Image	* const	im = backend.file.get_image ();


	if ( ! im )
	{
		m_statusbar_undo->setText ( "?" );
		return;
	}


	char		buf [ 1024 ];

	mtPixy::image_print_geometry ( im, buf, sizeof ( buf ) );

	print_zoom_info ( buf, sizeof ( buf ),
		m_canvas_main->get_zoom_scale (),
		m_canvas_main->get_zoom_percent() );

	if ( is_split_visible () )
	{
		print_zoom_info ( buf, sizeof ( buf ),
			m_canvas_split->get_zoom_scale (),
			m_canvas_split->get_zoom_percent() );
	}

	set_statusbar_geometry ( buf );
}

void Mainwindow::update_statusbar_undo ()
{
	set_statusbar_undo ( QString ( "%1+%2" )
		.arg ( backend.file.get_undo_steps () )
		.arg ( backend.file.get_redo_steps () )
		);
}

void Mainwindow::update_statusbar_selection ()
{
	switch ( backend.file.get_tool_mode () )
	{
	case mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_PASTE:
	case mtPixyUI::File::TOOL_MODE_PASTING:
	{
		int		x1, y1, x2, y2;


		backend.file.rectangle_overlay.get_xy ( x1, y1, x2, y2 );
		set_statusbar_selection ( QString ( "%1,%2 : %3 x %4" )
			.arg ( MIN ( x1, x2 ) )
			.arg ( MIN ( y1, y2 ) )
			.arg ( abs ( x2 - x1 ) + 1 )
			.arg ( abs ( y2 - y1 ) + 1 ) );
	}
		break;

	default:
		set_statusbar_selection ( "" );
		return;
	}
}

void Mainwindow::create_statusbar ()
{
	QStatusBar	* const	sb = statusBar ();
	QWidget			* w;


	sb->setStyleSheet ( "QStatusBar::item { border: 0px}" );

///	LEFT

	m_statusbar_geometry = new QLabel ( "" );
	sb->addWidget ( m_statusbar_geometry );

	w = new QWidget;
	w->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Preferred );
	sb->addWidget ( w );

	m_statusbar_cursor = new QLabel ( "" );
	sb->addWidget ( m_statusbar_cursor );

///	RIGHT

	m_statusbar_selection = new QLabel ( "" );
	sb->addPermanentWidget ( m_statusbar_selection );

	w = new QWidget;
	w->setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Preferred );
	sb->addPermanentWidget ( w );

	m_statusbar_undo = new QLabel ( "" );
	sb->addPermanentWidget ( m_statusbar_undo );
}

