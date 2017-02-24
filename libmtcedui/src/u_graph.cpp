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



#define GR_PAGE_MIN			1.0
#define GR_PAGE_MAX			4000.0

#define GR_DEFAULT_PAGE_WIDTH		640.0
#define GR_DEFAULT_PAGE_HEIGHT		320.0

#define GR_MAX_ITEMS			10000



typedef struct renSTATE renSTATE;
struct renSTATE
{
	CedBook		* book;
	CedSheet	* sheet;

	double		// Specifics - set after pass 1 ready for pass 2
			page_width,
			page_height,
			page_x_pad,
			page_y_pad,

			graph_x_pad,
			graph_y_pad,
			graph_x1,
			graph_x2,
			graph_y1,
			graph_y2,

			plot_x_pad,
			plot_y_pad,
			plot_x1,
			plot_x2,
			plot_y1,
			plot_y2,
			plot_line_size,

			plot_x_axis_top_text_height,
			plot_x_axis_text_height,
			plot_y_axis_right_text_width,
			plot_y_axis_text_width,

			plot_x_axis_top_label_height,
			plot_x_axis_label_height,
			plot_y_axis_right_label_width,
			plot_y_axis_label_width,

			x_axis1,
			x_axis2,
			y_axis1,
			y_axis2,

			// Generic Defaults set every instruction
			fill_color,
			text_color,
			line_color,
			text_size,
			line_size,
			x_justify,
			y_justify,
			x_pad,
			y_pad,
			arrowhead,
			size,
			gap,
			antialias,
			min,
			max,

			// Logical -> Physical multipliers
			lpx,
			lpy,

			// Scratchpad variables for calculations
			x, y, w, h,
			x1, x2,
			y1, y2,
			line_angle
			;

	int		r, c,
			r1, r2,
			c1, c2,
			tmp_i_a;

	char		* text,
			* data_txt;


	double		scale;		// Zoom: 1 = 100% (10% .. 1000%)


	cairo_surface_t		* surface;
	cairo_t			* cr;

	PangoFontDescription	* font_desc;
	PangoLayout		* p_layout;
	PangoRectangle		ink_rect;
	PangoRectangle		logical_rect;

	renSTATE ();
};

renSTATE::renSTATE ()
	:
	book		(),
	sheet		(),
	page_width	( 0.0 ),
	page_height	( 0.0 ),
	page_x_pad	( 0.0 ),
	page_y_pad	( 0.0 ),
	graph_x_pad	( 0.0 ),
	graph_y_pad	( 0.0 ),
	graph_x1	( 0.0 ),
	graph_x2	( 0.0 ),
	graph_y1	( 0.0 ),
	graph_y2	( 0.0 ),
	plot_x_pad	( 0.0 ),
	plot_y_pad	( 0.0 ),
	plot_x1		( 0.0 ),
	plot_x2		( 0.0 ),
	plot_y1		( 0.0 ),
	plot_y2		( 0.0 ),
	plot_line_size	( 0.0 ),
	plot_x_axis_top_text_height ( 0.0 ),
	plot_x_axis_text_height	( 0.0 ),
	plot_y_axis_right_text_width ( 0.0 ),
	plot_y_axis_text_width	( 0.0 ),
	plot_x_axis_top_label_height ( 0.0 ),
	plot_x_axis_label_height ( 0.0 ),
	plot_y_axis_right_label_width ( 0.0 ),
	plot_y_axis_label_width	( 0.0 ),
	x_axis1		( 0.0 ),
	x_axis2		( 0.0 ),
	y_axis1		( 0.0 ),
	y_axis2		( 0.0 ),
	fill_color	( 0.0 ),
	text_color	( 0.0 ),
	line_color	( 0.0 ),
	text_size	( 0.0 ),
	line_size	( 0.0 ),
	x_justify	( 0.0 ),
	y_justify	( 0.0 ),
	x_pad		( 0.0 ),
	y_pad		( 0.0 ),
	arrowhead	( 0.0 ),
	size		( 0.0 ),
	gap		( 0.0 ),
	antialias	( 0.0 ),
	min		( 0.0 ),
	max		( 0.0 ),
	lpx		( 0.0 ),
	lpy		( 0.0 ),
	x		( 0.0 ),
	y		( 0.0 ),
	w		( 0.0 ),
	h		( 0.0 ),
	x1		( 0.0 ),
	x2		( 0.0 ),
	y1		( 0.0 ),
	y2		( 0.0 ),
	line_angle	( 0.0 ),
	r		( 0 ),
	c		( 0 ),
	r1		( 0 ),
	r2		( 0 ),
	c1		( 0 ),
	c2		( 0 ),
	tmp_i_a		( 0 ),
	text		(),
	data_txt	(),
	scale		( 0.0 ),
	surface		(),
	cr		(),
	font_desc	(),
	p_layout	()
{
	ink_rect.x = 0;
	ink_rect.y = 0;
	ink_rect.width = 0;
	ink_rect.height = 0;
	logical_rect.x = 0;
	logical_rect.y = 0;
	logical_rect.width = 0;
	logical_rect.height = 0;
}



typedef int (* renFUNC) (
	renSTATE	* state,
	mtUtreeNode	* node,
	int		pass
	);



///	FORWARD DEFS

static void gr_logical_to_physical (
	renSTATE	* const	state
	)
{
	state->x1 = state->plot_x1 + (state->x1 - state->x_axis1) * state->lpx;
	state->x2 = state->plot_x1 + (state->x2 - state->x_axis1) * state->lpx;
	state->y1 = state->plot_y1 + (state->y1 - state->y_axis1) * state->lpy;
	state->y2 = state->plot_y1 + (state->y2 - state->y_axis1) * state->lpy;
}



///	CORE

#define GRAPH_FILENAME(X) mtkit_string_join ( CUI_GRAPH_NAME_PREFIX, X, NULL, NULL )



CedBookFile * cui_graph_new (
	CedBook		* const	book,
	char		* const	mem,
	int		const	memsize,
	char	const	* const	graph_name
	)
{
	char		* filename = NULL;
	CedBookFile	* nb;


	if (	! book ||
		! ( filename = GRAPH_FILENAME ( graph_name ) )
		)
	{
		return NULL;
	}

	nb = ced_book_add_file ( book, mem, memsize, filename );
	free ( filename );

	return nb;
}

CedBookFile * cui_graph_get (
	CedBook		* const	book,
	char	const	* const	graph_name
	)
{
	char		* filename = NULL;
	CedBookFile	* res;


	if (	! book ||
		! ( filename = GRAPH_FILENAME ( graph_name ) )
		)
	{
		return NULL;
	}

	res = ced_book_get_file ( book, filename );
	free ( filename );

	return res;
}

int cui_graph_destroy (
	CedBook		* const	book,
	char	const	* const	graph_name
	)
{
	char		* filename = NULL;
	int		res;


	if (	! book ||
		! ( filename = GRAPH_FILENAME ( graph_name ) )
		)
	{
		return 1;
	}

	res = ced_book_destroy_file ( book, filename );
	free ( filename );

	return res;
}



typedef struct
{
	CedBook		* book;
	CuiGraphScan	callback;
	void		* user_data;
} scanSTATE;



static int cui_graph_scan_cb (
	mtTreeNode	* const	node,
	void		* const	user_data
	)
{
	scanSTATE	* const	state = (scanSTATE *)user_data;


	if (	node->key &&
		strncmp ( (char *)node->key,
			CUI_GRAPH_NAME_PREFIX,
			CUI_GRAPH_NAME_PREFIX_LEN ) == 0
		)
	{
		// This is a graph so tell the caller about it
		return state->callback ( state->book, (char *)node->key +
			CUI_GRAPH_NAME_PREFIX_LEN,
			(CedBookFile *)node->data, state->user_data );
	}

	return 0;		// This is not a graph so keep looking
}

