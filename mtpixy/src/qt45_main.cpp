/*
	Copyright (C) 2016-2017 Mark Tyler

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



Mainwindow::Mainwindow (
	QApplication	&app,
	Backend		&be
	)
	:
	backend			( be ),
	prefs			( be.prefs ),
	m_cursor		( *this ),
	m_canvas_main		(),
	m_paste_drag_x		( 0 ),
	m_paste_drag_y		( 0 ),
	m_paste_committed	( 0 ),
	m_last_zoom_scale	( 1 ),
	m_view_mode_flags 	( 0 )
{
	setEnabled ( false );


	mtPixy::Image	* im_screenshot = NULL;


	if ( backend.get_cline_screenshot () )
	{
		im_screenshot = get_screenshot ();
	}

	create_menu ();
	create_statusbar ();
	create_easel ();

	setGeometry ( prefs.getInt ( PREFS_WINDOW_X ),
		prefs.getInt ( PREFS_WINDOW_Y ),
		prefs.getInt ( PREFS_WINDOW_W ),
		prefs.getInt ( PREFS_WINDOW_H ) );

	if ( 1 == prefs.getInt ( PREFS_WINDOW_MAXIMIZED ) )
	{
		showMaximized ();
	}
	else
	{
		show ();
	}

	backend.detect_ui_scale ( menuBar ()->height () );

	create_toolbar ();
	create_icons ();
	create_prefs_callbacks ();
	create_dock ();

	tb_select_rectangle->setChecked ( true );
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE );

	// Restore positions of the docks and toolbars
	QByteArray qba;
	if ( 0 == mtQEX::qt_get_state ( &prefs, PREFS_WINDOW_STATE, &qba ) )
	{
		restoreState ( qba );
	}

	// Hide all widgets for now
	toggle_view_mode ();

	app.processEvents ();	// Needed to centralise large images properly

	if ( im_screenshot )
	{
		im_screenshot->palette_set_default (
			prefs.getInt ( PREFS_FILE_NEW_PALETTE_TYPE ),
			prefs.getInt ( PREFS_FILE_NEW_PALETTE_NUM ) );

		project_new ( im_screenshot );
		im_screenshot = NULL;
		toggle_view_mode ();	// Get widgets back
	}
	else if (	backend.get_cline_filename () &&
			0 == project_load ( backend.get_cline_filename () )
			)
	{
		// Successfully loaded image file, so stay in view mode and
		// start with selection tool.
	}
	else
	{
		project_new ();
		toggle_view_mode ();	// Get widgets back
		tb_paint->setChecked ( true );
		set_tool_mode ( mtPixyUI::File::TOOL_MODE_PAINT );
	}

	// Disallow focus on these widgets
	m_scroll_main->setFocusPolicy ( Qt::NoFocus );
	m_scroll_split->setFocusPolicy ( Qt::NoFocus );
	m_palette_holder->setFocusPolicy ( Qt::NoFocus );

	// This is the only widget in the main window that accepts key input
	m_canvas_main->setFocusPolicy ( Qt::StrongFocus );
	m_key_eater = new keyPressEater ( *this );
	m_canvas_main->installEventFilter ( m_key_eater );

	update_ui ( UPDATE_RECENT_FILES );

	setEnabled ( true );
	m_canvas_main->setFocus ();
}

Mainwindow::~Mainwindow ()
{
	if ( m_view_mode_flags )
	{
		toggle_view_mode ();
	}

	if ( isMaximized () )
	{
		prefs.set ( PREFS_WINDOW_MAXIMIZED, 1 );
	}
	else
	{
		prefs.set ( PREFS_WINDOW_MAXIMIZED, 0 );

		prefs.set ( PREFS_WINDOW_X, geometry().x () );
		prefs.set ( PREFS_WINDOW_Y, geometry().y () );
		prefs.set ( PREFS_WINDOW_W, geometry().width () );
		prefs.set ( PREFS_WINDOW_H, geometry().height () );
	}

	QByteArray qba = saveState ();
	mtQEX::qt_set_state ( &prefs, PREFS_WINDOW_STATE, &qba );

	delete m_key_eater;
}

void Mainwindow::update_titlebar ()
{
	char		buf[2048];


	backend.get_titlebar_text ( buf, sizeof ( buf ) );
	setWindowTitle ( mtQEX::qstringFromC ( buf ) );
}

void Mainwindow::closeEvent (
	QCloseEvent	* const	ev
	)
{
	if ( isEnabled () == false )
	{
		// Main window is currently disabled so ignore all requests
		ev->ignore ();

		return;
	}

	if ( ok_to_lose_changes () )
	{
		// No changes, or user happy to lose them
		ev->accept ();
	}
	else
	{
		// Changes have occured, user not consenting to lose them
		ev->ignore ();
	}
}

void Mainwindow::changeEvent (
	QEvent	* const	ev
	)
{
	QWidget::changeEvent ( ev );

	if (	! m_canvas_main					||
		ev->type () != QEvent::ActivationChange
		)
	{
		return;
	}

	// HACK: needed to ensure that the keyboard is always read by my key
	// eater.  Sometimes the focus gets 'lost' after a changeEvent.
	m_canvas_main->setFocus ();
}

int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Backend		backend;


	// Parse command line and initialize prefs as required
	if ( backend.command_line ( argc, argv ) )
	{
		return 0;
	}


	// I don't want Qt snooping or changing my command line.
	int		dummy_argc	= 1;
	char		dummy_str[1]	= { 0 };
	char		* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	Mainwindow	window ( app, backend );

	return app.exec ();
}

