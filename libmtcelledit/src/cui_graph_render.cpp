/*
	Copyright (C) 2011-2022 Mark Tyler

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

#include "cui_graph.h"



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



///	PAGE

static int renfunc_page (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	mtBulkDouble	table_d[] = {
			{ "width",	&state->m_page_width	},
			{ "height",	&state->m_page_height	},
			{ NULL,		NULL }
			};


	if ( pass == 2 )
	{
		state->m_x1 = 0;
		state->m_y1 = 0;
		state->m_x2 = state->m_page_width;
		state->m_y2 = state->m_page_height;

		state->render_box ( 1 );

		return 0;
	}

	state->m_page_width = GR_DEFAULT_PAGE_WIDTH;
	state->m_page_height = GR_DEFAULT_PAGE_HEIGHT;

	// Disallows negative values
	if ( state->m_x_pad > state->m_page_x_pad )
	{
		state->m_page_x_pad = state->m_x_pad;
	}

	if ( state->m_y_pad > state->m_page_y_pad )
	{
		state->m_page_y_pad = state->m_y_pad;
	}

	gr_utree_bulk_parse ( state->m_sheet, node, table_d );

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
		state->m_x1 = state->m_graph_x1;
		state->m_y1 = state->m_graph_y1;
		state->m_x2 = state->m_graph_x2;
		state->m_y2 = state->m_graph_y2;

		state->render_box ( 1 );

		return 0;
	}

	// Disallows negative values
	if ( state->m_x_pad > state->m_graph_x_pad )
	{
		state->m_graph_x_pad = state->m_x_pad;
	}

	if ( state->m_y_pad > state->m_graph_y_pad )
	{
		state->m_graph_y_pad = state->m_y_pad;
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
		state->m_x1 = state->m_plot_x1;
		state->m_y1 = state->m_plot_y1;
		state->m_x2 = state->m_plot_x2;
		state->m_y2 = state->m_plot_y2;

		state->render_box ( 0 );

		return 0;
	}

	// Disallows negative values
	if ( state->m_x_pad > state->m_plot_x_pad )
	{
		state->m_plot_x_pad = state->m_x_pad;
	}

	if ( state->m_y_pad > state->m_plot_y_pad )
	{
		state->m_plot_y_pad = state->m_y_pad;
	}

	if ( state->m_line_color >= 0 )
	{
		state->m_plot_line_size = state->m_line_size;
	}

	return 0;			// Success
}

static int renfunc_x_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	ARG_UNUSED ( pass )
	)
{
	state->m_x_axis1 = state->m_min;
	state->m_x_axis2 = state->m_max;

	if ( state->m_x_axis1 == state->m_x_axis2 )
	{
		state->m_lpx = 0;
	}
	else
	{
		state->m_lpx =	( state->m_plot_x2 - state->m_plot_x1 ) /
				( state->m_x_axis2 - state->m_x_axis1 );
	}

	return 0;			// Success
}

static int renfunc_y_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	ARG_UNUSED ( pass )
	)
{
	state->m_y_axis1 = state->m_min;
	state->m_y_axis2 = state->m_max;

	if ( state->m_y_axis1 == state->m_y_axis2 )
	{
		state->m_lpy = 0;
	}
	else
	{
		state->m_lpy =	( state->m_plot_y2 - state->m_plot_y1 ) /
				( state->m_y_axis2 - state->m_y_axis1 );
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
	if (	cell->type != CED_CELL_TYPE_VALUE &&
		cell->type != CED_CELL_TYPE_FORMULA &&
		cell->type != CED_CELL_TYPE_FORMULA_EVAL &&
		cell->type != CED_CELL_TYPE_DATE
		)
	{
		return 0;
	}

	// Find out which item we are on
	renSTATE	* const	state = static_cast<renSTATE *>(user_data);
	int		const	i = MAX ( row - state->m_r1, col - state->m_c1);

	state->m_x1 = state->m_x_axis1 + i * state->m_gap;
	state->m_x2 = state->m_x1 + state->m_gap;
	state->m_y1 = 0;
	state->m_y2 = cell->value;

	if ( state->m_x1 > state->m_x_axis2 )
	{
		return 1;		// Finish
	}

	state->logical_to_physical ();
	state->render_box ( 0 );

	return 0;			// Continue
}



///	PLOT

static int scan_box_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	renSTATE	* const	state = static_cast<renSTATE *>(user_data);


	if ( state->plot_read_xycoords ( row, col ) )
	{
		return 0;
	}

	if ( state->m_tmp_i_a == 0 )
	{
		state->render_box ( 0 );
	}
	else
	{
		state->render_ellipse ();
	}

	if ( ! state->get_text_extents ( state->m_cbuf ) )
	{
		return 0;
	}

	state->m_x1 += state->m_x_pad;
	state->m_x2 -= state->m_x_pad;
	state->m_y1 += state->m_y_pad;
	state->m_y2 -= state->m_y_pad;

	state->justify_box ();
	state->render_text ( state->m_cbuf );

	return 0;			// Continue
}

static int renfunc_plot_boxellipse (
	int		const	type,
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED(node),
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;		// Nothing to do
	}

	if ( state->plot_state_init () )
	{
		return 1;
	}

	state->m_tmp_i_a = type;

	ced_sheet_scan_area ( state->m_sheet, state->m_r1, state->m_c1,
		state->m_r2 - state->m_r1 + 1, state->m_c2 - state->m_c1 + 1,
		scan_box_cb, state );

	state->plot_state_finish ();

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
	renSTATE	* const	state = static_cast<renSTATE *>(user_data);
	double		xj, yj, xo, yo;


	if ( state->plot_read_xycoords ( row, col ) )
	{
		return 0;
	}

	state->render_line ();

	if ( ! state->get_text_extents ( state->m_cbuf ) )
	{
		return 0;
	}

	xo = state->m_x_pad * cos ( state->m_line_angle );
	yo = state->m_y_pad * sin ( state->m_line_angle );

	state->m_x1 = state->m_x2 - state->m_w + xo;
	state->m_y1 = state->m_y2 - state->m_h - yo;
	state->m_x2 = state->m_x2 + state->m_w + xo;
	state->m_y2 = state->m_y2 + state->m_h - yo;

	xj = state->m_x_justify;
	yj = state->m_y_justify;

	if ( xj == 0.5 && yj == 0.5 )
	{
		double		ta[4];


		/* Automatically set the text position around the end of the
		line */

		ta[0] = atan ( state->m_h / state->m_w );
		ta[1] = M_PI - ta[0];
		ta[2] = M_PI + ta[0];
		ta[3] = 2 * M_PI - ta[0];

		if (	state->m_line_angle <= ta[0] ||
			state->m_line_angle >= ta[3]
			)
		{
			// Line hits left side of text box

			state->m_x_justify = 1.0;
			state->m_y_justify = 0.5 - state->m_w *
				tan ( state->m_line_angle ) /
				( 2 * state->m_h );
		}
		else if ( state->m_line_angle <= ta[1] )
		{
			// Line hits bottom side of text box

			if ( state->m_line_angle != M_PI / 2 )
			{
				/* state->line_angle == PI / 2 =>
					state->x_justify = 0.5; */

				state->m_x_justify = 0.5 + state->m_h /
					( 2 * state->m_w *
					tan ( state->m_line_angle ) );
			}

			state->m_y_justify = 0.0;
		}
		else if ( state->m_line_angle <= ta[2] )
		{
			// Line hits right side of text box

			state->m_x_justify = 0.0;
			state->m_y_justify = 0.5 + state->m_w *
				tan ( state->m_line_angle ) /
				( 2 * state->m_h );
		}
		else
		{
			// Line hits top side of text box

			if ( state->m_line_angle != 3 * M_PI / 2 )
			{
				/* state->m_line_angle == 3 * PI / 2 =>
					state->m_x_justify = 0.5; */

				state->m_x_justify = 0.5 - state->m_h /
					( 2 * state->m_w *
					tan ( state->m_line_angle ) );
			}

			state->m_y_justify = 1.0;
		}
	}

	state->justify_box ();
	state->render_text ( state->m_cbuf );

	state->m_x_justify = xj;
	state->m_y_justify = yj;

	return 0;			// Continue
}