int cui_graph_scan (
	CedBook		* const	book,
	CuiGraphScan	const	callback,
	void		* const	user_data
	)
{
	scanSTATE	state = { book, callback, user_data };


	if ( ! book || ! callback )
	{
		return 1;
	}

	return mtkit_tree_scan ( book->files, cui_graph_scan_cb, &state, 0 );
}

static int gr_utree_bulk_parse (
	CedSheet	* const	sheet,
	mtUtreeNode	* const	node,
	mtBulkDouble	* const	table_d
	)
{
	int		i;
	mtTreeNode	* tn;
	CedParser	parser_state;


	for ( i = 0; table_d[i].name != NULL; i++ )
	{
		// Find name in the node attributes - continue if absent
		tn = mtkit_tree_node_find ( node->attribute_tree,
			table_d[i].name );

		if ( ! tn )
		{
			continue;
		}

		if ( mtkit_strtoddt ( (char const *)tn->data, table_d[i].var )
			== 0 )
		{
			continue;	// Parsed as a date/time
		}

		// Parse this text via libmtcelledit
		parser_state = ced_sheet_parse_text ( sheet, 1, 1,
			(char const *)tn->data, NULL );

		if (	parser_state.ced_errno ||
			(parser_state.flag & CED_PARSER_FLAG_ERROR)
			)
		{
			continue;
		}

		// Store result
		table_d[i].var[0] = parser_state.data;
	}

	return 0;
}

static int gr_utree_get_defaults (
	renSTATE	* const	state,
	mtUtreeNode	* const	node
	)
{
	mtBulkDouble	table_d[] = {
		{ "arrowhead",		&state->arrowhead	},
		{ "fill_color",		&state->fill_color	},
		{ "text_color",		&state->text_color	},
		{ "line_color",		&state->line_color	},
		{ "text_size",		&state->text_size	},
		{ "line_size",		&state->line_size	},
		{ "x_justify",		&state->x_justify	},
		{ "y_justify",		&state->y_justify	},
		{ "x_pad",		&state->x_pad		},
		{ "y_pad",		&state->y_pad		},
		{ "antialias",		&state->antialias	},
		{ "size",		&state->size		},
		{ "gap",		&state->gap		},
		{ "min",		&state->min		},
		{ "max",		&state->max		},
		{ NULL,			NULL }
		};

	mtTreeNode	* tn;
	mtBulkStr	table_s[] = {
		{ "text",		&state->text		},
		{ "data",		&state->data_txt	},
		{ NULL,			NULL }
		};


	state->arrowhead = 0;
	state->fill_color = 0xFFFFFF;
	state->text_color = 0;
	state->line_color = 0;
	state->text_size = 12;
	state->line_size = 1;
	state->x_justify = 0.5;
	state->y_justify = 0.5;
	state->x_pad = 0;
	state->y_pad = 0;
	state->antialias = 1;
	state->size = 0;
	state->gap = 0;
	state->min = 0;
	state->max = 0;

	// Get the sheet first, as it may be referenced in the parsed text
	tn = mtkit_tree_node_find ( node->attribute_tree, "sheet" );
	if ( tn )
	{
		state->sheet = ced_book_get_sheet ( state->book,
			(char const *)tn->data );
		/* Note - we don't clear if attribute not set so it is inherited
		if absent */
	}

	mtkit_strfreedup ( &state->text, NULL );
	mtkit_strfreedup ( &state->data_txt, NULL );
	mtkit_utree_bulk_get ( node, NULL, NULL, table_s );

	gr_utree_bulk_parse ( state->sheet, node, table_d );

	if ( state->antialias == 1 )
	{
		cairo_set_antialias ( state->cr, CAIRO_ANTIALIAS_DEFAULT );
	}
	else
	{
		cairo_set_antialias ( state->cr, CAIRO_ANTIALIAS_NONE );
	}

	return 0;
}

static void gr_set_rgb (
	renSTATE	* const	state,
	int		const	rgb
	)
{
	cairo_set_source_rgb ( state->cr,
		(double)mtPixy::int_2_red ( rgb ) / 255.0,
		(double)mtPixy::int_2_green ( rgb ) / 255.0,
		(double)mtPixy::int_2_blue ( rgb ) / 255.0
		);
}

static void gr_render_text (
	renSTATE	* const	state
	)
{
	if ( ! state->text || state->text_color < 0 )
	{
		return;
	}

	gr_set_rgb ( state, (int)state->text_color );

	cairo_save ( state->cr );

	cairo_translate ( state->cr, state->x, state->y );
	pango_layout_set_text ( state->p_layout, state->text, -1 );

	pango_font_description_set_weight ( state->font_desc,
		PANGO_WEIGHT_NORMAL );
	pango_font_description_set_size ( state->font_desc,
		(gint)( state->text_size * PANGO_SCALE * 0.75 ) );
	pango_layout_set_font_description ( state->p_layout, state->font_desc );

	pango_cairo_update_layout ( state->cr, state->p_layout );
	pango_cairo_show_layout ( state->cr, state->p_layout );

	cairo_restore ( state->cr );
}

static void gr_render_box (
	renSTATE	* const	state,
	int			line_inside
	)
{
	if ( line_inside )
	{
		line_inside = 1;
	}

	if ( state->fill_color >= 0 )
	{
		gr_set_rgb ( state, (int)state->fill_color );
		cairo_rectangle ( state->cr,
			state->x1,
			state->y1,
			state->x2 - state->x1,
			state->y2 - state->y1 );
		cairo_fill ( state->cr );
	}

	if (	state->fill_color != state->line_color &&
		state->line_color >= 0 )
	{
		gr_set_rgb ( state, (int)state->line_color );
		cairo_set_line_width ( state->cr, state->line_size );
		cairo_rectangle ( state->cr,
			state->x1 + line_inside * state->line_size/2,
			state->y1 + line_inside * state->line_size/2,
			state->x2 - state->x1 - line_inside * state->line_size,
			state->y2 - state->y1 - line_inside * state->line_size
			);
		cairo_stroke ( state->cr );
	}
}

static void gr_render_ellipse (
	renSTATE	* const	state
	)
{
	double		w,
			h;


	w = (state->x2 - state->x1) / 2;
	h = (state->y2 - state->y1) / 2;

	if ( state->fill_color >= 0 )
	{
		gr_set_rgb ( state, (int)state->fill_color );

		cairo_save ( state->cr );
		cairo_translate ( state->cr, state->x1 + w, state->y1 + h );
		cairo_scale ( state->cr, w, h );
		cairo_arc ( state->cr, 0, 0, 1, 0, 2 * M_PI );
		cairo_restore ( state->cr );

		cairo_fill ( state->cr );
	}

	if (	state->fill_color != state->line_color &&
		state->line_color >= 0 )
	{
		gr_set_rgb ( state, (int)state->line_color );
		cairo_set_line_width ( state->cr, state->line_size );

		cairo_save ( state->cr );
		cairo_translate ( state->cr, state->x1 + w, state->y1 + h );
		cairo_scale ( state->cr, w, h );
		cairo_arc ( state->cr, 0, 0, 1, 0, 2 * M_PI );
		cairo_restore ( state->cr );

		cairo_stroke ( state->cr );
	}
}

