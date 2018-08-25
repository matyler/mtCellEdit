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



static void decode_rgb (
	uint8_t		* const	dest,
	uint8_t	const *		src,
	size_t		const	src_tot
	)
{
	size_t const tot = src_tot / 8;

	for ( size_t i = 0; i < tot; i++ )
	{
		dest[i] = (uint8_t)(
				((src[0] & 1) << 0) |
				((src[1] & 1) << 1) |
				((src[2] & 1) << 2) |
				((src[3] & 1) << 3) |
				((src[4] & 1) << 4) |
				((src[5] & 1) << 5) |
				((src[6] & 1) << 6) |
				((src[7] & 1) << 7) );
		src += 8;
	}
}

int mtDW::TapOp::decode_image (
	mtPixy::Image	* const	image,
	char	const * const	output
	)
{
	uint64_t const pixels = (uint64_t)(image->get_width () *
					image->get_height ());

	mtKit::ByteBuf buf;

	buf.array_len = (size_t)(pixels * (24 / 3));
	buf.array = (uint8_t *)calloc ( 1, buf.array_len );

	try
	{
		if ( ! buf.array )
		{
			std::cerr << "Unable to allocate buffer memory.\n";
			throw 123;
		}

		decode_rgb( buf.array, image->get_canvas(), (size_t)(pixels*3));

		if ( buf.save ( output ) )
		{
			std::cerr << "Unable to save buffer.\n";
			throw 123;
		}

		SodaFile soda;
		if ( 0 == soda.open ( output ) )
		{
			return TapFile::TYPE_RGB_1;
		}

		// Fall through, removing the temp file
	}
	catch ( ... )
	{
	}

	remove ( output );

	return TapFile::TYPE_INVALID;
}

