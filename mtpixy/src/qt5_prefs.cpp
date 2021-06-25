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



void Mainwindow::create_prefs_callbacks ()
{
	auto change_ui_canvas = [this]()
		{
			m_canvas_main->update_canvas_easel_rgb ();
			m_canvas_split->update_canvas_easel_rgb ();
			m_palette_holder->update_canvas_easel_rgb ();

			update_canvas_grid_menu ();
			update_ui ( Mainwindow::UPDATE_CANVAS );
		};

	backend.uprefs.set_callback ( PREFS_CANVAS_EASEL_RGB, change_ui_canvas);
	backend.uprefs.set_callback ( PREFS_CANVAS_ZOOM_GRID_GREY,
		change_ui_canvas);
	backend.uprefs.set_callback ( PREFS_CANVAS_ZOOM_GRID_SHOW,
		change_ui_canvas);

	backend.uprefs.set_callback ( PREFS_PALETTE_NUMBER_OPACITY, [this]()
		{
			m_palette_holder->rebuild ();
		} );

	backend.uprefs.set_callback ( PREFS_UI_SCALE, [this]()
		{
			backend.calc_ui_scale ();
			m_palette_holder->rebuild ();
			create_icons ();
		} );

	backend.uprefs.set_callback ( PREFS_UI_SCALE_PALETTE, [this]()
		{
			m_palette_holder->rebuild ();
		} );

	backend.uprefs.set_callback ( PREFS_UNDO_MB_MAX, [this]()
		{
			backend.file.set_undo_mb_max ( mprefs.undo_mb_max );
		} );

	backend.uprefs.set_callback ( PREFS_UNDO_STEPS_MAX, [this]()
		{
			backend.file.set_undo_steps_max( mprefs.undo_steps_max);
		} );
}

void Mainwindow::update_canvas_grid_menu ()
{
	int const zm = mprefs.canvas_zoom_grid_show;

	if ( zm )
	{
		act_options_zoom_grid->setChecked ( true );
	}
	else
	{
		act_options_zoom_grid->setChecked ( false );
	}

	m_canvas_main->set_zoom_grid ( zm );
	m_canvas_split->set_zoom_grid ( zm );
}