static void gr_render_line (
	renSTATE	* const	state
	)
{
	double		x[2],
			y[2],
			a,
			r;


	if ( state->x1 == state->x2 && state->y1 == state->y2 )
	{
		state->line_angle = 0;

		return;
	}

	if ( state->x1 == state->x2 )
	{
		if ( state->y1 > state->y2 )
		{
			a = M_PI / 2;
		}
		else
		{
			a = 3 * M_PI / 2;
		}
	}
	else
	{
		a = -atan( (state->y2 - state->y1) / (state->x2 - state->x1) );

		if ( state->x1 > state->x2 )
		{
			a += M_PI;
		}
	}

	// Ensure that 0 <= a <= 2 * M_PI
	a = fmod ( a, 2 * M_PI );
	if ( a < 0 )
	{
		a += 2 * M_PI;
	}

	state->line_angle = a;
	a += M_PI;
	r = state->arrowhead;

	if ( state->line_color >= 0 )
	{
		if ( state->arrowhead > 0 )
		{
			// This stops the end of the line spoiling the arrow tip
			x[0] = state->x2 + r * cos ( a ) / 2;
			y[0] = state->y2 - r * sin ( a ) / 2;
		}
		else
		{
			x[0] = state->x2;
			y[0] = state->y2;
		}

		gr_set_rgb ( state, (int)state->line_color );
		cairo_set_line_width ( state->cr, state->line_size );
		cairo_move_to ( state->cr, state->x1, state->y1 );
		cairo_line_to ( state->cr, x[0], y[0] );
		cairo_stroke ( state->cr );
	}

	if ( state->fill_color >= 0 && state->arrowhead > 0 )
	{
		x[0] = state->x2 + r * cos ( a + M_PI / 6 );
		y[0] = state->y2 - r * sin ( a + M_PI / 6 );
		x[1] = state->x2 + r * cos ( a - M_PI / 6 );
		y[1] = state->y2 - r * sin ( a - M_PI / 6 );

		gr_set_rgb ( state, (int)state->fill_color );
		cairo_move_to ( state->cr, x[0], y[0] );
		cairo_line_to ( state->cr, state->x2, state->y2 );
		cairo_line_to ( state->cr, x[1], y[1] );
		cairo_close_path ( state->cr );
		cairo_fill ( state->cr );
	}
}

static void gr_justify_box (
	renSTATE	* const	state
	)
{
	state->x = (1 - state->x_justify) * (state->x1) +
			state->x_justify  * (state->x2 - state->w);

	state->y = (1 - state->y_justify) * (state->y1) +
			state->y_justify  * (state->y2 - state->h);
}

static int gr_get_text_extents (
	renSTATE	* const	state
	)
{
	if ( ! state->text )
	{
		return 0;		// Nothing to do
	}

	pango_layout_set_text ( state->p_layout, state->text, -1 );
	pango_font_description_set_weight ( state->font_desc,
		PANGO_WEIGHT_NORMAL );
	pango_font_description_set_size ( state->font_desc,
		(gint)( state->text_size * PANGO_SCALE * 0.75 ) );
	pango_layout_set_font_description ( state->p_layout, state->font_desc );
	pango_layout_get_pixel_extents ( state->p_layout,
		&state->ink_rect, &state->logical_rect );

	state->h = state->logical_rect.height;
	state->w = state->logical_rect.width;

	return 1;
}



///	PAGE

static int renfunc_page (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	mtBulkDouble	table_d[] = {
			{ "width",	&state->page_width	},
			{ "height",	&state->page_height	},
			{ NULL,		NULL }
			};


	if ( pass == 2 )
	{
		state->x1 = 0;
		state->y1 = 0;
		state->x2 = state->page_width;
		state->y2 = state->page_height;

		gr_render_box ( state, 1 );

		return 0;
	}

	state->page_width = GR_DEFAULT_PAGE_WIDTH;
	state->page_height = GR_DEFAULT_PAGE_HEIGHT;

	// Disallows negative values
	if ( state->x_pad > state->page_x_pad )
	{
		state->page_x_pad = state->x_pad;
	}

	if ( state->y_pad > state->page_y_pad )
	{
		state->page_y_pad = state->y_pad;
	}

	gr_utree_bulk_parse ( state->sheet, node, table_d );

	return 0;			// Success
}

static int renfunc_graph (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	pass
	)
{
	if ( pass == 2 )
	{
		state->x1 = state->graph_x1;
		state->y1 = state->graph_y1;
		state->x2 = state->graph_x2;
		state->y2 = state->graph_y2;

		gr_render_box ( state, 1 );

		return 0;
	}

	// Disallows negative values
	if ( state->x_pad > state->graph_x_pad )
	{
		state->graph_x_pad = state->x_pad;
	}

	if ( state->y_pad > state->graph_y_pad )
	{
		state->graph_y_pad = state->y_pad;
	}

	return 0;			// Success
}

static int renfunc_plot (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	pass
	)
{
	if ( pass == 2 )
	{
		state->x1 = state->plot_x1;
		state->y1 = state->plot_y1;
		state->x2 = state->plot_x2;
		state->y2 = state->plot_y2;

		gr_render_box ( state, 0 );

		return 0;
	}

	// Disallows negative values
	if ( state->x_pad > state->plot_x_pad )
	{
		state->plot_x_pad = state->x_pad;
	}

	if ( state->y_pad > state->plot_y_pad )
	{
		state->plot_y_pad = state->y_pad;
	}

	if ( state->line_color >= 0 )
	{
		state->plot_line_size = state->line_size;
	}

	return 0;			// Success
}

static int renfunc_x_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	ARG_UNUSED ( pass )
	)
{
	state->x_axis1 = state->min;
	state->x_axis2 = state->max;

	if ( state->x_axis1 == state->x_axis2 )
	{
		state->lpx = 0;
	}
	else
	{
		state->lpx =	( state->plot_x2 - state->plot_x1 ) /
				( state->x_axis2 - state->x_axis1 );
	}

	return 0;			// Success
}

static int renfunc_y_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	ARG_UNUSED ( pass )
	)
{
	state->y_axis1 = state->min;
	state->y_axis2 = state->max;

	if ( state->y_axis1 == state->y_axis2 )
	{
		state->lpy = 0;
	}
	else
	{
		state->lpy =	( state->plot_y2 - state->plot_y1 ) /
				( state->y_axis2 - state->y_axis1 );
	}

	return 0;			// Success
}



///	BAR

static int scan_bar_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	renSTATE	* const	state = (renSTATE *)user_data;
	int			i;


	if (	cell->type != CED_CELL_TYPE_VALUE &&
		cell->type != CED_CELL_TYPE_FORMULA &&
		cell->type != CED_CELL_TYPE_FORMULA_EVAL &&
		cell->type != CED_CELL_TYPE_DATE
		)
	{
		return 0;
	}

	// Find out which item we are on
	i = MAX ( row - state->r1, col - state->c1 );

	state->x1 = state->x_axis1 + i * state->gap;
	state->x2 = state->x1 + state->gap;
	state->y1 = 0;
	state->y2 = cell->value;

	if ( state->x1 > state->x_axis2 )
	{
		return 1;		// Finish
	}

	gr_logical_to_physical ( state );
	gr_render_box ( state, 0 );

	return 0;			// Continue
}



///	PLOT

static void gr_plot_clip_set (
	renSTATE	* const	state
	)
{
	double	x = state->plot_x1 + state->plot_line_size * 0.5,
		y = state->plot_y1 + state->plot_line_size * 0.5,
		w = state->plot_x2 - state->plot_x1 - state->plot_line_size,
		h = state->plot_y2 - state->plot_y1 - state->plot_line_size;


	cairo_rectangle ( state->cr, x, y, w, h );
	cairo_clip ( state->cr );
}

static void gr_plot_clip_clear (
	renSTATE	* const	state
	)
{
	cairo_reset_clip ( state->cr );
}

static int gr_plot_state_init (
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	renSTATE	* const	state
	)
{
	CedCellRef	r1, r2;


	if (	! state->sheet ||
		! state->data_txt ||
		state->x_axis1 == state->x_axis2 ||
		state->y_axis1 == state->y_axis2
		)
	{
		return 1;
	}

	if ( ced_strtocellrange ( state->data_txt, &r1, &r2, NULL, 1 ) )
	{
		return 1;
	}

	if (	r1.row_m ||
		r1.col_m ||
		r1.row_d < 1 ||
		r1.col_d < 1 ||
		r2.row_m ||
		r2.col_m ||
		r2.row_d < 1 ||
		r2.col_d < 1
		)
	{
		return 1;
	}

	state->r1 = MIN ( r1.row_d, r2.row_d );
	state->r2 = MAX ( r1.row_d, r2.row_d );
	state->c1 = MIN ( r1.col_d, r2.col_d );
	state->c2 = MAX ( r1.col_d, r2.col_d );

	gr_plot_clip_set ( state );

	return 0;			// Success
}

