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

#include "tap.h"



mtDW::TapFile::TapFile ()
	:
	op		( new TapFile::Op () )
{
}

mtDW::TapFile::~TapFile ()
{
	op->delete_soda_filename ();

	delete op;
}

int mtDW::TapFile::open_info (
	char	const * const	filename,
	int			& type
	)
{
	op->m_capacity = 0;
	op->m_image.reset ( mtPixy::Image::load ( filename ) );
	mtPixy::Image * const image = op->m_image.get ();

	if ( image )
	{
		if ( image->get_type () != mtPixy::Image::TYPE_RGB )
		{
			op->m_image.reset ( NULL );

			return report_error ( ERROR_IMAGE_INVALID_BOTTLE );
		}

		size_t const w = (size_t)image->get_width ();
		size_t const h = (size_t)image->get_height ();

		op->m_capacity = (w * h) * 3 / 8;

		type = TYPE_RGB;
		return 0;
	}

	try
	{
		op->m_audio.reset ( new TapAudioRead () );
		TapAudioRead * const audio = op->m_audio.get ();

		if ( 0 == audio->open ( filename ) )
		{
			op->m_capacity =(size_t)audio->get_read_capacity ();

			type = TYPE_SND;
			return 0;
		}
	}
	catch ( ... )
	{
	}

	op->m_audio.reset ( NULL );

	// Future additions go here

	return report_error ( ERROR_TAP_UNKNOWN_BOTTLE );
}

int mtDW::TapFile::open_soda (
	char	const * const	filename,
	int			& type
	)
{
	if ( ! filename )
	{
		return report_error ( ERROR_TAP_OPEN_SODA_INSANITY );
	}

	std::string		tmp_file;

	mtDW::get_temp_filename ( tmp_file, filename );

	RETURN_ON_ERROR ( open_info ( filename, type ) )

	switch ( type )
	{
	case TYPE_RGB:
		{
			RETURN_ON_ERROR( Tap::Op::decode_image (
				op->m_image.get (), tmp_file.c_str (), type ) )

			if ( TYPE_RGB == type )
			{
				// Empty bottle (no Soda)
				return 0;
			}

			op->set_soda_filename ( tmp_file );

			return 0;
		}

	case TYPE_SND:
		{
			RETURN_ON_ERROR ( Tap::Op::decode_audio ( filename,
				tmp_file.c_str (), type ) )

			if ( TYPE_SND == type )
			{
				// Empty bottle (no Soda)
				return 0;
			}

			op->set_soda_filename ( tmp_file );

			return 0;
		}

	// Future additions go here

	}

	// Shouldn't ever get here if open_info() type matches switch above
	return report_error ( ERROR_TAP_UNKNOWN_BOTTLE );
}

size_t mtDW::TapFile::get_capacity () const
{
	return op->m_capacity;
}

std::string const & mtDW::TapFile::get_soda_filename () const
{
	return op->m_soda_file;
}

