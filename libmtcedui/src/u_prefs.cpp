/*
	Copyright (C) 2012-2016 Mark Tyler

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



typedef struct
{
	int		item;
	int		value;
	char	const	* charp;
	int		value_mask;	// Border type mask: 0 = set as value
					// > 0 = use as bit mask on current
					// value before or'ing the value.
					// -1 = Use value as a perimeter around
					// selection.
	int		r1;
	int		c1;
	int		r2;
	int		c2;
	int		rowtot;
	int		coltot;

	CedCellPrefs	const	* dcprefs;	// Default cell prefs

} prefnewSTATE;



static int cb_prep_cell (
	prefnewSTATE	* const	ARG_UNUSED ( state ),
	CedCell		* const	cell
	)
{
	if ( ! cell->prefs )
	{
		cell->prefs = ced_cell_prefs_new ();
		if ( ! cell->prefs )
		{
			return 1;
		}
	}

	return 0;
}

static void cb_check_cell (
	prefnewSTATE	* const	state,
	CedCell		* const	cell
	)
{
	if ( ! memcmp ( cell->prefs, state->dcprefs, sizeof ( CedCellPrefs ) ) )
	{
		// Remove zombie prefs

		ced_cell_prefs_destroy ( cell->prefs );
		cell->prefs = NULL;
	}
}



#define CB_TINY( NAME, ACTION ) \
static int cb_ ## NAME( \
	CedSheet	* const	ARG_UNUSED ( sheet ), \
	CedCell		* const	cell, \
	int		const	ARG_UNUSED ( row ), \
	int		const	ARG_UNUSED ( col ), \
	void		* const	user_data \
	) \
{ \
	prefnewSTATE	* const	state = (prefnewSTATE *)user_data; \
\
\
	if ( cb_prep_cell ( state, cell ) ) \
	{ \
		return 1; \
	} \
\
	ACTION \
\
	cb_check_cell ( state, cell ); \
\
	return 0;		/* Continue */\
}



CB_TINY ( align_horizontal,	cell->prefs->align_horizontal = state->value; )
CB_TINY ( color_background,	cell->prefs->color_background = state->value; )
CB_TINY ( color_foreground,	cell->prefs->color_foreground = state->value; )
CB_TINY ( format,		cell->prefs->format = state->value; )
CB_TINY ( width,		cell->prefs->width = state->value; )
CB_TINY ( num_decimal_places,	cell->prefs->num_decimal_places = state->value; )
CB_TINY ( num_zeros,		cell->prefs->num_zeros = state->value; )
CB_TINY ( text_style,		cell->prefs->text_style = state->value; )
CB_TINY ( locked,		cell->prefs->locked = state->value; )
CB_TINY ( border_type,		cell->prefs->border_type = state->value; )
CB_TINY ( border_color,		cell->prefs->border_color = state->value; )
CB_TINY ( format_datetime,	mtkit_strfreedup ( &cell->prefs->format_datetime, state->charp ); )
CB_TINY ( num_thousands,	mtkit_strfreedup ( &cell->prefs->num_thousands, state->charp ); )
CB_TINY ( text_prefix,		mtkit_strfreedup ( &cell->prefs->text_prefix, state->charp ); )
CB_TINY ( text_suffix,		mtkit_strfreedup ( &cell->prefs->text_suffix, state->charp ); )


static CedFuncScanArea const func_change[CUI_CELLPREFS_TOTAL] = {
	cb_align_horizontal,
	cb_color_background,
	cb_color_foreground,
	cb_format,
	cb_width,
	cb_num_decimal_places,
	cb_num_zeros,
	cb_text_style,
	cb_locked,
	cb_border_type,
	cb_border_color,
	cb_format_datetime,
	cb_num_thousands,
	cb_text_prefix,
	cb_text_suffix
	};


