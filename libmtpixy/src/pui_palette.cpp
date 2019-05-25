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



int mtPixyUI::File::palette_new_chores (
	int	const	num
	)
{
	if ( num )
	{
		return num;
	}

	m_undo_stack.add_next_step ( m_image );
	m_modified = 1;

	return 0;
}

int mtPixyUI::File::palette_set (
	mtPixy::Palette	const * const	pal
	)
{
	return palette_new_chores ( m_image->get_palette ()->copy ( pal ) );
}

int mtPixyUI::File::palette_set_size (
	int	const	num
	)
{
	return palette_new_chores ( m_image->get_palette ()->
		set_color_total ( num ) );
}

int mtPixyUI::File::palette_load (
	char	const * const	fn
	)
{
	int		res = 1;
	mtPixy::Image	* im = mtPixy::Image::load ( fn );

	if ( im )
	{
		res = palette_new_chores ( m_image->get_palette ()->
			copy ( im->get_palette () ) );

		delete im;
		im = NULL;
	}

	return res;
}

int mtPixyUI::File::palette_save (
	char	const * const	fn
	)
{
	return m_image->get_palette ()->save ( fn );
}

int mtPixyUI::File::palette_load_default (
	int	const	pal_type,
	int	const	pal_num
	)
{
	m_image->palette_set_default ( pal_type, pal_num );

	return palette_new_chores ( 0 );
}

int mtPixyUI::File::palette_load_color (
	unsigned char	const	idx,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	mtPixy::Color * const col = m_image->get_palette ()->get_color ();
	if ( ! col )
	{
		return 1;
	}

	col[idx].red = r;
	col[idx].green = g;
	col[idx].blue = b;

	return palette_new_chores ( 0 );
}

int mtPixyUI::File::palette_append (
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	int const idx = m_image->get_palette ()->append_color ( r, g, b );
	if ( idx >= 0 )
	{
		palette_new_chores ( 0 );
		return idx;
	}

	return -1;
}

int mtPixyUI::File::palette_sort (
	unsigned char	const	i_start,
	unsigned char	const	i_end,
	mtPixy::Image::PaletteSortType	const	s_type,
	bool		const	reverse
	)
{
	return palette_new_chores ( m_image->palette_sort ( i_start, i_end,
		s_type, reverse ) );
}

int mtPixyUI::File::palette_merge_duplicates (
	int	* const	tot
	)
{
	return palette_new_chores ( m_image->palette_merge_duplicates ( tot ) );
}

int mtPixyUI::File::palette_remove_unused (
	int	* const	tot
	)
{
	return palette_new_chores ( m_image->palette_remove_unused ( tot ) );
}

int mtPixyUI::File::palette_create_gradient ()
{
	return palette_new_chores ( m_image->get_palette ()->create_gradient (
		brush.get_color_a_index (), brush.get_color_b_index () ) );
}

int mtPixyUI::File::palette_create_from_canvas ()
{
	return palette_new_chores ( m_image->palette_create_from_canvas () );
}

int mtPixyUI::File::palette_quantize_pnn ()
{
	return palette_new_chores ( m_image->quantize_pnn (
		m_image->get_palette ()->get_color_total () ) );
}

int mtPixyUI::File::palette_changed ()
{
	return palette_new_chores ( 0 );
}

int mtPixyUI::File::palette_swap_ab ()
{
	if ( ! m_image )
	{
		return 1;
	}

	unsigned char	const	a = brush.get_color_a_index ();
	unsigned char	const	b = brush.get_color_b_index ();
	mtPixy::Color * const col = m_image->get_palette ()->get_color ();

	brush.set_color_ab ( b, a, col );

	return 0;
}

static int mask_num (
	char		* const	mem,
	int		const	num,
	mtPixy::Image	* const	im
	)
{
	if ( ! im )
	{
		return 1;
	}

	memset ( mem, num, (size_t)im->get_palette ()->get_color_total () );

	return 0;
}

int mtPixyUI::File::palette_mask_all ()
{
	return mask_num ( palette_mask.color, 1, m_image );
}

int mtPixyUI::File::palette_unmask_all ()
{
	return mask_num ( palette_mask.color, 0, m_image );
}

int mtPixyUI::File::update_brush_colors ()
{
	if ( ! m_image )
	{
		return 1;
	}

	unsigned char	const a = brush.get_color_a_index ();
	unsigned char	const b = brush.get_color_b_index ();
	mtPixy::Color	* const col = m_image->get_palette ()->get_color ();

	brush.set_color_ab ( a, b, col );

	return 0;
}

