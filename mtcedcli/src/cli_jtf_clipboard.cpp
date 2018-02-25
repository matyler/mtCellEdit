/*
	Copyright (C) 2012-2017 Mark Tyler

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



enum
{
	CLIP_TRANSPOSE			= 0,
	CLIP_FLIP_HORIZONTAL		= 1,
	CLIP_FLIP_VERTICAL		= 2,
	CLIP_ROTATE_CLOCKWISE		= 3,
	CLIP_ROTATE_ANTICLOCKWISE	= 4,

	CLIP_TOTAL			= 5
};



static int transform_clipboard (
	int	const	item
	)
{
	CedSheet	* sheet = backend.clipboard()->sheet;

	if ( ! sheet )
	{
		fprintf ( stderr, "No clipboard exists.\n\n" );

		return 2;
	}

	switch ( item )
	{
	case CLIP_TRANSPOSE:
		sheet = ced_sheet_transpose ( sheet );
		break;

	case CLIP_FLIP_HORIZONTAL:
		sheet = ced_sheet_flip_horizontal ( sheet );
		break;

	case CLIP_FLIP_VERTICAL:
		sheet = ced_sheet_flip_vertical ( sheet );
		break;

	case CLIP_ROTATE_CLOCKWISE:
		sheet = ced_sheet_rotate_clockwise ( sheet );
		break;

	case CLIP_ROTATE_ANTICLOCKWISE:
		sheet = ced_sheet_rotate_anticlockwise ( sheet );
		break;

	default:
		goto fail;
	}

	if ( ! sheet )
	{
		goto fail;
	}

	if (	cui_clip_flush ( backend.clipboard() )	||
		backend.clipboard()->sheet )
	{
		ced_sheet_destroy ( sheet );
		sheet = NULL;

		goto fail;
	}

	backend.clipboard()->sheet = sheet;
	sheet = NULL;

	if ( item != CLIP_FLIP_HORIZONTAL && item != CLIP_FLIP_VERTICAL )
	{
		int	tmp = backend.clipboard()->rows;


		// X/Y Geometry has been swapped

		backend.clipboard()->rows = backend.clipboard()->cols;
		backend.clipboard()->cols = tmp;
	}

	return 0;

fail:
	fprintf ( stderr, "Unable to transform clipboard.\n\n" );

	return 2;
}

int jtf_clip_flip_h (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( CLIP_FLIP_HORIZONTAL );
}

int jtf_clip_flip_v (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( CLIP_FLIP_VERTICAL );
}

int jtf_clip_rotate_a (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( CLIP_ROTATE_ANTICLOCKWISE );
}

int jtf_clip_rotate_c (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( CLIP_ROTATE_CLOCKWISE );
}

int jtf_clip_transpose (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( CLIP_TRANSPOSE );
}

int jtf_clip_load (
	char	const * const * const	args
	)
{
	if ( cui_clip_load_temp_file ( backend.clipboard (), args[0] ) )
	{
		fprintf ( stderr, "jtf_clip_load: Unable to load clipboard.\n\n"
			);

		return 2;
	}

	return 0;
}

int jtf_clip_save (
	char	const * const * const	args
	)
{
	if ( cui_clip_save_temp_file ( backend.clipboard (), args[0] ) )
	{
		fprintf ( stderr, "jtf_clip_save: Unable to save clipboard.\n\n"
			);

		return 2;
	}

	return 0;
}

static int copy_selection_to_clip ()
{
	if ( cui_clip_copy ( backend.file(), backend.clipboard() ) )
	{
		fprintf ( stderr,
			"copy_selection_to_clip: Unable to copy selection.\n\n"
			);

		return 2;
	}

	return 0;		// Success
}

int jtf_copy (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip () )
	{
		return 2;
	}

	return 0;
}

static int copy_output_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	char * const txt = ced_cell_create_output ( cell, NULL );

	free ( cell->text );
	cell->text = txt;

	cell->type = CED_CELL_TYPE_TEXT_EXPLICIT;
	cell->value = 0;

	return 0;
}

int jtf_copy_output (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip () )
	{
		return 2;
	}

	if ( ced_sheet_scan_area ( backend.clipboard()->sheet, 1, 1, 0, 0,
		copy_output_scan, NULL )
		)
	{
		fprintf ( stderr,
			"jtf_copy_output: Unable to create clipboard.\n\n" );

		return 2;
	}

	return 0;
}

static int copy_value_scan (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	ARG_UNUSED ( user_data )
	)
{
	char		txt[256];


	switch ( cell->type )
	{
	case CED_CELL_TYPE_FORMULA:
	case CED_CELL_TYPE_FORMULA_EVAL:
		snprintf ( txt, sizeof ( txt ), CED_PRINTF_NUM, cell->value );
		if ( ! mtkit_strfreedup ( &cell->text, txt ) )
		{
			cell->type = CED_CELL_TYPE_VALUE;
		}
		else
		{
			return 1;
		}
		break;

	default:
		break;
	}

	return 0;
}

int jtf_copy_values (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip () )
	{
		return 2;
	}

	if ( ced_sheet_scan_area ( backend.clipboard()->sheet, 1, 1, 0, 0,
		copy_value_scan, NULL ) )
	{
		fprintf ( stderr,
			"jtf_copy_values: Unable to create clipboard.\n\n" );

		return 2;
	}

	return 0;
}

static int clear_selection (
	int	const	mode
	)
{
	CedSheet * const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return 1;
	}

	int const r = MIN ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	int const c = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	int const rtot = 1 + abs ( sheet->prefs.cursor_r1 -
		sheet->prefs.cursor_r2 );
	int const ctot = 1 + abs ( sheet->prefs.cursor_c1 -
		sheet->prefs.cursor_c2 );

	if ( r < 1 || c < 1 )
	{
		goto fail;
	}

	{
		int const res = cui_sheet_clear_area ( backend.file()->cubook,
			sheet, r, c, rtot, ctot, mode );

		backend.undo_report_updates ( res );

		if (	res == CUI_ERROR_LOCKED_CELL ||
			res == CUI_ERROR_NO_CHANGES
			)
		{
			goto fail;
		}
	}

	backend.update_changes_chores ();

	return 0;		// Success

fail:
	fprintf ( stderr, "clear_selection: Unable to clear selection.\n\n" );

	return 1;
}

int jtf_cut (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip () ||
		clear_selection ( 0 ) )
	{
		return 2;
	}

	return 0;
}

int jtf_clear (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( clear_selection ( 0 ) )
	{
		return 2;
	}

	return 0;
}

int jtf_clear_content (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( clear_selection ( CED_PASTE_CONTENT ) )
	{
		return 2;
	}

	return 0;
}

int jtf_clear_prefs (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	if ( clear_selection ( CED_PASTE_PREFS ) )
	{
		return 2;
	}

	return 0;
}

static CedSheet * obtain_paste ()
{
	CedSheet * const sheet = backend.sheet ();

	if ( ! sheet )
	{
		return NULL;
	}

	if ( ! backend.clipboard()->sheet )
	{
		fprintf ( stderr, "obtain_paste: No clipboard.\n\n" );
		return NULL;
	}

	return sheet;
}

static int paste_clipboard_at_cursor (
	int	const	mode
	)
{
	if ( ! obtain_paste () )
	{
		goto fail;		// No paste or sheet found
	}

	{
		int const res = cui_clip_paste ( backend.file(),
			backend.clipboard(), mode );

		if ( res == 1 )
		{
			goto fail;
		}

		backend.undo_report_updates ( res );

		if (	res == CUI_ERROR_LOCKED_CELL	||
			res == CUI_ERROR_LOCKED_SHEET	||
			res == CUI_ERROR_NO_CHANGES
			)
		{
			goto fail;			// Nothing changed
		}
	}

	backend.update_changes_chores ();

	return 0;				// Paste committed

fail:
	fprintf ( stderr, "paste_clipboard_at_cursor: Unable to paste.\n\n" );
	return 2;
}

int jtf_paste (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return paste_clipboard_at_cursor ( 0 );
}

int jtf_paste_content (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return paste_clipboard_at_cursor ( CED_PASTE_CONTENT );
}

int jtf_paste_prefs (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return paste_clipboard_at_cursor ( CED_PASTE_PREFS );
}