static void gr_plot_state_finish (
	renSTATE	* const	state
	)
{
	gr_plot_clip_clear ( state );
}

static int read_xycoords (
	renSTATE	* const	state,
	int		const	row,
	int		const	col
	)
{
	CedCell		* cell;


	cell = ced_sheet_get_cell ( state->sheet, row, col );
	if ( ! cell )
	{
		return 1;
	}
	state->x1 = cell->value;

	cell = ced_sheet_get_cell ( state->sheet, row, col + 1 );
	if ( ! cell )
	{
		return 1;
	}
	state->x2 = cell->value;

	cell = ced_sheet_get_cell ( state->sheet, row, col + 2 );
	if ( ! cell )
	{
		return 1;
	}
	state->y1 = cell->value;

	cell = ced_sheet_get_cell ( state->sheet, row, col + 3 );
	if ( ! cell )
	{
		return 1;
	}
	state->y2 = cell->value;

	// Convert logical coordinates to physical page coordinates
	gr_logical_to_physical ( state );

	free ( state->text );
	state->text = NULL;
	cell = ced_sheet_get_cell ( state->sheet, row, col + 4 );
	state->text = ced_cell_create_output ( cell, NULL );

	return 0;			// Success
}

static int scan_box_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	renSTATE	* const	state = (renSTATE *)user_data;


	if ( read_xycoords ( state, row, col ) )
	{
		return 0;
	}

	if ( state->tmp_i_a == 0 )
	{
		gr_render_box ( state, 0 );
	}
	else
	{
		gr_render_ellipse ( state );
	}

	if ( ! gr_get_text_extents ( state ) )
	{
		return 0;
	}

	state->x1 += state->x_pad;
	state->x2 -= state->x_pad;
	state->y1 += state->y_pad;
	state->y2 -= state->y_pad;

	gr_justify_box ( state );
	gr_render_text ( state );

	return 0;			// Continue
}

static int renfunc_plot_boxellipse (
	int		const	type,
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;		// Nothing to do
	}

	if ( gr_plot_state_init ( node, state ) )
	{
		return 1;
	}

	state->tmp_i_a = type;

	ced_sheet_scan_area ( state->sheet, state->r1, state->c1,
		state->r2 - state->r1 + 1, state->c2 - state->c1 + 1,
		scan_box_cb, state );

	gr_plot_state_finish ( state );

	return 0;
}

static int renfunc_plot_box (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	return renfunc_plot_boxellipse ( 0, state, node, pass );
}

static int renfunc_plot_ellipse (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	return renfunc_plot_boxellipse ( 1, state, node, pass );
}

static int plotline_scan_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	renSTATE	* const	state = (renSTATE *)user_data;
	double		xj, yj, xo, yo;


	if ( read_xycoords ( state, row, col ) )
	{
		return 0;
	}

	gr_render_line ( state );

	if ( ! gr_get_text_extents ( state ) )
	{
		return 0;
	}

	xo = state->x_pad * cos ( state->line_angle );
	yo = state->y_pad * sin ( state->line_angle );

	state->x1 = state->x2 - state->w + xo;
	state->y1 = state->y2 - state->h - yo;
	state->x2 = state->x2 + state->w + xo;
	state->y2 = state->y2 + state->h - yo;

	xj = state->x_justify;
	yj = state->y_justify;

	if ( xj == 0.5 && yj == 0.5 )
	{
		double		ta[4];


		/* Automatically set the text position around the end of the
		line */

		ta[0] = atan ( state->h / state->w );
		ta[1] = M_PI - ta[0];
		ta[2] = M_PI + ta[0];
		ta[3] = 2 * M_PI - ta[0];

		if (	state->line_angle <= ta[0] ||
			state->line_angle >= ta[3]
			)
		{
			// Line hits left side of text box

			state->x_justify = 1.0;
			state->y_justify = 0.5 - state->w *
				tan ( state->line_angle ) /
				( 2 * state->h );
		}
		else if ( state->line_angle <= ta[1] )
		{
			// Line hits bottom side of text box

			if ( state->line_angle != M_PI / 2 )
			{
				/* state->line_angle == PI / 2 =>
					state->x_justify = 0.5; */

				state->x_justify = 0.5 + state->h /
					( 2 * state->w *
					tan ( state->line_angle ) );
			}

			state->y_justify = 0.0;
		}
		else if ( state->line_angle <= ta[2] )
		{
			// Line hits right side of text box

			state->x_justify = 0.0;
			state->y_justify = 0.5 + state->w *
				tan ( state->line_angle ) /
				( 2 * state->h );
		}
		else
		{
			// Line hits top side of text box

			if ( state->line_angle != 3 * M_PI / 2 )
			{
				/* state->line_angle == 3 * PI / 2 =>
					state->x_justify = 0.5; */

				state->x_justify = 0.5 - state->h /
					( 2 * state->w *
					tan ( state->line_angle ) );
			}

			state->y_justify = 1.0;
		}
	}

	gr_justify_box ( state );
	gr_render_text ( state );

	state->x_justify = xj;
	state->y_justify = yj;

	return 0;			// Continue
}

static int renfunc_plot_line (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;		// Nothing to do
	}

	if ( gr_plot_state_init ( node, state ) )
	{
		return 1;
	}

	ced_sheet_scan_area ( state->sheet, state->r1, state->c1,
		state->r2 - state->r1 + 1, state->c2 - state->c1 + 1,
		plotline_scan_cb, state );

	gr_plot_state_finish ( state );

	return 0;
}

static int renfunc_plot_graph_bar (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;
	}

	if ( gr_plot_state_init ( node, state ) )
	{
		return 1;
	}

	// Have single row or column of data
	if ( state->r1 != state->r2 )
	{
		// All data in this column
		state->c2 = state->c1;
	}

	ced_sheet_scan_area ( state->sheet, state->r1, state->c1,
		state->r2 - state->r1 + 1, state->c2 - state->c1 + 1,
		scan_bar_cb, state );

	gr_plot_state_finish ( state );

	return 0;
}



///	LINE

static int gline_scan_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	renSTATE	* const	state = (renSTATE *)user_data;
	int		i;


	if ( (row - 1) > state->tmp_i_a )
	{
		// We have skipped one or more empty cells so impose a move
		state->r = 0;
	}

	if (	cell->type != CED_CELL_TYPE_VALUE &&
		cell->type != CED_CELL_TYPE_FORMULA &&
		cell->type != CED_CELL_TYPE_FORMULA_EVAL &&
		cell->type != CED_CELL_TYPE_DATE
		)
	{
		state->r = 0;

		return 0;		// This cell has no numerical data
	}

	// Find out which item we are on
	i = MAX ( row - state->r1, col - state->c1 );

	state->x1 = state->x_axis1 + i * state->gap;
	state->y1 = cell->value;
	state->tmp_i_a = row;

	if ( state->x1 > state->x_axis2 )
	{
		return 1;		// Finish
	}

	gr_logical_to_physical ( state );

	if ( state->r == 0 )
	{
		cairo_move_to ( state->cr, state->x1, state->y1 );
	}
	else
	{
		state->c ++;
		cairo_line_to ( state->cr, state->x1, state->y1 );
	}

	state->r ++;

	return 0;			// Continue
}

static int renfunc_plot_graph_line (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;
	}

	if ( gr_plot_state_init ( node, state ) )
	{
		return 1;
	}

	// Have single row or column of data
	if ( state->r1 != state->r2 )
	{
		// All data in this column
		state->c2 = state->c1;
	}

	cairo_set_line_cap ( state->cr, CAIRO_LINE_CAP_ROUND );
	cairo_set_line_join ( state->cr, CAIRO_LINE_JOIN_ROUND );
	gr_set_rgb ( state, (int)state->line_color );
	cairo_set_line_width ( state->cr, state->line_size );

	state->r = 0;			// 0 = move next >0 = draw next
	state->c = 0;			// Total lines drawn
	state->tmp_i_a = 0;		// Last active row found

	ced_sheet_scan_area ( state->sheet, state->r1, state->c1,
		state->r2 - state->r1 + 1, state->c2 - state->c1 + 1,
		gline_scan_cb, state );

	if ( state->c > 0 )
	{
		// Draw the line if we have enough points
		cairo_stroke ( state->cr );
	}

	cairo_set_line_join ( state->cr, CAIRO_LINE_JOIN_MITER );
	cairo_set_line_cap ( state->cr, CAIRO_LINE_CAP_BUTT );

	gr_plot_state_finish ( state );

	return 0;
}



