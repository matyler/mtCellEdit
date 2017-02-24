/*
	Copyright (C) 2016 Mark Tyler

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



int jtf_fill (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.file().selection_fill ();
}

int jtf_floodfill (
	char	const * const *	const	args
	)
{
	int		cx, cy;

	if (	mtKit::cli_parse_int ( args[0], cx, 0, 32767 )	||
		mtKit::cli_parse_int ( args[1], cy, 0, 32767 )
		)
	{
		return 1;
	}

	return backend.file().flood_fill ( cx, cy );
}

int jtf_outline (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.file().selection_outline ();
}

int jtf_paint (
	char	const * const *	const	args
	)
{
	int		x, y, res = 0;


	for ( int i = 0; args[i] && args[i + 1]; i += 2 )
	{
		if (	mtKit::cli_parse_int ( args[i], x, 0, 32767 )	||
			mtKit::cli_parse_int ( args[i + 1], y, 0, 32767 )
			)
		{
			res = 1;
			break;
		}

		backend.file().paint_brush_to ( x, y );
	}

	backend.file().paint_brush_finish ();

	return res;
}

int jtf_paste (
	char	const * const *	const	args
	)
{
	int		x, y;

	if (	mtKit::cli_parse_int ( args[0], x, -32766, 32767 )	||
		mtKit::cli_parse_int ( args[1], y, -32766, 32767 )
		)
	{
		return 1;
	}

	if (	backend.clipboard.paste ( backend.file(), x, y )	||
		backend.file().commit_undo_step ()
		)
	{
		return 1;
	}

	return operation_update ( 0 );
}

int jtf_set_brush_flow (
	char	const * const *	const	args
	)
{
	int		flow;

	if ( mtKit::cli_parse_int ( args[0], flow, 1, 1000 ) )
	{
		return 1;
	}

	backend.file().brush.set_flow ( flow );

	return 0;
}

int jtf_set_brush_pattern (
	char	const * const *	const	args
	)
{
	int		num;

	if ( mtKit::cli_parse_int ( args[0], num, 0, 100000 ) )
	{
		return 1;
	}

	return backend.file().brush.set_pattern ( num );
}

int jtf_set_brush_shape (
	char	const * const *	const	args
	)
{
	int		num;

	if ( mtKit::cli_parse_int ( args[0], num, 0, 100000 ) )
	{
		return 1;
	}

	return backend.file().brush.set_shape ( num );
}

int jtf_set_brush_spacing (
	char	const * const *	const	args
	)
{
	int		spacing;

	if ( mtKit::cli_parse_int ( args[0], spacing, 0, 100 ) )
	{
		return 1;
	}

	backend.file().brush.set_spacing ( spacing );

	return 0;
}

static int set_col (
	char	const * const *	const	args,
	int			const	b
	)
{
	mtPixy::Image	* im = backend.file().get_image ();
	if ( ! im )
	{
		return 1;
	}

	int		idx;

	if ( mtKit::cli_parse_int ( args[0], idx, 0, 255 ) )
	{
		return 1;
	}

	mtPixy::Color	* col = im->get_palette ()->get_color ();

	if ( b )
	{
		backend.file().brush.set_color_b ( (unsigned char)idx, col );
	}
	else
	{
		backend.file().brush.set_color_a ( (unsigned char)idx, col );
	}

	return backend.file().update_brush_colors ();
}

int jtf_set_color_a (
	char	const * const *	const	args
	)
{
	return set_col ( args, 0 );
}

int jtf_set_color_b (
	char	const * const *	const	args
	)
{
	return set_col ( args, 1 );
}

int jtf_set_color_swap (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.file().palette_swap_ab ();
}

int jtf_set_file (
	char	const * const *	const	args
	)
{
	int		idx;

	if ( mtKit::cli_parse_int ( args[0], idx, 0, 4 ) )
	{
		return 1;
	}

	return backend.set_file ( idx );
}