int cui_cellprefs_init (
	CedSheet	*	const	sheet,
	int		*	const	r1,
	int		*	const	c1,
	int		*	const	r2,
	int		*	const	c2,
	CedSheet	**	const	tmp_sheet
	)
{
	CedSheet	* newsheet,
			* tsh;


	if (	! sheet ||
		! tmp_sheet ||
		! r1 ||
		! c1 ||
		! r2 ||
		! c2
		)
	{
		return 1;
	}

	ced_sheet_cursor_max_min ( sheet, r1, c1, r2, c2 );

	newsheet = ced_sheet_copy_area ( sheet, r1[0], c1[0], r2[0], c2[0] );
	if ( ! newsheet )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	tmp_sheet[0] = newsheet;

	/*
	This trick is to ensure that every cell reference in the sheet range has
	cell available. This is needed so that we can use scan to change each of
	the prefs later.
	*/

	tsh = ced_sheet_new ();
	ced_sheet_set_cell_text ( tsh, 1, 1, "Y" ); // Y makes debugging easier
	ced_sheet_paste_area ( newsheet, tsh, 1, 1,
		r2[0] - r1[0] + 1, c2[0] - c1[0] + 1, 1, 1, CED_PASTE_CONTENT );
	ced_sheet_destroy ( tsh );

	return 0;
}

int cui_cellprefs_change (
	CuiBook		* const	cubook,
	CedSheet	* const	sheet,
	int		const	r1,
	int		const	c1,
	int		const	r2,
	int		const	c2,
	CedSheet	* const	tmp_sheet,
	int		const	pref_id,
	char	const	* const	pref_charp,
	int		const	pref_int
	)
{
	int		res,
			old_lock = 0;
	prefnewSTATE	state = { pref_id, pref_int, pref_charp, 0,
				r1, c1, r2, c2, r2 - r1 + 1, c2 - c1 + 1,NULL };


	if (	! cubook ||
		! sheet ||
		! tmp_sheet ||
		pref_id < 0 ||
		pref_id >= CUI_CELLPREFS_TOTAL
		)
	{
		return 1;
	}

	state.dcprefs = ced_cell_prefs_default ();

	ced_sheet_scan_area ( tmp_sheet, 1, 1, state.rowtot, state.coltot,
		func_change[ pref_id ], &state );

	if ( pref_id == CUI_CELLPREFS_locked )
	{
		// If changing locked setting for this cell we must disable
		// any current lock otherwise we can't change it
		old_lock = cubook->book->prefs.disable_locks;

		// If sheet lock is on stop all changes still
		cubook->book->prefs.disable_locks = ! sheet->prefs.locked;
	}

	// Paste the prefs onto the sheet
	res = cui_sheet_paste_area ( cubook, sheet, tmp_sheet,
		r1, c1, state.rowtot, state.coltot, state.rowtot, state.coltot,
		CED_PASTE_PREFS );

	if ( pref_id == CUI_CELLPREFS_locked )
	{
		// Restore old lock setting
		cubook->book->prefs.disable_locks = old_lock;
	}

	return res;
}

static int cb_text_style_change (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	prefnewSTATE	* const	state = (prefnewSTATE *)user_data;


	if ( ! cell->prefs )
	{
		cell->prefs = ced_cell_prefs_new ();
		if ( ! cell->prefs )
		{
			return 1;
		}
	}

	cell->prefs->text_style = (cell->prefs->text_style & state->value_mask)
		| state->value;

	return 0;			// Continue
}

