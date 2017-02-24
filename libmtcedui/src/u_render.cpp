/*
	Copyright (C) 2011-2016 Mark Tyler

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



static int cui_ren_cellwidth (		// Pixel width of this column
	CuiRender	* viewren,
	int		column
	);



/// CAIRO

typedef struct mrenSTATE mrenSTATE;
struct mrenSTATE
{
	int		row_start,
			col_start;
	CuiRender	* viewren;
	int		x,
			y,
			w,
			h
			;
	unsigned char	* rgb,
			* mem,
			* cell_active
			;
	int		r1,
			r2,
			c1,
			c2,
			row_h,
			cwidth,
			justify,
			mx,
			cur_row1,
			cur_row2,
			cur_col1,
			cur_col2,
			* col_x,
			* col_w
			;
	mtPixy::Image	* image;

	FILE		* fp;

	mrenSTATE ();
};

mrenSTATE::mrenSTATE ()
	:
	row_start	( 0 ),
	col_start	( 0 ),
	viewren		(),
	x		( 0 ),
	y		( 0 ),
	w		( 0 ),
	h		( 0 ),
	rgb		(),
	mem		(),
	cell_active	(),
	r1		( 0 ),
	r2		( 0 ),
	c1		( 0 ),
	c2		( 0 ),
	row_h		( 0 ),
	cwidth		( 0 ),
	justify		( 0 ),
	mx		( 0 ),
	cur_row1	( 0 ),
	cur_row2	( 0 ),
	cur_col1	( 0 ),
	cur_col2	( 0 ),
	col_x		(),
	col_w		(),
	image		(),
	fp		()
{
}



typedef struct outSTATE	outSTATE;
struct outSTATE
{
	mrenSTATE	mren_state;
	CuiRenPage	* page;		// Only used in multi-page PDF's

	int		filetype;
	CedSheet	* sheet;

	double		cell_pad,
			dx,
			dy,
			dw,
			dh,

			* colx,

			page_width,
			page_height,	// In points

			pgv_xl,
			pgv_xr,		// Visible left/right

			pgv_yh,
			pgv_ytop,	// Header/top Y

			pgv_yf,
			pgv_ybot	// Footer/bottom Y
			;

	int		rows,
			cols,		// Total in this sheet
			page_num,
			pages_total,
			rows_per_page,
			cols_per_page,
			row_origin,
			col_origin,
			row_pad		// Gap at top/bottom of cell
			;

	char	const	* head_txt[3];
	char	const	* foot_txt[3];
	char		date_txt[128],
			datetime_txt[128],
			pagenum_txt[128]
			;


	cairo_surface_t		* surface;
	cairo_t			* cr;

	char	const		* font_name;	// Usually "Sans"
	int			font_size;	// Usually 12

	PangoFontDescription	* font_desc;
	PangoLayout		* p_layout;

	PangoRectangle		logical_rect;
	int			std_baseline;

	int			row_height;
	int			row_y_start;
	int			glyph_w;

	outSTATE ();
};

outSTATE::outSTATE ()
	:
	page		(),
	filetype	( 0 ),
	sheet		(),
	cell_pad	(),
	dx		(),
	dy		(),
	dw		(),
	dh		(),
	colx		(),
	page_width	( 0.0 ),
	page_height	( 0.0 ),
	pgv_xl		( 0.0 ),
	pgv_xr		( 0.0 ),
	pgv_yh		( 0.0 ),
	pgv_ytop	( 0.0 ),
	pgv_yf		( 0.0 ),
	pgv_ybot	( 0.0 ),
	rows		( 0 ),
	cols		( 0 ),
	page_num	( 0 ),
	pages_total	( 0 ),
	rows_per_page	( 0 ),
	cols_per_page	( 0 ),
	row_origin	( 0 ),
	col_origin	( 0 ),
	row_pad		( 0 ),
	head_txt	(),
	foot_txt	(),
	date_txt	(),
	datetime_txt	(),
	pagenum_txt	(),
	surface		(),
	cr		(),
	font_name	(),
	font_size	( 0 ),
	font_desc	(),
	p_layout	(),
	std_baseline	( 0 ),
	row_height	( 0 ),
	row_y_start	( 0 ),
	glyph_w		( 0 )
{
	logical_rect.x = 0;
	logical_rect.y = 0;
	logical_rect.width = 0;
	logical_rect.height = 0;
}


static int export_cairo_output_back_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	outSTATE	* const	state = (outSTATE *)user_data;
	int		r = 255,
			g = 255,
			b = 255;


	if ( cell->text )
	{
		state->mren_state.cell_active[col - 1] = 1;
	}

///	RENDER BACKGROUND

	if ( cell->prefs )
	{
		r = mtPixy::int_2_red ( cell->prefs->color_background );
		g = mtPixy::int_2_green ( cell->prefs->color_background );
		b = mtPixy::int_2_blue ( cell->prefs->color_background );
	}

	// Geometry of this cell
	state->dx = state->pgv_xl + state->colx[ col - 1 ] -
		state->colx[ state->mren_state.c1 - 1 ];
	state->dy = state->pgv_ytop + (row - state->mren_state.r1) *
		state->row_height;
	state->dw = state->mren_state.col_w[ col - 1 ] * state->glyph_w;
	state->dh = state->row_height;

	if (	r != 255 ||
		g != 255 ||
		b != 255
		)
	{
		// Output a coloured rectangle for this cell
		cairo_set_source_rgb ( state->cr, r / 255.0, g / 255.0,
			b / 255.0 );

		cairo_rectangle ( state->cr, state->dx, state->dy, state->dw,
			state->dh );

		cairo_fill ( state->cr );
	}

	return 0;
}


static void exp_cairo_render_border (
	int		const	type,
	int		const	ARG_UNUSED ( col ),
	outSTATE	* const	state
	)
{
	double		co[3],
			d;
	int		i,
			hori[3] = {
		(type >> CED_CELL_BORDER_TOP_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_MIDDLE_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_BOTTOM_SHIFT) & CED_CELL_BORDER_MASK
			},
			vert[3] = {
		(type >> CED_CELL_BORDER_LEFT_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_CENTER_SHIFT) & CED_CELL_BORDER_MASK,
		(type >> CED_CELL_BORDER_RIGHT_SHIFT) & CED_CELL_BORDER_MASK
			};


	co[0] = state->dy + 0.5;
	co[1] = state->dy + state->dh / 2 - 1;
	co[2] = state->dy + state->dh - 2.5;

	for ( i = 0; i < 3; i++ )
	{
		if ( hori[i] == CED_CELL_BORDER_THIN )
		{
			d = co[i] + i;

			cairo_move_to ( state->cr, state->dx, d );
			cairo_line_to ( state->cr, state->dx + state->dw, d );
			cairo_stroke ( state->cr );
		}
		else if ( hori[i] == CED_CELL_BORDER_THICK )
		{
			d = co[i] + ( (double)i ) / 2;

			cairo_move_to ( state->cr, state->dx, d );
			cairo_line_to ( state->cr, state->dx + state->dw, d );
			cairo_stroke ( state->cr );

			cairo_move_to ( state->cr, state->dx, d + 1 );
			cairo_line_to ( state->cr, state->dx + state->dw,
				d + 1 );
			cairo_stroke ( state->cr );
		}
		else if ( hori[i] == CED_CELL_BORDER_DOUBLE )
		{
			cairo_move_to ( state->cr, state->dx, co[i] );
			cairo_line_to ( state->cr, state->dx + state->dw,
				co[i] );
			cairo_stroke ( state->cr );

			cairo_move_to ( state->cr, state->dx, co[i] + 2 );
			cairo_line_to ( state->cr, state->dx + state->dw,
				co[i] + 2 );
			cairo_stroke ( state->cr );
		}
	}

	co[0] = state->dx + 0.5;
	co[1] = state->dx + state->dw / 2 - 1;
	co[2] = state->dx + state->dw - 2.5;

	for ( i = 0; i < 3; i++ )
	{
		if ( vert[i] == CED_CELL_BORDER_THIN )
		{
			d = co[i] + i;

			cairo_move_to ( state->cr, d, state->dy );
			cairo_line_to ( state->cr, d, state->dy + state->dh );
			cairo_stroke ( state->cr );
		}
		else if ( vert[i] == CED_CELL_BORDER_THICK )
		{
			d = co[i] + ( (double)i ) / 2;

			cairo_move_to ( state->cr, d, state->dy );
			cairo_line_to ( state->cr, d, state->dy + state->dh );
			cairo_stroke ( state->cr );

			cairo_move_to ( state->cr, d + 1, state->dy );
			cairo_line_to ( state->cr, d + 1,
				state->dy + state->dh);
			cairo_stroke ( state->cr );
		}
		else if ( vert[i] == CED_CELL_BORDER_DOUBLE )
		{
			cairo_move_to ( state->cr, co[i], state->dy );
			cairo_line_to ( state->cr, co[i],
				state->dy + state->dh);
			cairo_stroke ( state->cr );

			cairo_move_to ( state->cr, co[i] + 2, state->dy );
			cairo_line_to ( state->cr, co[i] + 2,
				state->dy + state->dh );
			cairo_stroke ( state->cr );
		}
	}
}

// Convert mtCellEdit packed text style into mtPixy::Font style
static void font_set_attr (
	mtPixy::Font * const	font,
	int		const	input
	)
{
	int		bo=0, it=0, st=0;
	mtPixy::Font::StyleUnderline un = mtPixy::Font::STYLE_UNDERLINE_NONE;


	if ( CED_TEXT_STYLE_IS_BOLD ( input ) )
	{
		bo = 1;
	}

	if ( CED_TEXT_STYLE_IS_ITALIC ( input ) )
	{
		it = 1;
	}

	if ( CED_TEXT_STYLE_IS_UNDERLINE ( input ) )
	{
		switch ( input & CED_TEXT_STYLE_UNDERLINE_ANY )
		{
		case CED_TEXT_STYLE_UNDERLINE_DOUBLE:
			un = mtPixy::Font::STYLE_UNDERLINE_DOUBLE;
			break;

		case CED_TEXT_STYLE_UNDERLINE_WAVY:
			un = mtPixy::Font::STYLE_UNDERLINE_WAVY;
			break;

		default:
			un = mtPixy::Font::STYLE_UNDERLINE_SINGLE;
			break;
		}
	}

	if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( input ) )
	{
		st = 1;
	}

	font->set_style ( bo, it, un, st );
}

// FIXME - can we merge cui_font_set_attr() & font_set_attr() ???????????
static void cui_font_set_attr (
	PangoLayout	* const	layout,
	int		const	attr_val	// CED_TEXT_STYLE_* bit fields
	)
{
	PangoAttrList	* list;
	PangoAttribute	* a;


	list = pango_attr_list_new ();

	if ( CED_TEXT_STYLE_IS_BOLD ( attr_val ) )
	{
		a = pango_attr_weight_new ( PANGO_WEIGHT_BOLD );
		pango_attr_list_insert ( list, a );
	}

	if ( CED_TEXT_STYLE_IS_ITALIC ( attr_val ) )
	{
		a = pango_attr_style_new ( PANGO_STYLE_ITALIC );
		pango_attr_list_insert ( list, a );
	}

	if ( CED_TEXT_STYLE_IS_UNDERLINE ( attr_val ) )
	{
		switch ( attr_val & CED_TEXT_STYLE_UNDERLINE_ANY )
		{
		case CED_TEXT_STYLE_UNDERLINE_DOUBLE:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_DOUBLE );
			break;

		case CED_TEXT_STYLE_UNDERLINE_WAVY:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_ERROR );
			break;

		default:
			a = pango_attr_underline_new ( PANGO_UNDERLINE_SINGLE );
			break;
		}

		pango_attr_list_insert ( list, a );
	}

	if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( attr_val ) )
	{
		a = pango_attr_strikethrough_new ( TRUE );
		pango_attr_list_insert ( list, a );
	}

	pango_layout_set_attributes ( layout, list );
	pango_attr_list_unref ( list );
}

static int export_cairo_output_fore_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	outSTATE	* const	state = (outSTATE *)user_data;
	char		* new_text = NULL, * txt;
	int		r = 0, g = 0, b = 0, text_style = 0, yoff = 0;
	double		x, y, xo, vis_right;


	xo = state->pgv_xl - state->colx[ state->mren_state.c1 - 1 ];

	// Right hand side of rightmost visible cell
	vis_right = xo + state->colx[ state->mren_state.c2 - 1 ] +
		state->mren_state.col_w[ state->mren_state.c2 - 1 ] *
		state->glyph_w;

	// Geometry of this cell
	state->dx = xo + state->colx[ col - 1 ];
	state->dy = state->pgv_ytop + (row - state->mren_state.r1) *
		state->row_height;
	state->dw = state->mren_state.col_w[ col - 1 ] * state->glyph_w;
	state->dh = state->row_height;

///	RENDER BORDER

	if (	cell->prefs &&
		cell->prefs->border_type != 0 &&
		col >= state->mren_state.c1
		)		// Border must exist in visible range
	{
		r = mtPixy::int_2_red ( cell->prefs->border_color );
		g = mtPixy::int_2_green ( cell->prefs->border_color );
		b = mtPixy::int_2_blue ( cell->prefs->border_color );

		cairo_set_line_width ( state->cr, 1 );
		cairo_set_source_rgb ( state->cr, r / 255.0, g / 255.0,
			b / 255.0 );

		exp_cairo_render_border ( cell->prefs->border_type, col,
			state );
	}

///	RENDER FOREGROUND

	if ( ! cell->text )
	{
		return 0;
	}

	txt = ced_cell_create_output ( cell, &state->mren_state.justify );
	if ( ! txt )
	{
		return 1;
	}

	new_text = txt;

	if ( cell->prefs )
	{
		r = mtPixy::int_2_red ( cell->prefs->color_foreground );
		g = mtPixy::int_2_green ( cell->prefs->color_foreground );
		b = mtPixy::int_2_blue ( cell->prefs->color_foreground );

		text_style = cell->prefs->text_style;
	}
	else
	{
		r = g = b = text_style = 0;
	}

	cairo_set_source_rgb ( state->cr, r / 255.0, g / 255.0, b / 255.0 );
	pango_layout_set_text ( state->p_layout, txt, -1 );

	cui_font_set_attr ( state->p_layout, text_style );

	pango_layout_set_font_description ( state->p_layout, state->font_desc );
	pango_layout_get_extents ( state->p_layout, NULL, &state->logical_rect);
	pango_extents_to_pixels ( NULL, &state->logical_rect );

	switch ( state->mren_state.justify )
	{
	case CED_CELL_JUSTIFY_RIGHT:
		x = state->dx + state->dw - state->logical_rect.width -
			state->cell_pad;
		break;

	case CED_CELL_JUSTIFY_CENTER:
		x = state->dx + (state->dw - state->logical_rect.width) / 2;
		break;

	case CED_CELL_JUSTIFY_LEFT:
		/* FALL THROUGH */

	default:
		x = state->dx + state->cell_pad;

		break;
	}

	yoff = PANGO_PIXELS ( pango_layout_get_baseline ( state->p_layout ) );
	y = state->dy + state->row_y_start + state->std_baseline - yoff;

	// Left side cell expansion
	for (	g = col-2;	// -1 for shifting to array number,
				// -1 for shifting to next column on the left
		g >= 0 &&
		x < state->dx &&
		state->mren_state.cell_active[g] == 0;

		g-- )
	{
		state->dx = state->colx[g];
		state->dw += state->mren_state.col_w[g] * state->glyph_w;

		// state->cell_active[g] = 1;
		// Not required for leftwards, only rightwards
	}


	// Right side cell expansion
	for (	g = col;	// -1 for shifting to array number,
				// +1 for shifting to next column on the right
		g < state->cols &&
		(x + state->logical_rect.width) > (xo + state->colx[g]) &&

		state->mren_state.cell_active[g] == 0;

		g++ )
	{
		state->dw += state->mren_state.col_w[g] * state->glyph_w;

		// Don't allow reuse of this cell later
		state->mren_state.cell_active[g] = 1;
	}


	// Bail out if the text is not visible on the page
	if (	x > vis_right ||
		(x + state->logical_rect.width) < state->pgv_xl
		)
	{
		goto finish;
	}

	// Left side clipping, by page visibility
	if ( state->dx < state->pgv_xl )
	{
		state->dw -= (state->pgv_xl - state->dx);
		state->dx = state->pgv_xl;
	}

	// Right side clipping, by page visibility
	if ( (state->dx + state->dw) > vis_right )
	{
		state->dw = vis_right - state->dx;
	}

	cairo_rectangle ( state->cr, state->dx, state->dy, state->dw,
		state->dh );
	cairo_clip ( state->cr );

	cairo_save ( state->cr );

	cairo_translate ( state->cr, x, y );
	pango_cairo_update_layout ( state->cr, state->p_layout );
	pango_cairo_show_layout ( state->cr, state->p_layout );

	cairo_restore ( state->cr );

