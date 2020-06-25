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

#include "private.h"



mtKit::LineFileRead::LineFileRead ()
	:
	m_fp		( NULL ),
	m_text		( NULL ),
	m_field		( NULL )
{
}

mtKit::LineFileRead::~LineFileRead ()
{
	close ();
}

void mtKit::LineFileRead::close ()
{
	if ( m_fp )
	{
		fclose ( m_fp );
		m_fp = NULL;
	}

	free ( m_text );
	m_text = NULL;

	m_field = NULL;
}

int mtKit::LineFileRead::open ( std::string const & filename )
{
	FILE * fp = fopen ( filename.c_str (), "r" );

	if ( ! fp )
	{
		std::cerr << "Unable to open file '" << filename << "'\n";
		return 1;
	}

	close ();

	m_fp = fp;

	return 0;
}

void mtKit::LineFileRead::open ( FILE * const fp )
{
	close ();

	m_fp = fp;
}

int mtKit::LineFileRead::read_line ()
{
	if ( ! m_fp )
	{
		return 2;
	}

	free ( m_text );
	m_text = mtkit_file_readline ( m_fp, NULL, NULL );
	m_field = m_text;

	if ( ! m_text )
	{
		return 1;
	}

	return 0;
}

int mtKit::LineFileRead::get_double ( double & result )
{
	if ( ! m_field )
	{
		return 1;
	}

	char * next;

	if ( mtkit_strtod ( m_field, &result, &next, 0 ) )
	{
		std::cerr << "Error parsing double " << m_field << "\n";
		return 1;
	}

	m_field = next;

	return 0;
}

int mtKit::LineFileRead::get_int ( int & result )
{
	if ( ! m_field )
	{
		return 1;
	}

	char * next;

	if ( mtkit_strtoi ( m_field, &result, &next, 0 ) )
	{
		std::cerr << "Error parsing int " << m_field << "\n";
		return 1;
	}

	m_field = next;

	return 0;
}