int cui_cellprefs_text_style (
	CuiFile		* const	uifile,
	int		const	style
	)
{
	prefnewSTATE	state		= { 0, 0, NULL, 0, 0,0,0, 0,0,0, NULL };
	int		res		= 0;
	int		old_style	= 0;
	int		style_mask	= 0;
	int		style_val	= 0;
	CedSheet	* sheet;
	CedSheet	* tmp_sheet;
	CedCell		* cell		= NULL;


	if ( ! uifile )
	{
		return 1;
	}

	sheet = cui_file_get_sheet ( uifile );
	if ( cui_cellprefs_init ( sheet, &state.r1, &state.c1, &state.r2,
		&state.c2, &tmp_sheet ) )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	cell = ced_sheet_get_cell ( sheet, state.r1, state.c1 );
	if ( cell && cell->prefs )
	{
		old_style = cell->prefs->text_style;
	}

	state.rowtot = state.r2 - state.r1 + 1;
	state.coltot = state.c2 - state.c1 + 1;

	if ( style == CED_TEXT_STYLE_CLEAR )
	{
		style_mask = ~0;
		style_val = 0;
	}
	else
	{
		// Invert the old_style value to get the new style
		old_style = ~ old_style;

		if ( CED_TEXT_STYLE_IS_BOLD ( style ) )
		{
			style_val |= (old_style & CED_TEXT_STYLE_BOLD);
			style_mask |= CED_TEXT_STYLE_BOLD;
		}

		if ( CED_TEXT_STYLE_IS_ITALIC ( style ) )
		{
			style_val |= (old_style & CED_TEXT_STYLE_ITALIC);
			style_mask |= CED_TEXT_STYLE_ITALIC;
		}

		if ( CED_TEXT_STYLE_IS_UNDERLINE ( style ) )
		{
			if ( CED_TEXT_STYLE_IS_UNDERLINE ( ~ old_style ) )
			{
				// Old style had underlining so clear it here
			}
			else
			{
				style_val |=
					(style & CED_TEXT_STYLE_UNDERLINE_ANY);
			}

			style_mask |= CED_TEXT_STYLE_UNDERLINE_ANY;
		}

		if ( CED_TEXT_STYLE_IS_STRIKETHROUGH ( style ) )
		{
			style_val |= (old_style & CED_TEXT_STYLE_STRIKETHROUGH);
			style_mask |= CED_TEXT_STYLE_STRIKETHROUGH;
		}
	}

	state.value_mask = ~ style_mask;
	state.value = style_val;

	if ( ced_sheet_scan_area ( tmp_sheet, 1, 1, state.rowtot, state.coltot,
		cb_text_style_change, &state ) )
	{
		res = CUI_ERROR_NO_CHANGES;
	}
	else
	{
		// Paste the prefs onto the sheet
		res = cui_sheet_paste_area ( uifile->cubook, sheet, tmp_sheet,
			state.r1, state.c1, state.rowtot, state.coltot,
			state.rowtot, state.coltot, CED_PASTE_PREFS );
	}

	ced_sheet_destroy ( tmp_sheet );

	return res;
}



#define BTOP	CED_CELL_BORDER_TOP_SHIFT
#define BMID	CED_CELL_BORDER_MIDDLE_SHIFT
#define BBOT	CED_CELL_BORDER_BOTTOM_SHIFT
#define BLEF	CED_CELL_BORDER_LEFT_SHIFT
#define BCEN	CED_CELL_BORDER_CENTER_SHIFT
#define BRIG	CED_CELL_BORDER_RIGHT_SHIFT

#define BNON	CED_CELL_BORDER_NONE
#define BHIN	CED_CELL_BORDER_THIN
#define BICK	CED_CELL_BORDER_THICK
#define BDOU	CED_CELL_BORDER_DOUBLE

#define BMAS	CED_CELL_BORDER_MASK



static int cb_border_type_mask (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	prefnewSTATE	* const	state = (prefnewSTATE *)user_data;


	if ( ! cell->prefs )
	{
		cell->prefs = ced_cell_prefs_new ();
		if ( ! cell->prefs )
		{
			return 1;
		}
	}

	if ( state->value_mask == 0 )
	{
		// Set all border types at once

		cell->prefs->border_type = state->value;
	}
	else if ( state->value_mask > 0 )
	{
		// Use mask

		cell->prefs->border_type &= state->value_mask;
		cell->prefs->border_type |= state->value;
	}
	else if ( state->value_mask == -1 )
	{
		// Set according to perimeter

		if ( col == 1 )
		{
			// LEFT
			cell->prefs->border_type &= 0x3fffffff -
				( BMAS << BLEF );
			cell->prefs->border_type |= ( state->value << BLEF );
		}
		if ( col == state->coltot )
		{
			// RIGHT
			cell->prefs->border_type &= 0x3fffffff -
				( BMAS << BRIG );
			cell->prefs->border_type |= ( state->value << BRIG );
		}
		if ( row == 1 )
		{
			// TOP
			cell->prefs->border_type &= 0x3fffffff -
				( BMAS << BTOP );
			cell->prefs->border_type |= ( state->value << BTOP );
		}
		if ( row == state->rowtot )
		{
			// BOTTOM
			cell->prefs->border_type &= 0x3fffffff -
				( BMAS << BBOT );
			cell->prefs->border_type |= ( state->value << BBOT );
		}
	}

	return 0;			// Continue
}

