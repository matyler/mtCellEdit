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



eleanaElection::eleanaElection ()
	:
	sheetResults	(),
	treePolymap	(),
	iTable		(),
	indexSeatID	(),
	sheetSummary	(),

	iSeats		(0)
{
}

eleanaElection::~eleanaElection ()
{
	clear ();
}



typedef struct
{
	CedSheet	* results_sheet;
	CedCell		* cell;
	int		* iTable;
	int		row;

	CedIndex	* idx_regions;
	CedIndex	* idx_counties;
	CedIndexItem	* index_item;
	int		* seats;

	eleanaIndex	* eindex;
	eleanaElection	* election;
} populSTATE;



static int scrape_seat_cb (
	CedSheet	* const	sheet,
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	int		rgb, r;
	populSTATE	* const	state = (populSTATE *)user_data;
	double		turnout, electorate, v, v1, v2, p1, p2;


	state->election->setTableValue ( state->row, EL_TAB_SHEET_ROW, row );

	// County
	state->cell = ced_sheet_get_cell ( sheet, row, FULL_COL_COUNTY );
	if ( state->cell )
	{
		if ( ced_index_query ( state->idx_counties, 0,
			state->cell->text, &state->index_item ) == 1 )
		{
			state->election->setTableValue ( state->row,
				EL_TAB_COUNTY, state->index_item->row );
		}
	}

	// Region
	state->cell = ced_sheet_get_cell ( sheet, row, FULL_COL_REGION );
	if ( state->cell )
	{
		if ( ced_index_query ( state->idx_regions, 0,
			state->cell->text, &state->index_item ) == 1 )
		{

			state->election->setTableValue ( state->row,
				EL_TAB_REGION, state->index_item->row );
		}

	}

	// Winning party
	state->cell = ced_sheet_get_cell ( sheet, row, FULL_COL_PARTY );
	if ( state->cell )
	{
		rgb = state->eindex->getPartyRGB ( state->cell->text );
	}
	else
	{
		rgb = mtPixy::rgb_2_int ( 180, 180, 180 );
	}

	state->election->setTableValue ( state->row, EL_TAB_WINNER_RGB, rgb );

	electorate = ced_sheet_get_cell_value( sheet, row, FULL_COL_ELECTORATE);
	turnout = v1 = v2 = 0;

	for ( r = row; ; r++ )
	{
		v = ced_sheet_get_cell_value ( sheet, r, FULL_COL_VOTES );
		if ( v < 1 )
		{
			break;
		}

		turnout += v;

		if ( r == row )
		{
			v1 = v;
		}
		else if ( r == (row + 1) )
		{
			v2 = v;
		}
	}

	// v should be 0.0 .. 1.0
	v = turnout / electorate;

	if ( v < 0.4 )
	{
		rgb = 0;
	}
	else if ( v > 0.8 )
	{
		rgb = 255;
	}
	else
	{
		// 0.3 <= v <= 0.9
		rgb = (int)( 255 * (v - 0.4) / 0.4 );
	}

	state->election->setTableValue ( state->row, EL_TAB_TURNOUT,
		mtPixy::rgb_2_int ( rgb, rgb, 100-100*rgb/255 ) );

	state->election->setTableValue ( state->row, EL_TAB_VOTES,
		(int)turnout );

	p1 = v1 / turnout;
	p2 = v2 / turnout;
	v = p1 - p2;

	if ( v < 0 )
	{
		rgb = 0;
	}
	else if ( v > 0.2 )
	{
		rgb = 255;
	}
	else
	{
		// 0.0 <= v <= 0.4
		rgb = (int)( 255 * v / 0.2 );
	}

	state->election->setTableValue ( state->row, EL_TAB_MARGINALITY,
		mtPixy::rgb_2_int ( 255-rgb, 255-rgb, 100*rgb/255 ) );

	state->row ++;

	return 0;
}

static int count_seat_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	populSTATE	* const	state = (populSTATE *)user_data;


	if ( ! cell->text )
	{
		return 0;
	}

	state->seats[0] ++;

	return 0;			// Continue
}

