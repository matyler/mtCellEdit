/*
	Copyright (C) 2016-2017 Mark Tyler

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

#include <readline/readline.h>
#include <readline/history.h>



Backend::Backend ()
	:
	m_exit_value	( 0 ),
	m_exit_now	( 0 ),
	m_file_p	( &m_file[0] )
{
	for ( int i = 0; i < FILE_TOTAL; i++ )
	{
		m_file[i].new_image ( mtPixy::Image::RGB, 100, 100, 1, 3 );
	}
}

Backend::~Backend ()
{
}

static int error_func (
	int		const	error,
	int		const	arg,
	int		const	argc,
	char	const	* const	argv[],
	void		* const	ARG_UNUSED ( user_data )
	)
{
	fprintf ( stderr, "error_func: Argument ERROR! - num=%i arg=%i/%i",
		error, arg, argc );

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n" );

	return 0;			// Keep parsing
}

int Backend::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	int		show_version	= 0;
	int		show_about	= 1;
	int		tab_text	= 0;
	int		rnd_seed	= 0;
	char	const	* fn_patterns	= NULL;
	char	const	* fn_shapes	= NULL;
	mtArg	const	arg_list[] = {
		{ "-help",	MTKIT_ARG_SWITCH, &show_version, 2, NULL },
		{ "-version",	MTKIT_ARG_SWITCH, &show_version, 1, NULL },
		{ "patterns",	MTKIT_ARG_STRING, &fn_patterns, 0, NULL },
		{ "q",		MTKIT_ARG_SWITCH, &show_about, 0, NULL },
		{ "seed",	MTKIT_ARG_INT, &rnd_seed, 0, NULL },
		{ "shapes",	MTKIT_ARG_STRING, &fn_shapes, 0, NULL },
		{ "t",		MTKIT_ARG_SWITCH, &tab_text, 1, NULL },
		{ NULL, 0, NULL, 0, NULL }
		};


	mtkit_arg_parse ( argc, argv, arg_list, NULL, error_func, NULL );

	switch ( show_version )
	{
	case 1:
		printf ( "%s\n\n", VERSION );

		return 1;		// Quit program

	case 2:
		printf (
		"%s\n\n"
		"For further information consult the man page "
		"%s(1) or the mtPixy Handbook.\n"
		"\n", VERSION, BIN_NAME );

		return 1;		// Quit program
	}

	if ( show_about )
	{
		print_about ();
	}

	rl_variable_bind ( "expand-tilde", "on" );

	if ( tab_text )
	{
		rl_bind_key ( '\t', rl_insert );
	}

	if ( fn_patterns )
	{
		if ( backend.file().brush.load_patterns ( fn_patterns ) )
		{
			fprintf ( stderr, "ERROR: Pattern file '%s' invalid.\n",
				fn_patterns );
		}
	}
	else
	{
		fprintf ( stderr, "WARNING: No brush patterns loaded\n" );
	}

	if ( fn_shapes )
	{
		if ( backend.file().brush.load_shapes ( fn_shapes ) )
		{
			fprintf ( stderr, "ERROR: Shape file '%s' invalid.\n",
				fn_shapes );
		}
	}
	else
	{
		fprintf ( stderr, "WARNING: No brush shapes loaded\n" );
	}

	if ( rnd_seed )
	{
		srand ( (unsigned int)rnd_seed );
	}
	else
	{
		srand ( (unsigned int)time ( NULL ) );
	}

	return 0;			// Continue program
}

int Backend::init_table ()
{
	if (	0
		|| m_clitab.add_item ( "canvas flip_h", jtf_canvas_flip_h )
		|| m_clitab.add_item ( "canvas flip_v", jtf_canvas_flip_v )
		|| m_clitab.add_item ( "canvas indexed", jtf_canvas_indexed,
			1, 1, "<DITHER none | basic | floyd>" )
		|| m_clitab.add_item ( "canvas rgb", jtf_canvas_rgb )
		|| m_clitab.add_item ( "canvas rotate_a", jtf_canvas_rotate_a )
		|| m_clitab.add_item ( "canvas rotate_c", jtf_canvas_rotate_c )
		|| m_clitab.add_item ( "clip flip_h", jtf_clip_flip_h )
		|| m_clitab.add_item ( "clip flip_v", jtf_clip_flip_v )
		|| m_clitab.add_item ( "clip load", jtf_clip_load, 1, 1,
			"<1..12>" )
		|| m_clitab.add_item ( "clip rotate_a", jtf_clip_rotate_a )
		|| m_clitab.add_item ( "clip rotate_c", jtf_clip_rotate_c )
		|| m_clitab.add_item ( "clip save", jtf_clip_save, 1, 1,
			"<1..12>" )
		|| m_clitab.add_item ( "copy", jtf_copy )
		|| m_clitab.add_item ( "crop", jtf_crop )
		|| m_clitab.add_item ( "cut", jtf_cut )
		|| m_clitab.add_item ( "delete alpha", jtf_delete_alpha )
		|| m_clitab.add_item ( "effect bacteria", jtf_effect_bacteria,
			1, 1, "<STRENGTH 1..100>" )
		|| m_clitab.add_item ( "effect edge_detect",
			jtf_effect_edge_detect )
		|| m_clitab.add_item ( "effect emboss", jtf_effect_emboss )
		|| m_clitab.add_item ( "effect invert", jtf_effect_invert )
		|| m_clitab.add_item ( "effect sharpen", jtf_effect_sharpen,
			1, 1, "<STRENGTH 1..100>" )
		|| m_clitab.add_item ( "effect soften", jtf_effect_soften, 1, 1,
			"<STRENGTH 1..100>" )
		|| m_clitab.add_item ( "effect trans_color",
			jtf_effect_trans_color, 6, 6,
			"<GAMMA -100..100> <BRIGHTNESS -100..100> "
			"<CONTRAST -100..100> <SATURATION -100..100> "
			"<HUE -1529..1529> <POSTERIZE 1..8>" )
		|| m_clitab.add_item ( "fill", jtf_fill )
		|| m_clitab.add_item ( "floodfill", jtf_floodfill, 2, 2,
			"<X 0..> <Y 0..>" )
		|| m_clitab.add_item ( "help", jtf_help, 0, 100, "[ARG]..." )
		|| m_clitab.add_item ( "info", jtf_info )
		|| m_clitab.add_item ( "lasso", jtf_lasso )
		|| m_clitab.add_item ( "load", jtf_load, 1, 1, "<OS FILENAME>" )
		|| m_clitab.add_item ( "new", jtf_new, 3, 3,
			"<WIDTH 1..32767> <HEIGHT 1..32767> <rgb | indexed>" )
		|| m_clitab.add_item ( "outline", jtf_outline )
		|| m_clitab.add_item ( "paint", jtf_paint, 1, 100,
			"< <X 0..> <Y 0..> >...", 2 )
		|| m_clitab.add_item ( "palette color", jtf_palette_color, 4, 4,
			"<INDEX 0..255> <RED 0..255> <GREEN 0..255> "
			"<BLUE 0..255>" )
		|| m_clitab.add_item ( "palette del_unused",
			jtf_palette_del_unused )
		|| m_clitab.add_item ( "palette from_canvas",
			jtf_palette_from_canvas )
		|| m_clitab.add_item ( "palette gradient", jtf_palette_gradient)
		|| m_clitab.add_item ( "palette load", jtf_palette_load, 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "palette mask all", jtf_palette_mask_all)
		|| m_clitab.add_item ( "palette mask index",
			jtf_palette_mask_index, 1, 1, "<0..255>" )
		|| m_clitab.add_item ( "palette merge_dups",
			jtf_palette_merge_dups )
		|| m_clitab.add_item ( "palette move", jtf_palette_move, 2, 2,
			"<FROM INDEX 0..255> <TO INDEX 0..255>" )
		|| m_clitab.add_item ( "palette quantize", jtf_palette_quantize)
		|| m_clitab.add_item ( "palette save", jtf_palette_save, 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "palette set", jtf_palette_set, 2, 2,
			"<uniform | balanced> <2..6>" )
		|| m_clitab.add_item ( "palette size", jtf_palette_size, 1, 1,
			"<2..255>" )
		|| m_clitab.add_item ( "palette sort", jtf_palette_sort, 3, 4,
			"<START 0..255> <FINISH 0..255> <hue | saturation | "
			"value | min | brightness | red | green | blue | "
			"frequency > [reverse]" )
		|| m_clitab.add_item ( "palette unmask all",
			jtf_palette_unmask_all )
		|| m_clitab.add_item ( "palette unmask index",
			jtf_palette_unmask_index, 1, 1, "<0..255>" )
		|| m_clitab.add_item ( "paste", jtf_paste, 2, 2,
			"<X -32766..> <Y -32766..>" )
		|| m_clitab.add_item ( "q", jtf_quit )
		|| m_clitab.add_item ( "quit", jtf_quit )
		|| m_clitab.add_item ( "redo", jtf_redo )
		|| m_clitab.add_item ( "resize", jtf_resize, 4, 4,
			"<X -32766..32767> <Y -32766..32767> <WIDTH 1..32767> "
			"<HEIGHT 1..32767>" )
		|| m_clitab.add_item ( "save", jtf_save )
		|| m_clitab.add_item ( "save as", jtf_save_as, 1, 3,
			"<OS FILENAME> [ <bmp | png | jpeg | gif> "
			"[COMPRESSION 0..9 | 0..100] ]" )
		|| m_clitab.add_item ( "save undo", jtf_save_undo, 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "scale", jtf_scale, 2, 3,
			"<WIDTH 1..32767> <HEIGHT 1..32767> [smooth]" )
		|| m_clitab.add_item ( "select all", jtf_select_all )
		|| m_clitab.add_item ( "select polygon", jtf_select_polygon,
			1, 100, "< <X 0..> <Y 0..> >...", 2 )
		|| m_clitab.add_item ( "select rectangle", jtf_select_rectangle,
			4, 4, "<X 0..> <Y 0..> <WIDTH 1..> <HEIGHT 1..>" )
		|| m_clitab.add_item ( "set brush flow", jtf_set_brush_flow,
			1, 1, "<1..1000>" )
		|| m_clitab.add_item ( "set brush pattern",
			jtf_set_brush_pattern, 1, 1, "<0..>" )
		|| m_clitab.add_item ( "set brush shape", jtf_set_brush_shape,
			1, 1, "<0..>" )
		|| m_clitab.add_item ( "set brush spacing",
			jtf_set_brush_spacing, 1, 1, "<0..100>" )
		|| m_clitab.add_item ( "set color a", jtf_set_color_a, 1, 1,
			"<0..255>" )
		|| m_clitab.add_item ( "set color b", jtf_set_color_b, 1, 1,
			"<0..255>" )
		|| m_clitab.add_item ( "set color swap", jtf_set_color_swap )
		|| m_clitab.add_item ( "set file", jtf_set_file, 1, 1, "<0..4>")
		|| m_clitab.add_item ( "text", jtf_text, 3, 7, "<STRING> "
			"<FONT NAME> <FONT SIZE 1..100> [bold] [italic] "
			"[underline] [strikethrough]" )
		|| m_clitab.add_item ( "undo", jtf_undo )
		)
	{
		return 1;
	}

	return 0;
}

void Backend::main_loop ()
{
	if ( init_table () )
	{
		m_exit_now = 1;
		m_exit_value = 1;

		fprintf ( stderr, "Error: main_loop().init_table()\n" );

		return;
	}

	while ( 0 == m_exit_now )
	{
		char * line = readline ( BIN_NAME" > " );

		if ( ! line )
		{
			break;
		}

		if ( line[0] )
		{
			add_history ( line );
		}

		m_clitab.parse ( line );

		free ( line );
	}

	clear_history ();
}

void Backend::print_about ()
{
	printf ( "%s\n"
		"Copyright (C) 2016 Mark Tyler\n"
		"Type 'help' for command hints.  Read the manual for more "
		"specific info.\n\n", VERSION );
}

mtPixyUI::File &Backend::file ()
{
	return *m_file_p;
}

int Backend::set_file (
	int	const	n
	)
{
	if ( n < 0 || n >= FILE_TOTAL )
	{
		return 1;
	}

	m_file_p = &m_file[n];

	return 0;
}

int Backend::get_file_number () const
{
	int n;

	for ( n = 0; n < FILE_TOTAL; n++ )
	{
		if ( m_file_p == &m_file[n] )
		{
			break;
		}
	}

	return n >= FILE_TOTAL ? -1 : n;
}

int Backend::get_help (
	char	const * const * const	argv
	) const
{
	return m_clitab.print_help ( argv );
}

