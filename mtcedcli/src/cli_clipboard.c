/*
	Copyright (C) 2012-2015 Mark Tyler

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



static int copy_selection_to_clip (
	CedCli_STATE	* const	state
	)
{
	if ( cui_clip_copy ( CEDCLI_FILE, state->clipboard ) )
	{
		fprintf ( stderr,
			"copy_selection_to_clip: Unable to copy selection.\n\n"
			);

		return 1;
	}

	return 0;		// Success
}

static int clear_selection (
	CedCli_STATE	* const	state,
	int		const	mode
	)
{
	CedSheet	* sheet;
	int		r,
			c,
			rtot,
			ctot;
	int		res;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	r = MIN ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	c = MIN ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	rtot = 1 + abs ( sheet->prefs.cursor_r1 - sheet->prefs.cursor_r2 );
	ctot = 1 + abs ( sheet->prefs.cursor_c1 - sheet->prefs.cursor_c2 );

	if ( r<1 || c<1 )
	{
		goto fail;
	}

	res = cui_sheet_clear_area ( CEDCLI_FILE->cubook, sheet, r, c, rtot,
		ctot, mode );
	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		goto fail;
	}

	update_changes_chores ( state, sheet );

	return 0;		// Success

fail:
	fprintf ( stderr, "clear_selection: Unable to clear selection.\n\n" );

	return 1;
}

static int check_clipboard_exists (
	CedCli_STATE	* const	state
	)
{
	if ( ! state->clipboard->sheet )
	{
		fprintf ( stderr, "No clipboard exists.\n\n" );

		return 1;
	}

	return 0;
}

static int transform_clipboard (
	CedCli_STATE	* const	state,
	int		const	item
	)
{
	CedSheet	* sheet;


	if ( check_clipboard_exists ( state ) )
	{
		return 1;
	}

	switch ( item )
	{
	case 0:
		sheet = ced_sheet_transpose ( state->clipboard->sheet );
		break;

	case 1:
		sheet = ced_sheet_flip_horizontal ( state->clipboard->sheet );
		break;
	case 2:
		sheet = ced_sheet_flip_vertical ( state->clipboard->sheet );
		break;
	case 3:
		sheet = ced_sheet_rotate_clockwise ( state->clipboard->sheet );
		break;
	case 4:
		sheet = ced_sheet_rotate_anticlockwise ( state->clipboard->sheet
			);
		break;

	default:
		goto fail;
	}

	if ( ! sheet )
	{
		goto fail;
	}

	if ( cui_clip_flush ( state->clipboard ) || state->clipboard->sheet )
	{
		ced_sheet_destroy ( sheet );

		goto fail;
	}

	state->clipboard->sheet = sheet;

	if ( item != 1 && item != 2 )
	{
		int	tmp = state->clipboard->rows;


		// X/Y Geometry has been swapped

		state->clipboard->rows = state->clipboard->cols;
		state->clipboard->cols = tmp;
	}

	return 0;

fail:
	fprintf ( stderr, "Unable to transform clipboard.\n\n" );

	return 1;
}

int jtf_clip_flip_h (
	CedCli_STATE	* const state,
	char		** const ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( state, 1 );
}

int jtf_clip_flip_v (
	CedCli_STATE	* const state,
	char		** const ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( state, 2 );
}

int jtf_clip_rotate_a (
	CedCli_STATE	* const state,
	char		** const ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( state, 4 );
}

int jtf_clip_rotate_c (
	CedCli_STATE	* const state,
	char		** const ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( state, 3 );
}

int jtf_clip_transpose (
	CedCli_STATE	* const state,
	char		** const ARG_UNUSED ( args )
	)
{
	return transform_clipboard ( state, 0 );
}

int jtf_clip_load (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	if ( cui_clip_load_temp_file ( state->clipboard, args[0] ) )
	{
		fprintf ( stderr, "jtf_clip_load: Unable to load clipboard.\n\n"
			);

		return 1;
	}

	return 0;
}

int jtf_clip_save (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	if ( cui_clip_save_temp_file ( state->clipboard, args[0] ) )
	{
		fprintf ( stderr, "jtf_clip_save: Unable to save clipboard.\n\n"
			);

		return 1;
	}

	return 0;
}

int jtf_copy (
	CedCli_STATE	* const state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip ( state ) )
	{
		return 1;
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
	char		* txt;


	txt = ced_cell_create_output ( cell, NULL );
	free ( cell->text );

	cell->text = txt;
	cell->type = CED_CELL_TYPE_TEXT_EXPLICIT;
	cell->value = 0;

	return 0;
}

int jtf_copy_output (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip ( state ) )
	{
		return 1;
	}

	if ( ced_sheet_scan_area ( state->clipboard->sheet, 1, 1, 0, 0,
		copy_output_scan, NULL )
		)
	{
		fprintf ( stderr,
			"jtf_copy_output: Unable to create clipboard.\n\n" );

		return 1;
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
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip ( state ) )
	{
		return 1;
	}

	if ( ced_sheet_scan_area ( state->clipboard->sheet, 1, 1, 0, 0,
		copy_value_scan, NULL ) )
	{
		fprintf ( stderr,
			"jtf_copy_values: Unable to create clipboard.\n\n" );

		return 1;
	}

	return 0;
}

int jtf_cut (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( copy_selection_to_clip ( state ) ||
		clear_selection ( state, 0 ) )
	{
		return 1;
	}

	return 0;
}

int jtf_clear (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( clear_selection ( state, 0 ) )
	{
		return 1;
	}

	return 0;
}

int jtf_clear_content (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( clear_selection ( state, CED_PASTE_CONTENT ) )
	{
		return 1;
	}

	return 0;
}

int jtf_clear_prefs (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( clear_selection ( state, CED_PASTE_PREFS ) )
	{
		return 1;
	}

	return 0;
}

static CedSheet * obtain_paste (
	CedCli_STATE	* const	state
	)
{
	CedSheet	* sheet;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return NULL;
	}

	if ( ! state->clipboard->sheet )
	{
		fprintf ( stderr, "obtain_paste: No clipboard.\n\n" );
		sheet = NULL;
	}

	return sheet;
}

static int paste_clipboard_at_cursor (
	CedCli_STATE	* const	state,
	int		const	mode
	)
{
	CedSheet	* sheet;
	int		res;


	sheet = obtain_paste ( state );
	if ( ! sheet )
	{
		goto fail;		// No paste or sheet found
	}

	res = cui_clip_paste ( CEDCLI_FILE, state->clipboard, mode );
	if ( res == 1 )
	{
		goto fail;
	}

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_LOCKED_SHEET ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		goto fail;			// Nothing changed
	}

	update_changes_chores ( state, sheet );

	return 0;				// Paste committed

fail:
	fprintf ( stderr, "paste_clipboard_at_cursor: Unable to paste.\n\n" );
	return 1;
}

int jtf_paste (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return paste_clipboard_at_cursor ( state, 0 );
}

int jtf_paste_content (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return paste_clipboard_at_cursor ( state, CED_PASTE_CONTENT );
}

int jtf_paste_prefs (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	return paste_clipboard_at_cursor ( state, CED_PASTE_PREFS );
}
