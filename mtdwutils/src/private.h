/*
	Copyright (C) 2020 Mark Tyler

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

#include <stdlib.h>
#include <string.h>

#include <mtkit.h>
#include <mtdatawell.h>



class Global;



class Global
{
public:
	Global () :
		i_font		( 5 ),
		i_verbose	( 0 ),
		s_arg		( NULL ),
		s_bottle	( NULL ),
		s_o		( "." ),
		s_db_path	( NULL )
	{}

/// ----------------------------------------------------------------------------

	mtDW::Database	db;
	mtDW::Homoglyph	hg_index;
	mtDW::Utf8Font	font_index;
	mtKit::Exit	exit;

	int		i_font;
	int		i_verbose;

	char	const	* s_arg;	// Current command line argument
	char	const	* s_bottle;
	char	const	* s_o;
	char	const	* s_db_path;
};



// Note: must match ff_errtab in main.c
enum
{
	ERROR_BOTTLE_MISSING	= 1,
	ERROR_SODA_ENCODE	= 2,
	ERROR_SODA_DECODE	= 3,
	ERROR_BOTTLE_ENCODE	= 4,
	ERROR_BOTTLE_DECODE	= 5,
	ERROR_DB_OPEN		= 6,
	ERROR_FONT_ENCODE	= 7,
	ERROR_FONT_DECODE	= 8,
	ERROR_HG_ENCODE		= 9,
	ERROR_HG_DECODE		= 10
};



extern Global		global;



/*	Command functions

	Return 0 = success.
	Return > 0 = Generic error to be reported by caller (main.c ff_errtab).
*/

int dwut_decbot ();
int dwut_decfont ();
int dwut_dechg ();
int dwut_decsoda ();
int dwut_encbot ();
int dwut_encfont ();
int dwut_enchg ();
int dwut_encsoda ();

