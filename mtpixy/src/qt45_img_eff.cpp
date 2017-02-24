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



int Mainwindow::operation_update (
	int		const	res,
	char	const *	const	txt,
	int		const	updt
	)
{
	if ( res )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Operation '%1' was unsuccessful." )
			.arg ( txt ) );
	}

	if ( backend.file.reset_tool_mode () )
	{
		update_ui ( UPDATE_ALL );
	}
	else
	{
		update_ui ( updt );
	}

	return res;
}

void Mainwindow::press_image_to_rgb ()
{
	operation_update ( backend.file.convert_to_rgb (), "Convert To RGB",
		UPDATE_ALL_IMAGE );
}

void Mainwindow::press_image_to_indexed ()
{
	DialogImageIndexed	dialog ( *this );
}

void Mainwindow::press_image_delete_alpha ()
{
	operation_update ( backend.file.destroy_alpha(), "Delete Alpha Channel",
		UPDATE_ALL );
}

void Mainwindow::press_image_scale ()
{
	DialogImageScale	dialog ( *this );
}

void Mainwindow::press_image_resize ()
{
	DialogImageResize	dialog ( *this );
}

void Mainwindow::press_image_crop ()
{
	operation_update ( backend.file.crop (), "Crop", UPDATE_ALL_IMAGE );
}

void Mainwindow::press_image_flip_horizontally ()
{
	operation_update ( backend.file.flip_horizontally (),
		"Flip Horizontally", UPDATE_ALL_IMAGE );
}

void Mainwindow::press_image_flip_vertically ()
{
	operation_update ( backend.file.flip_vertically (),
		"Flip Vertically", UPDATE_ALL_IMAGE );
}

void Mainwindow::press_image_rotate_clockwise ()
{
	operation_update ( backend.file.rotate_clockwise (),
		"Rotate Clockwise", UPDATE_ALL_IMAGE );
}

void Mainwindow::press_image_rotate_anticlockwise ()
{
	operation_update ( backend.file.rotate_anticlockwise (),
		"Rotate Anti-Clockwise", UPDATE_ALL_IMAGE );
}

void Mainwindow::press_image_information ()
{
	DialogImageInfo		dialog ( backend );
}

void Mainwindow::press_effects_transform_color ()
{
	mtPixy::Image	* const im = backend.file.get_image ();
	if ( im )
	{
		DialogTransColor	dialog ( *this, im );
	}
}

void Mainwindow::reconfigure_views ()
{
	m_canvas_main->reconfigure ();
	m_canvas_split->reconfigure ();
}

void Mainwindow::rebuild_palette ()
{
	m_palette_holder->rebuild ();
}

void Mainwindow::press_effects_invert ()
{
	operation_update ( backend.file.effect_invert (), "Invert",
		UPDATE_ALL );
}

void Mainwindow::press_effects_edge_detect ()
{
	operation_update ( backend.file.effect_edge_detect (),
		"Edge Detect", UPDATE_ALL_IMAGE );
}

void Mainwindow::press_effects_sharpen ()
{
	DialogGetInt	dialog ( 1, 100, 50, "Sharpen", "Strength", 1 );


	while ( dialog.exec () == QDialog::Accepted )
	{
		// Apply pressed
		operation_update ( backend.file.effect_sharpen (
			dialog.get_int () ), "Sharpen", UPDATE_ALL_IMAGE );
	}
}

void Mainwindow::press_effects_soften ()
{
	DialogGetInt	dialog ( 1, 100, 50, "Soften", "Strength", 1 );


	while ( dialog.exec () == QDialog::Accepted )
	{
		// Apply pressed
		operation_update ( backend.file.effect_soften (
			dialog.get_int () ), "Soften", UPDATE_ALL_IMAGE );
	}
}

void Mainwindow::press_effects_emboss ()
{
	operation_update ( backend.file.effect_emboss (), "Emboss",
		UPDATE_ALL_IMAGE );
}

void Mainwindow::press_effects_bacteria ()
{
	DialogGetInt	dialog ( 1, 100, 10, "Bacteria", "Strength", 1 );


	while ( dialog.exec () == QDialog::Accepted )
	{
		// Apply pressed
		operation_update ( backend.file.effect_bacteria (
			dialog.get_int () ), "Bacteria", UPDATE_ALL_IMAGE );
	}
}

