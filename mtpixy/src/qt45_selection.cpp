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



void Mainwindow::press_selection_all ()
{
	if ( backend.file.select_all () )
	{
		return;
	}

	tb_select_rectangle->setChecked ( true );
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE );
}

void Mainwindow::press_selection_none ()
{
	press_select_rectangle ();
}

void Mainwindow::press_selection_lasso ()
{
	if ( copy_selection () )
	{
		return;
	}

	if ( backend.file.selection_lasso ( backend.clipboard ) )
	{
		QMessageBox::critical ( this, "Error", QString (
			"Unable to lasso the clipboard" ) );

		return;
	}

	press_edit_paste ();
}

void Mainwindow::press_selection_fill ()
{
	if ( backend.file.selection_fill () )
	{
		return;
	}

	update_ui ( UPDATE_ALL_IMAGE );
}

void Mainwindow::press_selection_outline ()
{
	if ( backend.file.selection_outline () )
	{
		return;
	}

	update_ui ( UPDATE_ALL_IMAGE );
}

void Mainwindow::press_selection_flip_v ()
{
	backend.clipboard.flip_vertical ();
	update_ui ( UPDATE_CANVAS );
}

void Mainwindow::press_selection_flip_h ()
{
	backend.clipboard.flip_horizontal ();
	update_ui ( UPDATE_CANVAS );
}

void Mainwindow::press_selection_rotate_c ()
{
	backend.file.clipboard_rotate_clockwise ( backend.clipboard );
	update_ui ( UPDATE_CANVAS );
}

void Mainwindow::press_selection_rotate_a ()
{
	backend.file.clipboard_rotate_anticlockwise ( backend.clipboard );
	update_ui ( UPDATE_CANVAS );
}

