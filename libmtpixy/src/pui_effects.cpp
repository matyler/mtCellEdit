/*
	Copyright (C) 2016-2023 Mark Tyler

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
	mtPixmap	* const	pixmap
	)
{
	mtPixy::Pixmap tmp ( pixmap );

	if ( m_undo_stack.add_next_step ( pixmap ) )
	{
		return 1;
	}

	m_pixmap.reset ( tmp.release() );
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
	return image_new_chores( pixy_pixmap_resize( get_pixmap(), x, y, w,h ));
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
	int	const	scaletype
	)
{
	return image_new_chores ( pixy_pixmap_scale ( get_pixmap(), w, h,
		scaletype ) );
}

int mtPixyUI::File::convert_to_rgb ()
{
	return image_new_chores ( pixy_pixmap_convert_to_rgb( get_pixmap() ) );
}

int mtPixyUI::File::convert_to_indexed (
	int	const	dt
	)
{
	return image_new_chores ( pixy_pixmap_convert_to_indexed ( get_pixmap(),
		dt ) );
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
	return image_new_chores ( pixy_effect_transform_color ( get_pixmap(),
		ga, br, co, sa, hu, po ) );
}

int mtPixyUI::File::effect_invert ()
{
	return image_new_chores ( pixy_pixmap_effect_invert ( get_pixmap() ) );
}

int mtPixyUI::File::effect_crt ( int const scale )
{
	return image_new_chores( pixy_pixmap_effect_crt( get_pixmap(), scale ));
}

int mtPixyUI::File::effect_edge_detect ()
{
	return image_new_chores( pixy_pixmap_effect_edge_detect( get_pixmap()));
}

int mtPixyUI::File::effect_sharpen (
	int	const	n
	)
{
	return image_new_chores( pixy_pixmap_effect_sharpen( get_pixmap(), n ));
}

int mtPixyUI::File::effect_soften (
	int	const	n
	)
{
	return image_new_chores ( pixy_pixmap_effect_soften( get_pixmap(), n ));
}

int mtPixyUI::File::effect_emboss ()
{
	return image_new_chores ( pixy_pixmap_effect_emboss ( get_pixmap() ) );
}

int mtPixyUI::File::effect_normalize ()
{
	return image_new_chores ( pixy_pixmap_effect_normalize( get_pixmap() ));
}

int mtPixyUI::File::effect_bacteria (
	int	const	n
	)
{
	return image_new_chores( pixy_pixmap_effect_bacteria( get_pixmap(), n));
}

int mtPixyUI::File::flip_horizontally ()
{
	return image_new_chores( pixy_pixmap_flip_horizontally( get_pixmap() ));
}

int mtPixyUI::File::flip_vertically ()
{
	return image_new_chores ( pixy_pixmap_flip_vertically( get_pixmap() ) );
}

int mtPixyUI::File::rotate_clockwise ()
{
	return image_new_chores ( pixy_pixmap_rotate_clockwise( get_pixmap() ));
}

int mtPixyUI::File::rotate_anticlockwise ()
{
	return image_new_chores ( pixy_pixmap_rotate_anticlockwise (
		get_pixmap() ) );
}

int mtPixyUI::File::destroy_alpha ()
{
	if ( 0 == pixy_pixmap_destroy_alpha ( get_pixmap() ) )
	{
		m_undo_stack.add_next_step ( get_pixmap() );
		m_modified = 1;

		return 0;
	}

	return 1;
}

int mtPixyUI::File::effect_equalize_image_info (
	unsigned char rgb_min_max[6]
	) const
{
	return pixy_pixmap_equalize_image_info ( get_pixmap(), rgb_min_max );
}

int mtPixyUI::File::effect_equalize_palette_info (
	unsigned char rgb_min_max[6]
	) const
{
	return pixy_pixmap_equalize_palette_info ( get_pixmap(), rgb_min_max );
}

int mtPixyUI::File::effect_equalize_image (
	unsigned char const rgb_min_max[6]
	)
{
	return image_new_chores ( pixy_pixmap_equalize_image (
		get_pixmap(), rgb_min_max ) );
}

int mtPixyUI::File::effect_equalize_palette (
	unsigned char const rgb_min_max[6]
	)
{
	return image_new_chores ( pixy_pixmap_equalize_palette (
		get_pixmap(), rgb_min_max ) );
}

