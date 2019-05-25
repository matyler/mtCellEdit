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



mtDW::Tap::Tap ()
	:
	op	( new Tap::Op () )
{
}

mtDW::Tap::~Tap ()
{
	delete op;
}

int mtDW::Tap::decode (
	Butt		* const	butt,
	char	const * const	bottle_in,
	char	const * const	file_out
	)
{
	if ( ! bottle_in || ! file_out )
	{
		return report_error ( ERROR_TAP_DECODE_INSANITY );
	}

	TapFile	tap;
	int	type;

	RETURN_ON_ERROR ( tap.open_soda ( bottle_in, type ) )

	if ( tap.get_soda_filename ().size () < 1 )
	{
		return report_error ( ERROR_TAP_BOTTLE_INVALID );
	}

	return mtDW::Soda::multi_decode ( butt,
		tap.get_soda_filename ().c_str (), file_out );
}

int mtDW::Tap::multi_decode (
	Butt		* const	butt,
	char	const * const	bottle_in,
	char	const * const	file_out
	)
{
	if ( ! bottle_in || ! file_out )
	{
		return report_error ( ERROR_TAP_DECODE_INSANITY );
	}

	FilenameSwap	name ( file_out );

	name.m_res = Tap::decode ( butt, bottle_in, file_out );
	if ( name.m_res )
	{
		// We must decode at least once (and remove rogue output temp)
		return name.m_res;
	}

	// Some errors are not really errors in this context
	mtDW::set_stderr_less ();

	do
	{
		name.m_res = Tap::decode ( butt, name.f1, name.f2 );
		name.swap ();

	} while ( 0 == name.m_res );

	mtDW::set_stderr_more ();

	// Certain failures are allowed, i.e. we have extracted the input file
	// which will NOT have a valid Soda header or be another USED bottle.
	switch ( name.m_res )
	{
	case ERROR_TAP_BOTTLE_INVALID:	// Original file is PNG/FLAC
	case ERROR_TAP_UNKNOWN_BOTTLE:	// Original file is NOT PNG/FLAC
		name.m_res = 0;		// Keep output file
		return 0;
	}

//	Report any error as it was suppressed earlier on
	return report_error ( name.m_res );
}

int mtDW::Tap::encode (
	Well		* const	well,
	Butt		* const	butt,
	Soda		* const	soda,
	char	const * const	bottle_in,
	char	const * const	file_in,
	char	const * const	bottle_out
	) const
{
	if ( ! soda || ! bottle_in || ! file_in || ! bottle_out )
	{
		return report_error ( ERROR_TAP_ENCODE_INSANITY );
	}

	TapFile		tap;
	std::string	temp_file;

	mtDW::get_temp_filename ( temp_file, bottle_out );

	// Get tap to delete this file after use
	tap.op->set_soda_filename ( temp_file );

	RETURN_ON_ERROR ( soda->encode ( butt, file_in, temp_file.c_str () ) )

	int type;

	RETURN_ON_ERROR ( tap.open_info ( bottle_in, type ) );

	WellSaveState wss ( well );

	switch ( type )
	{
	case TapFile::TYPE_RGB:
	case TapFile::TYPE_RGB_1:
		{
			mtPixy::Image * const image = tap.op->m_image.get();

			RETURN_ON_ERROR ( op->encode_image ( well, image,
				temp_file.c_str () ) )

			if ( image->save_png ( bottle_out, 6 ) )
			{
				return report_error (ERROR_TAP_ENCODE_SAVE_PNG);
			}

			break;
		}

	case TapFile::TYPE_SND:
	case TapFile::TYPE_SND_1:
		{
			RETURN_ON_ERROR ( op->encode_audio (
				tap.op->m_audio.get (), well, temp_file.c_str(),
				bottle_out ) )

			break;
		}

	// Future additions go here

	default:
		return report_error ( ERROR_TAP_ENCODE_BAD_BOTTLE );
	}

	return 0;
}

