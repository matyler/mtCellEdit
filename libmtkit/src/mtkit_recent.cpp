/*
	Copyright (C) 2016-2020 Mark Tyler

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



void mtKit::RecentFile::init (
	mtKit::UserPrefs	& prefs,
	char	const * const	prefix,
	size_t		const	items
	)
{
	if (	m_items.size() > 0
		|| items < TOTAL_MIN
		|| items > TOTAL_MAX
		|| ! prefix
		)
	{
		std::cerr << "mtKit::RecentFile::init arg error\n";
		return;		// Only initialize once and with proper args!
	}

	m_items.resize ( items );

	for ( size_t i = 0; i < items; i++ )
	{
		char buf[20];
		snprintf ( buf, sizeof(buf), ".%03i", (int)(i+1) );

		std::string key ( prefix );
		key += buf;

		prefs.add_string ( key.c_str(), m_items[i], "" );
		prefs.set_invisible ( key.c_str() );
	}
}

std::string mtKit::RecentFile::filename ( size_t const idx ) const
{
	if ( m_items.size() < idx || idx < 1 )
	{
		return "";
	}

	return m_items[ idx - 1 ];
}

std::string mtKit::RecentFile::directory ( size_t const idx ) const
{
	std::string dir = filename ( idx ) ;

	size_t const sep = dir.rfind ( '/' );

	if ( sep != std::string::npos )
	{
		dir.resize ( sep );
	}

	return dir;
}


void mtKit::RecentFile::set ( std::string const & name )
{
	if ( m_items.size() < 1 )
	{
		return;		// Nothing to do
	}

	size_t const tot = m_items.size() - 1;
		// Don't bother to check last item, it will always be lost.

	size_t i = 0;

	for ( ; i < tot; i++ )
	{
		if ( name == m_items[i] )
		{
			break;
		}
	}

	if ( 0 == i )
	{
		return;		// Nothing to do, as it's already in place
	}

	for ( ; i > 0; i-- )
	{
		m_items[i] = m_items[i - 1];
	}

	m_items[0] = name;
}

