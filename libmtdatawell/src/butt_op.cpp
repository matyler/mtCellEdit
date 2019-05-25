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

#include "butt.h"



// Global prefs
#define PREFS_BUTT_NAME		"name"



mtDW::Butt::Op::Op (
	char	const * const	path
	)
	:
	m_active_otp	( *this ),
	m_read_otp	( *this ),
	m_butt_root	( mtDW::prepare_path ( path ) + "butt" + MTKIT_DIR_SEP )
{
	mtkit_mkdir ( m_butt_root.c_str() );

	if ( m_lock.set ( m_butt_root + "butt.lock" ) )
	{
		throw 123;
	}

	// Global Prefs files --------------------------------------------------

	static mtPrefTable const prefs_table[] = {
	{ PREFS_BUTT_NAME, MTKIT_PREF_TYPE_STR, "", NULL, NULL, 0, NULL, NULL },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
	};

	m_prefs.addTable ( prefs_table );

	std::string const filename = m_butt_root + "butt.prefs";

	m_prefs.load ( filename.c_str (), NULL );

	std::string name = m_prefs.getString ( PREFS_BUTT_NAME );

	if ( name.size () < 1 || m_active_otp.set_otp ( name ) )
	{
		// First time use or failure to load previously used butt,
		// so create a new randomly named butt.

		mtKit::Random random;
		random.set_seed_by_time ();

		// Try 5 times to be sure (unlikely to have a collision)
		for ( int i = 0; i < 5; i++ )
		{
			create_otp_name ( random, name );

			if ( 0 == m_active_otp.add_otp ( name ) )
			{
				break;
			}
		}
	}
}

mtDW::Butt::Op::~Op ()
{
}

void mtDW::Butt::Op::create_otp_name (
	mtKit::Random	& random,
	std::string	& str
	)
{
	str = "";

	for ( int i = 0; i < 5; i++ )
	{
		str += (char)('a' + random.get_int ( 26 ));
	}

	str += '_';

	for ( int i = 0; i < 5; i++ )
	{
		str += (char)('0' + random.get_int ( 10 ));
	}
}

void mtDW::Butt::Op::save_state ()
{
	m_prefs.set ( PREFS_BUTT_NAME, m_active_otp.get_name ().c_str () );
	m_prefs.save ();
}

