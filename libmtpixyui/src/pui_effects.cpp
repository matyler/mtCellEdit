/*
	Copyright (C) 2016-2017 Mark Tyler

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



int mtPixyUI::File::image_new_chores (
	mtPixy::Image	* const	i
	)
{
	if ( ! i )
	{
		return 1;
	}

	if ( m_undo_stack.add_next_step ( i ) )
	{
		delete i;
		return 1;
	}

	delete ( m_image );
	m_image = i;

	m_modified = 1;

	return 0;
}

int mtPixyUI::File::resize (
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h
	)
{
	return image_new_chores ( m_image->resize ( x, y, w, h ) );
}

int mtPixyUI::File::crop ()
{
	int		xx = 0, yy = 0, w = 1, h = 1;


	switch ( m_tool_mode )
	{
	case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
		rectangle_overlay.get_xywh ( xx, yy, w, h );
		break;

	case mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON:
		polygon_overlay.get_xywh ( xx, yy, w, h );
		break;

	default:
		return 1;
	}

	return resize ( xx, yy, w, h );
}

int mtPixyUI::File::scale (
	int	const	w,
	int	const	h,
	mtPixy::Image::ScaleType const	scaletype
	)
{
	return image_new_chores ( m_image->scale ( w, h, scaletype ) );
}

int mtPixyUI::File::convert_to_rgb ()
{
	return image_new_chores ( m_image->convert_to_rgb () );
}

int mtPixyUI::File::convert_to_indexed (
	mtPixy::Image::DitherType const dt
	)
{
	return image_new_chores ( m_image->convert_to_indexed ( dt ) );
}

int mtPixyUI::File::effect_transform_color (
	int	const	ga,
	int	const	br,
	int	const	co,
	int	const	sa,
	int	const	hu,
	int	const	po
	)
{
	return image_new_chores ( m_image->effect_transform_color ( ga, br, co,
		sa, hu, po ) );
}

int mtPixyUI::File::effect_invert ()
{
	return image_new_chores ( m_image->effect_invert () );
}

int mtPixyUI::File::effect_edge_detect ()
{
	return image_new_chores ( m_image->effect_edge_detect () );
}

int mtPixyUI::File::effect_sharpen (
	int	const	n
	)
{
	return image_new_chores ( m_image->effect_sharpen ( n ) );
}

int mtPixyUI::File::effect_soften (
	int	const	n
	)
{
	return image_new_chores ( m_image->effect_soften ( n ) );
}

int mtPixyUI::File::effect_emboss ()
{
	return image_new_chores ( m_image->effect_emboss () );
}

int mtPixyUI::File::effect_normalize ()
{
	return image_new_chores ( m_image->effect_normalize () );
}

int mtPixyUI::File::effect_bacteria (
	int	const	n
	)
{
	return image_new_chores ( m_image->effect_bacteria ( n ) );
}

int mtPixyUI::File::flip_horizontally ()
{
	return image_new_chores ( m_image->flip_horizontally () );
}

int mtPixyUI::File::flip_vertically ()
{
	return image_new_chores ( m_image->flip_vertically () );
}

int mtPixyUI::File::rotate_clockwise ()
{
	return image_new_chores ( m_image->rotate_clockwise () );
}

int mtPixyUI::File::rotate_anticlockwise ()
{
	return image_new_chores ( m_image->rotate_anticlockwise () );
}

int mtPixyUI::File::destroy_alpha ()
{
	if ( 0 == m_image->destroy_alpha () )
	{
		m_undo_stack.add_next_step ( m_image );
		m_modified = 1;

		return 0;
	}

	return 1;
}

