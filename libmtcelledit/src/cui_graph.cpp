/*
	Copyright (C) 2011-2024 Mark Tyler

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



renSTATE::~renSTATE()
{
	mtkit_strfreedup ( &m_data_txt, NULL );
}

void renSTATE::set_rgb ( int const rgb )
{
	m_canvas.set_color (
		(double)pixy_int_2_red ( rgb ) / 255.0,
		(double)pixy_int_2_green ( rgb ) / 255.0,
		(double)pixy_int_2_blue ( rgb ) / 255.0,
		1.0
		);
}

void renSTATE::logical_to_physical ()
{
	m_x1 = m_plot_x1 + (m_x1 - m_x_axis1) * m_lpx;
	m_x2 = m_plot_x1 + (m_x2 - m_x_axis1) * m_lpx;
	m_y1 = m_plot_y1 + (m_y1 - m_y_axis1) * m_lpy;
	m_y2 = m_plot_y1 + (m_y2 - m_y_axis1) * m_lpy;
}

void renSTATE::render_box ( int line_inside )
{
	if ( line_inside )
	{
		line_inside = 1;
	}

	if ( m_fill_color >= 0 )
	{
		set_rgb ( (int)m_fill_color );

		m_canvas.fill_rectangle( m_x1, m_y1, m_x2 - m_x1, m_y2 - m_y1 );
	}

	if (	m_fill_color != m_line_color &&
		m_line_color >= 0 )
	{
		set_rgb ( (int)m_line_color );
		m_canvas.set_stroke_width ( m_line_size );
		m_canvas.draw_rectangle (
			m_x1 + line_inside * m_line_size / 2,
			m_y1 + line_inside * m_line_size / 2,
			m_x2 - m_x1 - line_inside * m_line_size,
			m_y2 - m_y1 - line_inside * m_line_size
			);
	}
}

void renSTATE::render_text ( char const * const txt )
{
	if ( ! txt || m_text_color < 0 )
	{
		return;
	}

	set_rgb ( (int)m_text_color );

	cairo_t			* const cr = m_canvas.get_cairo ();
	PangoFontDescription	* const font_desc = m_canvas.get_font_desc ();
	PangoLayout		* const layout = m_canvas.get_layout ();

	cairo_save ( cr );

	cairo_translate ( cr, (int)(m_x + 0.5), (int)(m_y + 0.5) );
	pango_layout_set_text ( layout, txt, -1 );

	pango_font_description_set_size ( font_desc,
		(gint)( m_text_size * PANGO_SCALE * CUI_PANGO_SCALE ) );
	pango_layout_set_font_description ( layout, font_desc );

	pango_cairo_update_layout ( cr, layout );
	pango_cairo_show_layout ( cr, layout );

	cairo_restore ( cr );
}

void renSTATE::render_ellipse ()
{
	cairo_t	* const cr = m_canvas.get_cairo ();
	double	const tw = (m_x2 - m_x1) / 2;
	double	const th = (m_y2 - m_y1) / 2;

	if ( m_fill_color >= 0 )
	{
		set_rgb ( (int)m_fill_color );

		cairo_save ( cr );
		cairo_translate ( cr, m_x1 + tw, m_y1 + th );
		cairo_scale ( cr, tw, th );
		cairo_arc ( cr, 0, 0, 1, 0, 2 * M_PI );
		cairo_restore ( cr );

		cairo_fill ( cr );
	}

	if (	m_fill_color != m_line_color &&
		m_line_color >= 0 )
	{
		set_rgb ( (int)m_line_color );
		cairo_set_line_width ( cr, m_line_size );

		cairo_save ( cr );
		cairo_translate ( cr, m_x1 + tw, m_y1 + th );
		cairo_scale ( cr, tw, th );
		cairo_arc ( cr, 0, 0, 1, 0, 2 * M_PI );
		cairo_restore ( cr );

		cairo_stroke ( cr );
	}
}

void renSTATE::render_line ()
{
	double xx[2], yy[2], aa, rr;

	if ( m_x1 == m_x2 && m_y1 == m_y2 )
	{
		m_line_angle = 0;

		return;
	}

	if ( m_x1 == m_x2 )
	{
		if ( m_y1 > m_y2 )
		{
			aa = M_PI / 2;
		}
		else
		{
			aa = 3 * M_PI / 2;
		}
	}
	else
	{
		aa = -atan( (m_y2 - m_y1) / (m_x2 - m_x1) );

		if ( m_x1 > m_x2 )
		{
			aa += M_PI;
		}
	}

	// Ensure that 0 <= a <= 2 * M_PI
	aa = fmod ( aa, 2 * M_PI );
	if ( aa < 0 )
	{
		aa += 2 * M_PI;
	}

	cairo_t * const cr = m_canvas.get_cairo ();
	m_line_angle = aa;
	aa += M_PI;
	rr = m_arrowhead;

	if ( m_line_color >= 0 )
	{
		if ( m_arrowhead > 0 )
		{
			// This stops the end of the line spoiling the arrow tip
			xx[0] = m_x2 + rr * cos ( aa ) / 2;
			yy[0] = m_y2 - rr * sin ( aa ) / 2;
		}
		else
		{
			xx[0] = m_x2;
			yy[0] = m_y2;
		}

		set_rgb ( (int)m_line_color );

		cairo_set_line_width ( cr, m_line_size );
		cairo_move_to ( cr, m_x1, m_y1 );
		cairo_line_to ( cr, xx[0], yy[0] );
		cairo_stroke ( cr );
	}

	if ( m_fill_color >= 0 && m_arrowhead > 0 )
	{
		xx[0] = m_x2 + rr * cos ( aa + M_PI / 6 );
		yy[0] = m_y2 - rr * sin ( aa + M_PI / 6 );
		xx[1] = m_x2 + rr * cos ( aa - M_PI / 6 );
		yy[1] = m_y2 - rr * sin ( aa - M_PI / 6 );

		set_rgb ( (int)m_fill_color );

		cairo_move_to ( cr, xx[0], yy[0] );
		cairo_line_to ( cr, m_x2, m_y2 );
		cairo_line_to ( cr, xx[1], yy[1] );
		cairo_close_path ( cr );
		cairo_fill ( cr );
	}
}

void renSTATE::justify_box ()
{
	m_x = (1 - m_x_justify) * m_x1 + m_x_justify  * (m_x2 - m_w);
	m_y = (1 - m_y_justify) * m_y1 + m_y_justify  * (m_y2 - m_h);
}

int renSTATE::get_text_extents ( char const * const txt )
{
	if ( ! txt )
	{
		return 0;		// Nothing to do
	}

	PangoFontDescription	* const font_desc = m_canvas.get_font_desc ();
	PangoLayout		* const layout = m_canvas.get_layout ();

	pango_layout_set_text ( layout, txt, -1 );
	pango_font_description_set_size ( font_desc,
		(gint)( m_text_size * PANGO_SCALE * CUI_PANGO_SCALE ) );
	pango_layout_set_font_description ( layout, font_desc );
	pango_layout_get_pixel_extents ( layout,
		&m_ink_rect, &m_logical_rect );

	m_h = m_logical_rect.height;
	m_w = m_logical_rect.width;

	return 1;
}

int renSTATE::prepare_page ()
{
	if (	m_page_width < GR_PAGE_MIN	||
		m_page_width > GR_PAGE_MAX	||
		m_page_height < GR_PAGE_MIN	||
		m_page_height > GR_PAGE_MAX
		)
	{
		return 1;
	}

	m_graph_x1 = m_page_x_pad;
	m_graph_x2 = m_page_width - m_page_x_pad;

	m_graph_y1 = m_page_y_pad;
	m_graph_y2 = m_page_height - m_page_y_pad;

	m_plot_x1 = m_graph_x1 + m_graph_x_pad +
		m_plot_x_pad + m_plot_y_axis_text_width +
		m_plot_y_axis_label_width;

	m_plot_x2 = m_graph_x2 - m_graph_x_pad -
		m_plot_x_pad - m_plot_y_axis_right_text_width -
		m_plot_y_axis_right_label_width;

	m_plot_y1 = m_graph_y1 + m_graph_y_pad +
		m_plot_y_pad + m_plot_x_axis_top_text_height +
		m_plot_x_axis_top_label_height;

	m_plot_y2 = m_graph_y2 - m_graph_y_pad -
		m_plot_y_pad - m_plot_x_axis_text_height -
		m_plot_x_axis_label_height;

	return 0;		// Success - caller can render page
}

void renSTATE::draw_x_axis_mark (
	double		const	x,
	double		const	y1,
	double		const	y2
	)
{
	// Logical -> physical
	m_x1 = m_plot_x1 + (x - m_x_axis1) * m_lpx;
	m_x2 = m_x1;
	m_y1 = y1;
	m_y2 = y2;

	render_line ();
}

void renSTATE::draw_y_axis_mark (
	double		const	y,
	double		const	x1,
	double		const	x2
	)
{
	// Logical -> physical
	m_x1 = x1;
	m_x2 = x2;
	m_y1 = m_plot_y1 + (y - m_y_axis1) * m_lpy;
	m_y2 = m_y1;

	render_line ();
}

int renSTATE::setup_axis_cell (
	CedCell		* const	cell,
	CedCell	*	* const	newcell
	)
{
	if ( ! m_data_txt )
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


		if ( ced_strtocellrange ( m_data_txt, &r1, &r2, NULL, 1 ) )
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
		m_r = MAX ( 1, r1.row_d );
		m_c = MAX ( 1, r1.col_d );

		// Row/Column increment - restrict to left->right or top->bottom
		if ( r1.row_d == r2.row_d )
		{
			m_r1 = 0;
		}
		else
		{
			m_r1 = 1;
		}

		m_c1 = ! m_r1;
	}

	return 0;
}

void renSTATE::get_next_axis_cell ()
{
	CedCell const * const cell = ced_sheet_get_cell ( m_sheet, m_r, m_c );

	ced_cell_create_output ( cell, NULL, m_cbuf, sizeof(m_cbuf) );

	m_r += m_r1;
	m_c += m_c1;
}

void renSTATE::render_x_axis_labels (
	double		const	y,
	CedCell		* const	cell
	)
{
	CedCell		* newcell = NULL;

	setup_axis_cell ( cell, &newcell );

	double const xx1 = MIN ( m_x_axis1, m_x_axis2 );
	double const xx2 = MAX ( m_x_axis1, m_x_axis2 );

	for ( double x = xx1; x <= xx2; x += m_gap )
	{
		if ( x == xx2 && m_x_justify > 0 )
		{
			/* Only render the last label if sensible justification
			used. */

			break;
		}

		if ( newcell )
		{
			newcell->value = x;

			ced_cell_create_output ( newcell, NULL, m_cbuf,
				sizeof(m_cbuf) );
		}
		else
		{
			get_next_axis_cell ();
		}

		if ( ! m_cbuf[0] )
		{
			continue;
		}

		if ( get_text_extents ( m_cbuf ) )
		{
			// Logical -> physical
			m_x1 = m_plot_x1 + (x - m_x_axis1) * m_lpx - m_w / 2;

			m_x2 = m_plot_x1 + (x - m_x_axis1 + m_gap) * m_lpx +
				m_w / 2;

			m_y1 = y;
			m_y2 = m_y1 + m_h;

			justify_box ();
			render_text ( m_cbuf );
		}
	}

	if ( newcell )
	{
		ced_cell_destroy ( newcell );
		newcell = NULL;
	}
}

