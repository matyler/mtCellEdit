/*
	Copyright (C) 2016-2018 Mark Tyler

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



mtPixyUI::File::File ()
	:
	m_filename	(),
	m_image		(),
	m_brush_x	( 0 ),
	m_brush_y	( 0 ),
	m_modified	( 0 ),
	m_filetype	( mtPixy::File::TYPE_NONE ),
	m_tool_mode	( TOOL_MODE_PAINT )
{
}

mtPixyUI::File::~File ()
{
	free ( m_filename );
	m_filename = NULL;

	delete m_image;
	m_image = NULL;
}

void mtPixyUI::File::project_new_chores (
	mtPixy::Image	* ni
	)
{
	free ( m_filename );
	m_filename = NULL;

	delete m_image;
	m_image = ni;

	m_undo_stack.clear ();
	m_undo_stack.add_next_step ( m_image );

	m_modified = 0;
	m_filetype = mtPixy::File::TYPE_NONE;


	mtPixy::Color	* col = m_image->get_palette ()->get_color ();


	brush.set_color_a ( 1, col );
	brush.set_color_b ( 0, col );

	reset_tool_mode ();
}

int mtPixyUI::File::new_image (
	mtPixy::Image::Type const imtype,
	int		const	w,
	int		const	h,
	int		const	pal_type,
	int		const	pal_num
	)
{
	mtPixy::Image * const ni = mtPixy::Image::create ( imtype, w, h );
	if ( ! ni )
	{
		return 1;
	}

	ni->palette_set_default ( pal_type, pal_num );

	project_new_chores ( ni );

	return 0;
}

int mtPixyUI::File::load_image (
	char	const * const	fn,
	int		const	pal_type,
	int		const	pal_num
	)
{
	mtPixy::File::Type	ft;

	mtPixy::Image * const ni = mtPixy::Image::load ( fn, &ft );
	if ( ! ni )
	{
		return 1;
	}

	if ( 0 == ni->get_file_flag () )
	{
		ni->palette_set_default ( pal_type, pal_num );
	}

	project_new_chores ( ni );

	// Deliberately avoid keeping the palette filename to help avoid data
	// loss when trying to save a canvas to a GPL file.
	if ( ft != mtPixy::File::TYPE_GPL )
	{
		m_filename = strdup ( fn );
		m_filetype = ft;
	}

	return 0;
}

void mtPixyUI::File::set_image (
	mtPixy::Image	* const im
	)
{
	project_new_chores ( im );

	m_modified = 1;
}

int mtPixyUI::File::save_image (
	char	const * const	fn,
	mtPixy::File::Type const ft,
	int		const	comp
	)
{
	if ( m_image->save ( fn, ft, comp ) )
	{
		return 1;
	}

	m_filetype = ft;

	if ( fn != m_filename )
	{
		free ( m_filename );
		m_filename = strdup ( fn );
	}

	m_modified = 0;

	return 0;
}

int mtPixyUI::File::export_undo_images (
	char	const * const	fn
	)
{
	if ( ! fn )
	{
		return 1;
	}

	UndoStep	* step = m_undo_stack.get_step_current ();
	char		buf[32];

	for ( int i = m_undo_stack.get_undo_steps (); i >= 0; i-- )
	{
		if ( ! step )
		{
			return 1;
		}

		mtPixy::Image * im = step->get_image ();
		if ( ! im )
		{
			return 1;
		}

		snprintf ( buf, sizeof(buf), "_%04i.png", i );

		char * ns = mtkit_string_join ( fn, buf, NULL, NULL );
		if ( ! ns )
		{
			return 1;
		}

		int res = im->save_png ( ns, 5 );
		free ( ns );

		if ( res )
		{
			return 1;
		}

		step = step->get_step_previous ();
	}

	return 0;
}

int mtPixyUI::File::export_colormap (
	char	const * const	fn,
	int		const	comp
	)
{
	if ( ! m_image )
	{
		return 1;
	}

	return m_image->save_bp24 ( fn, comp );
}

mtPixy::Image * mtPixyUI::File::get_image ()
{
	return m_image;
}

mtPixy::Palette * mtPixyUI::File::get_palette ()
{
	if ( ! m_image )
	{
		return NULL;
	}

	return m_image->get_palette ();
}

char const * mtPixyUI::File::get_filename () const
{
	return m_filename;
}

int mtPixyUI::File::get_modified () const
{
	return m_modified;
}

mtPixy::File::Type mtPixyUI::File::get_filetype () const
{
	return m_filetype;
}

int mtPixyUI::File::undo ()
{
	if ( m_undo_stack.undo ( &m_image ) )
	{
		return 1;
	}

	m_modified = 1;

	brush.set_color_ab (
		brush.get_color_a_index (),
		brush.get_color_b_index (),
		m_image->get_palette ()->get_color () );

	return 0;
}

int mtPixyUI::File::redo ()
{
	if ( m_undo_stack.redo ( &m_image ) )
	{
		return 1;
	}

	m_modified = 1;

	brush.set_color_ab (
		brush.get_color_a_index (),
		brush.get_color_b_index (),
		m_image->get_palette ()->get_color () );

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
	char		const * const	filename,
	mtPixy::File::Type	const	filetype
	)
{
	switch ( filetype )
	{
	case mtPixy::File::TYPE_BMP:
		return mtkit_set_filename_extension ( filename, "bmp", NULL,
			NULL );

	case mtPixy::File::TYPE_PNG:
		return mtkit_set_filename_extension ( filename, "png", NULL,
			NULL );

	case mtPixy::File::TYPE_JPEG:
		return mtkit_set_filename_extension ( filename, "jpeg", "jpg",
			NULL );

	case mtPixy::File::TYPE_GIF:
		return mtkit_set_filename_extension ( filename, "gif", NULL,
			NULL );

	case mtPixy::File::TYPE_GPL:
		return mtkit_set_filename_extension ( filename, "gpl", NULL,
			NULL );

	case mtPixy::File::TYPE_PIXY:
		return mtkit_set_filename_extension ( filename, "pixy", NULL,
			NULL );

	case mtPixy::File::TYPE_BP24:
		return mtkit_set_filename_extension ( filename, "bp24", NULL,
			NULL );

	default:
		break;
	}

	return NULL;
}

