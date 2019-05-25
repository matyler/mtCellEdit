/*
	Copyright (C) 2016-2018 Mark Tyler

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



mtKit::RecentFile::RecentFile (
	char	const * const	prefix,
	int		const	tot
	)
	:
	m_total		( tot ),
	m_prefs_prefix	( strdup ( prefix ) ),
	m_prefs		()
{
}

mtKit::RecentFile::~RecentFile ()
{
	free ( m_prefs_prefix );
}

int mtKit::RecentFile::init_prefs (
	mtKit::Prefs	* const	pr
	)
{
	if ( m_prefs )
	{
		return 1;
	}

	m_prefs = pr;


	mtPrefTable	prefs_table[] = {
		{ "", MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },
		{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
		};


	for ( int i = 1; i <= m_total; i++ )
	{
		char * key = create_key ( i );

		prefs_table[0].key = key;
		m_prefs->addTable ( prefs_table );

		free ( key );
		key = NULL;
	}

	return 0;
}

char * mtKit::RecentFile::create_key (
	int	const	idx
	) const
{
	if ( idx < 1 || idx > m_total )
	{
		return NULL;
	}


	char cbuf[20];

	snprintf ( cbuf, sizeof(cbuf), ".%03i", idx );

	return mtkit_string_join ( m_prefs_prefix, cbuf, NULL, NULL );
}

char const * mtKit::RecentFile::get_filename (
	int	const	idx
	) const
{
	char		* const	key = create_key ( idx );
	char	const * const	val = m_prefs->getString ( key );

	free ( key );

	return val;
}

void mtKit::RecentFile::set_filename (
	char	const * const	name
	) const
{
	if ( ! name )
	{
		return;
	}

	int i;

	// Search for a current use of this filename
	for ( i = 1; i <= m_total; i++ )
	{
		char * key = create_key ( i );

		if ( ! key )
		{
			return;
		}

		char const * const val = m_prefs->getString ( key );

		free ( key );
		key = NULL;

		if ( ! val )
		{
			return;
		}

		if ( 0 == strcmp ( val, name ) )
		{
			break;
		}
	}


	// We need to duplicate 'name' now as it may be released from prefs
	char * dnam = strdup ( name );
	if ( ! dnam )
	{
		return;
	}

	// Shift items to accommodate the new most recent item
	for ( i = i - 1; i >= 1; i-- )
	{
		char * key = create_key ( i );
		if ( ! key )
		{
			goto finish;
		}

		char const * const val = m_prefs->getString ( key );

		free ( key );
		key = NULL;

		if ( ! val )
		{
			goto finish;
		}

		set_filename_idx ( i + 1, val );
	}

	set_filename_idx ( 1, dnam );

finish:
	free ( dnam );
}

void mtKit::RecentFile::set_filename_idx (
	int		const	idx,
	char	const * const	name
	) const
{
	char * key = create_key ( idx );

	if ( ! key )
	{
		return;
	}

	m_prefs->set ( key, name );
	free ( key );
	key = NULL;
}

