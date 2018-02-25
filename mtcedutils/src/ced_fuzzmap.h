/*
	Copyright (C) 2017 Mark Tyler

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

#include <ctype.h>
#include "ced.h"



typedef struct FuzzFile		FuzzFile;
typedef struct FuzzItem		FuzzItem;
typedef struct FuzzWord		FuzzWord;



struct FuzzFile
{
	CedSheet	* sheet;	// MUST never have cells removed
					// during lifetime of this FuzzFile

	FuzzItem	* item_first;
	FuzzItem	* item_last;
};

struct FuzzItem
{
	FuzzItem	* next;

	CedCell		* cell;		// Location in the FuzzFile->sheet

	int		word_tot;
	FuzzWord	* word_first;
	FuzzWord	* word_last;
};

struct FuzzWord
{
	FuzzWord	* next;

	char		* text;
};



FuzzFile * fuzz_file_new (
	char	const	* filename,
	int	const	* range
	);

void fuzz_file_destroy (
	FuzzFile	* fuzz
	);

int fuzz_file_match (
	FuzzFile	* fuzz_dict,
	FuzzFile	* fuzz_in
	);

CedSheet * fuzz_file_steal_sheet (
	FuzzFile	* fuzz
	);

int fuzz_item_new (
	FuzzFile	* file,
	CedCell		* cell
	);

void fuzz_item_destroy (
	FuzzItem	* item
	);

int fuzz_item_cmp (
	FuzzItem	* fuzzitem_a,
	FuzzItem	* fuzzitem_b
	);
	// Returns the number of word matches:
	// 0  = None
	// 1+ = Words matched

int fuzz_word_new (
	FuzzItem	* item,
	char		* text
	);

void fuzz_word_destroy (
	FuzzWord	* word
	);