finish:
	cairo_reset_clip ( state->cr );
	free ( new_text );

	return 0;			// Continue
}

static int col_width_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	if (	cell->prefs &&
		cell->prefs->width > 0
		)
	{
		outSTATE	* const	state = (outSTATE *)user_data;


		state->mren_state.col_w[ col - 1 ] = cell->prefs->width;
	}

	return 0;		// continue
}

static int outside_visible_area_cb (
	CedSheet	* const	sheet,
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	if ( ! cell->text )
	{
		return 0;	// Nothing to render so continue
	}

	export_cairo_output_fore_cb ( sheet, cell, row, col, user_data );

	return 1;		// Stop here as we have rendered something
}

static int ced_ren_page_cols_get (
	int		const	col,
	outSTATE	* const	state
	)
{
	int		i;
	double		cw,
			visible_width;


	visible_width = state->pgv_xr - state->pgv_xl;

	for ( i = col; i < state->cols; i++ )
	{
		cw = state->colx[i] - state->colx[col] +
			state->mren_state.col_w[i] * state->glyph_w;

		if ( cw > visible_width )
		{
			break;
		}
	}

	if ( i == col )
	{
		// This single column is greater than the page width

		return 1;
	}

	return (i - col);
}

static int ced_ren_page_recurse_rows (
	mtTreeNode	* const	node,
	outSTATE	* const	state
	)
{
	int		res,
			row;


	row = (int)(intptr_t)node->key;

	if ( node->left && row > state->mren_state.r1 )
	{
		res = ced_ren_page_recurse_rows ( node->left, state );
		if ( res )
		{
			return res;
		}
	}

	if (	row >= state->mren_state.r1 &&
		row <= state->mren_state.r2
		)
	{
		memset ( state->mren_state.cell_active,0, (size_t)state->cols );

		res = ced_sheet_scan_area ( state->sheet, row,
			state->mren_state.c1, 1, state->cols_per_page,
			export_cairo_output_back_cb, state );

		if ( res )
		{
			return res;
		}

		if (	state->mren_state.c1 > 1 &&
			state->mren_state.cell_active[
				state->mren_state.c1 - 1 ] == 0
			)
		{
			// Scan to left of visible area to catch long strings

			ced_sheet_scan_area_backwards ( state->sheet,
				row, state->mren_state.c1 - 1,
				1, 0, outside_visible_area_cb, state );
		}

		// Render all text in visible cells
		res = ced_sheet_scan_area ( state->sheet, row,
			state->mren_state.c1, 1, state->cols_per_page,
			export_cairo_output_fore_cb, state );

		if ( res )
		{
			return res;
		}

		if (	state->mren_state.c2 < state->cols &&
			state->mren_state.cell_active[
				state->mren_state.c2 - 1 ] == 0
			)
		{
			// Scan to right of visible area to catch long strings
			// that become visible.

			ced_sheet_scan_area ( state->sheet,
				row, state->mren_state.c2 + 1, 1, 0,
				outside_visible_area_cb, state );
		}
	}

	if ( node->right && row < state->mren_state.r2 )
	{
		res = ced_ren_page_recurse_rows ( node->right, state );
		if ( res )
		{
			return res;
		}
	}

	return 0;
}