void renSTATE::render_y_axis_labels (
	int		const	pass,
	CedCell		* const	cell
	)
{
	double		max_width = 0;
	CedCell		* newcell = NULL;
	double	const	yy1 = MIN ( m_y_axis1, m_y_axis2 );
	double	const	yy2 = MAX ( m_y_axis1, m_y_axis2 );


	if ( m_gap <= 0 )
	{
		goto finish;
	}

	setup_axis_cell ( cell, &newcell );

	for ( double y = yy1; y <= yy2; y += m_gap )
	{
		if ( newcell )
		{
			newcell->value = y;

			ced_cell_create_output ( newcell, NULL, m_cbuf,
				sizeof(m_cbuf) );
		}
		else
		{
			get_next_axis_cell ();
		}

		if ( ! m_cbuf[0] )
		{
			continue;
		}

		if ( get_text_extents ( m_cbuf ) )
		{
			max_width = MAX ( m_w, max_width );

			// Logical -> physical
			m_y1 = m_plot_y1 + (y - m_y_axis1) * m_lpy - m_h / 2;
			m_y2 = m_plot_y1 + (y - m_y_axis1 + m_gap) * m_lpy +
				m_h / 2;

			justify_box ();
			render_text ( m_cbuf );
		}
	}

	if ( newcell )
	{
		ced_cell_destroy ( newcell );
		newcell = NULL;
	}

finish:

	if ( pass == 1 )
	{
		m_w = max_width;
	}
}

