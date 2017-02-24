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



void Mainwindow::press_edit_undo ()
{
	operation_update ( backend.file.undo (), "Undo", UPDATE_ALL );
}

void Mainwindow::press_edit_redo ()
{
	operation_update ( backend.file.redo (), "Redo", UPDATE_ALL );
}

void Mainwindow::press_edit_cut ()
{
	if ( 0 == copy_selection () )
	{
		press_selection_fill ();
	}
}

void Mainwindow::press_edit_copy ()
{
	copy_selection ();
}

int Mainwindow::copy_selection ()
{
	if ( backend.file.selection_copy ( backend.clipboard ) )
	{
		QMessageBox::critical ( this, "Error", QString (
			"Unable to copy to clipboard" ) );

		return 1;
	}

	update_ui ( UPDATE_MENUS );

	return 0;
}

void Mainwindow::prepare_clipboard_paste (
	int	const	centre
	)
{
	mtPixy::Image	* im = backend.file.get_image ();
	mtPixy::Image	* clip_im = backend.clipboard.get_image ();

	if ( ! clip_im || ! im )
	{
		return;
	}

	if ( clip_im->get_type () != im->get_type () )
	{
		QMessageBox::critical ( this, "Error", QString (
			"Clipboard image type is different to the canvas "
			"image type." ) );

		return;
	}

	int		px, py;

	if ( centre )
	{
		double const cx = m_canvas_main->get_centre_canvas_x ();
		double const cy = m_canvas_main->get_centre_canvas_y ();
		int const clip_w = clip_im->get_width ();
		int const clip_h = clip_im->get_height ();
		int const iw = im->get_width ();
		int const ih = im->get_height ();

		px = (int)(iw * cx) - clip_w / 2;
		py = (int)(ih * cy) - clip_h / 2;
	}
	else
	{
		backend.clipboard.get_xy ( px, py );
	}

	backend.file.rectangle_overlay.set_paste ( backend.file.get_image (),
		clip_im, px, py );

	tb_select_rectangle->setChecked ( true );
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_PASTE );
	update_ui ( UPDATE_STATUS_SELECTION );
}

void Mainwindow::press_edit_paste ()
{
	prepare_clipboard_paste ( 0 );
}

void Mainwindow::press_edit_paste_centre ()
{
	prepare_clipboard_paste ( 1 );
}

void Mainwindow::press_edit_paste_text ()
{
	DialogPasteText		dialog ( *this );
}

void Mainwindow::press_brush_color ()
{
	mtPixy::Image	* const	im = backend.file.get_image ();
	if ( ! im )
	{
		return;
	}


	mtPixy::Color const * const col = im->get_palette ()->get_color ();
	unsigned char	const	idx = backend.file.brush.get_color_a_index ();
	QColorDialog		dialog ( this );


	dialog.setOptions ( QColorDialog::DontUseNativeDialog );
	dialog.setWindowTitle ( QString ( "Colour A [%1] Editor" ).arg ( idx ));
	dialog.move ( QCursor::pos () );
	dialog.setCurrentColor ( QColor ( col[idx].red, col[idx].green,
		col[idx].blue ) );

	if ( dialog.exec () == QDialog::Accepted )
	{
		QColor		qc = dialog.currentColor ();


		backend.file.palette_load_color ( idx,
			(unsigned char)qc.red (),
			(unsigned char)qc.green (),
			(unsigned char)qc.blue () );

		backend.file.brush.set_color_a ( idx, col );

		update_ui ( UPDATE_TOOLBAR | UPDATE_PALETTE | UPDATE_TITLEBAR |
			UPDATE_STATUS_UNDO | UPDATE_MENUS );
	}
}

void Mainwindow::press_brush_shape ()
{
	int		xx = -1, yy = -1;
	DialogClickImage dialog ( backend.file.brush.get_shapes_palette (), xx,
					yy );


	if ( dialog.exec () == QDialog::Accepted )
	{
		backend.file.brush.set_shape ( xx, yy );
		update_ui ( UPDATE_TOOLBAR );
	}
}

void Mainwindow::press_brush_pattern ()
{
	int		xx = -1, yy = -1;
	DialogClickImage dialog ( backend.file.brush.get_patterns_palette (),
				xx, yy );


	if ( dialog.exec () == QDialog::Accepted )
	{
		backend.file.brush.set_pattern ( xx, yy );
		update_ui ( UPDATE_TOOLBAR );
	}
}

void Mainwindow::press_brush_settings ()
{
	int		sp = backend.file.brush.get_spacing ();
	int		fl = backend.file.brush.get_flow ();
	DialogBrushSettings dialog ( sp, fl );


	if ( dialog.exec () == QDialog::Accepted )
	{
		backend.file.brush.set_spacing ( sp );
		backend.file.brush.set_flow ( fl );
	}
}

void Mainwindow::update_ui (
	int	const	updt
	)
{
	bool		updt_canvas = false;


	// MUST be done first
	if ( updt & UPDATE_BRUSH )
	{
		update_brush_colors ();
	}

	if ( updt & UPDATE_TOOLBAR )
	{
		update_toolbars ();

		switch ( backend.file.get_tool_mode () )
		{
		case mtPixyUI::File::TOOL_MODE_LINING:
		case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
		case mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON:
			updt_canvas = true;

		default:
			break;
		}

		m_cursor.redraw ();
	}

/// ANY ORDER

	if ( (updt & UPDATE_CANVAS) || updt_canvas )
	{
		reconfigure_views ();
	}

	if ( updt & UPDATE_PALETTE )
	{
		rebuild_palette ();
	}

	if ( updt & UPDATE_RECENT_FILES )
	{
		update_recent_files ();
	}

	if ( updt & UPDATE_MENUS )
	{
		update_menus ();
	}

	if ( updt & UPDATE_STATUS_UNDO )
	{
		update_statusbar_undo ();
	}

	if ( updt & UPDATE_STATUS_GEOMETRY )
	{
		update_statusbar_geometry ();
	}

	if ( updt & UPDATE_STATUS_SELECTION )
	{
		update_statusbar_selection ();
	}

	if ( updt & UPDATE_TITLEBAR )
	{
		update_titlebar ();
	}
}

void Mainwindow::press_edit_load_clipboard (
	int	const	num
	)
{
	if ( backend.clipboard.load ( num ) )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to load clipboard %1" ).arg ( num ) );
		return;
	}

	update_ui ( UPDATE_MENUS );

	press_edit_paste_centre ();
}

void Mainwindow::press_edit_save_clipboard (
	int	const	num
	)
{
	if ( backend.clipboard.save ( num ) )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to save clipboard %1" ).arg ( num ) );
	}
}

