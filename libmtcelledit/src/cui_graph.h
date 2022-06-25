/*
	Copyright (C) 2011-2021 Mark Tyler

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

#ifndef CUI_GRAPH_H_
#define CUI_GRAPH_H_



#include "mtpixy_cairo.h"
#include "cui.h"



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif


#ifdef __cplusplus
extern "C" {
#endif



#define GR_PAGE_MIN			1.0
#define GR_PAGE_MAX			4000.0

#define GR_DEFAULT_PAGE_WIDTH		640.0
#define GR_DEFAULT_PAGE_HEIGHT		320.0

#define GR_MAX_ITEMS			10000



int gr_utree_bulk_parse (
	CedSheet		* sheet,
	mtUtreeNode		* node,
	mtBulkDouble	const	* table_d
	);



struct renSTATE
{
	~renSTATE();

	int prepare_page ();

	void set_rgb ( int rgb );
	void logical_to_physical ();
	void render_box ( int line_inside );
	void render_text ( char const * txt );
	void render_ellipse ();
	void render_line ();
	void justify_box ();
	int get_text_extents ( char const * txt );

	void draw_x_axis_mark ( double x, double y1, double y2 );
	void draw_y_axis_mark ( double y, double x1, double x2 );

	void render_x_axis_labels ( double y, CedCell * cell );
	void render_y_axis_labels ( int pass, CedCell * cell );

	void get_x_axis_label_height ();
	void get_y_axis_label_width ( mtUtreeNode * node );

	CedCell * get_label_format_cell ( mtUtreeNode * node );

	void plot_clip_set ();
	void plot_clip_clear ();
	int plot_state_init ();
	void plot_state_finish ();
	int plot_read_xycoords ( int row, int col );

	int utree_get_defaults ( mtUtreeNode * node );

/// ----------------------------------------------------------------------------

	CedBook		* m_book	= nullptr;
	CedSheet	* m_sheet	= nullptr;

	double		// Specifics - set after pass 1 ready for pass 2
			m_page_width	= 0,
			m_page_height	= 0,
			m_page_x_pad	= 0,
			m_page_y_pad	= 0,

			m_graph_x_pad	= 0,
			m_graph_y_pad	= 0,
			m_graph_x1	= 0,
			m_graph_x2	= 0,
			m_graph_y1	= 0,
			m_graph_y2	= 0,

			m_plot_x_pad	= 0,
			m_plot_y_pad	= 0,
			m_plot_x1	= 0,
			m_plot_x2	= 0,
			m_plot_y1	= 0,
			m_plot_y2	= 0,
			m_plot_line_size = 0,

			m_plot_x_axis_top_text_height	= 0,
			m_plot_x_axis_text_height	= 0,
			m_plot_y_axis_right_text_width	= 0,
			m_plot_y_axis_text_width	= 0,

			m_plot_x_axis_top_label_height	= 0,
			m_plot_x_axis_label_height	= 0,
			m_plot_y_axis_right_label_width	= 0,
			m_plot_y_axis_label_width	= 0,

			m_x_axis1	= 0,
			m_x_axis2	= 0,
			m_y_axis1	= 0,
			m_y_axis2	= 0,

			// Generic Defaults set every instruction
			m_fill_color	= 0,
			m_text_color	= 0,
			m_line_color	= 0,
			m_text_size	= 0,
			m_line_size	= 0,
			m_x_justify	= 0,
			m_y_justify	= 0,
			m_x_pad		= 0,
			m_y_pad		= 0,
			m_arrowhead	= 0,
			m_size		= 0,
			m_gap		= 0,
			m_antialias	= 0,
			m_min		= 0,
			m_max		= 0,

			// Logical -> Physical multipliers
			m_lpx		= 0,
			m_lpy		= 0,

			// Scratchpad variables for calculations
			m_x		= 0,
			m_y		= 0,
			m_w		= 0,
			m_h		= 0,
			m_x1		= 0,
			m_x2		= 0,
			m_y1		= 0,
			m_y2		= 0,
			m_line_angle	= 0
			;

	int		m_r		= 0,
			m_c		= 0,
			m_r1		= 0,
			m_r2		= 0,
			m_c1		= 0,
			m_c2		= 0,
			m_tmp_i_a	= 0;

	char		* m_data_txt	= nullptr;
	char		m_cbuf[ 2000 ]	= {0};

	double		m_scale = 0.0;	// Zoom: 1 = 100% (10% .. 1000%)

	mtPixy::Canvas	m_canvas;

	PangoRectangle	m_ink_rect	= {0, 0, 0, 0};
	PangoRectangle	m_logical_rect	= {0, 0, 0, 0};

private:
	int setup_axis_cell ( CedCell * cell, CedCell ** newcell );
	void get_next_axis_cell ();
};



#ifdef __cplusplus
}
#endif



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// CUI_GRAPH_H_