///	GRID

static int renfunc_plot_x_axis_grid (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	pass
	)
{
	double		f,
			y;


	if ( pass == 1 )
	{
		return 0;
	}

	if (	state->gap <= 0 ||
		state->y_axis2 == state->y_axis1 ||
		abs ( (int)(state->y_axis2 - state->y_axis1) ) / state->gap >
			GR_MAX_ITEMS
		)
	{
		return 1;
	}

	state->x1 = state->x_axis1;
	state->x2 = state->x_axis2;

	// Convert logical coordinates to physical page coordinates
	state->x1 = state->plot_x1 + (state->x1 - state->x_axis1) * state->lpx;
	state->x2 = state->plot_x1 + (state->x2 - state->x_axis1) * state->lpx;

	f = fmod ( state->y_axis1 - state->min, state->gap );

	cairo_set_line_cap ( state->cr, CAIRO_LINE_CAP_SQUARE );
	gr_plot_clip_set ( state );

	if ( state->y_axis1 < state->y_axis2 )
	{
		if ( f <= 0 )
		{
			y = state->y_axis1 - f;
		}
		else
		{
			y = state->y_axis1 + state->gap - f;
		}

		for ( ; y <= state->y_axis2; y += state->gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->y1 = state->plot_y1 + (y - state->y_axis1) *
				state->lpy;

			state->y2 = state->plot_y1 + (y - state->y_axis1) *
				state->lpy;

			gr_render_line ( state );
		}
	}
	else
	{
		if ( f < 0 )
		{
			y = state->y_axis1 + state->gap - f;
		}
		else
		{
			y = state->y_axis1 - f;
		}

		for ( ; y >= state->y_axis2; y -= state->gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->y1 = state->plot_y1 + (y - state->y_axis1) *
				state->lpy;

			state->y2 = state->plot_y1 + (y - state->y_axis1) *
				state->lpy;

			gr_render_line ( state );
		}
	}

	gr_plot_clip_clear ( state );
	cairo_set_line_cap ( state->cr, CAIRO_LINE_CAP_BUTT );

	return 0;			// Success
}

static int renfunc_plot_y_axis_grid (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	pass
	)
{
	double		f,
			x;


	if ( pass == 1 )
	{
		return 0;
	}

	if (	state->gap <= 0 ||
		state->x_axis2 == state->x_axis1 ||
		abs ( (int)(state->x_axis2 - state->x_axis1) ) / state->gap >
			GR_MAX_ITEMS
		)
	{
		return 1;
	}

	state->y1 = state->y_axis1;
	state->y2 = state->y_axis2;

	// Convert logical coordinates to physical page coordinates
	state->y1 = state->plot_y1 + (state->y1 - state->y_axis1) * state->lpy;
	state->y2 = state->plot_y1 + (state->y2 - state->y_axis1) * state->lpy;

	f = fmod ( state->x_axis1 - state->min, state->gap );

	cairo_set_line_cap ( state->cr, CAIRO_LINE_CAP_SQUARE );
	gr_plot_clip_set ( state );

	if ( state->x_axis1 < state->x_axis2 )
	{
		if ( f <= 0 )
		{
			x = state->x_axis1 - f;
		}
		else
		{
			x = state->x_axis1 + state->gap - f;
		}

		for ( ; x <= state->x_axis2; x += state->gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->x1 = state->plot_x1 + (x - state->x_axis1) *
				state->lpx;

			state->x2 = state->plot_x1 + (x - state->x_axis1) *
				state->lpx;

			gr_render_line ( state );
		}
	}
	else
	{
		if ( f < 0 )
		{
			x = state->x_axis1 + state->gap - f;
		}
		else
		{
			x = state->x_axis1 - f;
		}

		for ( ; x >= state->x_axis2; x -= state->gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->x1 = state->plot_x1 + (x - state->x_axis1) *
				state->lpx;

			state->x2 = state->plot_x1 + (x - state->x_axis1) *
				state->lpx;

			gr_render_line ( state );
		}
	}

	gr_plot_clip_clear ( state );
	cairo_set_line_cap ( state->cr, CAIRO_LINE_CAP_BUTT );

	return 0;			// Success
}


///	AXIS

static void get_x_axis_label_height (
	renSTATE	* const	state
	)
{
	char		* tmp = state->text;


	state->text = strdup ( "123" );
	state->h = 0;

	gr_get_text_extents ( state );
	state->h += state->size + state->y_pad;

	free ( state->text );
	state->text = tmp;
}

static int setup_axis_cell (
	renSTATE	* const	state,
	CedCell		* const	cell,
	CedCell	*	* const	newcell
	)
{
	if ( ! state->data_txt )
	{
		// Label = number, so use cell duplicate or default

		if ( cell )
		{
			newcell[0] = ced_cell_duplicate ( cell );
		}
		else
		{
			newcell[0] = ced_cell_new ();
			if ( newcell[0] )
			{
				// This forces cell output to be created later
				mtkit_strfreedup ( &(newcell[0]->text), "" );
			}
		}

		if ( ! newcell[0] )
		{
			return 1;
		}
	}
	else
	{
		CedCellRef	r1, r2;


		if ( ced_strtocellrange ( state->data_txt, &r1, &r2, NULL, 1 ) )
		{
			return 1;
		}

		if (	r1.row_m != 0 || r1.col_m != 0 ||
			r2.row_m != 0 || r2.col_m != 0
			)
		{
			return 1;
		}

		// Start Cell
		state->r = MAX ( 1, r1.row_d );
		state->c = MAX ( 1, r1.col_d );

		// Row/Column increment - restrict to left->right or top->bottom
		if ( r1.row_d == r2.row_d )
		{
			state->r1 = 0;
		}
		else
		{
			state->r1 = 1;
		}

		state->c1 = ! state->r1;
	}

	return 0;
}

static void get_next_axis_cell (
	renSTATE	* const	state
	)
{
	CedCell		* cell;


	cell = ced_sheet_get_cell ( state->sheet, state->r, state->c );
	state->text = ced_cell_create_output ( cell, NULL );

	state->r += state->r1;
	state->c += state->c1;
}

static void render_x_axis_labels (
	renSTATE	* const	state,
	double		const	y,
	CedCell		* const	cell
	)
{
	double		x,
			xx1,
			xx2;
	char		* oldtxt = state->text;
	CedCell		* newcell = NULL;



	setup_axis_cell ( state, cell, &newcell );

	xx1 = MIN ( state->x_axis1, state->x_axis2 );
	xx2 = MAX ( state->x_axis1, state->x_axis2 );

	for ( x = xx1; x <= xx2; x += state->gap )
	{
		if ( x == xx2 && state->x_justify > 0 )
		{
			/* Only render the last label if sensible justification
			used. */

			break;
		}

		if ( newcell )
		{
			newcell->value = x;
			state->text = ced_cell_create_output ( newcell, NULL );
		}
		else
		{
			get_next_axis_cell ( state );
		}

		if ( ! state->text )
		{
			continue;
		}

		if ( gr_get_text_extents ( state ) )
		{
			// Logical -> physical
			state->x1 = state->plot_x1 +
				(x - state->x_axis1) * state->lpx -
				state->w / 2;

			state->x2 = state->plot_x1 +
				(x - state->x_axis1 + state->gap) * state->lpx +
				state->w / 2;

			state->y1 = y;
			state->y2 = state->y1 + state->h;

			gr_justify_box ( state );
			gr_render_text ( state );
		}

		free ( state->text );
		state->text = NULL;
	}

	if ( newcell )
	{
		ced_cell_destroy ( newcell );
		newcell = NULL;
	}

	state->text = oldtxt;
}