static void ced_ren_page_cleanup (
	outSTATE	* const	state,
	int		const	all
	)
{
	if ( all )
	{
		free ( state->mren_state.col_w );
		free ( state->colx );
		free ( state->mren_state.cell_active );

		state->mren_state.col_w = NULL;
		state->colx = NULL;
		state->mren_state.cell_active = NULL;
	}

	cairo_destroy ( state->cr );
	cairo_surface_destroy ( state->surface );

	state->cr = NULL;
	state->surface = NULL;
}

static void ced_ren_font_cleanup (
	outSTATE	* const	state
	)
{
	if ( state->font_desc )
	{
		pango_font_description_free ( state->font_desc );
		state->font_desc = NULL;
	}

	if ( state->p_layout )
	{
		g_object_unref ( state->p_layout );
		state->p_layout = NULL;
	}
}

static int cairo_page_init_font_extent (
	outSTATE	* const	state
	)
{
	PangoRectangle	logical;


	pango_layout_set_text ( state->p_layout, "0123456789", -1 );
	pango_font_description_set_weight ( state->font_desc,
		PANGO_WEIGHT_NORMAL );
	pango_font_description_set_size ( state->font_desc,
		(gint)( ((double)state->font_size) * PANGO_SCALE * 0.75 ) );
	pango_layout_set_font_description ( state->p_layout, state->font_desc );

	pango_layout_get_extents ( state->p_layout, NULL, &logical );
	pango_extents_to_pixels ( NULL, &logical );

	state->row_y_start = state->row_pad;
	state->glyph_w = logical.width / 10;
	state->row_height = logical.height + 2 * state->row_y_start;
	state->cell_pad = state->glyph_w / 2;
	state->std_baseline = PANGO_PIXELS ( pango_layout_get_baseline (
		state->p_layout ) );

	if ( state->cell_pad <= 0 )
	{
		return 1;
	}

	return 0;
}

static int cairo_page_init_core (
	outSTATE	* const	state
	)
{
	int		c, i;


	// Create and populate array of column widths/x coords
	state->mren_state.col_w = (int *)calloc ( (size_t)state->cols,
		sizeof( int ) );
	state->colx = (double *)calloc( (size_t)state->cols, sizeof ( double ));
	state->mren_state.cell_active = (unsigned char *)calloc (
		(size_t)state->cols, sizeof ( unsigned char ) );

	if (	! state->mren_state.col_w	||
		! state->colx			||
		! state->mren_state.cell_active
		)
	{
		return 1;
	}

	for ( i = 0; i < state->cols; i++ )
	{
		state->mren_state.col_w[i] = CUI_DEFAULT_CELLWIDTH_CHARS;
	}

	if ( ced_sheet_scan_area ( state->sheet, 0, 1, 1, 0, col_width_cb,
		state ) )
	{
		return 1;
	}

	for ( i = 0, c = 0; i < state->cols; i++ )
	{
		state->colx[i] = c * state->glyph_w;

		c += state->mren_state.col_w[i];
	}

	state->page_width = c * state->glyph_w;
	state->page_height = state->rows * state->row_height;

	return 0;
}

// On failure (return != 0), a full cleanup is done by the caller
static int ced_ren_page_init (
	outSTATE	* const	state
	)
{
	if ( ced_sheet_get_geometry ( state->sheet, &state->rows,
		&state->cols ) )
	{
		return -1;
	}

	if ( state->rows < 1 )
	{
		state->rows = 1;
	}

	if ( state->cols < 1 )
	{
		state->cols = 1;
	}

	state->surface = cairo_image_surface_create ( CAIRO_FORMAT_RGB24, 1000,
		1000 );
	state->cr = cairo_create ( state->surface );

	state->font_desc = pango_font_description_from_string (
		state->font_name );
	state->p_layout = pango_cairo_create_layout ( state->cr );

	if (	cairo_page_init_font_extent ( state ) ||
		cairo_page_init_core ( state )
		)
	{
		return 1;
	}

	if ( state->page_width <= 0 )
	{
		state->page_width = 1;
	}

	if ( state->page_height <= 0 )
	{
		state->page_height = 1;
	}

	ced_ren_page_cleanup ( state, 0 );

	// On success, partial cleanup here as further Cairo work is done later
	// by caller.

	return 0;
}


