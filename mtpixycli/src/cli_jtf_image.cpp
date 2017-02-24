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



int jtf_canvas_flip_h (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().flip_horizontally () );
}

int jtf_canvas_flip_v (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().flip_vertically () );
}

int jtf_canvas_indexed (
	char	const * const *	const	args
	)
{
	mtKit::CharInt	const	chint_tab[] = {
			{ "none",	mtPixy::Image::DITHER_NONE },
			{ "basic",	mtPixy::Image::DITHER_BASIC },
			{ "floyd",	mtPixy::Image::DITHER_FLOYD },
			{ NULL, 0 }
			};
	int		dither;

	if ( mtKit::cli_parse_charint ( args[0], chint_tab, dither ) )
	{
		return 1;
	}

	return operation_update( backend.file().convert_to_indexed (
		(mtPixy::Image::DitherType)dither ) );
}

int jtf_canvas_rgb (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().convert_to_rgb () );
}

int jtf_canvas_rotate_a (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().rotate_anticlockwise () );
}

int jtf_canvas_rotate_c (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().rotate_clockwise () );
}

int jtf_delete_alpha (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().destroy_alpha () );
}

int jtf_effect_bacteria (
	char	const * const *	const	args
	)
{
	int	tot;

	if ( mtKit::cli_parse_int( args[0], tot, 1, 100 ) )
	{
		return 1;
	}

	return operation_update ( backend.file().effect_bacteria ( tot ) );
}

int jtf_effect_edge_detect (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().effect_edge_detect () );
}

int jtf_effect_emboss (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().effect_emboss () );
}

int jtf_effect_invert (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().effect_invert () );
}

int jtf_effect_sharpen (
	char	const * const *	const	args
	)
{
	int	tot;

	if ( mtKit::cli_parse_int( args[0], tot, 1, 100 ) )
	{
		return 1;
	}

	return operation_update ( backend.file().effect_sharpen ( tot ) );
}

int jtf_effect_soften (
	char	const * const *	const	args
	)
{
	int	tot;

	if ( mtKit::cli_parse_int( args[0], tot, 1, 100 ) )
	{
		return 1;
	}

	return operation_update ( backend.file().effect_soften ( tot ) );
}

int jtf_effect_trans_color (
	char	const * const *	const	args
	)
{
	int gamma, brightness, contrast, saturation, hue, posterize;

	if (	mtKit::cli_parse_int ( args[0], gamma, -100, 100 )	||
		mtKit::cli_parse_int ( args[1], brightness, -100, 100 )	||
		mtKit::cli_parse_int ( args[2], contrast, -100, 100 )	||
		mtKit::cli_parse_int ( args[3], saturation, -100, 100 )	||
		mtKit::cli_parse_int ( args[4], hue, -1529, 1529 )	||
		mtKit::cli_parse_int ( args[5], posterize, 1, 8 )
		)
	{
		return 1;
	}

	return backend.file().effect_transform_color ( gamma, brightness,
		contrast, saturation, hue, posterize );
}

int jtf_resize (
	char	const * const *	const	args
	)
{
	int		x, y, w, h;

	if (	mtKit::cli_parse_int ( args[0], x, -32766, 32767 )	||
		mtKit::cli_parse_int ( args[1], y, -32766, 32767 )	||
		mtKit::cli_parse_int ( args[2], w, 1, 32767 )		||
		mtKit::cli_parse_int ( args[3], h, 1, 32767 )
		)
	{
		return 1;
	}

	return operation_update ( backend.file().resize ( x, y, w, h ) );
}

int jtf_scale (
	char	const * const *	const	args
	)
{
	mtPixy::Image::ScaleType	type = mtPixy::Image::BLOCKY;
	int				w, h;

	if (	mtKit::cli_parse_int ( args[0], w, 1, 32767 )		||
		mtKit::cli_parse_int ( args[1], h, 1, 32767 )
		)
	{
		return 1;
	}

	if ( args[2] )
	{
		int		t;
		mtKit::CharInt	const	chint_tab[] = {
				{ "smooth",	mtPixy::Image::SMOOTH },
				{ NULL, 0 }
				};

		if ( mtKit::cli_parse_charint ( args[2], chint_tab, t ) )
		{
			return 1;
		}

		type = (mtPixy::Image::ScaleType)t;
	}

	return operation_update ( backend.file().scale ( w, h, type ) );
}

