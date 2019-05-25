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



static int decode_buf (
	mtKit::ByteFileWrite	&file,
	short	const * const	src_mem,
	size_t		const	src_len
	)
{
	uint8_t		buf[8192];
	size_t	const	buflen = sizeof(buf);
	short	const	* src = src_mem;

	for ( size_t j = 0; j < src_len; )
	{
		size_t	const	remaining = src_len - j;
		size_t	const	tot = MIN ( buflen, remaining / 8 );

		if ( tot < 1 )
		{
			break;
		}

		for ( size_t i = 0; i < tot; i++ )
		{
			buf[ i ] = (uint8_t)(
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

		if ( file.write ( buf, tot ) )
		{
			return 1;
		}

		j += tot * 8;
	}

	return 0;
}

int mtDW::Tap::Op::decode_audio (
	char	const * const	input,
	char	const * const	output,
	int			& type
	)
{
	if ( ! input || ! output )
	{
		return report_error ( ERROR_AUDIO_DECODE_INSANITY );
	}

	try
	{
		TapAudioRead audio_in;

		RETURN_ON_ERROR ( audio_in.open ( input ) )

		SF_INFO const * const info = audio_in.get_info ();

		int const channels = info->channels;
		if ( channels < 1 )
		{
			return report_error ( ERROR_AUDIO_BAD_CHANNELS );
		}

		mtKit::ByteFileWrite file_out;
		if ( file_out.open ( output ) )
		{
			return report_error ( ERROR_SODA_OPEN_OUTPUT );
		}

		short		* audio_buf = NULL;
		size_t		audio_len = 0;

		while ( 1 )
		{
			int const roe = audio_in.read ( &audio_buf, &audio_len);
			if ( roe )
			{
				remove ( output );

				return roe;
			}

			if ( audio_len < 1 )
			{
				// EOF
				break;
			}

			if ( decode_buf ( file_out, audio_buf, audio_len ) )
			{
				remove ( output );

				return report_error ( ERROR_AUDIO_WRITE );
			}
		}

		SodaFile soda;
		if ( 0 == soda.open ( output ) )
		{
			type = TapFile::TYPE_SND_1;
			return 0;
		}

		// Not a Soda file so remove temp file
		remove ( output );

		type = TapFile::TYPE_SND;
		return 0;
	}
	catch ( ... )
	{
		remove ( output );
	}

	return report_error ( ERROR_AUDIO_DECODE_EXCEPTION );
}

