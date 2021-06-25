/*
	Copyright (C) 2016-2021 Mark Tyler

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

#include "private.h"



void mtPixyUI::File::project_new_chores (
	mtPixmap	* const ni
	)
{
	m_filename.clear();

	m_pixmap.reset ( ni );

	m_undo_stack.clear ();
	m_undo_stack.add_next_step ( ni );

	m_modified = 0;
	m_filetype = PIXY_FILE_TYPE_NONE;

	mtColor	const * const col = & ni->palette.color[0];

	brush.set_color_a ( 1, col );
	brush.set_color_b ( 0, col );

	reset_tool_mode ();
}

int mtPixyUI::File::new_image (
	int		const	bpp,
	int		const	w,
	int		const	h,
	int		const	pal_type,
	int		const	pal_num,
	std::string	const	& pal_filename
	)
{
	mtPixmap * const ni = pixy_pixmap_new ( bpp, w, h );
	if ( ! ni )
	{
		return 1;
	}

	pixy_pixmap_palette_set_default ( ni, pal_type, pal_num,
		pal_filename.c_str() );

	project_new_chores ( ni );

	return 0;
}

int mtPixyUI::File::load_image (
	char	const * const	fn,
	int		const	pal_type,
	int		const	pal_num,
	std::string	const	& pal_filename
	)
{
	int	ft;

	mtPixmap * const pixmap = pixy_pixmap_load ( fn, &ft );
	if ( ! pixmap )
	{
		return 1;
	}

	if ( 0 == pixmap->palette_file )
	{
		pixy_pixmap_palette_set_default ( pixmap, pal_type, pal_num,
			pal_filename.c_str() );
	}

	project_new_chores ( pixmap );

	// Deliberately avoid keeping the palette filename to help avoid data
	// loss when trying to save a canvas to a GPL file.
	if ( ft != PIXY_FILE_TYPE_GPL )
	{
		m_filename = fn;
		m_filetype = ft;
	}

	return 0;
}

void mtPixyUI::File::set_pixmap (
	mtPixmap	* const pixmap
	)
{
	project_new_chores ( pixmap );

	m_modified = 1;
}

int mtPixyUI::File::save_image (
	char	const * const	fn,
	int		const	ft,
	int		const	comp
	)
{
	if ( pixy_pixmap_save ( get_pixmap(), fn, ft, comp ) )
	{
		return 1;
	}

	m_filetype = ft;

	if ( fn != m_filename.c_str() )
	{
		m_filename = fn;
	}

	m_modified = 0;

	return 0;
}

int mtPixyUI::File::export_undo_images (
	char	const * const	fn
	) const
{
	if ( ! fn )
	{
		return 1;
	}

	std::string	const	path ( fn );
	UndoStep		* step = m_undo_stack.get_step_current ();
	char			buf[32];

	for ( int i = m_undo_stack.get_undo_steps (); i >= 0; i-- )
	{
		if ( ! step )
		{
			return 1;
		}

		mtPixmap const * const pixmap = step->get_pixmap ();
		if ( ! pixmap )
		{
			return 1;
		}

		snprintf ( buf, sizeof(buf), "_%04i.png", i );

		std::string const filename ( path + std::string (buf) );
		int const res = pixy_pixmap_save_png ( pixmap, filename.c_str(),
			5 );

		if ( res )
		{
			return 1;
		}

		step = step->get_step_previous ();
	}

	return 0;
}

mtPalette * mtPixyUI::File::get_palette () const
{
	return pixy_pixmap_get_palette ( get_pixmap() );
}

int mtPixyUI::File::undo ()
{
	if ( m_undo_stack.undo ( m_pixmap ) )
	{
		return 1;
	}

	m_modified = 1;

	brush.set_color_ab (
		brush.get_color_a_index (),
		brush.get_color_b_index (),
		&get_palette ()->color[0] );

	return 0;
}

int mtPixyUI::File::redo ()
{
	if ( m_undo_stack.redo ( m_pixmap ) )
	{
		return 1;
	}

	m_modified = 1;

	brush.set_color_ab (
		brush.get_color_a_index (),
		brush.get_color_b_index (),
		&get_palette ()->color[0] );

	return 0;
}

int mtPixyUI::File::get_undo_steps () const
{
	return m_undo_stack.get_undo_steps ();
}

double mtPixyUI::File::get_undo_mb () const
{
	int64_t const ub =
		m_undo_stack.get_undo_bytes () +
		m_undo_stack.get_redo_bytes () +
		m_undo_stack.get_canvas_bytes ();
		;

	return (double)ub / 1024 / 1024;
}

int mtPixyUI::File::get_redo_steps () const
{
	return m_undo_stack.get_redo_steps ();
}

void mtPixyUI::File::set_undo_mb_max (
	int	const	num
	)
{
	m_undo_stack.set_max_bytes ( ((int64_t)num) * 1024 * 1024 );
}

void mtPixyUI::File::set_undo_steps_max (
	int	const	num
	)
{
	m_undo_stack.set_max_steps ( num );
}

char * mtPixyUI::File::get_correct_filename (
	char	const * const	filename,
	int		const	filetype
	)
{
	switch ( filetype )
	{
	case PIXY_FILE_TYPE_BMP:
		return mtkit_set_filename_extension ( filename, "bmp", NULL,
			NULL );

	case PIXY_FILE_TYPE_PNG:
		return mtkit_set_filename_extension ( filename, "png", NULL,
			NULL );

	case PIXY_FILE_TYPE_JPEG:
		return mtkit_set_filename_extension ( filename, "jpeg", "jpg",
			NULL );

	case PIXY_FILE_TYPE_GIF:
		return mtkit_set_filename_extension ( filename, "gif", NULL,
			NULL );

	case PIXY_FILE_TYPE_GPL:
		return mtkit_set_filename_extension ( filename, "gpl", NULL,
			NULL );

	default:
		break;
	}

	return NULL;
}