static int renfunc_plot_line (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED(node),
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;		// Nothing to do
	}

	if ( state->plot_state_init () )
	{
		return 1;
	}

	ced_sheet_scan_area ( state->m_sheet, state->m_r1, state->m_c1,
		state->m_r2 - state->m_r1 + 1, state->m_c2 - state->m_c1 + 1,
		plotline_scan_cb, state );

	state->plot_state_finish ();

	return 0;
}

static int renfunc_plot_graph_bar (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED(node),
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;
	}

	if ( state->plot_state_init () )
	{
		return 1;
	}

	// Have single row or column of data
	if ( state->m_r1 != state->m_r2 )
	{
		// All data in this column
		state->m_c2 = state->m_c1;
	}

	ced_sheet_scan_area ( state->m_sheet, state->m_r1, state->m_c1,
		state->m_r2 - state->m_r1 + 1, state->m_c2 - state->m_c1 + 1,
		scan_bar_cb, state );

	state->plot_state_finish ();

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
	renSTATE	* const	state = static_cast<renSTATE *>(user_data);

	if ( (row - 1) > state->m_tmp_i_a )
	{
		// We have skipped one or more empty cells so impose a move
		state->m_r = 0;
	}

	if (	cell->type != CED_CELL_TYPE_VALUE &&
		cell->type != CED_CELL_TYPE_FORMULA &&
		cell->type != CED_CELL_TYPE_FORMULA_EVAL &&
		cell->type != CED_CELL_TYPE_DATE
		)
	{
		state->m_r = 0;

		return 0;		// This cell has no numerical data
	}

	// Find out which item we are on
	int const i = MAX ( row - state->m_r1, col - state->m_c1 );

	state->m_x1 = state->m_x_axis1 + i * state->m_gap;
	state->m_y1 = cell->value;
	state->m_tmp_i_a = row;

	if ( state->m_x1 > state->m_x_axis2 )
	{
		return 1;		// Finish
	}

	cairo_t * const cr = state->m_canvas.get_cairo ();

	state->logical_to_physical ();

	if ( state->m_r == 0 )
	{
		cairo_move_to ( cr, state->m_x1, state->m_y1 );
	}
	else
	{
		state->m_c ++;
		cairo_line_to ( cr, state->m_x1, state->m_y1 );
	}

	state->m_r ++;

	return 0;			// Continue
}

