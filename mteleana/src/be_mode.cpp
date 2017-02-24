/*
	Copyright (C) 2009-2016 Mark Tyler

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



static int get_rgb_from_sheet (
	CedSheet	* const	sheet,
	int		const	row,		// Row in sheet for seat
	int		* const	rgb,
	int		const	mode,
	char	const * const	party_name,
	int		const	turnout
	)
{
	int		rgb_hi[3], rgb_lo[3], r, place = 0;
	double		p = 1.0, share = 0.0, votes = 0;
	CedCell		* cell;


	rgb_lo[0] = 0;
	rgb_lo[1] = 0;
	rgb_lo[2] = 100;

	rgb_hi[0] = 255;
	rgb_hi[1] = 255;
	rgb_hi[2] = 0;

	for ( r = row; ; r++ )
	{
		cell = ced_sheet_get_cell ( sheet, r, FULL_COL_PARTY );
		if ( ! cell || ! cell->text )
		{
			break;
		}

		if ( 0 == strcmp ( cell->text, party_name ) )
		{
			votes = ced_sheet_get_cell_value ( sheet, r,
				FULL_COL_VOTES );
			place = r - row + 1;
			break;
		}
	}

	share = votes / turnout;

	if ( mode == MAP_MODE_PARTY_PLACING )
	{
		if ( place == 1 )
		{
			p = 1.0;
		}
		else if ( place == 2 )
		{
			p = 0.666666;
		}
		else if ( place == 3 )
		{
			p = 0.333333;
		}
		else
		{
			p = 0.0;
		}
	}
	else if ( mode == MAP_MODE_PARTY_VOTE_SHARE )
	{
		if ( share < 0.0 )
		{
			p = 0.0;
		}
		else if ( share > 0.5 )
		{
			p = 1.0;
		}
		else
		{
			// 0.0 <= share <= 0.5
			p = (share - 0.0) / 0.5;
		}
	}

	rgb[0] = mtPixy::rgb_2_int (
		(int)(rgb_hi[0] * p + rgb_lo[0] * (1-p)),
		(int)(rgb_hi[1] * p + rgb_lo[1] * (1-p)),
		(int)(rgb_hi[2] * p + rgb_lo[2] * (1-p)) );

	return 0;		// Success
}

int eleanaElection::getSeatRGB (
	int		const	seat_id,
	int		* const	rgb,
	int		const	mode,
	char	const * const	party_name
	) const
{
	int		res = 0, r = 0, t = 0;


	switch ( mode )
	{
	case MAP_MODE_WINNER:
		res = getTableValue ( seat_id, EL_TAB_WINNER_RGB, rgb );
		break;

	case MAP_MODE_PARTY_PLACING:
		getTableValue ( seat_id, EL_TAB_SHEET_ROW, &r );
		getTableValue ( seat_id, EL_TAB_VOTES, &t );
		res = get_rgb_from_sheet ( sheetResults, r, rgb, mode,
			party_name, t );
		break;

	case MAP_MODE_PARTY_VOTE_SHARE:
		getTableValue ( seat_id, EL_TAB_SHEET_ROW, &r );
		getTableValue ( seat_id, EL_TAB_VOTES, &t );
		res = get_rgb_from_sheet ( sheetResults, r, rgb, mode,
			party_name, t );
		break;

	case MAP_MODE_MARGINALITY:
		res = getTableValue ( seat_id, EL_TAB_MARGINALITY, rgb );
		break;

	case MAP_MODE_TURNOUT:
		res = getTableValue ( seat_id, EL_TAB_TURNOUT, rgb );
		break;

	default:
		fprintf ( stderr, "Invalid map mode - %i\n", mode );
		rgb[0] = mtPixy::rgb_2_int ( 255, 255, 255 );
		break;
	}

	return res;
}