static void ced_ren_page_prepare_clean (
	outSTATE	* const	state
	)
{
	cairo_select_font_face ( state->cr, state->font_name,
		CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
	cairo_set_font_size ( state->cr, state->font_size );
	cairo_set_line_width ( state->cr, 0 );

	// Clear background to white
	cairo_set_source_rgb ( state->cr, 1.0, 1.0, 1.0 );
	cairo_rectangle ( state->cr, 0, 0, state->page_width,
		state->page_height );
	cairo_fill ( state->cr );
}



#ifdef CAIRO_HAS_PDF_SURFACE

enum
{
	HEADFOOT_NONE,
	HEADFOOT_FILENAME_LONG,
	HEADFOOT_FILENAME_SHORT,
	HEADFOOT_SHEET_NAME,
	HEADFOOT_PAGE_NUM,
	HEADFOOT_DATE,
	HEADFOOT_DATETIME,

	HEADFOOT_TOTAL
};

static char const * ced_ren_page_setup_headfoot_text (
	outSTATE	* const	state,
	int		const	type,
	char	const	* const	filename
	)
{
	char	const	* st;


	switch ( type )
	{
	case HEADFOOT_FILENAME_SHORT:
		st = strrchr ( filename, MTKIT_DIR_SEP );
		if ( st )
		{
			return (st + 1);
		}

		// FALL THROUGH

	case HEADFOOT_FILENAME_LONG:
		return filename;

	case HEADFOOT_SHEET_NAME:
		if (	state->sheet->book_tnode &&
			state->sheet->book_tnode->key
			)
		{
			return (char const *)state->sheet->book_tnode->key;
		}

		return "?";

	case HEADFOOT_PAGE_NUM:
		return state->pagenum_txt;

	case HEADFOOT_DATE:
		return state->date_txt;

	case HEADFOOT_DATETIME:
		return state->datetime_txt;
	}

	return NULL;
}


static void send_headfoot (
	outSTATE	* const	state,
	char	const	* const	txt,
	double		* const	x,
	double		* const	y,
	double		const	xpos
	)
{
	if ( ! txt )
	{
		return;
	}

	// Get extents
	pango_layout_set_text ( state->p_layout, txt, -1 );
	pango_layout_get_pixel_extents ( state->p_layout, NULL,
		&state->logical_rect );

	x[0] = (1 - xpos) * (state->pgv_xl) +
		xpos * (state->pgv_xr - state->logical_rect.width );

	cairo_save ( state->cr );

	// Render
	cairo_translate ( state->cr, x[0], y[0] );
	pango_cairo_update_layout ( state->cr, state->p_layout );
	pango_cairo_show_layout ( state->cr, state->p_layout );

	cairo_restore ( state->cr );
}

static void ced_ren_page_prepare_header_footer (
	outSTATE	* const	state
	)
{
	double		x,
			y;


	cairo_set_source_rgb ( state->cr, 0, 0, 0 );

	y = state->pgv_yh + state->row_y_start;

	send_headfoot ( state, state->head_txt[0], &x, &y, 0.0 );
	send_headfoot ( state, state->head_txt[1], &x, &y, 0.5 );
	send_headfoot ( state, state->head_txt[2], &x, &y, 1.0 );

	y = state->pgv_yf - state->row_height + state->row_y_start;

	send_headfoot ( state, state->foot_txt[0], &x, &y, 0.0 );
	send_headfoot ( state, state->foot_txt[1], &x, &y, 0.5 );
	send_headfoot ( state, state->foot_txt[2], &x, &y, 1.0 );
}


static void ced_ren_page_setup_data_time_text (
	outSTATE	* const	state
	)
{
	time_t		now;
	struct tm	* now_tm;


	now = time ( NULL );
	now_tm = localtime ( &now );

	strftime ( state->date_txt, 120, "%F", now_tm );
	strftime ( state->datetime_txt, 120, "%F %T", now_tm );
}



#define mm_2_PT(MM)	(MM * 72 / 25.4)

#endif			// CAIRO_HAS_PDF_SURFACE



static int cui_ren_export_pdf_paged (
	CedSheet	* const	sheet,
	int		const	filetype,
	char	const	* const	filename,
	char	const	* const	mem_filename,
	CuiRenPage	* const	page,
	int		const	row_pad,
	char	const	* const	font_name,
	int		const	font_size
	)
{
#ifdef CAIRO_HAS_PDF_SURFACE
	outSTATE	st, * state = &st;
	int		i, res = 1, r, c, pages_across = 0, pages_down = 0;


	state->page = page;
	state->filetype = filetype;
	state->sheet = sheet;
	state->row_pad = row_pad;
	state->font_name = font_name;
	state->font_size = font_size;

	switch ( ced_ren_page_init ( state ) )
	{
	case -1:
		return 1;

	case 1:
		goto error;
	}

	state->page_width = mm_2_PT ( page->width );
	state->page_height = mm_2_PT ( page->height );

	state->surface = cairo_pdf_surface_create ( filename,
		state->page_width, state->page_height );

	if ( cairo_surface_status ( state->surface ) != CAIRO_STATUS_SUCCESS )
	{
		goto error;
	}

	state->cr = cairo_create ( state->surface );
	ced_ren_page_prepare_clean ( state );

	if (	! state->sheet->rows ||
		! state->sheet->rows->root
		)
	{
		goto finish;
	}

	ced_ren_page_setup_data_time_text ( state );

	for ( i = 0; i < 3; i++ )
	{
		state->head_txt[i] = ced_ren_page_setup_headfoot_text ( state,
			page->header[i], mem_filename );

		state->foot_txt[i] = ced_ren_page_setup_headfoot_text ( state,
			page->footer[i], mem_filename );
	}

	state->pgv_xl	= mm_2_PT ( page->margin_x );
	state->pgv_xr	= state->page_width - state->pgv_xl;
	state->pgv_yh	= mm_2_PT ( page->margin_y );
	state->pgv_ytop	= state->pgv_yh + state->row_height * 2;
	state->pgv_yf	= state->page_height - state->pgv_yh;
	state->pgv_ybot	= state->page_height - state->pgv_ytop;

	state->rows_per_page = (int)((state->page_height - 2*state->pgv_ytop) /
		state->row_height );

	if ( state->rows_per_page < 1 )
	{
		state->rows_per_page = 1;
	}

	// Calculate how many pages across are required
	pages_across = 0;

	for ( i = 1; i <= state->cols; i += state->cols_per_page )
	{
		state->cols_per_page = ced_ren_page_cols_get ( i-1, state );
		pages_across ++;
	}

	if ( pages_across < 1 )
	{
		pages_across = 1;
	}

	pages_down = (state->rows + state->rows_per_page - 1) /
		state->rows_per_page;

	if ( pages_down < 1 )
	{
		pages_down = 1;
	}

	state->pages_total = pages_down * pages_across;

	state->page_num = 1;
	for ( r = 1; r <= state->rows; r += state->rows_per_page )
	{
		state->row_origin = r;

		for ( c = 1; c <= state->cols; c += state->cols_per_page )
		{
			snprintf ( state->pagenum_txt,
				sizeof ( state->pagenum_txt ), "%i / %i",
				state->page_num, state->pages_total );

			state->col_origin = c;
			state->cols_per_page = ced_ren_page_cols_get ( c - 1,
				state );
			ced_ren_page_prepare_header_footer ( state );

			state->mren_state.r1 = r;
			state->mren_state.r2 = r + state->rows_per_page - 1;
			state->mren_state.c1 = c;
			state->mren_state.c2 = c + state->cols_per_page - 1;

			ced_ren_page_recurse_rows ( state->sheet->rows->root,
				state );

			// Output page to PDF file
			cairo_show_page ( state->cr );

			state->page_num ++;
			if ( state->page_num <= state->pages_total )
			{
				ced_ren_page_prepare_clean ( state );
			}
		}
	}

finish:
	res = 0;
error:
	ced_ren_page_cleanup ( state, 1 );
	ced_ren_font_cleanup ( state );

	return res;

#else
	return 1;
#endif			// CAIRO_HAS_PDF_SURFACE

}

static int cui_ren_export_page (
	CedSheet	* const	sheet,
	int		const	filetype,
	char	const	* const	filename,
	int		const	row_pad,
	char	const	* const	font_name,
	int		const	font_size
	)
{
	outSTATE	st, * state = &st;
	int		res = 1;


	state->sheet = sheet;
	state->filetype = filetype;
	state->row_pad = row_pad;
	state->font_name = font_name;
	state->font_size = font_size;

	switch ( ced_ren_page_init ( state ) )
	{
	case -1:
		return 1;

	case 1:
		goto error;
	}


	state->mren_state.r1 = 1;
	state->mren_state.r2 = state->rows;
	state->mren_state.c1 = 1;
	state->mren_state.c2 = state->cols;

	state->cols_per_page = state->cols;

	state->pgv_xr = state->page_width;
	state->pgv_yf = state->pgv_ybot = state->page_height;

	// Create new surface based on new geometry and target file
	switch ( state->filetype )
	{
#ifdef USE_CAIRO_EPS
	case CUI_SHEET_EXPORT_EPS:
		state->surface = cairo_ps_surface_create ( filename,
			state->page_width, state->page_height );
		cairo_ps_surface_set_eps ( state->surface, 1 );
		break;
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
	case CUI_SHEET_EXPORT_PDF:
		state->surface = cairo_pdf_surface_create ( filename,
			state->page_width, state->page_height );
		break;
#endif

#ifdef CAIRO_HAS_PNG_FUNCTIONS
	case CUI_SHEET_EXPORT_PNG:
		state->surface = cairo_image_surface_create (
			CAIRO_FORMAT_RGB24, (int)state->page_width,
			(int)state->page_height );
		break;
#endif

#ifdef CAIRO_HAS_PS_SURFACE
	case CUI_SHEET_EXPORT_PS:
		state->surface = cairo_ps_surface_create ( filename,
			state->page_width, state->page_height );
		break;
#endif

#ifdef CAIRO_HAS_SVG_SURFACE
	case CUI_SHEET_EXPORT_SVG:
		state->surface = cairo_svg_surface_create ( filename,
			state->page_width, state->page_height );
		break;
#endif
	default:
		goto error;
	}

	if ( cairo_surface_status ( state->surface ) != CAIRO_STATUS_SUCCESS )
	{
		goto error;
	}

	state->cr = cairo_create ( state->surface );
	cairo_set_antialias ( state->cr, CAIRO_ANTIALIAS_NONE );
	ced_ren_page_prepare_clean ( state );

	if ( state->sheet->rows && state->sheet->rows->root )
	{
		res = ced_ren_page_recurse_rows ( state->sheet->rows->root,
			state );
	}
	else
	{
		res = 0;
	}

#ifdef CAIRO_HAS_PNG_FUNCTIONS
	if ( state->filetype == CUI_SHEET_EXPORT_PNG && ! res )
	{
		if ( cairo_surface_write_to_png ( state->surface, filename ) !=
			CAIRO_STATUS_SUCCESS )
		{
			res = 1;
		}
	}
#endif

error:
	ced_ren_page_cleanup ( state, 1 );
	ced_ren_font_cleanup ( state );

	return res;
}

static int export_pdf_paged (
	mtPrefs		* const	prefs_file,
	CedSheet	* const	sheet,
	int		const	filetype,
	char	const	* const	filename,
	char	const	* const	gui_filename,
	int		const	row_pad,
	char	const	* const	font_name,
	int		const	font_size
	)
{
	CuiRenPage	page;

	mtBulkInt table_i[] = {
		{ CUI_INIFILE_PAGE_WIDTH,		&page.width },
		{ CUI_INIFILE_PAGE_HEIGHT,		&page.height },
		{ CUI_INIFILE_PAGE_MARGIN_X,		&page.margin_x },
		{ CUI_INIFILE_PAGE_MARGIN_Y,		&page.margin_y },
		{ CUI_INIFILE_PAGE_FOOTER_LEFT,		&page.footer[0] },
		{ CUI_INIFILE_PAGE_FOOTER_CENTRE,	&page.footer[1] },
		{ CUI_INIFILE_PAGE_FOOTER_RIGHT,	&page.footer[2] },
		{ CUI_INIFILE_PAGE_HEADER_LEFT,		&page.header[0] },
		{ CUI_INIFILE_PAGE_HEADER_CENTRE,	&page.header[1] },
		{ CUI_INIFILE_PAGE_HEADER_RIGHT,	&page.header[2] },
		{ NULL, NULL }
		};


	memset ( &page, 0, sizeof(page) );

	if ( mtkit_prefs_bulk_get ( prefs_file, table_i, NULL, NULL ) )
	{
		return 1;
	}

	return cui_ren_export_pdf_paged ( sheet, filetype, filename,
		gui_filename, &page, row_pad, font_name, font_size );
}

int cui_export_output (
	mtPrefs		* const	prefs_file,
	CedSheet	* const	sheet,
	char	const	* const	filename,
	char	const	*	gui_filename,
	int		const	filetype,
	int		const	row_pad,
	char	const	*	font_name,
	int		const	font_size
	)
{
	if ( ! filename || ! sheet )
	{
		return 1;
	}

	if ( ! gui_filename )
	{
		gui_filename = "";
	}

	if ( ! font_name )
	{
		font_name = "Sans";
	}

	switch ( filetype )
	{
	case CUI_SHEET_EXPORT_TSV:
		return ced_sheet_save ( sheet, filename,
			CED_FILE_TYPE_OUTPUT_TSV );

	case CUI_SHEET_EXPORT_TSV_QUOTED:
		return ced_sheet_save ( sheet, filename,
			CED_FILE_TYPE_OUTPUT_TSV_QUOTED );

	case CUI_SHEET_EXPORT_HTML:
		return ced_sheet_save ( sheet, filename,
			CED_FILE_TYPE_OUTPUT_HTML );

	case CUI_SHEET_EXPORT_PDF_PAGED:
		return export_pdf_paged ( prefs_file, sheet, filetype,
			filename, gui_filename, row_pad, font_name, font_size );
	}

	return cui_ren_export_page ( sheet, filetype, filename, row_pad,
		font_name, font_size );
}



/// GEOM

typedef struct
{
	int	c1,
		c2,			// Min / Max columns

		col_prev,		// Last column calculated
		col_x,			// Current X pixel position

		* ax,
		* aw,			// Arrays

		ai,			// Current array index
		as,			// Current array size

		default_width,
		cellw,			// Cell widths

		font_width		// Character width
		;
} cwidSTATE;


#define WIDTH_CB_CELL_WIDTH \
	if ( cell->prefs ) \
	{ \
		state->cellw = cell->prefs->width * state->font_width; \
		\
		if ( state->cellw == 0 ) \
		{ \
			state->cellw = state->default_width; \
		} \
		else if	( state->cellw < 0 ) \
		{ \
			state->cellw = 0; \
		} \
	} \
	else \
	{ \
		state->cellw = state->default_width; \
	} \



static int col_wid_array_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	cwidSTATE	* const	state = (cwidSTATE *)user_data;


	WIDTH_CB_CELL_WIDTH

	// Add in any skipped cells (fill with default widths)
	for (	;
		state->ai < (col - state->c1);
		state->ai ++
		)
	{
		state->ax[ state->ai ] = state->col_x;
		state->aw[ state->ai ] = state->default_width;

		state->col_x += state->default_width;
	}

	// Put width of this cell into the array
	state->ax[ state->ai ] = state->col_x;
	state->aw[ state->ai ] = state->cellw;
	state->ai ++;

	state->col_x += state->cellw;
	state->col_prev = col;

	return 0;		// Continue
}