static void render_y_axis_labels (
	renSTATE	* const	state,
	int		const	pass,
	CedCell		* const	cell
	)
{
	double		y,
			yy1,
			yy2,
			max_width = 0;
	char		* oldtxt = state->text;
	CedCell		* newcell = NULL;


	if ( state->gap <= 0 )
	{
		goto finish;
	}

	setup_axis_cell ( state, cell, &newcell );

	yy1 = MIN ( state->y_axis1, state->y_axis2 );
	yy2 = MAX ( state->y_axis1, state->y_axis2 );

	for ( y = yy1; y <= yy2; y += state->gap )
	{
		if ( newcell )
		{
			newcell->value = y;
			state->text = ced_cell_create_output ( newcell, NULL );
		}
		else
		{
			get_next_axis_cell ( state );
		}

		if ( ! state->text )
		{
			continue;
		}

		if ( gr_get_text_extents ( state ) )
		{
			max_width = MAX ( state->w, max_width );

			// Logical -> physical
			state->y1 = state->plot_y1 +
				(y - state->y_axis1) * state->lpy -
				state->h / 2;
			state->y2 = state->plot_y1 +
				(y - state->y_axis1 + state->gap) * state->lpy +
				state->h / 2;

			gr_justify_box ( state );
			gr_render_text ( state );
		}

		free ( state->text );
		state->text = NULL;
	}

	if ( newcell )
	{
		ced_cell_destroy ( newcell );
		newcell = NULL;
	}

finish:
	state->text = oldtxt;

	if ( pass == 1 )
	{
		state->w = max_width;
	}
}

static CedCell * get_label_format_cell (
	renSTATE	* const	state,
	mtUtreeNode	* const	node
	)
{
	CedCell		* cell = NULL;
	CedCellRef	cref;
	char		* newtext = NULL;
	mtBulkStr	table_s[] = {
			{ "label_format",	&newtext },
			{ NULL,			NULL }
			};


	mtkit_utree_bulk_get ( node, NULL, NULL, table_s );
	if ( ! state->sheet || ! newtext )
	{
		return NULL;
	}

	// Get cell reference
	if ( ced_strtocellref ( newtext, &cref, NULL, 1 ) )
	{
		free ( newtext );

		return NULL;
	}

	if ( cref.row_m == 0 && cref.col_m == 0 )
	{
		cell = ced_sheet_get_cell ( state->sheet, cref.row_d,
			cref.col_d );
	}

	free ( newtext );

	return cell;
}

static void draw_x_axis_mark (
	renSTATE	* const	state,
	double		const	x,
	double		const	y1,
	double		const	y2
	)
{
	// Logical -> physical
	state->x1 = state->plot_x1 + (x - state->x_axis1) * state->lpx;
	state->x2 = state->x1;
	state->y1 = y1;
	state->y2 = y2;

	gr_render_line ( state );
}

static void draw_y_axis_mark (
	renSTATE	* const	state,
	double		const	y,
	double		const	x1,
	double		const	x2
	)
{
	// Logical -> physical
	state->x1 = x1;
	state->x2 = x2;
	state->y1 = state->plot_y1 + (y - state->y_axis1) * state->lpy;
	state->y2 = state->y1;

	gr_render_line ( state );
}

static int renfunc_plot_x_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	// Sanity checking
	if (	state->gap > 0 &&
		abs ( (int)(state->x_axis2 - state->x_axis1) ) / state->gap >
			GR_MAX_ITEMS
		)
	{
		return 1;
	}

	if ( gr_get_text_extents ( state ) )
	{
		// We have text to render

		if ( pass == 1 )
		{
			state->h += state->y_pad;

			if ( state->h > state->plot_x_axis_text_height )
			{
				state->plot_x_axis_text_height = state->h;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->x1 = state->graph_x1 + state->graph_x_pad +
				state->plot_y_axis_text_width +
				state->plot_y_axis_label_width;

			state->x2 = state->graph_x2 - state->graph_x_pad -
				state->plot_y_axis_right_text_width -
				state->plot_y_axis_right_label_width;

			state->y2 = state->graph_y2 - state->graph_y_pad;
			state->y1 = state->y2 - state->plot_x_axis_text_height;

			gr_justify_box ( state );
			gr_render_text ( state );
		}
	}

	if ( state->size > 0 || state->gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			get_x_axis_label_height ( state );

			if ( state->h > state->plot_x_axis_label_height )
			{
				state->plot_x_axis_label_height = state->h;
			}
		}
		else	// pass == 2
		{
			if ( state->size > 0 )
			{
				// We have an axis line and marks to render

				state->x1 = state->plot_x1;
				state->x2 = state->plot_x2;
				state->y1 = state->y2 = state->plot_y2;

				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_SQUARE );
				gr_render_line ( state );
				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_BUTT );

				if ( state->gap > 0 )
				{
					double		x;


					for (	x = MIN ( state->x_axis1,
							state->x_axis2 );
						x <= MAX ( state->x_axis1,
							state->x_axis2 );
						x += state->gap )
					{
						draw_x_axis_mark ( state, x,
							state->plot_y2,
							state->plot_y2 +
								state->size );
					}
				}
			}

			if ( state->gap > 0 )
			{
				render_x_axis_labels ( state,
					state->plot_y2 + state->size +
						state->y_pad / 2,
					get_label_format_cell ( state, node )
					);
			}
		}
	}

	return 0;	// Success
}

static int renfunc_plot_x_axis_top (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	// Sanity checking
	if (	state->gap > 0 &&
		abs ( (int)(state->x_axis2 - state->x_axis1) ) / state->gap >
			GR_MAX_ITEMS
		)
	{
		return 1;
	}

	if ( gr_get_text_extents ( state ) )
	{
		// We have text to render

		if ( pass == 1 )
		{
			state->h += state->y_pad;

			if ( state->h > state->plot_x_axis_top_text_height )
			{
				state->plot_x_axis_top_text_height = state->h;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->x1 = state->graph_x1 + state->graph_x_pad +
				state->plot_y_axis_text_width +
				state->plot_y_axis_label_width;

			state->x2 = state->graph_x2 - state->graph_x_pad -
				state->plot_y_axis_right_text_width -
				state->plot_y_axis_right_label_width;

			state->y1 = state->graph_y1 + state->graph_y_pad;
			state->y2 = state->y1 +
				state->plot_x_axis_top_text_height;

			gr_justify_box ( state );
			gr_render_text ( state );
		}
	}

	if ( state->size > 0 || state->gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			get_x_axis_label_height ( state );

			if ( state->h > state->plot_x_axis_top_label_height )
			{
				state->plot_x_axis_top_label_height = state->h;
			}
		}
		else	// pass == 2
		{
			if ( state->size > 0 )
			{
				// We have an axis line and marks to render

				state->x1 = state->plot_x1;
				state->x2 = state->plot_x2;
				state->y1 = state->y2 = state->plot_y1;

				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_SQUARE );
				gr_render_line ( state );
				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_BUTT );

				if ( state->gap > 0 )
				{
					double		x;


					for (	x = MIN ( state->x_axis1,
							state->x_axis2 );
						x <= MAX ( state->x_axis1,
							state->x_axis2 );
						x += state->gap )
					{
						draw_x_axis_mark ( state, x,
							state->plot_y1,
							state->plot_y1 -
								state->size );
					}
				}
			}

			if ( state->gap > 0 )
			{
				render_x_axis_labels ( state,
					state->plot_y1 + state->y_pad / 2 -
					state->plot_x_axis_top_label_height,
					get_label_format_cell ( state, node )
					);
			}
		}
	}

	return 0;	// Success
}

static void get_y_axis_label_width (
	renSTATE	* const	state,
	mtUtreeNode	* const	node
	)
{
	char		* tmp = state->text;


	render_y_axis_labels ( state, 1,
		get_label_format_cell ( state, node ) );

	state->w += state->size + state->x_pad;

	state->text = tmp;
}