static int renfunc_plot_graph_line (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED(node),
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;
	}

	if ( state->plot_state_init () )
	{
		return 1;
	}

	// Have single row or column of data
	if ( state->m_r1 != state->m_r2 )
	{
		// All data in this column
		state->m_c2 = state->m_c1;
	}

	cairo_t * const cr = state->m_canvas.get_cairo ();

	cairo_set_line_cap ( cr, CAIRO_LINE_CAP_ROUND );
	cairo_set_line_join ( cr, CAIRO_LINE_JOIN_ROUND );
	state->set_rgb ( (int)state->m_line_color );
	cairo_set_line_width ( cr, state->m_line_size );

	state->m_r = 0;			// 0 = move next >0 = draw next
	state->m_c = 0;			// Total lines drawn
	state->m_tmp_i_a = 0;		// Last active row found

	ced_sheet_scan_area ( state->m_sheet, state->m_r1, state->m_c1,
		state->m_r2 - state->m_r1 + 1, state->m_c2 - state->m_c1 + 1,
		gline_scan_cb, state );

	if ( state->m_c > 0 )
	{
		// Draw the line if we have enough points
		cairo_stroke ( cr );
	}

	cairo_set_line_join ( cr, CAIRO_LINE_JOIN_MITER );
	cairo_set_line_cap ( cr, CAIRO_LINE_CAP_BUTT );

	state->plot_state_finish ();

	return 0;
}



///	GRID

