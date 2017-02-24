/*
	Copyright (C) 2014-2016 Mark Tyler

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

#include "be.h"



typedef struct
{
	char	const *	cell_type;
	elFindCB	callback;
	void		* user;
	char		* found_seat;
	int		seat_row;
} findState;



static int escan_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	findState	* const state = (findState *)user_data;


	if ( cell->text && cell->text[0] )
	{
		state->found_seat = cell->text;
		state->seat_row = row;

		return 1;	// Found
	}

	return 0;		// Keep searching
}

static int efind_cb (
	CedSheet	* const sheet,
	CedCell		* const	cell,
	int		const	row,
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	findState	* const state = (findState *)user_data;


	if ( ! cell->text || ! cell->text[0] )
	{
		return 0;	// Always continue
	}

	state->found_seat = NULL;

	ced_sheet_scan_area_backwards ( sheet, row, FULL_COL_SEAT_NAME,
		row - FULL_ROW_PARTY1 + 1, 1, escan_cb, user_data );

	if ( ! state->found_seat )
	{
		return 0;	// Always continue
	}

	state->callback ( state->seat_row, state->found_seat, state->cell_type,
		cell->text, NULL );

	return 0;		// Always continue
}

void eleanaElection::findText (
	char	const	* const	text,
	elFindCB	const	callback,
	void		* const	user
	)
{
	findState	state = { NULL, callback, user, NULL, 0 };


	state.cell_type = "Seat";
	ced_sheet_find_text ( sheetResults, text, CED_FIND_MODE_NONE,
		FULL_ROW_PARTY1, FULL_COL_SEAT_NAME, 0, 1, efind_cb, &state );

	state.cell_type = "Candidate";
	ced_sheet_find_text ( sheetResults, text, CED_FIND_MODE_NONE,
		FULL_ROW_PARTY1, FULL_COL_MP_NAME, 0, 1, efind_cb, &state );

	state.cell_type = "Party";
	ced_sheet_find_text ( sheetResults, text, CED_FIND_MODE_NONE,
		FULL_ROW_PARTY1, FULL_COL_PARTY, 0, 1, efind_cb, &state );

	state.cell_type = "County";
	ced_sheet_find_text ( sheetResults, text, CED_FIND_MODE_NONE,
		FULL_ROW_PARTY1, FULL_COL_COUNTY, 0, 1, efind_cb, &state );

	state.cell_type = "Region";
	ced_sheet_find_text ( sheetResults, text, CED_FIND_MODE_NONE,
		FULL_ROW_PARTY1, FULL_COL_REGION, 0, 1, efind_cb, &state );
}

