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

#include "cli.h"



Backend::Backend ()
	:
	m_file_p	( &m_file[0] )
{
	for ( int i = 0; i < FILE_TOTAL; i++ )
	{
		m_file[i].new_image ( PIXY_PIXMAP_BPP_RGB, 100, 100, 1, 3, "" );
	}
}

Backend::~Backend ()
{
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

	mtKit::Arg args;

	args.add ( "-help",	show_version, 2 );
	args.add ( "-version",	show_version, 1 );
	args.add ( "patterns",	fn_patterns );
	args.add ( "q",		show_about, 0 );
	args.add ( "seed",	rnd_seed );
	args.add ( "shapes",	fn_shapes );
	args.add ( "t",		tab_text, 1 );

	if ( args.parse ( argc, argv ) )
	{
		exit.set_value ( 1 );
		return 1;
	}

	if ( show_version )
	{
		printf ( "%s (part of %s)\n\n", argv[0], VERSION );

		if ( 2 == show_version )
		{
			printf (
				"For further information consult the man page "
				"%s(1) or the mtPixy Handbook.\n"
				"\n", argv[0] );
		}

		return 1;		// Quit program
	}

	if ( show_about )
	{
		print_about ();
	}

	m_clishell.bind_tilde ();

	if ( tab_text )
	{
		m_clishell.bind_tab ();
	}

	if ( fn_patterns )
	{
		if ( file().brush.load_patterns ( fn_patterns ) )
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
		if ( file().brush.load_shapes ( fn_shapes ) )
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



#define JTFUNC( X ) [this](char const * const * f ) { return jtf_ ## X (f); }



void Backend::main_loop ()
{
	if (	0
		|| m_clitab.add_item ( "canvas flip_h", JTFUNC(canvas_flip_h) )
		|| m_clitab.add_item ( "canvas flip_v", JTFUNC(canvas_flip_v) )
		|| m_clitab.add_item ( "canvas indexed", JTFUNC(canvas_indexed),
			1, 1, "<DITHER none | basic | average | floyd>" )
		|| m_clitab.add_item ( "canvas rgb", JTFUNC(canvas_rgb) )
		|| m_clitab.add_item ( "canvas rotate_a",
			JTFUNC(canvas_rotate_a) )
		|| m_clitab.add_item ( "canvas rotate_c",
			JTFUNC(canvas_rotate_c) )
		|| m_clitab.add_item ( "clip flip_h", JTFUNC(clip_flip_h) )
		|| m_clitab.add_item ( "clip flip_v", JTFUNC(clip_flip_v) )
		|| m_clitab.add_item ( "clip load", JTFUNC(clip_load), 1, 1,
			"<1..12>" )
		|| m_clitab.add_item ( "clip rotate_a", JTFUNC(clip_rotate_a) )
		|| m_clitab.add_item ( "clip rotate_c", JTFUNC(clip_rotate_c) )
		|| m_clitab.add_item ( "clip save", JTFUNC(clip_save), 1, 1,
			"<1..12>" )
		|| m_clitab.add_item ( "copy", JTFUNC(copy) )
		|| m_clitab.add_item ( "crop", JTFUNC(crop) )
		|| m_clitab.add_item ( "cut", JTFUNC(cut) )
		|| m_clitab.add_item ( "delete alpha", JTFUNC(delete_alpha) )

		|| m_clitab.add_item ( "effect bacteria",
			JTFUNC(effect_bacteria), 1, 1, "<STRENGTH 1..100>" )
		|| m_clitab.add_item ( "effect crt",
			JTFUNC(effect_crt), 1, 1, "<SCALE 2..32>" )
		|| m_clitab.add_item ( "effect edge_detect",
			JTFUNC(effect_edge_detect) )
		|| m_clitab.add_item ( "effect emboss", JTFUNC(effect_emboss) )
		|| m_clitab.add_item ( "effect invert", JTFUNC(effect_invert) )
		|| m_clitab.add_item ( "effect sharpen", JTFUNC(effect_sharpen),
			1, 1, "<STRENGTH 1..100>" )
		|| m_clitab.add_item ( "effect soften", JTFUNC(effect_soften),
			1, 1, "<STRENGTH 1..100>" )
		|| m_clitab.add_item ( "effect trans_color",
			JTFUNC(effect_trans_color), 6, 6,
			"<GAMMA -100..100> <BRIGHTNESS -100..100> "
			"<CONTRAST -100..100> <SATURATION -100..100> "
			"<HUE -1529..1529> <POSTERIZE 1..8>" )

		|| m_clitab.add_item ( "fill", JTFUNC(fill) )
		|| m_clitab.add_item ( "floodfill", JTFUNC(floodfill), 2, 2,
			"<X 0..> <Y 0..>" )
		|| m_clitab.add_item ( "help", JTFUNC(help), 0, 100, "[ARG]...")
		|| m_clitab.add_item ( "info", JTFUNC(info) )
		|| m_clitab.add_item ( "lasso", JTFUNC(lasso) )
		|| m_clitab.add_item ( "load", JTFUNC(load), 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "new", JTFUNC(new), 3, 3,
			"<WIDTH 1..32767> <HEIGHT 1..32767> <rgb | indexed>" )
		|| m_clitab.add_item ( "outline", JTFUNC(outline) )
		|| m_clitab.add_item ( "paint", JTFUNC(paint), 1, 100,
			"< <X 0..> <Y 0..> >...", 2 )
		|| m_clitab.add_item ( "palette color", JTFUNC(palette_color),
			4, 4, "<INDEX 0..255> <RED 0..255> <GREEN 0..255> "
			"<BLUE 0..255>" )
		|| m_clitab.add_item ( "palette del_unused",
			JTFUNC(palette_del_unused) )
		|| m_clitab.add_item ( "palette from_canvas",
			JTFUNC(palette_from_canvas) )
		|| m_clitab.add_item ( "palette gradient",
			JTFUNC(palette_gradient) )
		|| m_clitab.add_item ( "palette load", JTFUNC(palette_load),
			1, 1, "<OS FILENAME>" )
		|| m_clitab.add_item ( "palette mask all",
			JTFUNC(palette_mask_all) )
		|| m_clitab.add_item ( "palette mask index",
			JTFUNC(palette_mask_index), 1, 1, "<0..255>" )
		|| m_clitab.add_item ( "palette merge_dups",
			JTFUNC(palette_merge_dups) )
		|| m_clitab.add_item ( "palette move", JTFUNC(palette_move),
			2, 2, "<FROM INDEX 0..255> <TO INDEX 0..255>" )
		|| m_clitab.add_item ( "palette quantize",
			JTFUNC(palette_quantize) )
		|| m_clitab.add_item ( "palette save", JTFUNC(palette_save),
			1, 1, "<OS FILENAME>" )
		|| m_clitab.add_item ( "palette set", JTFUNC(palette_set), 2, 2,
			"<uniform | balanced> <2..6>" )
		|| m_clitab.add_item ( "palette size", JTFUNC(palette_size), 1, 1,
			"<2..255>" )
		|| m_clitab.add_item ( "palette sort", JTFUNC(palette_sort),
			3, 4,
			"<START 0..255> <FINISH 0..255> <hue | saturation | "
			"value | min | brightness | red | green | blue | "
			"frequency > [reverse]" )
		|| m_clitab.add_item ( "palette unmask all",
			JTFUNC(palette_unmask_all) )
		|| m_clitab.add_item ( "palette unmask index",
			JTFUNC(palette_unmask_index), 1, 1, "<0..255>" )
		|| m_clitab.add_item ( "paste", JTFUNC(paste), 2, 2,
			"<X -32766..> <Y -32766..>" )
		|| m_clitab.add_item ( "q", JTFUNC(quit) )
		|| m_clitab.add_item ( "quit", JTFUNC(quit) )
		|| m_clitab.add_item ( "redo", JTFUNC(redo) )
		|| m_clitab.add_item ( "resize", JTFUNC(resize), 4, 4,
			"<X -32766..32767> <Y -32766..32767> <WIDTH 1..32767> "
			"<HEIGHT 1..32767>" )
		|| m_clitab.add_item ( "save", JTFUNC(save) )
		|| m_clitab.add_item ( "save as", JTFUNC(save_as), 1, 3,
			"<OS FILENAME> [ <bmp | bp24 | gif | jpeg | pixy | "
			"png > [COMPRESSION 0..9 | 0..100] ]" )
		|| m_clitab.add_item ( "save undo", JTFUNC(save_undo), 1, 1,
			"<OS FILENAME>" )
		|| m_clitab.add_item ( "scale", JTFUNC(scale), 2, 3,
			"<WIDTH 1..32767> <HEIGHT 1..32767> [smooth]" )
		|| m_clitab.add_item ( "select all", JTFUNC(select_all) )
		|| m_clitab.add_item ( "select polygon", JTFUNC(select_polygon),
			1, 100, "< <X 0..> <Y 0..> >...", 2 )
		|| m_clitab.add_item ( "select rectangle",
			JTFUNC(select_rectangle), 4, 4,
			"<X 0..> <Y 0..> <WIDTH 1..> <HEIGHT 1..>" )
		|| m_clitab.add_item ( "set brush flow", JTFUNC(set_brush_flow),
			1, 1, "<1..1000>" )
		|| m_clitab.add_item ( "set brush pattern",
			JTFUNC(set_brush_pattern), 1, 1, "<0..>" )
		|| m_clitab.add_item ( "set brush shape",
			JTFUNC(set_brush_shape), 1, 1, "<0..>" )
		|| m_clitab.add_item ( "set brush spacing",
			JTFUNC(set_brush_spacing), 1, 1, "<0..100>" )
		|| m_clitab.add_item ( "set color a", JTFUNC(set_color_a), 1, 1,
			"<0..255>" )
		|| m_clitab.add_item ( "set color b", JTFUNC(set_color_b), 1, 1,
			"<0..255>" )
		|| m_clitab.add_item ( "set color swap", JTFUNC(set_color_swap))
		|| m_clitab.add_item ( "set file", JTFUNC(set_file), 1, 1,
			"<0..4>")
		|| m_clitab.add_item ( "text", JTFUNC(text), 3, 7, "<STRING> "
			"<FONT NAME> <FONT SIZE 1..100> [bold] [italic] "
			"[underline] [strikethrough]" )
		|| m_clitab.add_item ( "undo", JTFUNC(undo) )
		)
	{
		exit.abort ();
		exit.set_value ( 1 );

		fprintf ( stderr, "Error: main_loop().init_table()\n" );

		return;
	}

	while ( false == exit.aborted () )
	{
		std::string const & line = m_clishell.read_line( BIN_NAME" > ");

		if ( m_clishell.finished () )
		{
			break;
		}
		else if ( line[0] == '#' )
		{
			// Comment
		}
		else if ( line[0] )
		{
			m_clishell.add_history ();

			m_clitab.parse ( line.c_str() );
		}
		else
		{
			// Quietly ignore empty lines
		}
	}
}

void Backend::print_about ()
{
	printf ( "%s (part of %s)\n"
		"Copyright (C) " MT_COPYRIGHT_YEARS " Mark Tyler\n"
		"Type 'help' for command hints.  Read the manual for more "
		"specific info.\n\n", APP_NAME, VERSION );
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
	for ( int n = 0; n < FILE_TOTAL; n++ )
	{
		if ( m_file_p == &m_file[n] )
		{
			return n;
		}
	}

	return -1;
}

int Backend::get_help (
	char	const * const * const	argv
	) const
{
	return m_clitab.print_help ( argv );
}

int Backend::operation_update ( int const res )
{
	if ( 0 == res )
	{
		file().reset_tool_mode ();
	}

	return res;
}