static int cui_ren_column_width_array (	// Create arrays of pixel coords/widths
					// for each column
	int	const		col_start, // <= c1 <= c2
	CuiRender * const	viewren,
	int	const		x,	// Pixel X
	int	const		w,	// Pixel Width
	int	* const		c1,	// Put c1 column here
	int	* const		c2,	// Put c2 column here
	int	** const	col_x,	// Place array of column X coords here
	int	** const	col_w	// Place array of column widths here
	)
{
	cwidSTATE	state;


	memset ( &state, 0, sizeof(state) );

	state.c1 = cui_ren_column_from_x ( col_start, viewren, x );
	state.c2 = cui_ren_column_from_x ( col_start, viewren, (x + w - 1) );
	state.as = state.c2 - state.c1 + 1;
	state.ax = (int *)calloc ( (size_t)state.as, sizeof ( int ) );
	state.aw = (int *)calloc ( (size_t)state.as, sizeof ( int ) );

	if ( ! state.ax || ! state.aw )
	{
		free ( state.ax );
		free ( state.aw );

		return 1;
	}

	state.default_width = CUI_DEFAULT_CELLWIDTH ( viewren );
	state.font_width = viewren->font_width;

	state.ax[ 0 ] = cui_ren_x_from_column ( col_start, viewren, state.c1 );
	state.aw[ 0 ] = cui_ren_cellwidth ( viewren, state.c1 );
	state.ai = 1;
	state.col_prev = state.c1;
	state.col_x = state.ax[ 0 ] + state.aw[ 0 ];

	if ( state.c1 < state.c2 )
	{
		ced_sheet_scan_area ( viewren->sheet, 0, state.c1 + 1, 1,
			state.c2 - state.c1, col_wid_array_cb, &state );

		// Calculate final gap to c2 - all defaults as we have no cells
		// left.

		for (	;
			state.ai < state.as;
			state.ai ++
			)
		{
			state.ax[ state.ai ] = state.col_x;
			state.aw[ state.ai ] = state.default_width;

			state.col_x += state.default_width;
		}
	}

	col_x[0] = state.ax;
	col_w[0] = state.aw;
	c1[0] = state.c1;
	c2[0] = state.c2;

	return 0;
}

static int col_xfc_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	cwidSTATE	* const	state = (cwidSTATE *)user_data;


	WIDTH_CB_CELL_WIDTH

	// Add in any skipped cells (fill with default widths)
	state->col_x += state->default_width * (col - state->col_prev - 1) +
			state->cellw;
	state->col_prev = col;

	return 0;			// Continue
}

int cui_ren_x_from_column (
	int		const	col_start,
	CuiRender	* const	viewren,
	int		const	column
	)
{
	cwidSTATE	state;


	memset ( &state, 0, sizeof ( state ) );
	state.c1 = column;

	if ( column <= col_start )
	{
		return 0;
	}

	state.default_width = CUI_DEFAULT_CELLWIDTH ( viewren );
	state.font_width = viewren->font_width;
	state.col_prev = col_start - 1;

	ced_sheet_scan_area ( viewren->sheet, 0, col_start, 1,
		column - col_start, col_xfc_cb, &state );

	// Calculate final gap to column
	state.col_x += state.default_width * (column - state.col_prev - 1);

	return state.col_x;
}

int cui_ren_y_from_row (
	int		const	row_start,
	CuiRender	* const	viewren,
	int		const	row
	)
{
	return (row - row_start) * CUI_ROWHEIGHT ( viewren );
}

static int colfx_backwards_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	cwidSTATE	* const	state = (cwidSTATE *)user_data;


	WIDTH_CB_CELL_WIDTH

	// Add in any skipped cells (fill with default widths)
	for ( ; state->col_prev > col; state->col_prev -- )
	{
		state->col_x += state->default_width;

		if ( state->col_x >= state->c1 )
		{
			// Limit reached/exceeded
			state->col_prev --;

			return 1;
		}
	}

	state->col_x += state->cellw;
	state->col_prev --;

	if ( state->col_x >= state->c1 )
	{
		return 1;		// Limit reached/exceeded
	}

	return 0;			// Continue
}

int cui_ren_column_from_x_backwards (
	int		const	col_start,
	CuiRender	* const	viewren,
	int		const	x
	)
{
	cwidSTATE	state;


	memset ( &state, 0, sizeof ( state ) );
	state.c1 = x;

	if ( x < 0 )
	{
		return col_start;
	}

	state.default_width = CUI_DEFAULT_CELLWIDTH ( viewren );
	state.font_width = viewren->font_width;
	state.col_prev = col_start;

	// Sanity
	if ( state.default_width < 1 )
	{
		state.default_width = 1;
	}

	if ( state.font_width < 1 )
	{
		state.font_width = 1;
	}

	ced_sheet_scan_area_backwards ( viewren->sheet, 0, col_start, 1, 0,
		colfx_backwards_cb, &state );

	// Calculate final gap
	for ( ; state.col_x < x; state.col_prev -- )
	{
		state.col_x += state.default_width;
	}

	if ( state.col_x > x )
	{
		state.col_prev ++;
	}

	return state.col_prev;
}

static int colfx_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	cwidSTATE	* const	state = (cwidSTATE *)user_data;


	WIDTH_CB_CELL_WIDTH

	// Add in any skipped cells (fill with default widths)
	for ( ; state->col_prev < col; state->col_prev ++ )
	{
		state->col_x += state->default_width;

		if ( state->col_x >= state->c1 )
		{
			state->col_prev ++;

			return 1;	// Limit reached/exceeded
		}
	}

	state->col_x += state->cellw;
	state->col_prev ++;

	if ( state->col_x >= state->c1 )
	{
		return 1;		// Limit reached/exceeded
	}

	return 0;			// Continue
}

int cui_ren_column_from_x (
	int		const	col_start,
	CuiRender	* const	viewren,
	int		const	x
	)
{
	cwidSTATE	state;


	memset ( &state, 0, sizeof ( state ) );
	state.c1 = x;

	if ( x < 0 )
	{
		return col_start;
	}

	state.default_width = CUI_DEFAULT_CELLWIDTH ( viewren );
	state.font_width = viewren->font_width;
	state.col_prev = col_start;

	// Sanity
	if ( state.default_width < 1 )
	{
		state.default_width = 1;
	}

	if ( state.font_width < 1 )
	{
		state.font_width = 1;
	}

	ced_sheet_scan_area ( viewren->sheet, 0, col_start, 1, 0, colfx_cb,
		&state );

	// Calculate final gap
	for ( ; state.col_x < x; state.col_prev ++ )
	{
		state.col_x += state.default_width;
	}

	if ( state.col_x > x )
	{
		state.col_prev--;
	}

	return state.col_prev;
}

int cui_ren_row_from_y (
	int		const	row_start,
	CuiRender	* const	viewren,
	int		const	y
	)
{
	if ( y < 0 )
	{
		return row_start;
	}

	return ( row_start + y / CUI_ROWHEIGHT ( viewren ) );
}

static int cui_ren_cellwidth (
	CuiRender	* const	viewren,
	int		const	column
	)
{
	CedCell		* cell;
	int		cellw;


	cell = ced_sheet_get_cell ( viewren->sheet, 0, column );

	if ( ! cell )
	{
		return CUI_DEFAULT_CELLWIDTH ( viewren );
	}

	if ( cell->prefs )
	{
		cellw = cell->prefs->width * viewren->font_width;
	}
	else
	{
		cellw = 0;
	}

	if ( cellw == 0 )
	{
		return CUI_DEFAULT_CELLWIDTH ( viewren );
	}
	else if	( cellw < 0 )
	{
		return 0;
	}

	return cellw;
}



/// RENDER

#define CELL_H_BORDER 3



static void cui_ren_mem_to_rgb (
	unsigned char	* const	mem,		// Rendered font memory (1bpp)
	int		const	mx,		// X - text render onto the RGB
	int		const	my,		// Y - text render onto the RGB
	int		const	mw,
	int		const	mh,		// mem geometry
	int		const	mxo,		// X origin
	int		const	mxw,		// X width to render
	unsigned char	const	mr,		// Font red
	unsigned char	const	mg,		// Font green
	unsigned char	const	mb,		// Font blue
	unsigned char	* const	rgb,		// RGB destination (3bpp)
	int		const	x,
	int		const	y,		// RGB origin
	int		const	w,
	int		const	h		// RGB geometry
	)
{
	unsigned char	* src,
			* dest,
			b;
	int		x1 = mx,
			y1 = my,
			xw = mxw,
			yh = mh,
			i,
			j,
			mx1 = 0,
			my1 = 0;


	if ( x1 < x )
	{
		xw -= (x - x1);
		mx1 = (x - x1);
		x1 = x;
	}

	if ( (x1 + xw) > (x + w) )
	{
		xw = (x + w) - x1;
	}

	if ( y1 < y )
	{
		yh -= (y - y1);
		my1 = (y - y1);
		y1 = y;
	}

	if ( (y1 + yh) > (y + h) )
	{
		yh = (y + h) - y1;
	}

	for ( j = 0; j < yh; j++ )
	{
		src = mem + mw * ( j + my1 ) + mx1 + mxo;
		dest = rgb + 3 * ( w * (j + y1 - y) + (x1 - x) );

		for ( i = 0; i < xw; i++ )
		{
			b = *src++;

			if ( b )
			{
				dest[0] = (unsigned char)( ( b * mr +
					(255 - b) * dest[0] ) / 255 );

				dest[1] = (unsigned char)( ( b * mg +
					(255 - b) * dest[1] ) / 255 );

				dest[2] = (unsigned char)( ( b * mb +
					(255 - b) * dest[2] ) / 255 );
			}

			dest += 3;
		}
	}
}

static void get_cell_x_extent (
	mrenSTATE	* const	state,
	int		const	col
	)
{
	// Populate state->cwidth & mx including clipping
	state->cwidth = state->col_w[ col - state->c1 ];
	state->mx = state->col_x[ col - state->c1 ];

	// Left clip
	if ( state->mx < state->x )
	{
		state->cwidth -= (state->x - state->mx);
		state->mx = state->x;
	}

	// Right clip
	if ( (state->mx + state->cwidth) > (state->x + state->w) )
	{
		state->cwidth = state->x + state->w - state->mx;
	}
}

