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

#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>

#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>

#include "mtpixy.h"




#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API

struct mtPixmap
{
	int		width;
	int		height;

	int		bpp;		// Bytes per pixel: 1=Indexed 3=RGB
	int		palette_file;	// 1=Palette loaded from file

	unsigned char	* canvas;
	unsigned char	* alpha;

	mtPalette	palette;
};



enum
{
	PIXY_EFFECT_EDGE_DETECT		= 0,
	PIXY_EFFECT_SHARPEN		= 1,
	PIXY_EFFECT_SOFTEN		= 2,
	PIXY_EFFECT_EMBOSS		= 3
};


mtPixmap * pixy_pixmap_from_data (
	int		type,		// 0=Alpha only 1=Indexed 3=RGB
	int		width,
	int		height,
	unsigned char	* canvas,
	unsigned char	* alpha
	);

int pixy_pixmap_copy_alpha (
	mtPixmap	* dest,
	mtPixmap const	* src
	);

mtPixmap * pixy_pixmap_resize_trim_by_alpha (
	mtPixmap const	* pixmap,
	int		* minx,
	int		* miny
	);

int pixy_pixmap_move_alpha (
	mtPixmap	* dest,	// Move alpha to this image (same geometry)
	mtPixmap	* src
	);

int pixy_palette_merge_duplicates (
	mtPixmap	* pixmap,	// Must be INDEXED image
	int		* tot
	);

int pixy_palette_remove_unused (
	mtPixmap	* pixmap,	// Must be INDEXED image
	int		* tot
	);

int pixmap_palette_create_from_canvas (
	mtPixmap	* pixmap	// Must be RGB image
	);

int pixmap_pixmap_quantize_pnn (
	mtPixmap	* pixmap,	// Must be RGB image
	int		coltot,		// Total colours to quantize to
	mtPalette	* pal		// Destination palette,NULL= Use image
	);


/// PAINTING -------------------------------------------------------------------

void pixy_paint_flow (
	mtPixmap	* alpha_mask,
	int		flow		// brush.get_flow ();
	);

mtPixmap * pixy_effect_transform_color (
	mtPixmap const	* pixmap,
	int		ga,		// Gamma	-100..100
	int		br,		// Brightness	-255..255
	int		co,		// Contrast	-100..100
	int		sa,		// Saturation	-100..100
	int		hu,		// Hue		-1530..1530
	int		po		// Posterize	1..8
	);

mtPixmap * pixy_pixmap_effect_invert (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_effect_crt (
	mtPixmap const	* pixmap,
	int		scale
	);

mtPixmap * pixy_pixmap_effect_rgb_action (
	mtPixmap const	* pixmap,
	int		effect,
	int		it
	);

mtPixmap * pixy_pixmap_effect_edge_detect (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_effect_sharpen (
	mtPixmap const	* pixmap,
	int		n
	);

mtPixmap * pixy_pixmap_effect_soften (
	mtPixmap const	* pixmap,
	int		n
	);

mtPixmap * pixy_pixmap_effect_emboss (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_effect_normalize (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_effect_bacteria (
	mtPixmap const	* pixmap,
	int		n
	);

mtPixmap * pixy_pixmap_flip_horizontally (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_flip_vertically (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_rotate_clockwise (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_rotate_anticlockwise (
	mtPixmap const	* pixmap
	);

int pixy_lasso (
	mtPixmap	* pixmap,
	int		x,
	int		y
	);



/// PASTING --------------------------------------------------------------------

int pixy_pixmap_paste (		// Basic pixel paste (canvas & alpha)
	mtPixmap	* dest,
	mtPixmap const	* src,
	int		x,	// Paste src onto dest starting here
	int		y
	);

int pixy_pixmap_paste_alpha_blend ( // Paste using src alpha channel to
	mtPixmap	* dest,	// blend canvas pixels. Alpha unchanged.
	mtPixmap const	* src,
	int		x,	// Paste src onto dest starting here
	int		y
	);

int pixy_pixmap_paste_alpha_or (// Paste using src alpha channel to
	mtPixmap	* dest,	// or (|) alpha pixels.
	mtPixmap const	* src,
	int		x,	// Paste src onto dest starting here
	int		y
	);



#ifdef __cplusplus
}

// C++ API

namespace mtPixy
{

mtPixmap * text_render_paste (
	int bpp,
	Brush &bru,
	char const * utf8,
	char const * font_name,
	int size,
	int bold,
	int italics,
	Font::StyleUnderline underline,
	int strikethrough
	);

int paste_alpha_pattern (	// Paste using src alpha channel to
	mtPixmap * dest,	// blend canvas pixels with brush
	mtPixmap const * src,	// pattern. Alpha unchanged.
	Brush &bru,		// Paste src onto dest starting here
	int x,
	int y
	);

int paint_flood_fill (
	mtPixmap * pixmap,
	Brush &brush,
	int x,
	int y
	);

}		// namespace mtPixy



namespace mtPixyUI
{

class UndoStep
{
public:
	explicit UndoStep ( mtPixmap * pim );
	~UndoStep ();

	void insert_after ( UndoStep * us );
	int step_restore ( mtPixy::Pixmap & ppim ) const;

	inline mtPixmap * get_pixmap () const { return m_pixmap.get(); }

	UndoStep * get_step_previous () const;
	UndoStep * get_step_next () const;
	int64_t get_canvas_bytes () const;
	void delete_steps_next ();

private:
	void set_canvas_bytes ();

/// ----------------------------------------------------------------------------

	UndoStep	* m_step_previous	= nullptr;
	UndoStep	* m_step_next		= nullptr;

	mtPixy::Pixmap	const m_pixmap;

	// Bytes used by m_pixmap canvas
	int64_t		m_canvas_bytes		= 0;
};

}		// namespace mtPixyUI



#endif		// C++ API



#define PASTE_SETUP						\
	if ( x <= -src->width || y <= -src->height )		\
	{							\
		return 0;					\
	}							\
	int dx1 = x;						\
	int dy1 = y;						\
	int dx2 = x + src->width;				\
	int dy2 = y + src->height;				\
	int sx1 = 0;						\
	int sy1 = 0;						\
	/* Clip copy rectangle to destination */		\
	if ( dx1 < 0 )						\
	{							\
		sx1 = -dx1;					\
		dx1 = 0;					\
	}							\
	if ( dy1 < 0 )						\
	{							\
		sy1 = -dy1;					\
		dy1 = 0;					\
	}							\
	dx2 = MIN ( dx2, dest->width );				\
	dy2 = MIN ( dy2, dest->height );			\
	if (	sx1 >= src->width	||			\
		sy1 >= src->height	||			\
		dx2 < dx1		||			\
		dy2 < dy1					\
		)						\
	{							\
		/* No rectangle overlap so nothing to paste */	\
		return 0;					\
	}							\





#ifndef DEBUG
#pragma GCC visibility pop
#endif