static int renfunc_plot_x_axis_grid (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;
	}

	if (	state->m_gap <= 0 ||
		state->m_y_axis2 == state->m_y_axis1 ||
		abs ( (int)(state->m_y_axis2 - state->m_y_axis1) ) /
			state->m_gap > GR_MAX_ITEMS
		)
	{
		return 1;
	}

	state->m_x1 = state->m_x_axis1;
	state->m_x2 = state->m_x_axis2;

	// Convert logical coordinates to physical page coordinates
	state->m_x1 = state->m_plot_x1 + (state->m_x1 - state->m_x_axis1) *
		state->m_lpx;
	state->m_x2 = state->m_plot_x1 + (state->m_x2 - state->m_x_axis1) *
		state->m_lpx;

	double const f = fmod ( state->m_y_axis1 - state->m_min, state->m_gap );
	cairo_t * const cr = state->m_canvas.get_cairo ();

	cairo_set_line_cap ( cr, CAIRO_LINE_CAP_SQUARE );
	state->plot_clip_set ();

	if ( state->m_y_axis1 < state->m_y_axis2 )
	{
		double y;

		if ( f <= 0 )
		{
			y = state->m_y_axis1 - f;
		}
		else
		{
			y = state->m_y_axis1 + state->m_gap - f;
		}

		for ( ; y <= state->m_y_axis2; y += state->m_gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->m_y1 = state->m_plot_y1 + (y - state->m_y_axis1)*
				state->m_lpy;

			state->m_y2 = state->m_plot_y1 + (y - state->m_y_axis1)*
				state->m_lpy;

			state->render_line ();
		}
	}
	else
	{
		double y;

		if ( f < 0 )
		{
			y = state->m_y_axis1 + state->m_gap - f;
		}
		else
		{
			y = state->m_y_axis1 - f;
		}

		for ( ; y >= state->m_y_axis2; y -= state->m_gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->m_y1 = state->m_plot_y1 + (y - state->m_y_axis1)*
				state->m_lpy;

			state->m_y2 = state->m_plot_y1 + (y - state->m_y_axis1)*
				state->m_lpy;

			state->render_line ();
		}
	}

	state->plot_clip_clear ();
	cairo_set_line_cap ( cr, CAIRO_LINE_CAP_BUTT );

	return 0;			// Success
}

static int renfunc_plot_y_axis_grid (
	renSTATE	* const	state,
	mtUtreeNode	* const	ARG_UNUSED ( node ),
	int		const	pass
	)
{
	if ( pass == 1 )
	{
		return 0;
	}

	if (	state->m_gap <= 0 ||
		state->m_x_axis2 == state->m_x_axis1 ||
		abs ( (int)(state->m_x_axis2 - state->m_x_axis1) ) /
			state->m_gap > GR_MAX_ITEMS
		)
	{
		return 1;
	}

	state->m_y1 = state->m_y_axis1;
	state->m_y2 = state->m_y_axis2;

	// Convert logical coordinates to physical page coordinates
	state->m_y1 = state->m_plot_y1 + (state->m_y1 - state->m_y_axis1) *
		state->m_lpy;
	state->m_y2 = state->m_plot_y1 + (state->m_y2 - state->m_y_axis1) *
		state->m_lpy;

	double const f = fmod ( state->m_x_axis1 - state->m_min, state->m_gap );
	cairo_t * const cr = state->m_canvas.get_cairo ();

	cairo_set_line_cap ( cr, CAIRO_LINE_CAP_SQUARE );
	state->plot_clip_set ();

	if ( state->m_x_axis1 < state->m_x_axis2 )
	{
		double x;

		if ( f <= 0 )
		{
			x = state->m_x_axis1 - f;
		}
		else
		{
			x = state->m_x_axis1 + state->m_gap - f;
		}

		for ( ; x <= state->m_x_axis2; x += state->m_gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->m_x1 = state->m_plot_x1 + (x - state->m_x_axis1)*
				state->m_lpx;

			state->m_x2 = state->m_plot_x1 + (x - state->m_x_axis1)*
				state->m_lpx;

			state->render_line ();
		}
	}
	else
	{
		double x;

		if ( f < 0 )
		{
			x = state->m_x_axis1 + state->m_gap - f;
		}
		else
		{
			x = state->m_x_axis1 - f;
		}

		for ( ; x >= state->m_x_axis2; x -= state->m_gap )
		{
			/* Convert logical coordinates to physical page
			coordinates */

			state->m_x1 = state->m_plot_x1 + (x - state->m_x_axis1)*
				state->m_lpx;

			state->m_x2 = state->m_plot_x1 + (x - state->m_x_axis1)*
				state->m_lpx;

			state->render_line ();
		}
	}

	state->plot_clip_clear ();
	cairo_set_line_cap ( cr, CAIRO_LINE_CAP_BUTT );

	return 0;			// Success
}


