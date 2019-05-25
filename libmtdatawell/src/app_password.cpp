/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include "mtdatawell.h"



mtDW::AppPassword::AppPassword (
	bool	const	lowercase,
	bool	const	uppercase,
	bool	const	numbers,
	std::string const &other
	)
{
	std::string chr;

	if ( uppercase )
	{
		chr += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}

	if ( lowercase )
	{
		chr += "abcdefghijklmnopqrstuvwxyz";
	}

	if ( numbers )
	{
		chr += "0123456789";
	}

	if ( other.length () > 0 )
	{
		int const len = mtkit_utf8_len (
			(unsigned char const *)other.c_str (), 0 );

		if ( len > 0 && len <= mtDW::Well::PASSWORD_OTHER_MAX )
		{
			chr += other;
		}
		// else string is empty or too long so ignore it.
	}

	if ( chr.length () < 1 )
	{
		chr += '?';
	}

	int const chr_len = mtkit_utf8_len( (unsigned char const *)chr.c_str (),
		0 );

	char const * head = chr.c_str ();

	// Carve up the UTF8 characters into the vector
	for ( int i = 0; i < chr_len; i++ )
	{
		int const ch_len = mtkit_utf8_offset (
			(unsigned char const *)head, 1 );

		std::string ch;

		mtKit::string_from_data ( ch, head, (size_t)ch_len );
		m_chr_list.push_back ( ch );

		head += ch_len;
	}
}

void mtDW::AppPassword::get_password (
	Well	const * const	well,
	int		const	char_tot,
	std::string		&output
	) const
{
	output.clear ();

	if ( ! well )
	{
		return;
	}

	int const ch_max = mtDW::Well::PASSWORD_CHAR_MAX;
	int const tot = MAX( MIN(char_tot, ch_max), 1);
	int const chr_len = (int)m_chr_list.size ();

	for ( int d = 0; d < tot; d++ )
	{
		int const num = well->get_int ( chr_len );

		output += m_chr_list[ (size_t)num ];
	}
}

