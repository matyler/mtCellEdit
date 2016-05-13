/*
	Copyright (C) 2015 Mark Tyler

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

#include "private.h"

#include <sys/stat.h>



qexPrefs :: qexPrefs ()
	:
	prefsMem	(),
	prefsFilename	()
{
	prefsMem = mtkit_prefs_new ( NULL );
}

qexPrefs :: ~qexPrefs ()
{
	save ();
	mtkit_prefs_destroy ( prefsMem );

	free ( prefsFilename );
	prefsFilename = NULL;
}

int qexPrefs :: load (
	char	const * const	filename,
	char	const * const	bin_name
	)
{
	char		* tmp_str = NULL;


	if ( filename )
	{
		tmp_str = strdup ( filename );

		if ( ! tmp_str )
		{
			return 1;
		}
	}
	else if ( bin_name )
	{
		mtString	* str;


		str = mtkit_string_new ( mtkit_file_home () );

		if (	! str ||
			mtkit_string_append ( str, "/.config" )
			)
		{
			mtkit_string_destroy ( str );

			return 1;
		}

		mkdir ( mtkit_string_get_buf ( str ),
			S_IRWXU | S_IRWXG | S_IRWXO );

		if ( 	mtkit_string_append ( str, "/" ) ||
			mtkit_string_append ( str, bin_name )
			)
		{
			mtkit_string_destroy ( str );

			return 1;
		}

		mkdir ( mtkit_string_get_buf ( str ),
			S_IRWXU | S_IRWXG | S_IRWXO );

		if ( mtkit_string_append ( str, "/prefs.txt" ) )
		{
			mtkit_string_destroy ( str );

			return 1;
		}

		tmp_str = mtkit_string_destroy_get_buf ( str );
	}
	else
	{
		// No filename and no binary name - i.e. no prefs to load!
		return 0;
	}

	if ( mtkit_prefs_load ( prefsMem, tmp_str ) )
	{
		// Error loading prefs
		free ( tmp_str );
		tmp_str = NULL;

		return 1;
	}

	// Success so start using new filename
	free ( prefsFilename );
	prefsFilename = tmp_str;

	return 0;
}

int qexPrefs :: save ()
{
	if ( mtkit_prefs_save ( prefsMem, prefsFilename ) )
	{
		// Error saving prefs
		return 1;
	}

	return 0;
}

int qexPrefs :: addTable (
	mtPrefTable const * const	table
	)
{
	return mtkit_prefs_add ( prefsMem, table, NULL );
}

mtPrefs * qexPrefs :: getPrefsMem ()
{
	return prefsMem;
}

void qexPrefs :: set (
	char	const *	key,
	int		value
	)
{
	mtkit_prefs_set_int ( prefsMem, key, value );
}

void qexPrefs :: set (
	char	const *	key,
	double		value
	)
{
	mtkit_prefs_set_double ( prefsMem, key, value );
}

void qexPrefs :: set (
	char	const *	key,
	char	const * value
	)
{
	mtkit_prefs_set_str ( prefsMem, key, value );
}

int qexPrefs :: getInt (
	char	const	* key
	)
{
	int		res = 0;


	mtkit_prefs_get_int ( prefsMem, key, &res );

	return res;
}

double qexPrefs :: getDouble (
	char	const	* key
	)
{
	double		res = 0;


	mtkit_prefs_get_double ( prefsMem, key, &res );

	return res;
}

char const * qexPrefs :: getString (
	char	const	* key
	)
{
	static char	const	* const	nothing	= "";
	char		const	*	res	= NULL;


	mtkit_prefs_get_str ( prefsMem, key, &res );

	if ( ! res )
	{
		return nothing;
	}

	return res;
}

