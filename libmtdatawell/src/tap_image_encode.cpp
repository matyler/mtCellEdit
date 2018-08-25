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



static void encode_rgb (
	uint8_t		*	dst,
	uint8_t	const * const	src,
	int		const	tot
	)
{
	for ( int i = 0; i < tot; i++ )
	{
		dst[0] = (uint8_t)((dst[0] & 0xFE) | ((src[i]>>0) & 1));
		dst[1] = (uint8_t)((dst[1] & 0xFE) | ((src[i]>>1) & 1));
		dst[2] = (uint8_t)((dst[2] & 0xFE) | ((src[i]>>2) & 1));
		dst[3] = (uint8_t)((dst[3] & 0xFE) | ((src[i]>>3) & 1));
		dst[4] = (uint8_t)((dst[4] & 0xFE) | ((src[i]>>4) & 1));
		dst[5] = (uint8_t)((dst[5] & 0xFE) | ((src[i]>>5) & 1));
		dst[6] = (uint8_t)((dst[6] & 0xFE) | ((src[i]>>6) & 1));
		dst[7] = (uint8_t)((dst[7] & 0xFE) | ((src[i]>>7) & 1));

		dst += 8;
	}
}

int mtDW::TapOp::encode_image (
	Butt		* const	butt,
	mtPixy::Image	* const	image,
	char	const * const	input
	)
{
	if ( ! image || ! input )
	{
		return 1;
	}

	int tot;
	char * mem = mtkit_file_load ( input, &tot, 0, 0 );

	if ( ! mem )
	{
		std::cerr << "Unable to load flavour file.\n";
		return 1;
	}

	mtKit::ByteBuf buf;
	buf.array = (uint8_t *)mem;
	buf.array_len = (uint32_t)tot;

	uint64_t const pixels = (uint64_t)(image->get_width () *
					image->get_height ());
	uint64_t const done = buf.array_len * (24 / 3);
	uint64_t const todo = pixels * 3 - done;

	if ( done > (pixels * 3) )
	{
		std::cerr << "Bottle is too small.\n";
		return 1;
	}

	encode_rgb ( image->get_canvas (), buf.array, tot );

	// This stops Twitter mangling a PNG into a JPEG
	if ( ! image->get_alpha () )
	{
		image->create_alpha ();
		unsigned char * const alpha = image->get_alpha ();

		if ( alpha )
		{
			memset ( alpha, 255, (size_t)pixels );

			alpha[0] ^= 1;
		}
	}

	if ( todo < 1 || ! butt )
	{
		return 0;
	}

	uint64_t const major = todo / 8;
	uint64_t const minor = todo % 8;
	uint64_t const bytes = major + MIN ( minor, 1 );

	free ( buf.array );
	buf.array = (uint8_t *)malloc ( (size_t)bytes );
	if ( ! buf.array )
	{
		// No memory so can't fill
		return 0;
	}

	if ( butt->otp_get_data ( buf.array, (size_t)bytes ) )
	{
		// No OTP data so can't fill
		return 0;
	}

	uint8_t * dst = image->get_canvas () + done;
	uint8_t const * const src = buf.array;

	if ( major > 0 )
	{
		// Major bytes at the beginning - lumps of 8

		for ( size_t i = 0; i < major; i++ )
		{
			dst[0] = (uint8_t)((dst[0] & 0xFE) | ((src[i]>>0) & 1));
			dst[1] = (uint8_t)((dst[1] & 0xFE) | ((src[i]>>1) & 1));
			dst[2] = (uint8_t)((dst[2] & 0xFE) | ((src[i]>>2) & 1));
			dst[3] = (uint8_t)((dst[3] & 0xFE) | ((src[i]>>3) & 1));
			dst[4] = (uint8_t)((dst[4] & 0xFE) | ((src[i]>>4) & 1));
			dst[5] = (uint8_t)((dst[5] & 0xFE) | ((src[i]>>5) & 1));
			dst[6] = (uint8_t)((dst[6] & 0xFE) | ((src[i]>>6) & 1));
			dst[7] = (uint8_t)((dst[7] & 0xFE) | ((src[i]>>7) & 1));

			dst += 8;
		}
	}

	if ( minor > 0 )
	{
		// Minor bytes at the end

		int b = src[ major ];

		for ( size_t i = 0; i < minor; i++ )
		{
			dst[i] = (uint8_t)( (dst[i] & 0xFE) | (b & 1) );

			b = b >> 1;
		}
	}

	return 0;
}