void renSTATE::get_x_axis_label_height ()
{
	m_h = 0;

	get_text_extents ( "123" );
	m_h += m_size + m_y_pad;
}

void renSTATE::get_y_axis_label_width ( mtUtreeNode * const node )
{
	render_y_axis_labels ( 1, get_label_format_cell ( node ) );

	m_w += m_size + m_x_pad;
}

CedCell * renSTATE::get_label_format_cell ( mtUtreeNode * const node )
{
	CedCell		* cell = NULL;
	CedCellRef	cref;
	char		* newtext = NULL;
	mtBulkStr	table_s[] = {
			{ "label_format",	&newtext },
			{ NULL,			NULL }
			};


	mtkit_utree_bulk_get ( node, NULL, NULL, table_s );
	if ( ! m_sheet || ! newtext )
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
		cell = ced_sheet_get_cell ( m_sheet, cref.row_d, cref.col_d );
	}

	free ( newtext );

	return cell;
}

void renSTATE::plot_clip_set ()
{
	double const x = m_plot_x1 + m_plot_line_size * 0.5;
	double const y = m_plot_y1 + m_plot_line_size * 0.5;
	double const w = m_plot_x2 - m_plot_x1 - m_plot_line_size;
	double const h = m_plot_y2 - m_plot_y1 - m_plot_line_size;

	cairo_t * const cr = m_canvas.get_cairo ();

	cairo_rectangle ( cr, x, y, w, h );
	cairo_clip ( cr );
}