static int renfunc_plot_y_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	double		temp;


	// Sanity checking
	if (	state->gap > 0 &&
		abs ( (int)(state->y_axis2 - state->y_axis1) ) / state->gap >
			GR_MAX_ITEMS
		)
	{
		return 1;
	}

	cairo_rotate ( state->cr, -M_PI/2 );

	if ( gr_get_text_extents ( state ) )
	{
		// We have text to render

		state->w = state->logical_rect.height;
		state->h = state->ink_rect.width;

		if ( pass == 1 )
		{
			state->w += state->x_pad;

			if ( state->w > state->plot_y_axis_text_width )
			{
				state->plot_y_axis_text_width = state->w;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->x1 = state->graph_x1 + state->graph_x_pad;
			state->x2 = state->x1 + state->plot_y_axis_text_width;

			state->y1 = state->graph_y1 + state->graph_y_pad +
				state->plot_x_axis_top_text_height +
				state->plot_x_axis_top_label_height;

			state->y2 = state->graph_y2 - state->graph_y_pad -
				state->plot_x_axis_text_height -
				state->plot_x_axis_label_height;

			gr_justify_box ( state );

			temp = state->x;

			state->x = -state->y - state->ink_rect.width;
			state->y = temp;

			gr_render_text ( state );
		}
	}

	cairo_identity_matrix ( state->cr );
	cairo_scale ( state->cr, state->scale, state->scale );

	if ( state->size > 0 || state->gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			get_y_axis_label_width ( state, node );

			if ( state->w > state->plot_y_axis_label_width )
			{
				state->plot_y_axis_label_width = state->w;
			}
		}
		else	// pass == 2
		{
			if ( state->size > 0 )
			{
				// We have an axis line and marks to render

				state->x1 = state->plot_x1;
				state->x2 = state->plot_x1;
				state->y1 = state->plot_y1;
				state->y2 = state->plot_y2;

				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_SQUARE );

				gr_render_line ( state );

				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_BUTT );

				if ( state->gap > 0 )
				{
					double		y;


					for (	y = MIN ( state->y_axis1,
							state->y_axis2 );
						y <= MAX ( state->y_axis1,
							state->y_axis2 );
						y += state->gap )
					{
						draw_y_axis_mark ( state, y,
							state->plot_x1,
							state->plot_x1 -
								state->size );
					}
				}
			}

			if ( state->gap > 0 )
			{
				state->x_justify = 1.0;

				state->x1 = state->plot_x1 -
					state->plot_y_axis_text_width +
					state->x_pad / 2;

				state->x2 = state->plot_x1 - state->size -
					state->x_pad / 2;

				render_y_axis_labels ( state, pass,
					get_label_format_cell ( state, node ) );
			}
		}
	}

	return 0;	// Success
}

static int renfunc_plot_y_axis_right (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	double		temp;


	// Sanity checking
	if (	state->gap > 0 &&
		abs ( (int)(state->y_axis2 - state->y_axis1) ) / state->gap >
			GR_MAX_ITEMS
		)
	{
		return 1;
	}

	cairo_rotate ( state->cr, -M_PI/2 );

	if ( gr_get_text_extents ( state ) )
	{
		// We have text to render

		state->w = state->logical_rect.height;
		state->h = state->ink_rect.width;

		if ( pass == 1 )
		{
			state->w += state->x_pad;

			if ( state->w > state->plot_y_axis_right_text_width )
			{
				state->plot_y_axis_right_text_width = state->w;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->x2 = state->graph_x2 - state->graph_x_pad;
			state->x1 = state->x2 -
				state->plot_y_axis_right_text_width;

			state->y1 = state->graph_y1 + state->graph_y_pad +
				state->plot_x_axis_top_text_height +
				state->plot_x_axis_top_label_height;

			state->y2 = state->graph_y2 - state->graph_y_pad -
				state->plot_x_axis_text_height -
				state->plot_x_axis_label_height;

			gr_justify_box ( state );

			temp = state->x;

			state->x = -state->y - state->ink_rect.width;
			state->y = temp;

			gr_render_text ( state );
		}
	}

	cairo_identity_matrix ( state->cr );
	cairo_scale ( state->cr, state->scale, state->scale );

	if ( state->size > 0 || state->gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			get_y_axis_label_width ( state, node );

			if ( state->w > state->plot_y_axis_right_label_width )
			{
				state->plot_y_axis_right_label_width = state->w;
			}
		}
		else	// pass == 2
		{
			if ( state->size > 0 )
			{
				// We have an axis line and marks to render

				state->x1 = state->plot_x2;
				state->x2 = state->plot_x2;
				state->y1 = state->plot_y1;
				state->y2 = state->plot_y2;

				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_SQUARE );

				gr_render_line ( state );

				cairo_set_line_cap ( state->cr,
					CAIRO_LINE_CAP_BUTT );

				if ( state->gap > 0 )
				{
					double		y;


					for (	y = MIN ( state->y_axis1,
							state->y_axis2 );
						y <= MAX ( state->y_axis1,
							state->y_axis2 );
						y += state->gap
						)
					{
						draw_y_axis_mark ( state, y,
							state->plot_x2,
							state->plot_x2 +
								state->size );
					}
				}
			}

			if ( state->gap > 0 )
			{
				state->x_justify = 0.0;

				state->x1 = state->plot_x2 + state->size +
					state->x_pad / 2;

				state->x2 = state->plot_x2 +
					state->plot_y_axis_text_width -
					state->x_pad / 2;

				render_y_axis_labels ( state, pass,
					get_label_format_cell ( state, node ) );
			}
		}
	}

	return 0;	// Success
}



///	RENDER

static mtUtreeNode * get_utree (
	CedBook		* const	book,
	char	const	* const	graph_name,
	int		* const	breakpoint
	)
{
	CedBookFile	* graph;
	mtUtreeNode	* node = NULL;
	int		bpch = 0;


	if (	! book ||
		! (graph = cui_graph_get ( book, graph_name ) )
		)
	{
		return NULL;
	}

	if ( graph->size > 0 )
	{
		char		* bperr = NULL;


		node = mtkit_utree_load_mem ( NULL, graph->mem,
			(size_t)graph->size, &bperr );

		if ( bperr )
		{
			if ( node )
			{
				mtkit_utree_destroy_node ( node );
				node = NULL;
			}

			if ( graph->mem == bperr )
			{
				bpch = 0;
			}
			else
			{
				bpch = mtkit_utf8_len (
					(unsigned char *)graph->mem,
					(size_t)(bperr - graph->mem) );
			}
		}
	}

	if ( ! node && breakpoint )
	{
		breakpoint[0] = bpch;
	}

	return node;
}



typedef struct
{
	char	const	* name;
	renFUNC		func;
} renFUNCtable;



// NOTE - Table must remain strictly in alpha order!

static renFUNCtable	const	fn_table[] = {
{ "graph",		renfunc_graph			},
{ "page",		renfunc_page			},
{ "plot",		renfunc_plot			},
{ "plot_box",		renfunc_plot_box		},
{ "plot_ellipse",	renfunc_plot_ellipse		},
{ "plot_graph_bar",	renfunc_plot_graph_bar		},
{ "plot_graph_line",	renfunc_plot_graph_line		},
{ "plot_line",		renfunc_plot_line		},
{ "plot_x_axis",	renfunc_plot_x_axis		},
{ "plot_x_axis_grid",	renfunc_plot_x_axis_grid	},
{ "plot_x_axis_top",	renfunc_plot_x_axis_top		},
{ "plot_y_axis",	renfunc_plot_y_axis		},
{ "plot_y_axis_grid",	renfunc_plot_y_axis_grid	},
{ "plot_y_axis_right",	renfunc_plot_y_axis_right	},
{ "x_axis",		renfunc_x_axis			},
{ "y_axis",		renfunc_y_axis			}
};