///	AXIS

static int renfunc_plot_x_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	// Sanity checking
	if (	state->m_gap > 0 &&
		abs ( (int)(state->m_x_axis2 - state->m_x_axis1) ) /
			state->m_gap > GR_MAX_ITEMS
		)
	{
		return 1;
	}

	if ( state->get_text_extents ( state->m_cbuf ) )
	{
		// We have text to render

		if ( pass == 1 )
		{
			state->m_h += state->m_y_pad;

			if ( state->m_h > state->m_plot_x_axis_text_height )
			{
				state->m_plot_x_axis_text_height = state->m_h;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->m_x1 = state->m_graph_x1 + state->m_graph_x_pad +
				state->m_plot_y_axis_text_width +
				state->m_plot_y_axis_label_width;

			state->m_x2 = state->m_graph_x2 - state->m_graph_x_pad -
				state->m_plot_y_axis_right_text_width -
				state->m_plot_y_axis_right_label_width;

			state->m_y2 = state->m_graph_y2 - state->m_graph_y_pad;
			state->m_y1 = state->m_y2 -
				state->m_plot_x_axis_text_height;

			state->justify_box ();
			state->render_text ( state->m_cbuf );
		}
	}

	if ( state->m_size > 0 || state->m_gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			state->get_x_axis_label_height ();

			if ( state->m_h > state->m_plot_x_axis_label_height )
			{
				state->m_plot_x_axis_label_height = state->m_h;
			}
		}
		else	// pass == 2
		{
			if ( state->m_size > 0 )
			{
				// We have an axis line and marks to render

				state->m_x1 = state->m_plot_x1;
				state->m_x2 = state->m_plot_x2;
				state->m_y1 = state->m_y2 = state->m_plot_y2;

				cairo_t * const cr =state->m_canvas.get_cairo();

				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_SQUARE);
				state->render_line ();
				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_BUTT );

				if ( state->m_gap > 0 )
				{
					double		x;


					for (	x = MIN ( state->m_x_axis1,
							state->m_x_axis2 );
						x <= MAX ( state->m_x_axis1,
							state->m_x_axis2 );
						x += state->m_gap )
					{
						state->draw_x_axis_mark ( x,
							state->m_plot_y2,
							state->m_plot_y2 +
								state->m_size );
					}
				}
			}

			if ( state->m_gap > 0 )
			{
				state->render_x_axis_labels (
					state->m_plot_y2 + state->m_size +
						state->m_y_pad / 2,
					state->get_label_format_cell ( node )
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
	if (	state->m_gap > 0 &&
		abs ( (int)(state->m_x_axis2 - state->m_x_axis1) ) /
			state->m_gap > GR_MAX_ITEMS
		)
	{
		return 1;
	}

	if ( state->get_text_extents ( state->m_cbuf ) )
	{
		// We have text to render

		if ( pass == 1 )
		{
			state->m_h += state->m_y_pad;

			if ( state->m_h > state->m_plot_x_axis_top_text_height )
			{
				state->m_plot_x_axis_top_text_height =
					state->m_h;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->m_x1 = state->m_graph_x1 + state->m_graph_x_pad +
				state->m_plot_y_axis_text_width +
				state->m_plot_y_axis_label_width;

			state->m_x2 = state->m_graph_x2 - state->m_graph_x_pad -
				state->m_plot_y_axis_right_text_width -
				state->m_plot_y_axis_right_label_width;

			state->m_y1 = state->m_graph_y1 + state->m_graph_y_pad;
			state->m_y2 = state->m_y1 +
				state->m_plot_x_axis_top_text_height;

			state->justify_box ();
			state->render_text ( state->m_cbuf );
		}
	}

	if ( state->m_size > 0 || state->m_gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			state->get_x_axis_label_height ();

			if ( state->m_h > state->m_plot_x_axis_top_label_height)
			{
				state->m_plot_x_axis_top_label_height =
					state->m_h;
			}
		}
		else	// pass == 2
		{
			if ( state->m_size > 0 )
			{
				// We have an axis line and marks to render

				cairo_t * const cr =state->m_canvas.get_cairo();

				state->m_x1 = state->m_plot_x1;
				state->m_x2 = state->m_plot_x2;
				state->m_y1 = state->m_y2 = state->m_plot_y1;

				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_SQUARE);
				state->render_line ();
				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_BUTT );

				if ( state->m_gap > 0 )
				{
					double		x;


					for (	x = MIN ( state->m_x_axis1,
							state->m_x_axis2 );
						x <= MAX ( state->m_x_axis1,
							state->m_x_axis2 );
						x += state->m_gap )
					{
						state->draw_x_axis_mark ( x,
							state->m_plot_y1,
							state->m_plot_y1 -
								state->m_size );
					}
				}
			}

			if ( state->m_gap > 0 )
			{
				state->render_x_axis_labels (
					state->m_plot_y1 + state->m_y_pad / 2 -
					state->m_plot_x_axis_top_label_height,
					state->get_label_format_cell ( node )
					);
			}
		}
	}

	return 0;	// Success
}

