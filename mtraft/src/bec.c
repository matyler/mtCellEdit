/*
	Copyright (C) 2011-2015 Mark Tyler

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
#include "bec.h"



static mtPrefs		* prefs_mem;
static char		prefs_filename [ 2048 ];



static int prefs_add_table (
	mtPrefs		* const prefs
	)
{
	mtPrefTable	const	prefs_table[] =
	{
	{ PREFS_WINDOW_X, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_Y, MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_W, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ PREFS_WINDOW_H, MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
	{ NULL }
	};


	return mtkit_prefs_add ( prefs, prefs_table, NULL );
}

void prefs_init ( void )
{
	prefs_mem = mtkit_prefs_new ( NULL );
	prefs_add_table ( prefs_mem );

	snprintf ( prefs_filename, sizeof ( prefs_filename ),
		"%s/.config", mtkit_file_home () );

	mkdir ( prefs_filename, S_IRWXU | S_IRWXG | S_IRWXO );

	mtkit_strnncat ( prefs_filename, "/" BIN_NAME,
		sizeof ( prefs_filename ) );

	mkdir ( prefs_filename, S_IRWXU | S_IRWXG | S_IRWXO );

	mtkit_strnncat ( prefs_filename, "/prefs.txt",
		sizeof ( prefs_filename ) );
}

void prefs_load ( void )
{
	mtkit_prefs_load ( prefs_mem, prefs_filename );
}

void prefs_save ( void )
{
	mtkit_prefs_save ( prefs_mem, prefs_filename );
}

void prefs_close ( void )
{
	mtkit_prefs_destroy ( prefs_mem );
	prefs_mem = NULL;
}

void prefs_set_int (
	char	const	* key,
	int		value
	)
{
	mtkit_prefs_set_int ( prefs_mem, key, value );
}

int prefs_get_int (
	char	const	* key
	)
{
	int		res = 0;


	mtkit_prefs_get_int ( prefs_mem, key, &res );

	return res;
}

