/*
	Copyright (C) 2018-2020 Mark Tyler

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

int mtDW::Tap::Op::encode_image (
	Well		* const	well,
	mtPixmap const * const	image,
	char	const * const	input
	)
{
	if ( ! image || ! input )
	{
		return report_error ( ERROR_IMAGE_ENCODE_INSANITY );
	}

	int tot;
	char * mem = mtkit_file_load ( input, &tot, 0, 0 );

	if ( ! mem )
	{
		return report_error ( ERROR_LOAD_INPUT );
	}

	ByteBuf buf;

	buf.set ( (uint8_t *)mem, (size_t)tot );

	uint64_t const pixels = (uint64_t)(pixy_pixmap_get_width (image) *
					pixy_pixmap_get_height (image));
	uint64_t const done = buf.get_size () * (24 / 3);
	uint64_t const todo = pixels * 3 - done;

	if ( done > (pixels * 3) )
	{
		return report_error ( ERROR_IMAGE_TOO_SMALL );
	}

	encode_rgb ( pixy_pixmap_get_canvas (image), buf.get_buf (), tot );

	if ( todo < 1 || ! well )
	{
		return 0;
	}

	uint64_t const major = todo / 8;
	uint64_t const minor = todo % 8;
	uint64_t const bytes = major + MIN ( minor, 1 );

	buf.set ( (uint8_t *)malloc ( (size_t)bytes ), (size_t)bytes );

	if ( ! buf.get_buf () )
	{
		return report_error ( ERROR_HEAP_EMPTY );
	}

	well->get_data ( buf.get_buf (), (size_t)bytes );

	uint8_t * dst = pixy_pixmap_get_canvas (image) + done;
	uint8_t const * const src = buf.get_buf ();

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