static int main_background_ren_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	mrenSTATE	* const	state = (mrenSTATE *)user_data;
	unsigned char	color[3], * dest;


	if ( cell->text )
	{
		state->cell_active[ col - state->c1 ] = 1;
	}

	if (	! cell->prefs ||
		cell->prefs->color_background == 16777215
		)
	{
		return 0;		// Already default
	}

	if (	row >= state->cur_row1 &&
		row <= state->cur_row2 &&
		col >= state->cur_col1 &&
		col <= state->cur_col2
		)
	{
		// Cursor covers this background, so do nothing

		return 0;
	}

	get_cell_x_extent ( state, col );

	color[0] = (unsigned char)mtPixy::int_2_red (
		cell->prefs->color_background );

	color[1] = (unsigned char)mtPixy::int_2_green (
		cell->prefs->color_background );

	color[2] = (unsigned char)mtPixy::int_2_blue (
		cell->prefs->color_background );

	dest = state->rgb + 3 * (state->mx - state->x);

	for ( state->mx = state->cwidth; state->mx > 0; state->mx -- )
	{
		*dest ++ = color[0];
		*dest ++ = color[1];
		*dest ++ = color[2];
	}

	return 0;			// Continue
}

// Render the current mtPixy::Image
static void write_ren_text (
	mrenSTATE	* const	state,
	int		const	pos_x,
	int		const	pos_w,
	int		const	row,
	int		const	col,
	CedCell		* const	cell,
	int		const	mx
	)
{
	unsigned char	color[3] = {0};


	if (	row >= state->cur_row1 &&
		row <= state->cur_row2 &&
		col >= state->cur_col1 &&
		col <= state->cur_col2
		)
	{
		// Cursor covers this background, so use white text

		color[0] = 255;
		color[1] = 255;
		color[2] = 255;
	}
	else
	{
		if ( cell->prefs )
		{
			color[0] = (unsigned char)mtPixy::int_2_red (
					cell->prefs->color_foreground );

			color[1] = (unsigned char)mtPixy::int_2_green (
					cell->prefs->color_foreground );

			color[2] = (unsigned char)mtPixy::int_2_blue (
					cell->prefs->color_foreground );
		}
	}

	cui_ren_mem_to_rgb ( state->mem, mx, 0,
		state->image->get_width (),
		state->image->get_height (),
		pos_x, pos_w,
		color[0], color[1], color[2],
		state->rgb, state->x, 0,
		state->w, state->row_h );
}


// Expand the rendered text as required, and then render
static void prepare_ren_expansion (
	mrenSTATE	* const	state,
	int			pos_x,
	int			pos_w,
	int		const	row,
	int		const	col,
	CedCell		* const	cell,
	int		const	col_exp
	)
{
	int		i, j, c1, c2;


	c1 = col_exp;
	c2 = col_exp;

	// Expansion to the left
	if (	state->justify == CED_CELL_JUSTIFY_CENTER ||
		state->justify == CED_CELL_JUSTIFY_RIGHT
		)
	{
		for (	i = col_exp - 1;
			i >= state->c1 && pos_x > 0;
			i-- )
		{
			j = i - state->c1;

			// Is this cell free for us to expand over?
			if ( state->cell_active[j] != 0 )
			{
				break;
			}

			// Adjust X coords one cell to the left, expand width
			pos_x -= state->col_w[ j ];
			pos_w += state->col_w[ j ];
			state->mx -= state->col_w[ j ];

			c1 = i;		// The new leftmost column
		}

		// Have we expanded beyond start of bitmap?
		if ( pos_x < 0 )
		{
			pos_w += pos_x;
			state->mx -= pos_x;
			pos_x = 0;
		}
	}

	// Expansion to the right
	if (	state->justify == CED_CELL_JUSTIFY_CENTER ||
		state->justify == CED_CELL_JUSTIFY_LEFT
		)
	{
		int		im_width;


		im_width = state->image->get_width ();

		// Only ever attempt to expand if the text has already been
		// truncated to the right.

		if ( (pos_x + pos_w) < im_width )
		{
			for ( i = col_exp + 1; i <= state->c2; i++ )
			{
				j = i - state->c1;

				// Is this cell free for us to expand over?
				if ( state->cell_active[j] != 0 )
				{
					break;
				}

/*
For every new empty cell that we encroach set the cell flag to 1 so that right
justified over-runs to the right of this cell don't clash with this text.
NOTE: We don't bother doing this when overlapping to the left because it serves
no practical purpose as we render from left to right.
*/
				state->cell_active[j] = 1;

				c2 = i;		// The new rightmost column

				// Expand width
				pos_w += state->col_w[ j ];

				// Have we expanded beyond the bitmap width?
				if ( (pos_x + pos_w) >= im_width )
				{
					pos_w = im_width - pos_x;

					break;
				}
			}
		}
	}

	if (	row < state->cur_row1 ||
		row > state->cur_row2 ||
		c2  < state->cur_col1 ||
		c1  > state->cur_col2
		)
	{
		// No selection area means we render in one go
		write_ren_text ( state, pos_x, pos_w, row, c1, cell,
			state->mx );
	}
	else
	{
		int		xi[3],
				wi[3],
				ci[3] = { -1, -1, -1 },
				mxi[3],
				tmp;
/*
Expansion took place AND cursor interfering: split required
We now have up to 3 renders as we have the cursor area to traverse:
?i[0]	Text to left of cursor
?i[1]	Text over cursor (or skipped if the row/col sits outside the selection)
?i[2]	Text to right of cursor
*/

		if ( c1 < state->cur_col1 )
		{
			// Text exists to left

			xi[0] = pos_x;
			wi[0] = state->col_x[ MIN (state->cur_col1, state->c2) -
				state->c1 ] - state->mx;
			ci[0] = c1;
			mxi[0] = state->mx;
		}

		// Only render white text on the cursor section if the cell is
		// selected itself.

		if ( col >= state->cur_col1 && col <= state->cur_col2 )
		{
			// Text exists over the selection (by virtue of being
			// inside this 'else')

			if ( ci[0] >= 0 )
			{
/*
Its safe to use state->cur_col1 as array reference in this section because it
has to be >= state->c1 and also <= state->c2 due to text existing to the left
of the cursor and the fact that the cursor is between state->c1 and state->c2
somehwere.
*/
				tmp = state->col_x[ state->cur_col1 -
					state->c1 ];

				// Start of render is flush to the cell edge on
				// the left.

				xi[1] = pos_x + (tmp - state->mx);

				// Chop off width to the left
				wi[1] = pos_w - (tmp - state->mx);

				mxi[1] = tmp;
			}
			else
			{
				// Start of render is pos_x as there is no text
				// to the left.

				xi[1] = pos_x;
				wi[1] = pos_w;
				mxi[1] = state->mx;
			}

			if ( c2 > state->cur_col2 )
			{
				// Chop off stuff to right of cursor
				wi[1] -= pos_w - (
					state->col_x[ state->cur_col2 -
						state->c1 ] +
					state->col_w[ state->cur_col2 -
						state->c1 ] -
					state->mx
					);
			}

			ci[1] = col;
		}

		if ( c2 > state->cur_col2 )
		{
			// Text exists to right

			tmp =	state->col_x[ state->cur_col2 - state->c1 ] +
				state->col_w[ state->cur_col2 - state->c1 ];

			xi[2] = pos_x + (tmp - state->mx);
			wi[2] = pos_w - (tmp - state->mx);
			ci[2] = state->cur_col2 + 1;
			mxi[2] = tmp;
		}

		for ( i = 0; i < 3; i++ )
		{
			if ( ci[i] < 0 )
			{
				continue;	// Skip this section
			}

			write_ren_text ( state, xi[i], wi[i], row, ci[i],
				cell, mxi[i] );
		}
	}
}

// Create an mtPixy::Image for this cell text +
// set mx position based on justification
static int prepare_ren_text (
	mrenSTATE	* const	state,
	CedCell		* const	cell
	)
{
	int		res = 1, w, text_style = 0;
	char		* new_text = NULL;


	new_text = ced_cell_create_output ( cell, &state->justify );

	if ( cell->prefs )
	{
		text_style = cell->prefs->text_style;
	}


	font_set_attr ( state->viewren->font, text_style );
	state->viewren->font->set_row_pad ( state->viewren->row_pad );
	state->image = state->viewren->font->render_image ( new_text,
		CUI_CELL_MAX_PIXELS );
	if ( ! state->image )
	{
		goto finish;
	}

	state->mem = state->image->get_alpha ();
	if ( ! state->mem )
	{
		delete state->image;
		state->image = NULL;

		goto finish;
	}

	w = state->image->get_width ();

	switch ( state->justify )
	{
	case CED_CELL_JUSTIFY_RIGHT:
		state->mx += state->cwidth - w - CELL_H_BORDER;
		break;

	case CED_CELL_JUSTIFY_CENTER:
		state->mx += (state->cwidth - w) / 2;
		break;

	case CED_CELL_JUSTIFY_LEFT:
	default:
		state->mx += CELL_H_BORDER;
		break;
	}

	res = 0;		// Success

finish:
	free ( new_text );

	return res;
}

static int expose_left_ren_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	mrenSTATE	* const	state = (mrenSTATE *)user_data;
	int		w, pos_x, pos_w;


	if ( ! cell->text )
	{
		return 0;		// Nothing to render
	}

	// X coordinate of first visible cells left edge
	pos_x = state->col_x[0];

	// X coordinate of this cell
	state->mx = pos_x - cui_ren_x_from_column ( col, state->viewren,
		state->c1 );
	state->cwidth = cui_ren_cellwidth ( state->viewren, col );

	if ( prepare_ren_text ( state, cell ) )
	{
		return 1;		// Problem / Nothing to do
	}

	w = state->image->get_width ();

	// Is the text going to be visible?
	if ( (state->mx + w) > pos_x )
	{
		// Chop off missing left portion
		pos_w		= w - (pos_x - state->mx);
		state->mx	= pos_x;
		pos_x		= w - pos_w;

		if ( pos_w > state->col_w[0] )
		{
			// This exposure goes beyond this cell so clip to first
			// cell.

			pos_w = state->col_w[0];
		}

		prepare_ren_expansion ( state, pos_x, pos_w, row, col, cell,
			state->c1 );
	}

	delete state->image;
	state->image = NULL;

	return 1;
}