int cui_cellprefs_border (
	CuiFile		* const	uifile,
	int		const	border_type
	)
{
	prefnewSTATE	state = { 0, 0, NULL, 0, 0,0,0, 0,0,0, NULL };
	CedSheet	* sheet;
	CedSheet	* tmp_sheet;
	int		res;


	if (	! uifile ||
		border_type < CUI_CELLBORD_MIN ||
		border_type > CUI_CELLBORD_MAX
		)
	{
		return 1;
	}

	sheet = cui_file_get_sheet ( uifile );
	if ( cui_cellprefs_init ( sheet, &state.r1, &state.c1,
		&state.r2, &state.c2, &tmp_sheet ) )
	{
		return CUI_ERROR_NO_CHANGES;
	}

	state.rowtot = state.r2 - state.r1 + 1;
	state.coltot = state.c2 - state.c1 + 1;

	if ( border_type < 0 )
	{
		switch ( border_type )
		{
		case -1:
			state.value = 0;
			break;

		case -2:
			state.value = CED_CELL_BORDER_THIN;
			state.value_mask = -1;
			break;

		case -3:
			state.value = CED_CELL_BORDER_THICK;
			state.value_mask = -1;
			break;

		case -4:
			state.value = CED_CELL_BORDER_DOUBLE;
			state.value_mask = -1;
			break;

		case -5:
			state.value = (CED_CELL_BORDER_THIN <<
					CED_CELL_BORDER_TOP_SHIFT) |
				(CED_CELL_BORDER_THIN <<
					CED_CELL_BORDER_BOTTOM_SHIFT);
			break;

		case -6:
			state.value = (CED_CELL_BORDER_THICK <<
					CED_CELL_BORDER_TOP_SHIFT) |
				(CED_CELL_BORDER_THICK <<
					CED_CELL_BORDER_BOTTOM_SHIFT);
			break;

		case -7:
			state.value = (CED_CELL_BORDER_DOUBLE <<
					CED_CELL_BORDER_TOP_SHIFT) |
				(CED_CELL_BORDER_DOUBLE <<
					CED_CELL_BORDER_BOTTOM_SHIFT);
			break;
		}
	}
	else
	{
		static int border_table[24][2] = {

			// Horizontal
			{ BNON, BTOP }, { BNON, BMID }, { BNON, BBOT },
			{ BHIN, BTOP }, { BHIN, BMID }, { BHIN, BBOT },
			{ BICK, BTOP }, { BICK, BMID }, { BICK, BBOT },
			{ BDOU, BTOP }, { BDOU, BMID }, { BDOU, BBOT },

			// Vertical
			{ BNON, BLEF }, { BNON, BCEN }, { BNON, BRIG },
			{ BHIN, BLEF }, { BHIN, BCEN }, { BHIN, BRIG },
			{ BICK, BLEF }, { BICK, BCEN }, { BICK, BRIG },
			{ BDOU, BLEF }, { BDOU, BCEN }, { BDOU, BRIG },

			};


		state.value = (border_table[ border_type ][0] <<
			border_table[ border_type ][1]);
		state.value_mask = 0x3fffffff -
			(BMAS << border_table[ border_type ][1]);
	}

	if ( ced_sheet_scan_area ( tmp_sheet, 1, 1, state.rowtot, state.coltot,
		cb_border_type_mask, &state ) )
	{
		res = CUI_ERROR_NO_CHANGES;
	}
	else
	{
		// Paste the prefs onto the sheet
		res = cui_sheet_paste_area ( uifile->cubook, sheet, tmp_sheet,
			state.r1, state.c1, state.rowtot, state.coltot,
			state.rowtot, state.coltot, CED_PASTE_PREFS );
	}

	ced_sheet_destroy ( tmp_sheet );

	return res;
}