static int execute_node (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	int		a, b, c,
			res,
			fn_table_size;


	// Quietly ignore stuff we don't need
	if (	node->type != MTKIT_UTREE_NODE_TYPE_ELEMENT ||
		! node->text
		)
	{
		return 0;
	}

	fn_table_size = sizeof ( fn_table ) / sizeof ( fn_table[0] );
	a = 0;
	c = fn_table_size - 1;

	while ( a <= c )		// Binary search
	{
		b = ( a + c ) >> 1;

		res = strcmp ( node->text, fn_table[b].name );

		if ( res == 0 )
		{
			// Instruction found: Get defaults and execute function
			gr_utree_get_defaults ( state, node );

			return fn_table[b].func ( state, node, pass );
		}

		// There is no match, so do we check the upper or lower section
		// next?
		if ( res < 0 )
		{
			c = b - 1;
		}
		else
		{
			a = b + 1;
		}
	}

	return 0;			// Success
}

static int prepare_page (
	renSTATE	* const	state
	)
{
	if (	state->page_width < GR_PAGE_MIN ||
		state->page_width > GR_PAGE_MAX ||
		state->page_height < GR_PAGE_MIN ||
		state->page_height > GR_PAGE_MAX )
	{
		return 1;
	}

	state->graph_x1 = state->page_x_pad;
	state->graph_x2 = state->page_width - state->page_x_pad;

	state->graph_y1 = state->page_y_pad;
	state->graph_y2 = state->page_height - state->page_y_pad;

	state->plot_x1 = state->graph_x1 + state->graph_x_pad +
		state->plot_x_pad + state->plot_y_axis_text_width +
		state->plot_y_axis_label_width;

	state->plot_x2 = state->graph_x2 - state->graph_x_pad -
		state->plot_x_pad - state->plot_y_axis_right_text_width -
		state->plot_y_axis_right_label_width;

	state->plot_y1 = state->graph_y1 + state->graph_y_pad +
		state->plot_y_pad + state->plot_x_axis_top_text_height +
		state->plot_x_axis_top_label_height;

	state->plot_y2 = state->graph_y2 - state->graph_y_pad -
		state->plot_y_pad - state->plot_x_axis_text_height -
		state->plot_x_axis_label_height;

	return 0;		// Success - caller can render page
}

static int renstate_init (
	renSTATE	* const	state,
	CedBook		* const	book,
	mtUtreeNode	* const	root,
	int		const	filetype,
	char	const	* const	filename,
	double			scale
	)
{
	mtUtreeNode	* node;
	double		pw, ph;


	state->book = book;

	if (	scale < 0.1 ||
		scale > 1000 )
	{
		scale = 1;
	}

	if (	filetype != CUI_GRAPH_TYPE_PNG	&&
		filetype != -1
		)
	{
		// Ensures clipping is always on an integer for line width of 1
		// as Cairo doesn't work with clip points with decimal places
		// for some reason.  Only needed for vector based files.
		scale *= 2;
	}

	state->scale = scale;

	// Set up dummy surface/context to allow text extents to be calculated
	// in pass 1.

	state->surface = cairo_image_surface_create ( CAIRO_FORMAT_RGB24,
		10, 10 );

	if ( cairo_surface_status ( state->surface ) != CAIRO_STATUS_SUCCESS )
	{
		goto error;
	}

	state->cr = cairo_create ( state->surface );
	state->font_desc = pango_font_description_from_string ( "Sans" );
	state->p_layout = pango_cairo_create_layout ( state->cr );

	// PASS 1 = traverse tree and populate the state with any useful values
	for ( node = root->child; node != NULL; node = node->next )
	{
		execute_node ( state, node, 1 );
	}

	cairo_destroy ( state->cr );
	cairo_surface_destroy ( state->surface );
	state->cr = NULL;
	state->surface = NULL;

	if (	state->page_width < 1 ||
		state->page_height < 1 )
	{
		// Bad page size
		goto error;
	}

	pw = state->page_width * scale;
	ph = state->page_height * scale;

	// Create new surface based on new geometry and target file
	switch ( filetype )
	{
#ifdef USE_CAIRO_EPS
	case CUI_GRAPH_TYPE_EPS:
		state->surface = cairo_ps_surface_create ( filename, pw, ph );
		cairo_ps_surface_set_eps ( state->surface, 1 );
		break;
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
	case CUI_GRAPH_TYPE_PDF:
		state->surface = cairo_pdf_surface_create ( filename, pw, ph );
		break;
#endif

#ifdef CAIRO_HAS_PNG_FUNCTIONS
	case CUI_GRAPH_TYPE_PNG:
#endif
	case -1:
		state->surface = cairo_image_surface_create (
			CAIRO_FORMAT_RGB24, (int)pw, (int)ph );
		break;

#ifdef CAIRO_HAS_PS_SURFACE
	case CUI_GRAPH_TYPE_PS:
		state->surface = cairo_ps_surface_create ( filename, pw, ph );
		break;
#endif

#ifdef CAIRO_HAS_SVG_SURFACE
	case CUI_GRAPH_TYPE_SVG:
		state->surface = cairo_svg_surface_create ( filename, pw, ph );
		break;
#endif
	default:
		goto error;		// Unknown
	}

	if ( cairo_surface_status ( state->surface ) != CAIRO_STATUS_SUCCESS )
	{
		goto error;
	}

	state->cr = cairo_create ( state->surface );

	cairo_scale ( state->cr, scale, scale );

	if ( prepare_page ( state ) )
	{
		goto error;
	}

	// PASS 2 = traverse tree and render the graph
	for ( node = root->child; node != NULL; node = node->next )
	{
		execute_node ( state, node, 2 );
	}

#ifdef CAIRO_HAS_PNG_FUNCTIONS
	if ( filetype == CUI_GRAPH_TYPE_PNG )
	{
		if ( cairo_surface_write_to_png ( state->surface, filename ) !=
			CAIRO_STATUS_SUCCESS )
		{
			goto error;
		}
	}
#endif

	return 0;			// Success

error:
	return 1;			// Fail - caller cleans up
}

static void renstate_destroy (
	renSTATE	* state
	)
{
	if ( state->cr )
	{
		cairo_destroy ( state->cr );
		state->cr = NULL;
	}

	if ( state->surface )
	{
		cairo_surface_destroy ( state->surface );
		state->surface = NULL;
	}

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

	mtkit_strfreedup ( &state->text, NULL );
	mtkit_strfreedup ( &state->data_txt, NULL );
}

mtPixy::Image * cui_graph_render_image (
	CedBook		* const	book,
	char	const	* const	graph_name,
	int		* const	breakpoint,
	double		const	scale
	)
{
	mtPixy::Image	* image = NULL;
	mtUtreeNode	* node = NULL;
	renSTATE	state;


	node = get_utree ( book, graph_name, breakpoint );
	if ( ! node )
	{
		return NULL;
	}

	if ( renstate_init ( &state, book, node, -1, NULL, scale ) )
	{
		if ( breakpoint )
		{
			breakpoint[0] = -1;
		}

		goto error;
	}

	image = mtPixy::image_from_cairo ( state.surface );
	if ( ! image )
	{
		if ( breakpoint )
		{
			breakpoint[0] = -1;
		}

		goto error;
	}

error:
	renstate_destroy ( &state );
	mtkit_utree_destroy_node ( node );

	return image;
}


int cui_graph_render_file (
	CedBook		* const	book,
	char	const	* const	graph_name,
	char	const	* const	filename,
	int		const	filetype,
	int		* const	breakpoint,
	double		const	scale
	)
{
	mtUtreeNode	* node = NULL;
	renSTATE	state;
	int		res = 1;


	node = get_utree ( book, graph_name, breakpoint );
	if ( ! node )
	{
		return 1;
	}

	if ( renstate_init ( &state, book, node, filetype, filename, scale ) )
	{
		if ( breakpoint )
		{
			breakpoint[0] = -1;
		}

		goto error;
	}

	res = 0;			// Success

error:
	renstate_destroy ( &state );
	mtkit_utree_destroy_node ( node );

	return res;
}

