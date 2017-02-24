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



enum
{
	SUMMARY_ROW_TURNOUT		= 1,
	SUMMARY_ROW_TOTAL		= 2,
	SUMMARY_ROW_START		= 4
};

enum
{
	SUMMARY_COL_PARTY		= 1,
	SUMMARY_COL_VOTES		= 2,
	SUMMARY_COL_VOTES_PERC		= 3,
	SUMMARY_COL_SEATS		= 4,
	SUMMARY_COL_CANDIDATES		= 5
};



typedef struct
{
	int		votes;
	int		seats;
	int		candidates;
} dataParty;

typedef struct
{
	CedSheet	* sheet_results;
	CedCell		* cell;
	CedCellPrefs	* cellprefs;

	int		tot_votes;
	int		tot_electorate;
	int		row;

	mtTree		* tree_parties;	// Key=Party Name Data=dataParty
	CedSheet	* sheet_summary;
} summarySTATE;



static int set_cellprefs_cb (
	CedSheet	* const	sheet,
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	summarySTATE	* const	state = (summarySTATE *)user_data;


	ced_sheet_set_cell_prefs ( sheet, row, col, state->cellprefs, NULL );

	return 0;
}

static int count_votes_cb (
	CedSheet	* const	sheet,
	CedCell		* const	cell,
	int		const	row,
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	summarySTATE	* const	state = (summarySTATE *)user_data;
	mtTreeNode	* node;
	dataParty	* dp;


	if ( ! cell || ! cell->text )
	{
		return 0;
	}

	node = mtkit_tree_node_find ( state->tree_parties, cell->text );
	if ( ! node )
	{
		// Party not yet in tree so add it

		dp = (dataParty *)calloc ( 1, sizeof(*dp) );
		if ( ! dp )
		{
			fprintf ( stderr, "count_votes_cb calloc error\n" );
			return 0;
		}

		if ( 0 == mtkit_tree_node_add ( state->tree_parties, cell->text,
			dp )
			)
		{
			fprintf ( stderr, "count_votes_cb tree error\n" );
			free ( dp );
			return 0;
		}
	}
	else
	{
		dp = (dataParty *)node->data;
	}

	// Votes
	state->cell = ced_sheet_get_cell ( sheet, row, FULL_COL_VOTES );
	if ( state->cell )
	{
		dp->votes		+= (int)state->cell->value;
		state->tot_votes	+= (int)state->cell->value;
	}

	dp->candidates ++;

	// Winning party
	state->cell = ced_sheet_get_cell ( sheet, row, FULL_COL_SEAT_NAME );
	if ( state->cell && state->cell->text )
	{
		// This party won this seat
		dp->seats ++;

		state->cell = ced_sheet_get_cell ( sheet, row,
			FULL_COL_ELECTORATE );

		if ( state->cell )
		{
			state->tot_electorate += (int)state->cell->value;
		}
	}

	return 0;
}

static int treep_cmp (
	void	const	* k1,
	void	const	* k2
	)
{
	return strcmp ( (const char *)k1, (const char *)k2 );
}

static void treep_del (
	mtTreeNode	* node
	)
{
	free ( node->data );
}

static int treep_scan (
	mtTreeNode	* node,
	void		* user_data
	)
{
	summarySTATE	* const	state = (summarySTATE *)user_data;
	dataParty	* const dp = (dataParty *)node->data;


	ced_sheet_set_cell_text ( state->sheet_summary, state->row,
		SUMMARY_COL_PARTY, (char const *)node->key );

	ced_sheet_set_cell_value ( state->sheet_summary, state->row,
		SUMMARY_COL_VOTES, (double)dp->votes );

	ced_sheet_set_cell ( state->sheet_summary, state->row,
		SUMMARY_COL_VOTES_PERC, "=100*RC2/R2C2" );

	ced_sheet_set_cell_value ( state->sheet_summary, state->row,
		SUMMARY_COL_SEATS, (double)dp->seats );

	ced_sheet_set_cell_value ( state->sheet_summary, state->row,
		SUMMARY_COL_CANDIDATES, (double)dp->candidates );

	state->row ++;

	return 0;		// Continue
}

CedSheet * eleanaElection::createSummary ()
{
	summarySTATE state = { sheetResults, NULL, NULL, 0, 0, 0, NULL, NULL };


	ced_sheet_destroy ( sheetSummary );
	sheetSummary = ced_sheet_new ();
	state.sheet_summary = sheetSummary;

	state.tree_parties = mtkit_tree_new ( treep_cmp, treep_del );

	ced_sheet_scan_area ( sheetResults, FULL_ROW_PARTY1, FULL_COL_PARTY,
		0, 1, count_votes_cb, &state );

	ced_sheet_set_cell_text ( sheetSummary, SUMMARY_ROW_TURNOUT,
		SUMMARY_COL_PARTY, "Electorate / Turnout" );
	ced_sheet_set_cell_value ( sheetSummary, SUMMARY_ROW_TURNOUT,
		SUMMARY_COL_VOTES, (double)state.tot_electorate );
	ced_sheet_set_cell ( sheetSummary, SUMMARY_ROW_TURNOUT,
		SUMMARY_COL_VOTES_PERC, "=100*r[1]c[-1]/rc[-1]" );

	ced_sheet_set_cell_text ( sheetSummary, SUMMARY_ROW_TOTAL,
		SUMMARY_COL_PARTY, "Total" );
	ced_sheet_set_cell ( sheetSummary, SUMMARY_ROW_TOTAL,
		SUMMARY_COL_VOTES, "=sum(r[1]c:r_c)" );
	ced_sheet_set_cell ( sheetSummary, SUMMARY_ROW_TOTAL,
		SUMMARY_COL_VOTES_PERC, "=sum(r[1]c:r_c)" );
	ced_sheet_set_cell ( sheetSummary, SUMMARY_ROW_TOTAL,
		SUMMARY_COL_SEATS, "=sum(r[1]c:r_c)" );
	ced_sheet_set_cell ( sheetSummary, SUMMARY_ROW_TOTAL,
		SUMMARY_COL_CANDIDATES, "=sum(r[1]c:r_c)" );

	state.row = SUMMARY_ROW_START;
	mtkit_tree_scan ( state.tree_parties, treep_scan, &state, 0 );


	int		cols[3] = { 2, 1, 0 };


	ced_sheet_sort_rows ( sheetSummary, SUMMARY_ROW_START, 0, cols,
		CED_SORT_MODE_DESCENDING, NULL );

	ced_sheet_recalculate ( sheetSummary, NULL, 0 );
	ced_sheet_recalculate ( sheetSummary, NULL, 1 );

	mtkit_tree_destroy ( state.tree_parties );

	state.cellprefs = ced_cell_prefs_new ();
	if ( state.cellprefs )
	{
		mtkit_strfreedup ( &state.cellprefs->num_thousands, "," );

		ced_sheet_scan_area ( sheetSummary, 1, SUMMARY_COL_VOTES,
			0, 1, set_cellprefs_cb, &state );

		state.cellprefs->format = 2;
		state.cellprefs->num_decimal_places = 2;

		ced_sheet_scan_area ( sheetSummary, 1, SUMMARY_COL_VOTES_PERC,
			0, 1, set_cellprefs_cb, &state );

		ced_cell_prefs_destroy ( state.cellprefs );
		state.cellprefs = NULL;
	}

	return sheetSummary;
}

