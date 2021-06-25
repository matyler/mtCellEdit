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



int mtPixyUI::File::palette_new_chores (
	int	const	num
	)
{
	if ( num )
	{
		return num;
	}

	m_undo_stack.add_next_step ( get_pixmap () );
	m_modified = 1;

	return 0;
}

int mtPixyUI::File::palette_set (
	mtPalette	const * const	pal
	)
{
	mtPixmap * const pixmap = get_pixmap ();
	return palette_new_chores ( pixy_palette_copy ( &pixmap->palette,
		pal ) );
}

int mtPixyUI::File::palette_set_size (
	int	const	num
	)
{
	mtPixmap * const pixmap = get_pixmap ();
	return palette_new_chores ( pixy_palette_set_size (
		&pixmap->palette, num ) );
}

int mtPixyUI::File::palette_load (
	char	const * const	fn
	)
{
	int		res = 1;
	mtPixy::Pixmap	const im ( pixy_pixmap_load ( fn, NULL ) );

	if ( im.get() )
	{
		mtPixmap * const pixmap = get_pixmap ();
		res = palette_new_chores ( pixy_palette_copy (
			& pixmap->palette, &im.get()->palette ) );
	}

	return res;
}

int mtPixyUI::File::palette_save (
	char	const * const	fn
	) const
{
	mtPixmap const * const pixmap = get_pixmap ();
	return pixy_palette_save ( &pixmap->palette, fn );
}

int mtPixyUI::File::palette_load_default (
	int	const	pal_type,
	int	const	pal_num,
	std::string const & pal_filename
	)
{
	pixy_pixmap_palette_set_default ( get_pixmap(), pal_type, pal_num,
		pal_filename.c_str() );

	return palette_new_chores ( 0 );
}

int mtPixyUI::File::palette_load_color (
	unsigned char	const	idx,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b
	)
{
	mtPixmap * const pixmap = get_pixmap ();
	mtColor * const col = &pixmap->palette.color[0];

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
	mtPixmap * const pixmap = get_pixmap ();
	int const idx = pixy_palette_append_color ( &pixmap->palette,
		r, g, b );
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
	int		const	s_type,
	bool		const	reverse
	)
{
	return palette_new_chores ( pixy_pixmap_palette_sort ( get_pixmap (),
		i_start, i_end, s_type, reverse ) );
}

int mtPixyUI::File::palette_merge_duplicates (
	int	* const	tot
	)
{
	return palette_new_chores ( pixy_palette_merge_duplicates (
		get_pixmap (), tot ) );
}

int mtPixyUI::File::palette_remove_unused (
	int	* const	tot
	)
{
	return palette_new_chores ( pixy_palette_remove_unused ( get_pixmap (),
		tot ) );
}

int mtPixyUI::File::palette_create_gradient ()
{
	mtPixmap * const pixmap = get_pixmap ();
	return palette_new_chores ( pixy_palette_create_gradient (
		& pixmap->palette,
		brush.get_color_a_index (), brush.get_color_b_index () ) );
}

int mtPixyUI::File::palette_create_from_canvas ()
{
	return palette_new_chores ( pixmap_palette_create_from_canvas (
		get_pixmap () ) );
}

int mtPixyUI::File::palette_quantize_pnn ()
{
	mtPixmap * const pixmap = get_pixmap ();

	return palette_new_chores ( pixmap_pixmap_quantize_pnn ( pixmap,
		pixmap->palette.size, NULL ) );
}

int mtPixyUI::File::palette_changed ()
{
	return palette_new_chores ( 0 );
}

int mtPixyUI::File::palette_swap_ab ()
{
	mtPixmap const * const pixmap = get_pixmap ();

	if ( ! pixmap )
	{
		return 1;
	}

	unsigned char	const	a = brush.get_color_a_index ();
	unsigned char	const	b = brush.get_color_b_index ();
	mtColor const * const	col = &pixmap->palette.color[0];

	brush.set_color_ab ( b, a, col );

	return 0;
}

static int mask_num (
	char		* const	mem,
	int		const	num,
	mtPixmap const * const	im
	)
{
	if ( ! im )
	{
		return 1;
	}

	memset ( mem, num, (size_t)pixy_pixmap_get_palette_size ( im ) );

	return 0;
}

int mtPixyUI::File::palette_mask_all ()
{
	return mask_num ( palette_mask.color, 1, get_pixmap () );
}

int mtPixyUI::File::palette_unmask_all ()
{
	return mask_num ( palette_mask.color, 0, get_pixmap () );
}

int mtPixyUI::File::update_brush_colors ()
{
	mtPixmap const * const pixmap = get_pixmap ();

	if ( ! pixmap )
	{
		return 1;
	}

	unsigned char	const	a = brush.get_color_a_index ();
	unsigned char	const	b = brush.get_color_b_index ();
	mtColor const * const	col = &pixmap->palette.color[0];

	brush.set_color_ab ( a, b, col );

	return 0;
}

