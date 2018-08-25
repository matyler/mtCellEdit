/*
	Copyright (C) 2018 Mark Tyler

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
	op		( new TapFileOp () )
{
}

mtDW::TapFile::~TapFile ()
{
	op->delete_soda_filename ();

	delete op;
}

int mtDW::TapFile::open_info ( char const * const filename )
{
	op->m_capacity = 0;
	op->m_image.reset ( mtPixy::Image::load ( filename ) );
	mtPixy::Image * const image = op->m_image.get ();

	if ( image )
	{
		if ( image->get_type () != mtPixy::Image::TYPE_RGB )
		{
			op->m_image.reset ( NULL );

			return TYPE_INVALID;
		}

		size_t const w = (size_t)image->get_width ();
		size_t const h = (size_t)image->get_height ();

		op->m_capacity = (w * h) * 3 / 8;

		return TYPE_RGB;
	}

	try
	{
		op->m_audio.reset ( new TapAudioRead () );
		TapAudioRead * const audio = op->m_audio.get ();

		if ( 0 == audio->open ( filename ) )
		{
			op->m_capacity =(size_t)audio->get_read_capacity ();

			return TYPE_SND;
		}
	}
	catch ( ... )
	{
	}

	op->m_audio.reset ( NULL );

	// Future additions go here

	return TYPE_INVALID;
}

int mtDW::TapFile::open_soda ( char const * const filename )
{
	std::string		tmp_file;

	get_temp_filename ( tmp_file, filename );

	switch ( open_info ( filename ) )
	{
	case TYPE_RGB:
		{
			int const res = TapOp::decode_image ( op->m_image.get(),
				tmp_file.c_str () );

			if ( TYPE_INVALID == res )
			{
				return TYPE_RGB;
			}

			op->set_soda_filename ( tmp_file );

			return res;
		}

	case TYPE_SND:
		{
			int const res = TapOp::decode_audio ( filename,
				tmp_file.c_str () );

			if ( TYPE_INVALID == res )
			{
				return TYPE_SND;
			}

			op->set_soda_filename ( tmp_file );

			return res;
		}

	// Future additions go here

	}

	return TYPE_INVALID;
}

size_t mtDW::TapFile::get_capacity () const
{
	return op->m_capacity;
}

std::string const & mtDW::TapFile::get_soda_filename () const
{
	return op->m_soda_file;
}

