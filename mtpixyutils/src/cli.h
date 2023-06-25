/*
	Copyright (C) 2016-2022 Mark Tyler

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

#ifndef CLI_H_
#define CLI_H_

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include <mtkit.h>
#include <mtpixy.h>

#include "cli_static.h"



#undef APP_NAME
#define APP_NAME	"mtPixyCLI"

#undef BIN_NAME
#define BIN_NAME	"mtpixycli"



class Backend;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class Backend
{
public:
	enum
	{
		FILE_TOTAL = 5
	};

	Backend ();
	~Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program with exit.value()

	void main_loop ();

	mtKit::Exit		exit;

private:
	void cli_parse_line ( char const * input );

	mtPixyUI::File &file ();
	int set_file ( int n );
	int get_file_number () const;

	int get_help ( char const * const * argv ) const;

	int operation_update ( int res );
		// Resets tool if res = 0. returns res

	int jtf_canvas_flip_h		( char const * const * );
	int jtf_canvas_flip_v		( char const * const * );
	int jtf_canvas_indexed		( char const * const * );
	int jtf_canvas_rgb		( char const * const * );
	int jtf_canvas_rotate_a		( char const * const * );
	int jtf_canvas_rotate_c		( char const * const * );
	int jtf_clip_flip_h		( char const * const * );
	int jtf_clip_flip_v		( char const * const * );
	int jtf_clip_load		( char const * const * );
	int jtf_clip_rotate_a		( char const * const * );
	int jtf_clip_rotate_c		( char const * const * );
	int jtf_clip_save		( char const * const * );
	int jtf_copy			( char const * const * );
	int jtf_crop			( char const * const * );
	int jtf_cut			( char const * const * );
	int jtf_delete_alpha		( char const * const * );
	int jtf_effect_bacteria		( char const * const * );
	int jtf_effect_crt		( char const * const * );
	int jtf_effect_edge_detect	( char const * const * );
	int jtf_effect_emboss		( char const * const * );
	int jtf_effect_invert		( char const * const * );
	int jtf_effect_sharpen		( char const * const * );
	int jtf_effect_soften		( char const * const * );
	int jtf_effect_trans_color	( char const * const * );
	int jtf_fill			( char const * const * );
	int jtf_floodfill		( char const * const * );
	int jtf_help			( char const * const * );
	int jtf_info			( char const * const * );
	int jtf_lasso			( char const * const * );
	int jtf_load			( char const * const * );
	int jtf_new			( char const * const * );
	int jtf_outline			( char const * const * );
	int jtf_paint			( char const * const * );
	int jtf_palette_color		( char const * const * );
	int jtf_palette_del_unused	( char const * const * );
	int jtf_palette_from_canvas	( char const * const * );
	int jtf_palette_gradient	( char const * const * );
	int jtf_palette_load		( char const * const * );
	int jtf_palette_mask_all	( char const * const * );
	int jtf_palette_mask_index	( char const * const * );
	int jtf_palette_merge_dups	( char const * const * );
	int jtf_palette_move		( char const * const * );
	int jtf_palette_quantize	( char const * const * );
	int jtf_palette_save		( char const * const * );
	int jtf_palette_set		( char const * const * );
	int jtf_palette_size		( char const * const * );
	int jtf_palette_sort		( char const * const * );
	int jtf_palette_unmask_all	( char const * const * );
	int jtf_palette_unmask_index	( char const * const * );
	int jtf_paste			( char const * const * );
	int jtf_quit			( char const * const * );
	int jtf_redo			( char const * const * );
	int jtf_resize			( char const * const * );
	int jtf_save			( char const * const * );
	int jtf_save_as			( char const * const * );
	int jtf_save_undo		( char const * const * );
	int jtf_scale			( char const * const * );
	int jtf_select_all		( char const * const * );
	int jtf_select_polygon		( char const * const * );
	int jtf_select_rectangle	( char const * const * );
	int jtf_set_brush_flow		( char const * const * );
	int jtf_set_brush_pattern	( char const * const * );
	int jtf_set_brush_shape		( char const * const * );
	int jtf_set_brush_spacing	( char const * const * );
	int jtf_set_color_a		( char const * const * );
	int jtf_set_color_b		( char const * const * );
	int jtf_set_color_swap		( char const * const * );
	int jtf_set_file		( char const * const * );
	int jtf_text			( char const * const * );
	int jtf_undo			( char const * const * );

	static void print_about ();

	int prep_save ( int &comp, int &ft );
	int set_col ( char const * const * args, int b );

/// ----------------------------------------------------------------------------

	mtPixyUI::File		* m_file_p;
	mtPixyUI::File		m_file[ FILE_TOTAL ];

	mtPixyUI::Clipboard	clipboard;

	mtKit::CliShell		m_clishell;
	mtKit::CliTab		m_clitab;
};



#endif		// CLI_H_

