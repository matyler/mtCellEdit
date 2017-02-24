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



int jtf_clip_flip_h (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.clipboard.flip_horizontal ();
}

int jtf_clip_flip_v (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.clipboard.flip_vertical ();
}

int jtf_clip_load (
	char	const * const *	const	args
	)
{
	int		num;

	if ( mtKit::cli_parse_int ( args[0], num, 1, 12 ) )
	{
		return 1;
	}

	return backend.clipboard.load ( num );
}

int jtf_clip_rotate_a (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.file().clipboard_rotate_anticlockwise(backend.clipboard);
}

int jtf_clip_rotate_c (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.file().clipboard_rotate_clockwise ( backend.clipboard );
}

int jtf_clip_save (
	char	const * const *	const	args
	)
{
	int		num;

	if ( mtKit::cli_parse_int ( args[0], num, 1, 12 ) )
	{
		return 1;
	}

	return backend.clipboard.save ( num );
}

int jtf_copy (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.file().selection_copy ( backend.clipboard );
}

int jtf_crop (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().crop () );
}

int jtf_cut (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	if ( jtf_copy ( NULL ) )
	{
		return 1;
	}

	return jtf_fill ( NULL );
}

int jtf_lasso (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	if ( jtf_copy ( NULL ) )
	{
		return 1;
	}

	return backend.file().selection_lasso ( backend.clipboard );
}

int jtf_select_all (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return backend.file().select_all ();
}

int jtf_select_polygon (
	char	const * const *	const	args
	)
{
	int		x, y, res = 0;


	backend.file().polygon_overlay.clear ();

	for ( int i = 0; args[i] && args[i + 1]; i += 2 )
	{
		if (	mtKit::cli_parse_int ( args[i], x, 0, 32767 )	||
			mtKit::cli_parse_int ( args[i + 1], y, 0, 32767 )
			)
		{
			res = 1;
			break;
		}

		backend.file().polygon_overlay.set_start ( x, y );

		if ( backend.file().polygon_overlay.add () )
		{
			res = 1;
			break;
		}
	}

	if ( res == 0 )
	{
		backend.file().set_tool_mode (
			mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON );
	}

	return res;
}

int jtf_select_rectangle (
	char	const * const *	const	args
	)
{
	int		x, y, w, h;

	if (	mtKit::cli_parse_int ( args[0], x, 0, 32767 )	||
		mtKit::cli_parse_int ( args[1], y, 0, 32767 )	||
		mtKit::cli_parse_int ( args[2], w, 1, 32767 )	||
		mtKit::cli_parse_int ( args[3], h, 1, 32767 )
		)
	{
		return 1;
	}

	if ( backend.file().rectangle_overlay.set ( x, y, w, h,
		backend.file().get_image () )
		)
	{
		return 1;
	}

	backend.file().set_tool_mode (
		mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE );

	return 0;
}

