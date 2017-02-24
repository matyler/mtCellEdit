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

extern "C" {

	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#include <time.h>
	#include <ctype.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
	#include <errno.h>

}



#include "mtcelledit.h"



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#define CED_CELL_MAX_BYTES	2000



int ced_cell_paste (			// Copy cell contents and prefs
	CedCell		* dest,
	CedCell		* src
	);

// WARNING - sheet/row/column must be valid when calling this internal function
CedCell * ced_cell_set_find (		// Find a cell or create an empty new
					// one
	CedSheet	* sheet,
	int		row,
	int		column
	);

int ced_cell_set_prefs (
	CedCell		* cell,
	CedCellPrefs	* prefs		// NULL = set to default
	);
	// -1 = failure during change
	//  0 = success

int ced_cell_stack_push (		// Push a new cell reference onto the
					// stack
	CedCellStack	** root,
	int		row,
	int		column
	);

int ced_cell_stack_del_cells (		// Destroy all cells referenced in the
					// stack
	CedCellStack	* root,
	CedSheet	* sheet
	);

void ced_cell_stack_destroy (		// Destroy the stack
	CedCellStack	* root
	);

int ced_cmp_cell (
	void	const	* k1,
	void	const	* k2
	);
	// -1 = k1 < k2
	//  0 = k1 = k2
	//  1 = k1 > k2

void ced_del_cell (
	mtTreeNode	* node
	);

int ced_sheet_delete_cell (
	CedSheet	* sheet,
	int		row,
	int		column
	);
	// -1 = error
	//  0 = nothing to delete
	//  1 = deleted

int ced_token_exe (			// Execute a function token/args
	CedFuncState	* state
	);

CedToken const * ced_token_get (	// Get token structure for a function
	char	const	* text		// Function name
	);



#ifndef DEBUG
#pragma GCC visibility pop
#endif

