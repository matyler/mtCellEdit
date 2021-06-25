/*
	Copyright (C) 2016-2020 Mark Tyler

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



Mainwindow::Mainwindow ( Backend &be )
	:
	backend			( be ),
	mprefs			( be.mprefs ),
	m_cursor		( *this ),
	m_canvas_main		(),
	m_paste_drag_x		( 0 ),
	m_paste_drag_y		( 0 ),
	m_paste_committed	( 0 ),
	m_last_zoom_scale	( 1 ),
	m_view_mode_flags 	( 0 )
{
	setEnabled ( false );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/scalable/apps/"
		BIN_NAME ".svg" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	mtPixmap * im_screenshot = NULL;
	if ( backend.get_cline_screenshot () )
	{
		im_screenshot = get_screenshot ();
	}

	create_menu ();
	create_statusbar ();
	create_easel ();

	setGeometry ( mprefs.window_x, mprefs.window_y,
		mprefs.window_w, mprefs.window_h );

	if ( 1 == mprefs.window_maximized )
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
	update_canvas_grid_menu ();

	// Restore positions of the docks and toolbars
	QByteArray qba;
	if ( 0 == mtQEX::qt_get_state( backend.uprefs, PREFS_WINDOW_STATE, qba))
	{
		restoreState ( qba );
	}

	// Needed to centralise large images properly
	mtQEX::process_qt_pending ();

	// Hide all widgets for now
	toggle_view_mode ();

	// Disallow focus on these widgets
	m_scroll_main->setFocusPolicy ( Qt::NoFocus );
	m_scroll_split->setFocusPolicy ( Qt::NoFocus );
	m_palette_holder->setFocusPolicy ( Qt::NoFocus );

	// This is the only widget in the main window that accepts key input
	m_canvas_main->setFocusPolicy ( Qt::StrongFocus );
	m_key_eater.reset ( new keyPressEater ( *this ) );
	m_canvas_main->installEventFilter ( m_key_eater.get() );

	m_file_key_eater.reset ( new FileListKeyEater ( *this ) );
	m_files_table->installEventFilter ( m_file_key_eater.get() );

	update_ui ( UPDATE_RECENT_FILES );

	// Needed to centralise large images properly
	mtQEX::process_qt_pending ();

	if ( im_screenshot )
	{
		pixy_pixmap_palette_set_default ( im_screenshot,
			mprefs.file_new_palette_type,
			mprefs.file_new_palette_num,
			mprefs.file_new_palette_file.c_str() );

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

	setEnabled ( true );
	m_canvas_main->setFocus ();

	create_cline_files ();
}

Mainwindow::~Mainwindow ()
{
	if ( m_view_mode_flags )
	{
		toggle_view_mode ();
	}

	QByteArray qba = saveState ();
	mtQEX::qt_set_state ( backend.uprefs, PREFS_WINDOW_STATE, qba );
}

bool Mainwindow::check_main_key_event ( QKeyEvent * ev )
{
	return m_key_eater->key_filter ( ev );
}

void Mainwindow::set_canvas_focus () const
{
	m_canvas_main->setFocus ();
}

void Mainwindow::store_window_geometry ()
{
	if ( isMaximized () )
	{
		backend.uprefs.set ( PREFS_WINDOW_MAXIMIZED, 1 );
	}
	else
	{
		backend.uprefs.set ( PREFS_WINDOW_MAXIMIZED, 0 );

		backend.uprefs.set ( PREFS_WINDOW_X, geometry().x () );
		backend.uprefs.set ( PREFS_WINDOW_Y, geometry().y () );
		backend.uprefs.set ( PREFS_WINDOW_W, geometry().width () );
		backend.uprefs.set ( PREFS_WINDOW_H, geometry().height () );
	}

	if ( m_files_table->isVisible () )
	{
		QList<int>	const size_list = m_file_split->sizes();
		double		const w = m_file_split->width();

		if ( size_list.size () > 1 && w > 0.0 )
		{
			double const p = (double)size_list.at (0) / w;

			backend.uprefs.set ( PREFS_WINDOW_FILE_LIST_SPLIT, p );
		}
	}
}

void Mainwindow::update_titlebar ()
{
	std::string const txt ( backend.get_titlebar_text () );

	setWindowTitle ( mtQEX::qstringFromC ( txt.c_str() ) );
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
		store_window_geometry ();
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
	int	dummy_argc	= 1;
	char	dummy_str[1]	= { 0 };
	char	* dummy_argv	= dummy_str;


	QApplication	app ( dummy_argc, &dummy_argv );
	Mainwindow	window ( backend );

	return app.exec ();
}

