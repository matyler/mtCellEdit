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



static void change_ui_canvas (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	callback_ptr
	)
{
	Mainwindow	* const	mw = (Mainwindow *)callback_ptr;


	mw->update_canvas_easel_rgb ();
	mw->update_ui ( Mainwindow::UPDATE_CANVAS );
}

static void change_ui_scale (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	callback_ptr
	)
{
	Mainwindow	* const	mw = (Mainwindow *)callback_ptr;


	mw->update_ui_scale ();
}

static void change_ui_scale_palette (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	callback_ptr
	)
{
	Mainwindow	* const	mw = (Mainwindow *)callback_ptr;


	mw->update_ui_scale_palette ();
}

static void change_undo_mb (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	callback_ptr
	)
{
	Mainwindow	* const	mw = (Mainwindow *)callback_ptr;


	mw->update_undo_mb_max ();
}

static void change_undo_steps (
	mtPrefValue	* const	ARG_UNUSED ( piv ),
	int		const	ARG_UNUSED ( callback_data ),
	void		* const	callback_ptr
	)
{
	Mainwindow	* const	mw = (Mainwindow *)callback_ptr;


	mw->update_undo_steps_max ();
}

void Mainwindow::create_prefs_callbacks ()
{
	mtPrefs		* p = prefs.getPrefsMem ();


	mtkit_prefs_set_callback ( p, PREFS_CANVAS_EASEL_RGB,
		change_ui_canvas, this );
	mtkit_prefs_set_callback ( p, PREFS_CANVAS_ZOOM_GRID_GREY,
		change_ui_canvas, this );

	mtkit_prefs_set_callback ( p, PREFS_PALETTE_NUMBER_OPACITY,
		change_ui_scale_palette, this );
	mtkit_prefs_set_callback ( p, PREFS_UI_SCALE, change_ui_scale, this );
	mtkit_prefs_set_callback ( p, PREFS_UI_SCALE_PALETTE,
		change_ui_scale_palette, this );
	mtkit_prefs_set_callback ( p, PREFS_UNDO_MB_MAX, change_undo_mb, this );
	mtkit_prefs_set_callback ( p, PREFS_UNDO_STEPS_MAX, change_undo_steps,
		this );
}

void Mainwindow::update_ui_scale ()
{
	backend.calc_ui_scale ();
	update_ui_scale_palette ();
	create_icons ();
}

void Mainwindow::update_ui_scale_palette ()
{
	m_palette_holder->rebuild ();
}

void Mainwindow::update_undo_mb_max ()
{
	backend.file.set_undo_mb_max ( prefs.getInt ( PREFS_UNDO_MB_MAX ) );
}

void Mainwindow::update_undo_steps_max ()
{
	backend.file.set_undo_steps_max( prefs.getInt( PREFS_UNDO_STEPS_MAX));
}

void Mainwindow::update_canvas_easel_rgb ()
{
	m_canvas_main->update_canvas_easel_rgb ();
	m_canvas_split->update_canvas_easel_rgb ();
	m_palette_holder->update_canvas_easel_rgb ();
}