void renSTATE::plot_clip_clear ()
{
	cairo_reset_clip ( m_canvas.get_cairo () );
}

int renSTATE::plot_state_init ()
{
	if (	! m_sheet		||
		! m_data_txt		||
		m_x_axis1 == m_x_axis2	||
		m_y_axis1 == m_y_axis2
		)
	{
		return 1;
	}

	CedCellRef	r1, r2;

	if ( ced_strtocellrange ( m_data_txt, &r1, &r2, NULL, 1 ) )
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

	m_r1 = MIN ( r1.row_d, r2.row_d );
	m_r2 = MAX ( r1.row_d, r2.row_d );
	m_c1 = MIN ( r1.col_d, r2.col_d );
	m_c2 = MAX ( r1.col_d, r2.col_d );

	plot_clip_set ();

	return 0;			// Success
}

void renSTATE::plot_state_finish ()
{
	plot_clip_clear ();
}

int renSTATE::plot_read_xycoords (
	int	const	row,
	int	const	col
	)
{
	CedCell		* cell;

	cell = ced_sheet_get_cell ( m_sheet, row, col );
	if ( ! cell )
	{
		return 1;
	}

	m_x1 = cell->value;

	cell = ced_sheet_get_cell ( m_sheet, row, col + 1 );
	if ( ! cell )
	{
		return 1;
	}

	m_x2 = cell->value;

	cell = ced_sheet_get_cell ( m_sheet, row, col + 2 );
	if ( ! cell )
	{
		return 1;
	}

	m_y1 = cell->value;

	cell = ced_sheet_get_cell ( m_sheet, row, col + 3 );
	if ( ! cell )
	{
		return 1;
	}

	m_y2 = cell->value;

	// Convert logical coordinates to physical page coordinates
	logical_to_physical ();

	cell = ced_sheet_get_cell ( m_sheet, row, col + 4 );
	if ( ced_cell_create_output ( cell, NULL, m_cbuf, sizeof(m_cbuf) ) )
	{
		return 1;
	}

	return 0;			// Success
}

