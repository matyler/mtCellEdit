/*
	Copyright (C) 2022-2023 Mark Tyler

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

#include <readline/readline.h>
#include <readline/history.h>



std::string const & mtKit::CliShell::read_line ( char const * const prompt )
{
	mtKit::CMemPtr<char> line ( ::readline ( prompt ) );

	m_old_line = m_line;

	if ( ! line )
	{
		m_finished = true;
		m_line.clear ();
	}
	else
	{
		m_line = line.ptr ();
	}

	return m_line;
}

void mtKit::CliShell::add_history ()
{
	if ( m_line != m_old_line && m_line.size() > 0 )
	{
		::add_history ( m_line.c_str() );
	}
}

void mtKit::CliShell::clear_history ()
{
	::clear_history ();

	m_line.clear ();
	m_old_line.clear ();
}

void mtKit::CliShell::bind_tab ()
{
	rl_bind_key ( '\t', rl_insert );
}

void mtKit::CliShell::bind_tilde ()
{
	rl_variable_bind ( "expand-tilde", "on" );
}