static int populate_table (
	eleanaIndex	* const		eindex,
	eleanaElection	* const		election,
	CedSheet	* const		sheet,
	int		* const		seats,
	int		** const	table_out
	)
{
	populSTATE	state = { sheet, NULL, NULL, 0, NULL, NULL, NULL, NULL,
					eindex, election };
	CedSheet	* sh;
	int		res = 1;


	// Count up seat names - put into 'row'
	state.seats = seats;
	seats[0] = 0;

	ced_sheet_scan_area ( sheet, FULL_ROW_PARTY1, FULL_COL_SEAT_NAME, 0, 1,
		count_seat_cb, &state );

	if ( seats[0] < 0 || seats[0] > (INT_MAX - 1) )
	{
		goto error;
	}

	state.iTable = (int *)calloc ( sizeof ( int ),
		(size_t)(EL_TAB_TOTAL * (seats[0] + 1)) );

	if ( ! state.iTable )
	{
		goto error;
	}

	table_out[0] = state.iTable;

	// Set up 2 indexes for names of regions and counties - use row as
	// reference in table
	sh = eindex->getCountySheet ();
	state.idx_counties = ced_index_new ( CED_INDEX_TYPE_TEXT );

	if ( ! sh || ! state.idx_counties )
	{
		goto error;
	}

	ced_index_add_items ( state.idx_counties, sh, 1, 1, 0, 1 );

	sh = eindex->getRegionSheet ();
	state.idx_regions = ced_index_new ( CED_INDEX_TYPE_TEXT );

	if ( ! sh || ! state.idx_regions )
	{
		goto error;
	}

	ced_index_add_items ( state.idx_regions, sh, 1, 1, 0, 1 );

	// Do second scan to scrape the data for each seat and put it into the
	// table.

	state.row = 0;
	ced_sheet_scan_area ( sheet, FULL_ROW_PARTY1, FULL_COL_SEAT_NAME, 0, 1,
		scrape_seat_cb, &state );

	res = 0;			// Success

error:
	ced_index_destroy ( state.idx_regions );
	ced_index_destroy ( state.idx_counties );

	if ( res )
	{
		free ( state.iTable );
	}

	return res;
}

static int get_cxy (
	unsigned char	const	c,
	int		* const	idx,
	unsigned char	* const	mem,
	int		* const	x,
	int		* const	y,
	int		const	w,
	int		const	idxmax
	)
{
	do
	{
		if ( mem[ idx[c] ] == c )
		{
			idx[c] += 1;

			x[0] = idx[c] % w;
			y[0] = idx[c] / w;

			return 0;
		}

		idx[c] += 1;
	}
	while ( idx[c] < idxmax );

	printf ( "get_cxy ERROR - Unable to find %i\n", (int)c );

	return 1;
}

void eleanaElection::createMapData (
	mtPixy::Image	* const	image
	)
{
	int		i, w, h, a,
			idx[256] = {0},	// Next offset for this colour
			target[256] = {0}, // Totals for each region/county
			idxmax, x, y;
	unsigned char	c,
			* mem;		// PNG Map


	w = image->get_width ();
	h = image->get_height ();

	idxmax = w * h - 1;
	mem = image->get_canvas ();

	for ( i = 0; i < iSeats; i++ )
	{
		a = 0;		// Error fallback
		getTableValue ( i, EL_TAB_REGION, &a );
		c = (unsigned char)( 200 + a );

		target[c] ++;

		a = 0;		// Error fallback
		getTableValue ( i, EL_TAB_COUNTY, &a );
		c = (unsigned char)( a );

		target[c] ++;

		if ( idx[c] > idxmax )
		{
			continue;
		}

		if ( ! get_cxy ( c, idx, mem, &x, &y, w, idxmax ) )
		{
			setTableValue ( i, EL_TAB_CARTOGRAM_X, x );
			setTableValue ( i, EL_TAB_CARTOGRAM_Y, y );
		}
		else
		{
			// Not found
		}
	}

	memset ( idx, 0, 256 * sizeof ( int ) );
	x = 0;

	for ( i = 0; i < idxmax; i += 1 )
	{
		c = mem[ i ];
		idx[c] ++;

		if ( c >= 1 && c <= 150 )
		{
			x++;
		}
	}

	if ( x != iSeats )
	{
		printf ( "create_map_data: Total seats found: %i"
			" (should be %i)\n\n", x, iSeats );

		for ( i = 1; i < 256; i++ )
		{
			if ( idx[i] != target[i] )
			{
				printf ( "%3i png = %5i   tsv = %5i  %i\n",
					i, idx[i], target[i],
					target[i] - idx[i] );
			}
		}
	}
}

