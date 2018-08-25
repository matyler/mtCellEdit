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



mtDW::Tap::Tap ()
	:
	op	( new TapOp () )
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
	TapFile tap;

	tap.open_soda ( bottle_in );

	if ( 1 > tap.get_soda_filename ().size () )
	{
		std::cerr << "Error: Bottle does not contain any Soda.";
		return 1;
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
	FilenameSwap	name ( file_out );
	int		tot = 0;
	TapFile		tap;

	tap.open_soda ( bottle_in );

	if ( 1 > tap.get_soda_filename ().size () )
	{
		std::cerr << "Error: Bottle does not contain any Soda.";
		return 1;
	}

	name.m_res = mtDW::Soda::multi_decode ( butt,
		tap.get_soda_filename ().c_str (), name.f1 );

	tap.op->delete_soda_filename ();

	if ( 0 == name.m_res )
	{
		tot ++;

		while ( 1 )
		{
			tap.open_soda ( name.f1 );

			if ( 1 > tap.get_soda_filename ().size () )
			{
				// Bottle does not contain any Soda.
				// The last extracted file becomes file_out.
				name.swap ();
				break;
			}

			name.m_res = mtDW::Soda::multi_decode ( butt,
				tap.get_soda_filename ().c_str (), name.f2 );

			tap.op->delete_soda_filename ();

			name.swap ();

			if ( name.m_res )
			{
				// Error in Soda decoding
				break;
			}
		}
	}

	if ( tot > 0 )
	{
		name.m_res = 0;
	}
	else
	{
		name.m_res = 1;
	}

	return name.m_res;
}

int mtDW::Tap::encode (
	Butt		* const	butt,
	char	const * const	bottle_in,
	char	const * const	file_in,
	char	const * const	bottle_out
	) const
{
	{
		SodaFile soda;

		if ( soda.open ( file_in ) )
		{
			std::cerr << "Input file must be a Soda format.\n";
			return 1;
		}
	}

	TapFile tap;

	int const res = tap.open_info ( bottle_in );

	switch ( res )
	{
	case TapFile::TYPE_RGB:
	case TapFile::TYPE_RGB_1:
		{
			mtPixy::Image * const image = tap.op->m_image.get();

			if ( op->encode_image ( butt, image, file_in ) )
			{
				return 1;
			}

			if ( image->save_png ( bottle_out, 6 ) )
			{
				std::cerr <<
					"Unable to save output PNG bottle.\n";
				return 1;
			}

			break;
		}

	case TapFile::TYPE_SND:
	case TapFile::TYPE_SND_1:
		{
			if ( op->encode_audio ( tap.op->m_audio.get (), butt,
				file_in, bottle_out ) )
			{
				std::cerr <<
					"Unable to save output FLAC bottle.\n";
				return 1;
			}

			break;
		}

	// Future additions go here

	default:
		std::cerr << "Bottle file not valid.\n";
		return 1;
	}

	return 0;
}