static void border_line (
	unsigned char	*	dest,
	int		const	stride,
	int			tot,
	unsigned char	const	rgb[3]
	)
{
	for ( ; tot>0; tot--, dest += stride )
	{
		dest[0] = rgb[0];
		dest[1] = rgb[1];
		dest[2] = rgb[2];
	}
}

static void render_border (
	int		const	type,
	int		const	col,
	mrenSTATE	* const	state,
	unsigned char	const	rgb[3]
	)
{
	int		co[3],
			i,
			stride,
			offset,
			hori[3] = {
				(type >> CED_CELL_BORDER_TOP_SHIFT) &
					CED_CELL_BORDER_MASK,
				(type >> CED_CELL_BORDER_MIDDLE_SHIFT) &
					CED_CELL_BORDER_MASK,
				(type >> CED_CELL_BORDER_BOTTOM_SHIFT) &
					CED_CELL_BORDER_MASK
				},
			vert[3] = {
				(type >> CED_CELL_BORDER_LEFT_SHIFT) &
					CED_CELL_BORDER_MASK,
				(type >> CED_CELL_BORDER_CENTER_SHIFT) &
					CED_CELL_BORDER_MASK,
				(type >> CED_CELL_BORDER_RIGHT_SHIFT) &
					CED_CELL_BORDER_MASK
				};
	unsigned char	* dest;


	co[0] = 0;
	co[1] = (state->row_h >> 1) - 1;
	co[2] = state->row_h - 3;

	get_cell_x_extent ( state, col );

	stride = 3 * state->w;

	for ( i = 0; i < 3; i++ )
	{
		dest = state->rgb + 3 * (state->mx - state->x) + co[i] * stride;

		if ( hori[i] == CED_CELL_BORDER_THIN )
		{
			border_line ( dest + i * stride, 3, state->cwidth,
				rgb );
		}
		else if ( hori[i] == CED_CELL_BORDER_THICK )
		{
			border_line ( dest + (i / 2) * stride, 3,
				state->cwidth, rgb );
			border_line ( dest + (i / 2 + 1) * stride, 3,
				state->cwidth, rgb );
		}
		else if ( hori[i] == CED_CELL_BORDER_DOUBLE )
		{
			border_line ( dest, 3, state->cwidth, rgb );
			border_line ( dest + 2 * stride, 3, state->cwidth,
				rgb );
		}
	}

	co[0] = state->col_x[ col - state->c1 ];
	co[1] = co[0] + state->col_w[ col - state->c1 ] / 2 - 1;
	co[2] = co[0] + state->col_w[ col - state->c1 ] - 3;

	for ( i = 0; i < 3; i++ )
	{
		dest = state->rgb + 3 * (co[i] - state->x);

		if ( vert[i] == CED_CELL_BORDER_THIN )
		{
			if (	(co[i] + i) >= state->x &&
				(co[i] + i) < (state->x + state->w)
				)
			{
				border_line ( dest + i * 3, stride,
					state->row_h, rgb );
			}
		}
		else if ( vert[i] == CED_CELL_BORDER_THICK )
		{
			offset = i / 2;

			if (	(co[i] + offset) >= state->x &&
				(co[i] + offset) < (state->x + state->w)
				)
			{
				border_line ( dest + 3 * offset, stride,
					state->row_h, rgb );
			}

			if (	(co[i] + offset + 1) >= state->x &&
				(co[i] + offset + 1) < (state->x + state->w)
				)
			{
				border_line ( dest + 3 * ( offset + 1), stride,
					state->row_h, rgb );
			}
		}
		else if ( vert[i] == CED_CELL_BORDER_DOUBLE )
		{
			if (	co[i] >= state->x &&
				co[i] < (state->x + state->w)
				)
			{
				border_line ( dest, stride, state->row_h, rgb );
			}

			if (	(co[i] + 2) >= state->x &&
				(co[i] + 2) < (state->x + state->w)
				)
			{
				border_line ( dest + 6, stride, state->row_h,
					rgb );
			}
		}
	}
}

static int main_text_ren_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	mrenSTATE	* const	state = (mrenSTATE *)user_data;
	int		pos_x = 0, pos_w;


	if (	cell->prefs &&
		cell->prefs->border_type != 0 && // Border must exist
		col <= state->c2 &&		// Must be a visible cell
		state->row_h > 3		// Row must be tall enough
		)
	{
		unsigned char		rgb[3] = { 0, 150, 255 };


		if (	row < state->cur_row1 ||
			row > state->cur_row2 ||
			col < state->cur_col1 ||
			col > state->cur_col2
			)
		{
			rgb[0] = (unsigned char)mtPixy::int_2_red (
				cell->prefs->border_color );

			rgb[1] = (unsigned char)mtPixy::int_2_green (
				cell->prefs->border_color );

			rgb[2] = (unsigned char)mtPixy::int_2_blue (
				cell->prefs->border_color );
		}

		render_border ( cell->prefs->border_type, col, state, rgb );
	}

	if ( ! cell->text )
	{
		// Continue searching for an active cell to render
		return 0;
	}

	if ( state->c2 < col )
	{
		// We can stop scanning now if the last visible cell is active
		if ( state->cell_active[ state->c2 - state->c1 ] != 0 )
		{
			return 1;
		}

		// X coordinate of last visible cells right edge + 1
		pos_x = state->col_x[ state->c2 - state->c1 ] +
			state->col_w[ state->c2 - state->c1 ];

		state->mx = pos_x + cui_ren_x_from_column ( state->c2 + 1,
			state->viewren, col );
		state->cwidth = cui_ren_cellwidth ( state->viewren, col );

		if ( prepare_ren_text ( state, cell ) )
		{
			goto finish;
		}

		// Is the text going to be visible?
		if ( state->mx < pos_x )
		{
			if ( state->mx >=
				state->col_x[ state->c2 - state->c1 ] )
			{
				// This exposure only covers part of the final
				// cell.

				pos_w = pos_x - state->mx;
				pos_x = 0;
			}
			else
			{
				// This exposure exists before final cell so
				// clip to final cell.

				pos_x = state->col_x[ state->c2 - state->c1 ] -
					state->mx;
				pos_w = state->col_w[ state->c2 - state->c1 ];
				state->mx = state->col_x[ state->c2 -
					state->c1 ];
			}

			prepare_ren_expansion ( state, pos_x, pos_w, row, col,
				cell, state->c2 );
		}

		delete state->image;
		state->image = NULL;

		return 1;
	}

	state->cwidth = state->col_w[ col - state->c1 ];
	state->mx = state->col_x[ col - state->c1 ];

	if ( prepare_ren_text ( state, cell ) )
	{
		goto finish;
	}

	pos_w = state->image->get_width ();

	// Clipping on left
	if ( state->mx < (state->col_x[ col - state->c1 ]) )
	{
		pos_x = state->col_x[ col - state->c1 ] - state->mx;
		pos_w -= pos_x;
		state->mx = state->col_x[ col - state->c1 ];
	}

	// Clipping on right
	if ( (state->mx + pos_w) >
		(state->col_x[ col - state->c1 ] + state->cwidth) )
	{
		pos_w = state->col_x[ col - state->c1 ] + state->cwidth -
			state->mx;
	}

	// If we have clipped see if this can be clawed back by expanding over
	// adjacent cell(s)

	if ( pos_w != state->image->get_width () )
	{
		prepare_ren_expansion ( state, pos_x, pos_w, row, col, cell,
			col );
	}
	else
	{
		write_ren_text ( state, pos_x, pos_w, row, col, cell,
			state->mx );
	}

	delete state->image;
	state->image = NULL;

finish:
	return 0;			// Continue
}



#define CURSOR_R	20
#define CURSOR_G	60
#define CURSOR_B	120
#define CURSOR_R2	0
#define CURSOR_G2	0
#define CURSOR_B2	45