static int renfunc_plot_y_axis (
	renSTATE	* const	state,
	mtUtreeNode	* const	node,
	int		const	pass
	)
{
	// Sanity checking
	if (	state->m_gap > 0 &&
		abs ( (int)(state->m_y_axis2 - state->m_y_axis1) ) /
			state->m_gap > GR_MAX_ITEMS
		)
	{
		return 1;
	}

	cairo_t * const cr = state->m_canvas.get_cairo ();

	cairo_rotate ( cr, -M_PI/2 );

	if ( state->get_text_extents ( state->m_cbuf ) )
	{
		// We have text to render

		state->m_w = state->m_logical_rect.height;
		state->m_h = state->m_ink_rect.width;

		if ( pass == 1 )
		{
			state->m_w += state->m_x_pad;

			if ( state->m_w > state->m_plot_y_axis_text_width )
			{
				state->m_plot_y_axis_text_width = state->m_w;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->m_x1 = state->m_graph_x1 + state->m_graph_x_pad;
			state->m_x2 = state->m_x1 +
				state->m_plot_y_axis_text_width;

			state->m_y1 = state->m_graph_y1 + state->m_graph_y_pad +
				state->m_plot_x_axis_top_text_height +
				state->m_plot_x_axis_top_label_height;

			state->m_y2 = state->m_graph_y2 - state->m_graph_y_pad -
				state->m_plot_x_axis_text_height -
				state->m_plot_x_axis_label_height;

			state->justify_box ();

			double const temp = state->m_x;

			state->m_x = -state->m_y - state->m_ink_rect.width;
			state->m_y = temp;

			state->render_text ( state->m_cbuf );
		}
	}

	cairo_identity_matrix ( cr );
	cairo_scale ( cr, state->m_scale, state->m_scale );

	if ( state->m_size > 0 || state->m_gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			state->get_y_axis_label_width ( node );

			if ( state->m_w > state->m_plot_y_axis_label_width )
			{
				state->m_plot_y_axis_label_width = state->m_w;
			}
		}
		else	// pass == 2
		{
			if ( state->m_size > 0 )
			{
				// We have an axis line and marks to render

				state->m_x1 = state->m_plot_x1;
				state->m_x2 = state->m_plot_x1;
				state->m_y1 = state->m_plot_y1;
				state->m_y2 = state->m_plot_y2;

				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_SQUARE);

				state->render_line ();

				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_BUTT );

				if ( state->m_gap > 0 )
				{
					double		y;


					for (	y = MIN ( state->m_y_axis1,
							state->m_y_axis2 );
						y <= MAX ( state->m_y_axis1,
							state->m_y_axis2 );
						y += state->m_gap )
					{
						state->draw_y_axis_mark ( y,
							state->m_plot_x1,
							state->m_plot_x1 -
								state->m_size );
					}
				}
			}

			if ( state->m_gap > 0 )
			{
				state->m_x_justify = 1.0;

				state->m_x1 = state->m_plot_x1 -
					state->m_plot_y_axis_text_width +
					state->m_x_pad / 2;

				state->m_x2 = state->m_plot_x1 - state->m_size -
					state->m_x_pad / 2;

				state->render_y_axis_labels ( pass,
					state->get_label_format_cell ( node ) );
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
	// Sanity checking
	if (	state->m_gap > 0 &&
		abs ( (int)(state->m_y_axis2 - state->m_y_axis1) ) /
			state->m_gap > GR_MAX_ITEMS
		)
	{
		return 1;
	}

	cairo_t * const cr = state->m_canvas.get_cairo ();

	cairo_rotate ( cr, -M_PI/2 );

	if ( state->get_text_extents ( state->m_cbuf ) )
	{
		// We have text to render

		state->m_w = state->m_logical_rect.height;
		state->m_h = state->m_ink_rect.width;

		if ( pass == 1 )
		{
			state->m_w += state->m_x_pad;

			if ( state->m_w > state->m_plot_y_axis_right_text_width)
			{
				state->m_plot_y_axis_right_text_width =
					state->m_w;
			}
		}
		else	// pass == 2
		{
			// Corners of area for text
			state->m_x2 = state->m_graph_x2 - state->m_graph_x_pad;
			state->m_x1 = state->m_x2 -
				state->m_plot_y_axis_right_text_width;

			state->m_y1 = state->m_graph_y1 + state->m_graph_y_pad +
				state->m_plot_x_axis_top_text_height +
				state->m_plot_x_axis_top_label_height;

			state->m_y2 = state->m_graph_y2 - state->m_graph_y_pad -
				state->m_plot_x_axis_text_height -
				state->m_plot_x_axis_label_height;

			state->justify_box ();

			double const temp = state->m_x;

			state->m_x = -state->m_y - state->m_ink_rect.width;
			state->m_y = temp;

			state->render_text ( state->m_cbuf );
		}
	}

	cairo_identity_matrix ( cr );
	cairo_scale ( cr, state->m_scale, state->m_scale );

	if ( state->m_size > 0 || state->m_gap > 0 )
	{
		if ( pass == 1 )
		{
			// We have an axis line and marks to calculate extents
			state->get_y_axis_label_width ( node );

			if ( state->m_w > state->m_plot_y_axis_right_label_width )
			{
				state->m_plot_y_axis_right_label_width =
					state->m_w;
			}
		}
		else	// pass == 2
		{
			if ( state->m_size > 0 )
			{
				// We have an axis line and marks to render

				state->m_x1 = state->m_plot_x2;
				state->m_x2 = state->m_plot_x2;
				state->m_y1 = state->m_plot_y1;
				state->m_y2 = state->m_plot_y2;

				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_SQUARE);

				state->render_line ();

				cairo_set_line_cap ( cr, CAIRO_LINE_CAP_BUTT );

				if ( state->m_gap > 0 )
				{
					double		y;


					for (	y = MIN ( state->m_y_axis1,
							state->m_y_axis2 );
						y <= MAX ( state->m_y_axis1,
							state->m_y_axis2 );
						y += state->m_gap
						)
					{
						state->draw_y_axis_mark ( y,
							state->m_plot_x2,
							state->m_plot_x2 +
								state->m_size );
					}
				}
			}

			if ( state->m_gap > 0 )
			{
				state->m_x_justify = 0.0;

				state->m_x1 = state->m_plot_x2 + state->m_size +
					state->m_x_pad / 2;

				state->m_x2 = state->m_plot_x2 +
					state->m_plot_y_axis_text_width -
					state->m_x_pad / 2;

				state->render_y_axis_labels ( pass,
					state->get_label_format_cell ( node ) );
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
		char		const * bperr = NULL;


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



typedef int (* renFUNC) (
	renSTATE	* state,
	mtUtreeNode	* node,
	int		pass
	);

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
	// Quietly ignore stuff we don't need
	if (	node->type != MTKIT_UTREE_NODE_TYPE_ELEMENT ||
		! node->text
		)
	{
		return 0;
	}

	int const fn_table_size = (int)(sizeof(fn_table) / sizeof(fn_table[0]));
	int a = 0;
	int c = fn_table_size - 1;

	while ( a <= c )		// Binary search
	{
		int const b = ( a + c ) >> 1;
		int const res = strcmp ( node->text, fn_table[b].name );

		if ( res == 0 )
		{
			// Instruction found: Get defaults and execute function
			state->utree_get_defaults ( node );

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

static int renstate_init (
	renSTATE	* const	state,
	CedBook		* const	book,
	mtUtreeNode	* const	root,
	int		const	filetype,
	char	const	* const	filename,
	double			scale,
	char	const	* const	font_name
	)
{
	state->m_book = book;

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

	state->m_scale = scale;

	// Set up dummy surface/context to allow text extents to be calculated
	// in pass 1.

	if ( state->m_canvas.init ( mtPixy::Canvas::TYPE_PIXMAP, nullptr, 10,
		10 ) )
	{
		return 1;
	}

	// PASS 1 = traverse tree and populate the state with any useful values
	for ( mtUtreeNode *node = root->child; node != NULL; node = node->next )
	{
		execute_node ( state, node, 1 );
	}

	if (	state->m_page_width < 1 ||
		state->m_page_height < 1 )
	{
		// Bad page size
		return 1;
	}

	double const pw = state->m_page_width * scale;
	double const ph = state->m_page_height * scale;
	int res = 1;

	// Create new surface based on new geometry and target file
	switch ( filetype )
	{
	case CUI_GRAPH_TYPE_EPS:
		res = state->m_canvas.init ( mtPixy::Canvas::TYPE_EPS, filename,
			pw, ph );
		break;

	case CUI_GRAPH_TYPE_PDF:
		res = state->m_canvas.init ( mtPixy::Canvas::TYPE_PDF, filename,
			pw, ph );
		break;

	case CUI_GRAPH_TYPE_PNG:
	case -1:
		res = state->m_canvas.init ( mtPixy::Canvas::TYPE_PIXMAP,
			filename, pw, ph );
		break;

	case CUI_GRAPH_TYPE_PS:
		res = state->m_canvas.init ( mtPixy::Canvas::TYPE_PS, filename,
			pw, ph );
		break;

	case CUI_GRAPH_TYPE_SVG:
		res = state->m_canvas.init ( mtPixy::Canvas::TYPE_SVG, filename,
			pw, ph );
		break;
	}

	if ( res )
	{
		return 1;
	}

	cairo_scale ( state->m_canvas.get_cairo(), scale, scale );

	if ( font_name && font_name[0] )
	{
		state->m_canvas.set_font_name ( font_name );
	}

	if ( state->prepare_page () )
	{
		return 1;
	}

	// PASS 2 = traverse tree and render the graph
	for ( mtUtreeNode *node = root->child; node != NULL; node = node->next )
	{
		execute_node ( state, node, 2 );
	}

	if ( filetype == CUI_GRAPH_TYPE_PNG )
	{
		if ( state->m_canvas.save_png ( filename, 5 ) )
		{
			return 1;
		}
	}

	return 0;			// Success
}

mtPixmap * cui_graph_render_pixmap (
	CedBook		* const	book,
	char	const	* const	graph_name,
	int		* const	breakpoint,
	double		const	scale,
	char	const	* const	font_name
	)
{
	mtPixmap	* image = NULL;
	renSTATE	state;


	mtUtreeNode * const node = get_utree ( book, graph_name, breakpoint );
	if ( ! node )
	{
		return NULL;
	}

	if ( renstate_init ( &state, book, node, -1, NULL, scale, font_name ) )
	{
		if ( breakpoint )
		{
			breakpoint[0] = -1;
		}

		goto error;
	}

	image = pixy_pixmap_from_cairo ( state.m_canvas.get_surface () );
	if ( ! image )
	{
		if ( breakpoint )
		{
			breakpoint[0] = -1;
		}

		goto error;
	}

error:
	mtkit_utree_destroy_node ( node );

	return image;
}

int cui_graph_render_file (
	CedBook		* const	book,
	char	const	* const	graph_name,
	char	const	* const	filename,
	int		const	filetype,
	int		* const	breakpoint,
	double		const	scale,
	char	const	* const	font_name
	)
{
	renSTATE	state;
	int		res = 1;


	mtUtreeNode * const node = get_utree ( book, graph_name, breakpoint );
	if ( ! node )
	{
		return 1;
	}

	if ( renstate_init ( &state, book, node, filetype, filename, scale,
		font_name ) )
	{
		if ( breakpoint )
		{
			breakpoint[0] = -1;
		}

		goto error;
	}

	res = 0;			// Success

error:
	mtkit_utree_destroy_node ( node );

	return res;
}

