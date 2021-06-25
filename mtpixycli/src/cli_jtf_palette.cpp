/*
	Copyright (C) 2016-2020 Mark Tyler

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


int Backend::jtf_palette_color (
	char	const * const *	const	args
	)
{
	mtPixmap const * const pixmap = file().get_pixmap ();
	if ( ! pixmap )
	{
		return 1;
	}

	int	i, r, g, b;

	if (	mtKit::cli_parse_int( args[0], i, 0, 255 )	||
		mtKit::cli_parse_int( args[1], r, 0, 255 )	||
		mtKit::cli_parse_int( args[2], g, 0, 255 )	||
		mtKit::cli_parse_int( args[3], b, 0, 255 )	||
		file().palette_load_color ( (unsigned char)i,
		(unsigned char)r, (unsigned char)g, (unsigned char)b )
		)
	{
		return 1;
	}

	file().brush.set_color_a ( (unsigned char)i,
		&pixy_pixmap_get_palette_const ( pixmap )->color[0] );

	return 0;
}

int Backend::jtf_palette_del_unused (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return file().palette_remove_unused ( NULL );
}

int Backend::jtf_palette_from_canvas (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return file().palette_create_from_canvas ();
}

int Backend::jtf_palette_gradient (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return file().palette_create_gradient ();
}

int Backend::jtf_palette_load (
	char	const * const *	const	args
	)
{
	return file().palette_load ( args[0] );
}

int Backend::jtf_palette_mask_all (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return file().palette_mask_all ();
}

int Backend::jtf_palette_mask_index (
	char	const * const *	const	args
	)
{
	int	idx;

	if ( mtKit::cli_parse_int( args[0], idx, 0, 255 ) )
	{
		return 1;
	}

	file().palette_mask.color[ idx ] = 1;

	return 0;
}

int Backend::jtf_palette_merge_dups (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return file().palette_merge_duplicates ( NULL );
}

int Backend::jtf_palette_move (
	char	const * const *	const	args
	)
{
	mtPixmap * const pixmap = file().get_pixmap ();
	if ( ! pixmap )
	{
		return 1;
	}

	int	a, b;

	if (	mtKit::cli_parse_int( args[0], a, 0, 255 )	||
		mtKit::cli_parse_int( args[1], b, 0, 255 )
		)
	{
		return 1;
	}

	pixy_pixmap_palette_move_color ( pixmap, (unsigned char)a,
		(unsigned char)b );

	if (	file().update_brush_colors ()	||
		file().palette_changed ()
		)
	{
		return 1;
	}

	return 0;
}

int Backend::jtf_palette_quantize (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return file().palette_quantize_pnn ();
}

int Backend::jtf_palette_save (
	char	const * const *	const	args
	)
{
	return file().palette_save ( args[0] );
}

int Backend::jtf_palette_set (
	char	const * const *	const	args
	)
{
	mtKit::CharInt	const	chint_tab[] = {
				{ "uniform",	0 },
				{ "balanced",	1 },
				{ NULL, 0 }
				};
	int		type, num;

	if (	mtKit::cli_parse_charint ( args[0], chint_tab, type )	||
		mtKit::cli_parse_int ( args[1], num, 2, 6 )		||
		file().palette_load_default ( type, num, "" )
		)
	{
		return 1;
	}

	return file().update_brush_colors ();
}

int Backend::jtf_palette_size (
	char	const * const *	const	args
	)
{
	int	size;

	if ( mtKit::cli_parse_int( args[0], size, 2, 255 ) )
	{
		return 1;
	}

	return file().palette_set_size ( size );
}

int Backend::jtf_palette_sort (
	char	const * const *	const	args
	)
{
	mtKit::CharInt	const	chint_tab[] = {
				{ "hue",	PIXY_PALETTE_SORT_HUE },
				{ "saturation",	PIXY_PALETTE_SORT_SATURATION },
				{ "value",	PIXY_PALETTE_SORT_VALUE },
				{ "min",	PIXY_PALETTE_SORT_MIN },
				{ "brightness",	PIXY_PALETTE_SORT_BRIGHTNESS },
				{ "red",	PIXY_PALETTE_SORT_RED },
				{ "green",	PIXY_PALETTE_SORT_GREEN },
				{ "blue",	PIXY_PALETTE_SORT_BLUE },
				{ "frequency",	PIXY_PALETTE_SORT_FREQUENCY },
				{ NULL, 0 }
				};
	mtKit::CharInt	const	chint_rev[] = {
				{ "reverse",	1 },
				{ NULL, 0 }
				};
	int		start, finish, type, reverse = 0;

	if (	mtKit::cli_parse_int ( args[0], start, 0, 255 )
		|| mtKit::cli_parse_int ( args[1], finish, 0, 255 )
		|| mtKit::cli_parse_charint ( args[2], chint_tab, type )
		|| ( args[3] && mtKit::cli_parse_charint ( args[3], chint_rev,
			reverse ) )
		|| file().palette_sort ( (unsigned char)start,
			(unsigned char)finish, type,
			reverse == 1 ? true : false )
		)
	{
		return 1;
	}

	return file().update_brush_colors ();
}

int Backend::jtf_palette_unmask_all (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return file().palette_unmask_all ();
}

int Backend::jtf_palette_unmask_index (
	char	const * const *	const	args
	)
{
	int	idx;

	if ( mtKit::cli_parse_int ( args[0], idx, 0, 255 ) )
	{
		return 1;
	}

	file().palette_mask.color[ idx ] = 0;

	return 0;
}