int cui_ren_expose_main (
	int		const	row_start,
	int		const	col_start,
	CuiRender	* const	viewren,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	mrenSTATE	state;
	unsigned char	* dest;
	int		res = 1, r, c, my, draw_cursor = 0,
			ovl_r1, ovl_r2, ovl_c1, ovl_c2,
			cur_x = 0, cur_w = 0;

	state.row_start = row_start;
	state.col_start = col_start;
	state.viewren = viewren;
	state.x = x;
	state.y = y;
	state.w = w;
	state.h = h;
	state.row_h = CUI_ROWHEIGHT ( viewren );

	state.rgb = (unsigned char *)malloc ( (size_t)(w * state.row_h * 3) );
	if ( ! state.rgb )
	{
		return 1;
	}

	state.r1 = cui_ren_row_from_y ( row_start, viewren, y );
	state.r2 = cui_ren_row_from_y ( row_start, viewren, (y + h - 1) );

	if ( cui_ren_column_width_array ( col_start, viewren, x, w, &state.c1,
		&state.c2, &state.col_x, &state.col_w )
		)
	{
		goto error;
	}

	state.cell_active = (unsigned char *)malloc (
		(size_t)(state.c2 - state.c1 + 1) );

	if ( ! state.cell_active )
	{
		goto error;
	}

	/*
	NOTE: we don't use ced_sheet_cursor_max_min () because it disrespects a
	cursor of 0,0 which is useful for hiding the cursor when editing a
	background colour preference.
	*/

	state.cur_row1 = MIN (	viewren->sheet->prefs.cursor_r1,
				viewren->sheet->prefs.cursor_r2 );

	state.cur_row2 = MAX (	viewren->sheet->prefs.cursor_r1,
				viewren->sheet->prefs.cursor_r2 );

	state.cur_col1 = MIN (	viewren->sheet->prefs.cursor_c1,
				viewren->sheet->prefs.cursor_c2 );

	state.cur_col2 = MAX (	viewren->sheet->prefs.cursor_c1,
				viewren->sheet->prefs.cursor_c2 );

	// Get the overlap between the redraw area and the cursor area
	ovl_r1 = MAX ( state.cur_row1, state.r1 );
	ovl_r2 = MIN ( state.cur_row2, state.r2 );

	ovl_c1 = MAX ( state.cur_col1, state.c1 );
	ovl_c2 = MIN ( state.cur_col2, state.c2 );

	if ( ovl_r1 <= ovl_r2 && ovl_c1 <= ovl_c2 )
	{
		// Calculate the cursor geometry

		draw_cursor = 1;

		// Get cursor start X and Width
		c = ovl_c1 - state.c1;		// Left column edge in array
		r = ovl_c2 - state.c1;		// Right column edge in array
		cur_x = state.col_x[ c ];
		cur_w = state.col_x[ r ] + state.col_w[ r ] - cur_x;

		if ( cur_x < x )
		{
			cur_w -= (x - cur_x);
			cur_x  = x;
		}

		if ( (cur_x + cur_w) > (x + w) )
		{
			cur_w = (x + w) - cur_x;
		}
	}

	my = cui_ren_y_from_row ( row_start, viewren, state.r1 );

	for ( r = state.r1; r <= state.r2; r++, my += state.row_h )
	{
		// Clear row scanline
		memset ( state.rgb, 255, (size_t)(w * 3) );

		// Clear active cell flags
		memset ( state.cell_active, 0,
			(size_t)(state.c2 - state.c1 + 1) );

		// Draw cursor scanline
		if (	draw_cursor &&
			r >= state.cur_row1 &&
			r <= state.cur_row2
			)
		{
			dest = state.rgb + 3 * (cur_x - x);
			for ( c = 0; c < cur_w; c++ )
			{
				*dest++ = CURSOR_R;
				*dest++ = CURSOR_G;
				*dest++ = CURSOR_B;
			}
		}

		// Render coloured backgrounds for exposed cells
		ced_sheet_scan_area ( viewren->sheet, r, state.c1, 1,
			state.c2 - state.c1 + 1,
			main_background_ren_cb, &state );

		// Duplicate scanline to whole row
		for ( c = 1; c < state.row_h; c++ )
		{
			memcpy ( state.rgb + w * 3 * c, state.rgb,
				(size_t)(w * 3) );
		}

		if (	state.cell_active[0] == 0 &&
			state.c1 > 1
			)
		{
			/*
			Find the first active cell to the left of the first
			exposed cell just in case it trails into view/ Check
			only up to 100 columns to the left for the sake of
			sanity.
			*/
			ced_sheet_scan_area_backwards ( viewren->sheet, r,
				state.c1 - 1, 1, 100, expose_left_ren_cb,
				&state );
		}

		/*
		Render text for exposed cells + check first active cell to right
		of exposed area/ Check only up to 100 columns to the right for
		the sake of sanity.
		*/

		ced_sheet_scan_area ( viewren->sheet, r, state.c1, 1,
			state.c2 - state.c1 + 100,
			main_text_ren_cb, &state );

		callback ( x, my, w, state.row_h, state.rgb, callback_data );
	}

	res = 0;	// Success

error:
	free ( state.rgb );
	free ( state.col_x );
	free ( state.col_w );
	free ( state.cell_active );

	return res;
}

int cui_ren_expose_row_header (
	int		const	row_start,
	CuiRender	* const	viewren,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	char		txt[32];
	unsigned char	col[][3] = {
			{ 200, 200, 190 },
			{ 200, 200, 190 },
			{   0,   0, 100 },
			{   0,   0,   0 },
			{ 255, 255, 255 }
			},
			* dest, * mem, * rgb;
	int		c = 0, cur1, cur2, image_h, image_w,
			i, j, mx, my, rown, r1, r2, togo, yy;
	mtPixy::Image	* image;


	rgb = (unsigned char *)calloc ( (size_t)(w * h * 3), 1 );
	if ( ! rgb )
	{
		return 1;
	}

	cur1 = MIN (	viewren->sheet->prefs.cursor_r1,
			viewren->sheet->prefs.cursor_r2 );

	cur2 = MAX (	viewren->sheet->prefs.cursor_r1,
			viewren->sheet->prefs.cursor_r2 );

	dest = rgb;
	yy = y;

	togo = 0;
	for ( j = 0; j < h; j++ )
	{
		if ( togo < 1 )
		{
			rown = cui_ren_row_from_y ( row_start, viewren,
				yy + j );

			if ( rown >= cur1 && rown <= cur2 )
			{
				c = 2;
			}
			else
			{
				c = rown % 2;
			}

			togo =	cui_ren_y_from_row ( row_start, viewren, rown )
				+ CUI_ROWHEIGHT ( viewren ) - (yy + j);
		}

		for ( i = 0; i < w; i++ )
		{
			*dest++ = col[c][0];
			*dest++ = col[c][1];
			*dest++ = col[c][2];
		}

		togo --;
	}

	r1 = cui_ren_row_from_y ( row_start, viewren, yy );
	r2 = cui_ren_row_from_y ( row_start, viewren, (yy + h - 1) );

	viewren->font->set_row_pad ( viewren->row_pad );
	viewren->font->set_style ( 0, 0, (mtPixy::Font::StyleUnderline)0, 0 );

	for ( i = r1; i <= r2; i++ )
	{
		snprintf ( txt, sizeof ( txt ), "%i", i );

		image = viewren->font->render_image ( txt, 0 );
		if ( ! image )
		{
			continue;
		}

		mem = image->get_alpha ();
		if ( ! mem )
		{
			delete image;
			image = NULL;

			continue;
		}

		if ( i >= cur1 && i <= cur2 )
		{
			c = 4;
		}
		else
		{
			c = 3;
		}

		image_w = image->get_width ();
		image_h = image->get_height ();

		mx = (viewren->row_header_width - image_w) / 2;
		my = cui_ren_y_from_row ( row_start, viewren, i );
		cui_ren_mem_to_rgb ( mem, mx, my, image_w, image_h,
			0, image_w,
			col[c][0], col[c][1], col[c][2],
			rgb, x, yy, w, h );

		delete image;
		image = NULL;
	}

	callback ( x, y, w, h, rgb, callback_data );

	free ( rgb );

	return 0;
}

int cui_ren_expose_column_header (
	int		const	col_start,
	CuiRender	* const	viewren,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	CuiRenCB	const	callback,
	void		* const	callback_data
	)
{
	char		txt[32];
	unsigned char	col[][3] = {
			{ 200, 200, 190 },
			{ 200, 200, 190 },
			{   0,   0, 100 },
			{   0,   0,   0 },
			{ 255, 255, 255 }
			},
			* dest, * mem, * rgb;
	int		c = 0, c1, c2,
			* col_x	= NULL, * col_w	= NULL,
			coln, cwidth, cur1, cur2, image_w, image_h,
			i, j, mx, mxo, mxw, my, togo;
	mtPixy::Image	* image;


	rgb = (unsigned char *)calloc ( (size_t)(w * h * 3), 1 );
	if ( ! rgb )
	{
		return 1;
	}

	if ( cui_ren_column_width_array ( col_start, viewren, x, w, &c1, &c2,
		&col_x, &col_w )
		)
	{
		free ( rgb );

		return 1;
	}

	cur1 = MIN (	viewren->sheet->prefs.cursor_c1,
			viewren->sheet->prefs.cursor_c2 );

	cur2 = MAX (	viewren->sheet->prefs.cursor_c1,
			viewren->sheet->prefs.cursor_c2 );

	// We only need to render the top line of pixels - the others can be
	// memcpy'd.

	dest = rgb;
	togo = 0;
	coln = c1;

	for ( i = 0; i < w; i++ )
	{
		if ( togo < 1 )
		{
			// Get the column number from the X coord
			for ( ; coln < c2; coln ++ )
			{
				if ( col_x[coln - c1] + col_w[coln - c1] >
					(x + i)
					)
				{
					break;
				}
			}

			if ( coln >= cur1 && coln <= cur2 )
			{
				c = 2;
			}
			else
			{
				c = coln % 2;
			}

			togo =	col_x[coln - c1] +
				col_w[coln - c1] - (x + i);
		}

		*dest++ = col[c][0];
		*dest++ = col[c][1];
		*dest++ = col[c][2];

		togo --;
	}

	i = 3 * w;
	for ( j = 1; j < h; j++ )
	{
		dest = rgb + i * j;
		memcpy ( dest, rgb, (size_t)i );
	}

	viewren->font->set_row_pad ( 0 );
	viewren->font->set_style ( 0, 0, (mtPixy::Font::StyleUnderline)0, 0 );

	for ( i = c1; i <= c2; i++ )
	{
		snprintf ( txt, sizeof ( txt ), "%i", i );

		image = viewren->font->render_image ( txt, 0 );
		if ( ! image )
		{
			continue;
		}

		mem = image->get_alpha ();
		if ( ! mem )
		{
			delete image;
			image = NULL;

			continue;
		}

		if ( i >= cur1 && i <= cur2 )
		{
			c = 4;
		}
		else
		{
			c = 3;
		}

		// Get column X, Width
		cwidth = col_w[ i - c1 ];
		mx = col_x[ i - c1 ];

		image_w = image->get_width ();
		image_h = image->get_height ();

		if ( image_w > cwidth )
		{
			mxo = image_w - cwidth;
			mxw = cwidth;
		}
		else
		{
			mxo = 0;
			mxw = image_w;
			mx += (cwidth - image_w) / 2; // Centre justify
		}

		my = 0;
		cui_ren_mem_to_rgb ( mem, mx, my,
			image_w, image_h,
			mxo, mxw,
			col[c][0], col[c][1], col[c][2],
			rgb, x, y, w, h );

		delete image;
		image = NULL;
	}

	callback ( x, y, w, h, rgb, callback_data );

	free ( rgb );
	free ( col_x );
	free ( col_w );

	return 0;
}

