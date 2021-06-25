/*
	Copyright (C) 2017-2020 Mark Tyler

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

#include "ced_fuzzmap.h"



int Global::ced_fuzzmap ()
{
	if ( ! m_dict_filename )
	{
		m_dict_filename = s_arg;
		memcpy ( m_dict_range, i_range, sizeof(m_dict_range) );

		return 0;
	}

	// NOTE: inefficient to keep creating fuzz_dict for multiple input files
	FuzzFile * fuzz_dict = fuzz_file_new ( m_dict_filename, m_dict_range,
		i_csv );
	FuzzFile * fuzz_in = fuzz_file_new ( s_arg, i_range, i_csv );
	int const res = fuzz_file_match ( fuzz_dict, fuzz_in );

	if ( 0 == res )
	{
		set_sheet ( fuzz_file_steal_sheet ( fuzz_in ) );
	}

	fuzz_file_destroy ( fuzz_dict );
	fuzz_dict = NULL;

	fuzz_file_destroy ( fuzz_in );
	fuzz_in = NULL;

	return res;
}



static int init_scan_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	FuzzFile * const fuzzfile = static_cast<FuzzFile *>(user_data);

	if ( cell->text && fuzz_item_new ( fuzzfile, cell ) )
	{
		return 1;	// stop
	}

	return 0;		// continue
}

static int fuzz_file_init (
	FuzzFile	* const	fuzzfile,
	int	const * const	range
	)
{
	if ( ced_sheet_scan_area ( fuzzfile->sheet,
		range[0], range[2],
		range[1] - range[0] + 1, range[3] - range[2] + 1, init_scan_cb,
		fuzzfile ) )
	{
		fprintf ( stderr, "Error: fuzz_file_init\n" );
		return 1;
	}

	// Ensure that each of these items is unique according to the fuzzer:
	// any equal fuzz matches *MUST* be identical strings to avoid making
	// mistakes when substituting later.
	int res = 0;
	FuzzItem * a = fuzzfile->item_first;

	for ( ; a; a = a->next )
	{
		FuzzItem * b = fuzzfile->item_first;

		for ( ; b; b = b->next )
		{
			if (	a == b			||
				a->cell == b->cell	||
				0 == strcmp ( a->cell->text, b->cell->text )
				)
			{
				// Ignore literal string duplicates as they
				// don't cause ambiguity problems. e.g. same
				// candidate name in different seats.
				continue;
			}

			int const wm = fuzz_item_cmp ( a, b );

			if ( wm == a->word_tot && wm == b->word_tot )
			{
				// This makes a list invalid if 2 strings are
				// different, but fuzzing loses differences, e.g
				// 'foo, BAR' <=> 'bar, Foo'
				// This strictness avoids ambiguities when
				// replacing later.

				fprintf ( stderr, "Error: '%s' <=> '%s'\n",
					a->cell->text, b->cell->text );

				res = 1;
			}
		}
	}

	return res;
}

FuzzFile * fuzz_file_new (
	char	const * const	filename,
	int	const * const	range,
	int		const	csv
	)
{
	if ( ! filename || ! range )
	{
		return NULL;
	}

	auto const fuzzfile = static_cast<FuzzFile *>(calloc ( 1,
		sizeof(FuzzFile) ) );

	if ( ! fuzzfile )
	{
		goto error;
	}

	if ( csv )
	{
		fuzzfile->sheet = ced_sheet_load_csv ( filename, "ISO-8859-1" );
	}
	else
	{
		fuzzfile->sheet = ced_sheet_load( filename, "ISO-8859-1", NULL);
	}

	if ( ! fuzzfile->sheet )
	{
		goto error;
	}

	if ( fuzz_file_init ( fuzzfile, range ) )
	{
		goto error;
	}

	return fuzzfile;

error:
	fprintf ( stderr, "Error: unable to load '%s'\n", filename );

	fuzz_file_destroy ( fuzzfile );
//	fuzzfile = NULL;

	return NULL;
}

void fuzz_file_destroy (
	FuzzFile	* const	fuzz
	)
{
	if ( fuzz )
	{
		ced_sheet_destroy ( fuzz->sheet );
		fuzz->sheet = NULL;

		FuzzItem * i = fuzz->item_first;

		for ( ; i; )
		{
			FuzzItem * const ni = i->next;

			fuzz_item_destroy ( i );

			i = ni;
		}

		free ( fuzz );
	}
}

int fuzz_file_match (
	FuzzFile	* const	fuzz_dict,
	FuzzFile	* const	fuzz_in
	)
{
	if ( ! fuzz_dict || ! fuzz_in )
	{
		return 1;
	}

	FuzzItem * fi = fuzz_in->item_first;

	for ( ; fi; fi = fi->next )
	{
		// Dictionary pass 1 - look for perfect matches

		FuzzItem * di = fuzz_dict->item_first;

		for ( ; di; di = di->next )
		{
			int const wm = fuzz_item_cmp ( fi, di );

			if ( wm == fi->word_tot && wm == di->word_tot )
			{
				// Perfect match in both directions!
				// Put the dictionary text into the input file.
				free ( fi->cell->text );
				fi->cell->text = strdup ( di->cell->text );

				goto next_fi;
			}
		}

		// Dictionary pass 2 - look for partial matches -> stdout

		printf ( "%s", fi->cell->text );

		di = fuzz_dict->item_first;

		for ( ; di; di = di->next )
		{
			int const wm = fuzz_item_cmp ( fi, di );

			if ( wm > 0 )
			{
				printf ( "\t%s", di->cell->text );
			}
		}

		puts ( "" );

next_fi:
		continue;
	}

	return 0;
}

CedSheet * fuzz_file_steal_sheet (
	FuzzFile	* const	fuzz
	)
{
	if ( ! fuzz )
	{
		return NULL;
	}

	CedSheet * const ts = fuzz->sheet;

	fuzz->sheet = NULL;

	return ts;
}



/// -------------------------- FuzzItem ----------------------------------------



int fuzz_item_new (
	FuzzFile	* const file,
	CedCell		* const cell
	)
{
	if ( ! file || ! cell || ! cell->text )
	{
		return 1;
	}

	FuzzItem * item = static_cast<FuzzItem *>(calloc (1, sizeof(FuzzItem)));
	if ( ! item )
	{
		return 1;
	}

	item->cell = cell;

	char * text = strdup ( cell->text );
	if ( ! text )
	{
		fuzz_item_destroy ( item );
		return 1;
	}

	char * c;
	for ( c = text; 0 != *c; )
	{
		// Any errors here are quietly ignored (treat as single ASCII)
		int const len = MIN ( 1, mtkit_utf8_offset ( (unsigned char *)c,
			1 ) );

		if ( 1 == len )
		{
			// Only adjust raw ASCII

			if ( isalpha ( c[0] ) )
			{
				c[0] = (char)tolower ( c[0] );
			}
			else
			{
				// Remove non-alpha characters
				c[0] = ' ';
			}
		}

		c += len;
	}

	int t;
	for ( t = 0; ; t++ )
	{
		char * ns = mtkit_strtok ( text, " ", t );

		if ( ! ns )
		{
			break;
		}

		if ( fuzz_word_new ( item, ns ) )
		{
			free ( ns );
			ns = NULL;

			fuzz_item_destroy ( item );

			free ( text );
			text = NULL;

			return 1;
		}

		free ( ns );
		ns = NULL;
	}

	free ( text );
	text = NULL;

	if ( file->item_last )
	{
		file->item_last->next = item;
		file->item_last = item;
	}
	else
	{
		file->item_first = item;
		file->item_last = item;
	}

	return 0;
}

void fuzz_item_destroy (
	FuzzItem	* const item
	)
{
	if ( ! item )
	{
		return;
	}

	FuzzWord * w = item->word_first;

	for ( ; w; )
	{
		FuzzWord * const nw = w->next;

		fuzz_word_destroy ( w );

		w = nw;
	}

	free ( item );
}



#define WORD_MAX 16



int fuzz_item_cmp (
	FuzzItem	* const	fuzzitem_a,
	FuzzItem	* const	fuzzitem_b
	)
{
	if ( ! fuzzitem_a || ! fuzzitem_b )
	{
		return 0;
	}

	int		matched = 0;
	int		bmat[WORD_MAX] = {0};

	FuzzWord * fwa = fuzzitem_a->word_first;

	for ( ; fwa; fwa = fwa->next )
	{
		int i;
		FuzzWord * fwb = fuzzitem_b->word_first;

		for ( i = 0; fwb; fwb = fwb->next, i++ )
		{
			if ( bmat[i] )
			{
				// This word has already been matched. Check to
				// avoid:
				// 'West Bromwich West' <=> 'West Bromwich East'
				// i.e. counting a single word twice.
				continue;
			}

			if ( 0 == strcmp ( fwa->text, fwb->text ) )
			{
				bmat[i] = 1;
				matched++;
				goto next_fwa;
			}
		}
next_fwa:
		continue;
	}

	return matched;
}



/// -------------------------- FuzzWord ----------------------------------------



int fuzz_word_new (
	FuzzItem	* const	item,
	char		* const	text
	)
{
	if ( ! item || ! text || item->word_tot >= WORD_MAX )
	{
		return 1;
	}

	if (	0 == strcmp ( text, "the" )	||
		0 == strcmp ( text, "and" )	||
		0 == strcmp ( text, "of" )	||
		0 == strcmp ( text, "upon" )	||
		! isalpha ( text[0] )
		)
	{
		// Quietly ignore these words, or anything starting with " "
		return 0;
	}

	auto word = static_cast<FuzzWord *>(calloc ( 1, sizeof(FuzzWord) ));
	if ( ! word )
	{
		return 1;
	}

	word->text = strdup ( text );
	if ( ! word->text )
	{
		fuzz_word_destroy ( word );
		word = NULL;

		return 1;
	}

	// Another word has been successfully added
	item->word_tot ++;

	if ( item->word_last )
	{
		item->word_last->next = word;
		item->word_last = word;
	}
	else
	{
		item->word_first = word;
		item->word_last = word;
	}

	return 0;
}

void fuzz_word_destroy (
	FuzzWord	* const	word
	)
{
	if ( word )
	{
		free ( word->text );
		free ( word );
	}
}

