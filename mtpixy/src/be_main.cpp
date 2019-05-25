/*
	Copyright (C) 2016 Mark Tyler

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
	:
	recent_image	( PREFS_FILE_RECENT_IMAGE, 20 ),
	m_ui_scale	( 0 ),
	m_ui_scale_detected ( 1 ),
	m_screenshot	( 0 ),
	m_cline_filename(),
	m_prefs_filename()
{
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

Backend::~Backend ()
{
	prefs.save ();
}

int Backend::get_ui_scale () const
{
	return m_ui_scale;
}

int Backend::get_ui_scale_palette ()
{
	return get_ui_scale_generic ( prefs.getInt ( PREFS_UI_SCALE_PALETTE ) );
}

int Backend::get_ui_scale_generic (
	int		num
	) const
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
	m_ui_scale = prefs.getInt ( PREFS_UI_SCALE );

	if ( m_ui_scale )
	{
		// User has specified a scale factor in the prefs
		m_ui_scale = MAX ( m_ui_scale, 1 );
		m_ui_scale = MIN ( m_ui_scale, 10 );
	}
	else
	{
		// Use auto-detect scale method
		m_ui_scale = m_ui_scale_detected;
	}
}

void Backend::detect_ui_scale (
	int	const	menu_height
	)
{
	m_ui_scale_detected = 1 + menu_height / 16;
	m_ui_scale_detected = MAX ( m_ui_scale_detected, 1 );
	m_ui_scale_detected = MIN ( m_ui_scale_detected, 10 );

	calc_ui_scale ();
}

void Backend::get_titlebar_text (
	char		* const	buf,
	size_t		const	buflen
	)
{
	char		* fname = NULL;


	if ( file.get_filename () )
	{
		fname = mtkit_utf8_from_cstring ( file.get_filename () );
	}

	if ( fname )
	{
		char	const	* fn;


		fn = strrchr ( fname, MTKIT_DIR_SEP );

		if ( ! fn )
		{
			fn = fname;
		}
		else
		{
			fn ++;
		}

		mtkit_strnncpy ( buf, fn, buflen );
	}
	else
	{
		mtkit_strnncpy ( buf, "Untitled", buflen );
	}

	if ( file.get_modified () )
	{
		mtkit_strnncat ( buf, " (Modified)", buflen );
	}

	mtkit_strnncat ( buf, " [", buflen );
	mtkit_strnncat ( buf, mtPixy::File::type_text ( file.get_filetype () ),
		buflen );
	mtkit_strnncat ( buf, "] ", buflen );

	if ( fname )
	{
		char	* const	tmp = strrchr ( fname, MTKIT_DIR_SEP );


		if ( tmp )
		{
			tmp[1] = 0;
		}

		mtkit_strnncat ( buf, " - ", buflen );
		mtkit_strnncat ( buf, fname, buflen );

		free ( fname );
		fname = NULL;
	}

	mtkit_strnncat ( buf, "    ", buflen );
	mtkit_strnncat ( buf, VERSION, buflen );
}

char * Backend::get_last_directory ()
{
	char const * const cf = prefs.getString ( PREFS_FILE_RECENT_IMAGE
		".001" );

	if ( ! cf )
	{
		return NULL;
	}

	char * dir = strdup ( cf );

	if ( ! dir )
	{
		return NULL;
	}

	char * const sep = strrchr ( dir, '/' );

	if ( sep )
	{
		sep[0] = 0;
	}

	return dir;
}

