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



#define ELEANA_INDEX_ROW_START		2



eleanaIndex::eleanaIndex ()
	:
	dsPath		(),
	iRecords	(0),
	bookIndex	(),
	sheetIndex	(),
	treePartyRGB	()
{
}

eleanaIndex::~eleanaIndex ()
{
	ced_book_destroy ( bookIndex );
	bookIndex = NULL;

	free ( dsPath );
	dsPath = NULL;

	mtkit_tree_destroy ( treePartyRGB );
	treePartyRGB = NULL;
}



#define INDEX_HEADERS 5

// Sheet names in the index CED file
#define ELEANA_BOOK_SHEET_INDEX		"Index"
#define ELEANA_BOOK_SHEET_COUNTIES	"Counties"
#define ELEANA_BOOK_SHEET_REGIONS	"Regions"
#define ELEANA_BOOK_SHEET_PARTIES	"Parties"



static int invalid_index (
	CedSheet	* const	sheet
	)
{
	int		i;
	char	const	* const	heads[INDEX_HEADERS] = {
			"Year",
			"Result Summary",
			"Full Result",
			"Cartogram",
			"Polymap"
			};
	CedCell		* cell;


	for ( i = 0; i < INDEX_HEADERS; i++ )
	{
		cell = ced_sheet_get_cell ( sheet, 1, i + 1 );

		if ( ! cell )
		{
			return 1;
		}

		if ( strcmp ( cell->text, heads[i] ) )
		{
			return 1;
		}
	}

	return 0;
}

static int tpr_cmp_func (
	void	const	* k1,
	void	const	* k2
	)
{
	return strcmp ( (char const *)k1, (char const *)k2 );
}

int eleanaIndex::load (
	char	const * const	filename
	)
{
	int		i;
	int		j;
	char		* s;
	CedCell		* cell;

	char		* tp = NULL;
	CedBook		* tmp_book = NULL;
	CedSheet	* tmp_sheet = NULL;


	if ( ! filename )
	{
		return 1;
	}

	tp = strdup ( filename );
	if ( ! tp )
	{
		return 1;
	}

	s = strrchr ( tp, MTKIT_DIR_SEP );
	if ( s )
	{
		s[1] = 0;
	}

	tmp_book = ced_book_load ( filename, NULL, NULL );
	if ( ! tmp_book )
	{
		goto fail;
	}

	tmp_sheet = ced_book_get_sheet ( tmp_book, ELEANA_BOOK_SHEET_INDEX );

	if (	! tmp_sheet ||
		invalid_index ( tmp_sheet )
		)
	{
		goto fail;
	}

	for ( i = ELEANA_INDEX_ROW_START; ; i++ )
	{
		cell = ced_sheet_get_cell ( tmp_sheet, i,
			ELEANA_INDEX_COL_YEAR );

		if ( ! cell )
		{
			break;
		}

		for (	j = ELEANA_INDEX_COL_FULL;
			j <= ELEANA_INDEX_COL_POLYMAP;
			j++
			)
		{
			cell = ced_sheet_get_cell ( tmp_sheet, i, j );
			if ( ! cell )
			{
				continue;
			}

			s = mtkit_string_join ( tp, cell->text, NULL, NULL );

			if ( s )
			{
				ced_sheet_set_cell_text ( tmp_sheet, i, j, s );

				free ( s );
			}
		}
	}

	// Successful so commit to class
	free ( dsPath );
	dsPath = tp;

	ced_book_destroy ( bookIndex );
	bookIndex = tmp_book;

	iRecords = i - ELEANA_INDEX_ROW_START;
	sheetIndex = tmp_sheet;

	tmp_sheet = ced_book_get_sheet ( tmp_book, ELEANA_BOOK_SHEET_PARTIES );
	treePartyRGB = mtkit_tree_new ( tpr_cmp_func, NULL );

	for ( i = 1; ; i++ )
	{
		cell = ced_sheet_get_cell ( tmp_sheet, i, 1 );

		if ( ! cell || ! cell->text )
		{
			break;
		}

		s = cell->text;

		cell = ced_sheet_get_cell ( tmp_sheet, i, 2 );

		if ( ! cell )
		{
			break;
		}

		mtkit_tree_node_add ( treePartyRGB, s,
			(void *)(intptr_t)cell->value );
	}

	return 0;

fail:
	free ( tp );
	ced_book_destroy ( tmp_book );

	return 1;
}

char const * eleanaIndex::getText (
	int	const	row,
	int	const	col
	)
{
	CedCell		* cell;


	if (	! sheetIndex		||
		row >= iRecords		||
		col < 1			||
		col >= ELEANA_INDEX_COL_TOTAL
		)
	{
		return NULL;
	}

	cell = ced_sheet_get_cell ( sheetIndex, row + ELEANA_INDEX_ROW_START,
		col );

	if ( ! cell )
	{
		return NULL;
	}

	return cell->text;
}

CedSheet * eleanaIndex::getCountySheet ()
{
	return ced_book_get_sheet ( bookIndex, ELEANA_BOOK_SHEET_COUNTIES );
}

CedSheet * eleanaIndex::getRegionSheet ()
{
	return ced_book_get_sheet ( bookIndex, ELEANA_BOOK_SHEET_REGIONS );
}

int eleanaIndex::getRecords () const
{
	return iRecords;
}

int eleanaIndex::getPartyRGB (
	char	const * const	party
	)
{
	if ( ! party || ! treePartyRGB )
	{
		return -1;
	}


	mtTreeNode * tn = mtkit_tree_node_find ( treePartyRGB, party );


	if ( ! tn )
	{
		return mtPixy::rgb_2_int ( 180, 180, 180 );
	}

	return (int)(intptr_t)(tn->data);
}

