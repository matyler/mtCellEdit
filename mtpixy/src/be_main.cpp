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

#include "be.h"



Backend::Backend ()
{
	pixy_palette_init ( &palette );

	std::string path;

	mtKit::get_data_dir ( path, DATA_INSTALL "/" DATA_NAME
		"/shapes/default.png" );

	if ( file.brush.load_shapes ( path.c_str () ) )
	{
		fprintf ( stderr, "Backend Error: Unable to load shapes\n" );
	}

	mtKit::get_data_dir ( path, DATA_INSTALL "/" DATA_NAME
		"/patterns/default.png" );

	if ( file.brush.load_patterns ( path.c_str () ) )
	{
		fprintf ( stderr, "Backend Error: Unable to load patterns\n" );
	}
}

int Backend::get_ui_scale () const
{
	return m_ui_scale;
}

int Backend::get_ui_scale_palette () const
{
	return get_ui_scale_generic ( mprefs.ui_scale_palette );
}

int Backend::get_ui_scale_generic ( int num ) const
{
	if ( 0 == num )
	{
		num = m_ui_scale;
	}
	else
	{
		// Enforce sanity in the scale factor
		num = MAX ( num, 1 );
		num = MIN ( num, 10 );
	}

	return num;
}

void Backend::calc_ui_scale ()
{
	m_ui_scale = mprefs.ui_scale;

	if ( m_ui_scale )
	{
		// User has specified a scale factor in the prefs
	}
	else
	{
		// Use auto-detect scale method
		m_ui_scale = m_ui_scale_detected;
	}
}

void Backend::detect_ui_scale ( int const menu_height )
{
	m_ui_scale_detected = 1 + menu_height / 16;
	m_ui_scale_detected = MAX ( m_ui_scale_detected, 1 );
	m_ui_scale_detected = MIN ( m_ui_scale_detected, 10 );

	calc_ui_scale ();
}

std::string Backend::get_titlebar_text ()
{
	char	const * const	f = file.get_filename ();
	std::string	const	fullname ( f ? f : "" );
	size_t		const	fullname_sep = fullname.rfind ( MTKIT_DIR_SEP );

	std::string txt;

	if ( fullname.size() > 0 )
	{
		if ( fullname_sep == std::string::npos )
		{
			txt = fullname;
		}
		else
		{
			txt = std::string ( fullname, fullname_sep + 1 );
		}
	}
	else
	{
		txt = "Untitled";
	}

	if ( file.get_modified () )
	{
		txt += " (Modified)";
	}

	txt += " [";
	txt += pixy_file_type_text ( file.get_filetype () );
	txt += "]";

	if ( fullname_sep != std::string::npos )
	{
		txt += " - ";
		txt += std::string ( fullname, 0, fullname_sep );
	}

	txt += "    ";
	txt += VERSION;

/*
	size_t chop = 100;

	if ( txt.size() > chop )
	{
		// Check that we aren't chopping up a UTF-8 character midway
		if ( mtkit_utf8_string_legal ( (unsigned char const *)
			txt.c_str(), txt.size() )
			)
		{
			while (1)
			{
				auto const ch = (unsigned char)txt[ chop ];

				if ( ch < 0x80 || ch > 0xBF )
				{
					break;
				}

				chop++;
			}
		}

		txt.resize ( chop );
	}
*/

	return txt;
}

std::string Backend::get_last_directory () const
{
	return mprefs.recent_image.directory ();
}