int eleanaElection::load (
	eleanaIndex	* const	eindex,
	int		const	election
	)
{
	if (	! eindex				||
		election >= eindex->getRecords ()
		)
	{
		return 1;
	}

	clear ();


	mtPixy::Image	* cartogram;
	char	const	* filename;


	filename = eindex->getText ( election, ELEANA_INDEX_COL_FULL );
	if ( ! filename )
	{
		goto fail;
	}

	sheetResults = ced_sheet_load ( filename, "ISO-8859-1", NULL );
	if ( ! sheetResults )
	{
		goto fail;
	}

	if ( populate_table ( eindex, this, sheetResults, &iSeats, &iTable ) )
	{
		goto fail;
	}

	filename = eindex->getText ( election, ELEANA_INDEX_COL_CARTOGRAM );

	if ( filename )
	{
		cartogram = mtPixy::image_load ( filename );
		if ( cartogram )
		{
			createMapData ( cartogram );

			delete cartogram;
			cartogram = NULL;
		}
		else
		{
			fprintf ( stderr, "Unable to load cartogram %s\n",
				filename );
		}
	}

	filename = eindex->getText ( election, ELEANA_INDEX_COL_POLYMAP );
	loadPolymap ( filename );

	ced_sheet_recalculate ( sheetResults, NULL, 0 );

	return 0;

fail:
	clear ();

	return 1;
}

void eleanaElection::clear ()
{
	ced_sheet_destroy ( sheetResults );
	sheetResults = NULL;

	mtkit_tree_destroy ( treePolymap );
	treePolymap = NULL;

	free ( iTable );
	iTable = NULL;

	ced_index_destroy ( indexSeatID );
	indexSeatID = NULL;

	ced_sheet_destroy ( sheetSummary );
	sheetSummary = NULL;
}

CedSheet * eleanaElection::getResults ()
{
	return sheetResults;
}

int eleanaElection::getSeats () const
{
	return iSeats;
}

int eleanaElection::getCartogramXY (
	int	const	seat_id,
	int	* const	x,
	int	* const y
	) const
{

	if ( x && getTableValue ( seat_id, EL_TAB_CARTOGRAM_X, x ) )
	{
		return 1;
	}

	if ( y && getTableValue ( seat_id, EL_TAB_CARTOGRAM_Y, y ) )
	{
		return 1;
	}

	return 0;
}

int eleanaElection::getTableValue (
	int	const	seat_id,
	int	const	col,
	int	* const	val
	) const
{
	if (	! iTable		||
		! val			||
		seat_id < 0		||
		seat_id >= iSeats	||
		col < 0			||
		col >= EL_TAB_TOTAL
		)
	{
		return 1;
	}

	val[0] = iTable [ EL_TAB_TOTAL * seat_id + col ];

	return 0;
}

int eleanaElection::setTableValue (
	int	const	seat_id,
	int	const	col,
	int	const	val
	)
{
	if (	! iTable		||
		seat_id < 0		||
		seat_id >= iSeats	||
		col < 0		||
		col >= EL_TAB_TOTAL
		)
	{
		return 1;
	}

	iTable [ EL_TAB_TOTAL * seat_id + col ] = val;

	return 0;
}

