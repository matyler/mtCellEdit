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



void Mainwindow::press_options_full_screen ()
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

void Mainwindow::press_options_preferences ()
{
	mtQEX::prefs_window ( this, backend.uprefs, "Preferences" );
}

void Mainwindow::press_options_statusbar ()
{
	if ( act_options_statusbar->isChecked () )
	{
		statusBar ()->show ();
	}
	else
	{
		statusBar ()->hide ();
	}
}

void Mainwindow::press_options_pan_window ()
{
	DialogPan ( *this, backend.get_ui_scale (), backend.file.
		get_pixmap (), m_scroll_main );
}

void Mainwindow::press_options_zoom_main_in ()
{
	m_canvas_main->zoom_in ();
}

void Mainwindow::press_options_zoom_main_out ()
{
	m_canvas_main->zoom_out ();
}

void Mainwindow::press_options_zoom_main_3 ()
{
	m_canvas_main->set_zoom ( -9 );
}

void Mainwindow::press_options_zoom_main_100 ()
{
	m_canvas_main->set_zoom ( 0 );
}

void Mainwindow::press_options_zoom_main_3200 ()
{
	m_canvas_main->set_zoom ( 9 );
}

void Mainwindow::press_options_zoom_split_in ()
{
	m_canvas_split->zoom_in ();
}

void Mainwindow::press_options_zoom_split_out ()
{
	m_canvas_split->zoom_out ();
}

void Mainwindow::press_options_zoom_split_3 ()
{
	m_canvas_split->set_zoom ( -9 );
}

void Mainwindow::press_options_zoom_split_100 ()
{
	m_canvas_split->set_zoom ( 0 );
}

void Mainwindow::press_options_zoom_split_3200 ()
{
	m_canvas_split->set_zoom ( 9 );
}

void Mainwindow::press_options_zoom_grid ()
{
	int const zm = act_options_zoom_grid->isChecked () ? 1 : 0;

	backend.uprefs.set ( PREFS_CANVAS_ZOOM_GRID_SHOW, zm );

	m_canvas_main->set_zoom_grid ( zm );
	m_canvas_split->set_zoom_grid ( zm );
}

void Mainwindow::press_options_split_canvas ()
{
	if ( act_options_split_canvas->isChecked () )
	{
		split_show ();
	}
	else
	{
		split_hide ();
	}

	update_menus ();
}

void Mainwindow::press_options_split_switch ()
{
	split_switch ();
}

void Mainwindow::press_options_split_focus ()
{
	main_view_moved ();
}

void Mainwindow::press_options_show_files ()
{
	if ( act_options_show_files->isChecked () )
	{
		m_files_table->show ();
	}
	else
	{
		m_files_table->hide ();
	}
}

void Mainwindow::press_help_help ()
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

void Mainwindow::press_help_about ()
{
	mtQEX::DialogAbout dialog ( this, VERSION );

	dialog.add_info ( "About",
		VERSION"\n"
	"\n"
	"Copyright (C) " MT_COPYRIGHT_YEARS " Mark Tyler.\n"
	"\n"
	"Code ideas and portions from mtPaint:\n"
	"Copyright (C) 2004-2006 Mark Tyler\n"
	"Copyright (C) 2006-2016 Dmitry Groshev\n"
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

void Mainwindow::press_help_about_qt ()
{
	QMessageBox::aboutQt ( this );
}

