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

#include "qt5_mw.h"



void Mainwindow::create_menu ()
{
	QAction * act_file_open_db;
	QAction * act_file_quit;

	QEX_MENU ( file_open_db, "Open Database ...", "Ctrl+O", "document-open")
	QEX_MENU ( file_quit, "Quit", "Ctrl+Q", "application-exit" )

	QMenu * const file_menu = m_menu_bar->addMenu ( "&File" );
	file_menu->setTearOffEnabled ( true );
	file_menu->addAction ( act_file_open_db );
	file_menu->addSeparator ();


	for ( int i = 0; i < PREFS_CRUL_RECENT_DB_TOTAL; i++ )
	{
		act_db_recent[i] = new QAction ( "", this );

		connect ( act_db_recent[i], &QAction::triggered, [i, this] {
			press_file_recent ( i + 1 ); } );

		file_menu->addAction ( act_db_recent[i] );
	}

	file_menu->addSeparator ();
	file_menu->addAction ( act_file_quit );


	QAction * act_edit_prefs;
	QAction * act_edit_mode_camera;
	QAction * act_edit_mode_ruler;

	QEX_MENU ( edit_mode_camera, "Camera", "F2", NULL )
	QEX_MENU ( edit_mode_ruler, "Ruler", "F3", NULL )
	QEX_MENU ( edit_import_pts, "Import PTS ...", NULL, "document-open")
	QEX_MENU ( edit_import_model, "Import Model ...", NULL, "document-open")
	QEX_MENU ( edit_prefs, "Preferences ...", "Ctrl+P", "preferences-other")

	QMenu * const edit_menu = m_menu_bar->addMenu ( "&Edit" );
	edit_menu->setTearOffEnabled ( true );

	QMenu * const mode_menu = edit_menu->addMenu ( "Set Mode" );
	mode_menu->setTearOffEnabled ( true );
	mode_menu->addAction ( act_edit_mode_camera );
	mode_menu->addAction ( act_edit_mode_ruler );

	QActionGroup * group = new QActionGroup ( mode_menu );
	group->addAction ( act_edit_mode_camera );
	group->addAction ( act_edit_mode_ruler );
	group->isExclusive ();

	act_edit_mode_camera->setCheckable ( true );
	act_edit_mode_ruler->setCheckable ( true );
	act_edit_mode_camera->setChecked ( true );

	edit_menu->addSeparator ();
	edit_menu->addAction ( act_edit_import_pts );
	edit_menu->addAction ( act_edit_import_model );
	edit_menu->addSeparator ();
	edit_menu->addAction ( act_edit_prefs );


	QAction * act_view_select_a;
	QAction * act_view_select_b;
	QAction * act_view_reset_camera;
	QAction * act_view_spin_180;
	QAction * act_view_xyz_snap_nudge;
	QAction * act_view_clone_ab;
	QAction * act_view_clone_ba;
	QAction * act_view_nudge_up;
	QAction * act_view_nudge_down;

	QEX_MENU ( view_select_a, "Select A", "Escape", NULL )
	QEX_MENU ( view_select_b, "Select B", "Shift+Escape", NULL )
	QEX_MENU ( view_show_antialiasing, "Antialiasing", NULL, NULL )
	QEX_MENU ( view_show_crosshair, "Crosshair", NULL, NULL )
	QEX_MENU ( view_show_statusbar, "Statusbar", NULL, NULL )
	QEX_MENU ( view_show_rulers, "Rulers", NULL, NULL )
	QEX_MENU ( view_show_ruler_plane, "Active Ruler Plane", NULL, NULL )
	QEX_MENU ( view_show_cloud, "Cloud", NULL, NULL )
	QEX_MENU ( view_show_model, "Model", NULL, NULL )
	QEX_MENU ( view_split, "Split", "V", NULL )
	QEX_MENU ( view_split_switch, "Split Switch", "H", NULL )
	QEX_MENU ( view_reset_camera, "Reset Camera", "Home", NULL )
	QEX_MENU ( view_spin_180, "Spin 180 Degrees", "End", NULL )
	QEX_MENU ( view_xyz_snap_nudge, "XYZ snap to Nudge", NULL, NULL )
	QEX_MENU ( view_clone_ab, "Clone Camera A->B", NULL, NULL )
	QEX_MENU ( view_clone_ba, "Clone Camera B->A", NULL, NULL )
	QEX_MENU ( view_nudge_down, "Nudge --", "F8", NULL )
	QEX_MENU ( view_nudge_up, "Nudge ++", "F9", NULL )
	QEX_MENU ( view_res_low, "Low", "F5", NULL )
	QEX_MENU ( view_res_medium, "Medium", "F6", NULL )
	QEX_MENU ( view_res_high, "High", "F7", NULL )

	QMenu * const view_menu = m_menu_bar->addMenu ( "&View" );
	view_menu->setTearOffEnabled ( true );
	view_menu->addAction ( act_view_select_a );
	view_menu->addAction ( act_view_select_b );
	view_menu->addSeparator ();

	QMenu * const show_menu = view_menu->addMenu ( "Show" );
	show_menu->setTearOffEnabled ( true );
	show_menu->addAction ( act_view_show_antialiasing );
	show_menu->addAction ( act_view_show_crosshair );
	show_menu->addAction ( act_view_show_statusbar );
	show_menu->addAction ( act_view_show_rulers );
	show_menu->addAction ( act_view_show_ruler_plane );
	show_menu->addAction ( act_view_show_cloud );
	show_menu->addAction ( act_view_show_model );

	view_menu->addSeparator ();
	view_menu->addAction ( act_view_split );
	view_menu->addAction ( act_view_split_switch );
	view_menu->addSeparator ();
	view_menu->addAction ( act_view_reset_camera );
	view_menu->addAction ( act_view_spin_180 );
	view_menu->addAction ( act_view_xyz_snap_nudge );
	view_menu->addSeparator ();
	view_menu->addAction ( act_view_clone_ab );
	view_menu->addAction ( act_view_clone_ba );
	view_menu->addSeparator ();
	view_menu->addAction ( act_view_nudge_down );
	view_menu->addAction ( act_view_nudge_up );
	view_menu->addSeparator ();

	act_view_show_antialiasing->setCheckable ( true );
	act_view_show_crosshair->setCheckable ( true );
	act_view_show_statusbar->setCheckable ( true );
	act_view_show_rulers->setCheckable ( true );
	act_view_show_ruler_plane->setCheckable ( true );
	act_view_show_cloud->setCheckable ( true );
	act_view_show_model->setCheckable ( true );

	act_view_split->setCheckable ( true );
	act_view_split->setChecked ( true );

	QMenu * const res_menu = view_menu->addMenu ( "Resolution" );
	res_menu->setTearOffEnabled ( true );
	res_menu->addAction ( act_view_res_low );
	res_menu->addAction ( act_view_res_medium );
	res_menu->addAction ( act_view_res_high );

	group = new QActionGroup ( res_menu );
	group->addAction ( act_view_res_low );
	group->addAction ( act_view_res_medium );
	group->addAction ( act_view_res_high );
	group->isExclusive ();

	act_view_res_low->setCheckable ( true );
	act_view_res_medium->setCheckable ( true );
	act_view_res_high->setCheckable ( true );

	act_view_res_low->setChecked ( true );


	QAction * act_ruler_swap_ab;
	QAction * act_ruler_deselect;

	QEX_MENU ( ruler_swap_ab, "Swap A and B", NULL, NULL )
	QEX_MENU ( ruler_deselect, "Deselect", NULL, NULL )

	m_menu_ruler = m_menu_bar->addMenu ( "&Ruler" );
	m_menu_ruler->setTearOffEnabled ( true );

	m_menu_ruler->addAction ( act_ruler_swap_ab );
	m_menu_ruler->addAction ( act_ruler_deselect );


	QAction * act_help_about_qt;
	QAction * act_help_about;

	QEX_MENU ( help_about_qt, "About Qt ...", NULL, NULL )
	QEX_MENU ( help_about, "About ...", "F1", "help-about" )

	QMenu * const help_menu = m_menu_bar->addMenu ( "&Help" );
	help_menu->setTearOffEnabled ( true );

	help_menu->addAction ( act_help_about_qt );
	help_menu->addAction ( act_help_about );

	// Disable menu items as if no DB exists
	act_edit_import_pts->setEnabled ( false );
	act_edit_import_model->setEnabled ( false );
}

int Mainwindow::get_selected_resolution () const
{
	if ( act_view_res_medium->isChecked () )
	{
		return Crul::DB::CACHE_TYPE_MEDIUM;
	}
	else if ( act_view_res_high->isChecked () )
	{
		return Crul::DB::CACHE_TYPE_HIGH;
	}

	return Crul::DB::CACHE_TYPE_LOW;
}