int gr_utree_bulk_parse (
	CedSheet		* const	sheet,
	mtUtreeNode		* const	node,
	mtBulkDouble	const * const	table_d
	)
{
	CedParser	parser_state;


	for ( int i = 0; table_d[i].name != NULL; i++ )
	{
		// Find name in the node attributes - continue if absent
		mtTreeNode * const tn = mtkit_tree_node_find (
			node->attribute_tree, table_d[i].name );

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

int renSTATE::utree_get_defaults ( mtUtreeNode * const node )
{
	mtBulkDouble const table_d[] = {
		{ "arrowhead",		&m_arrowhead	},
		{ "fill_color",		&m_fill_color	},
		{ "text_color",		&m_text_color	},
		{ "line_color",		&m_line_color	},
		{ "text_size",		&m_text_size	},
		{ "line_size",		&m_line_size	},
		{ "x_justify",		&m_x_justify	},
		{ "y_justify",		&m_y_justify	},
		{ "x_pad",		&m_x_pad	},
		{ "y_pad",		&m_y_pad	},
		{ "antialias",		&m_antialias	},
		{ "size",		&m_size		},
		{ "gap",		&m_gap		},
		{ "min",		&m_min		},
		{ "max",		&m_max		},
		{ NULL,			NULL		}
		};

	mtTreeNode	* tn;
	mtBulkStr const table_s[] = {
		{ "data",		&m_data_txt	},
		{ NULL,			NULL		}
		};


	m_arrowhead = 0;
	m_fill_color = 0xFFFFFF;
	m_text_color = 0;
	m_line_color = 0;
	m_text_size = 12;
	m_line_size = 1;
	m_x_justify = 0.5;
	m_y_justify = 0.5;
	m_x_pad = 0;
	m_y_pad = 0;
	m_antialias = 1;
	m_size = 0;
	m_gap = 0;
	m_min = 0;
	m_max = 0;

	// Get the sheet first, as it may be referenced in the parsed text
	tn = mtkit_tree_node_find ( node->attribute_tree, "sheet" );
	if ( tn )
	{
		m_sheet = ced_book_get_sheet ( m_book, (char const *)tn->data );
		/* Note - we don't clear if attribute not set so it is inherited
		if absent */
	}

	mtkit_strfreedup ( &m_data_txt, NULL );
	mtkit_utree_bulk_get ( node, NULL, NULL, table_s );

	char const * val = nullptr;
	if (	mtkit_utree_get_attribute_string ( node, "text", &val )	||
		mtkit_strnncpy ( m_cbuf, val, sizeof(m_cbuf) )
		)
	{
		m_cbuf[0] = 0;
	}

	gr_utree_bulk_parse ( m_sheet, node, table_d );

	cairo_t * const cr = m_canvas.get_cairo ();

	if ( m_antialias == 1 )
	{
		cairo_set_antialias ( cr, CAIRO_ANTIALIAS_DEFAULT );
	}
	else
	{
		cairo_set_antialias ( cr, CAIRO_ANTIALIAS_NONE );
	}

	return 0;
}

