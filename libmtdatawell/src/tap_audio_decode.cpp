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



static int decode_buf (
	short			*	mem,
	size_t			const	len,
	mtKit::ByteBuf		* const	buf,
	mtKit::ByteFileWrite	* const	file
	)
{
	size_t const bytes = len / 8;

	for ( size_t i = 0; i < bytes; i++ )
	{
		buf->array[ i ] = (uint8_t)(
			((mem[0] & 1) << 0) |
			((mem[1] & 1) << 1) |
			((mem[2] & 1) << 2) |
			((mem[3] & 1) << 3) |
			((mem[4] & 1) << 4) |
			((mem[5] & 1) << 5) |
			((mem[6] & 1) << 6) |
			((mem[7] & 1) << 7) );

		mem += 8;
	}

	if ( bytes > 0 )
	{
		return file->write ( buf->array, bytes );
	}

	return 0;
}

int mtDW::TapOp::decode_audio (
	char	const * const	input,
	char	const * const	output
	)
{
	if ( ! input || ! output )
	{
		return 1;
	}

	try
	{
		TapAudioRead audio_in;
		if ( audio_in.open ( input ) )
		{
			throw 123;
		}

		mtKit::ByteFileWrite file_out;
		if ( file_out.open ( output ) )
		{
			std::cerr << "Unable to open output Soda file.\n";
			throw 123;
		}

		SF_INFO const * const info = audio_in.get_info ();
		if ( ! info )
		{
			std::cerr << "Unable to read audio file info.\n";
			throw 123;
		}

		int const channels = info->channels;
		if ( channels < 1 )
		{
			std::cerr << "Too few channels in the audio file.\n";
			throw 123;
		}

		int	const	tot = channels * TapAudioRead::BUF_FRAMES;
		mtKit::ByteBuf	buf ( (size_t)tot );
		short		* audio_buf = NULL;
		size_t		audio_len = 0;

		while ( 1 )
		{
			if ( audio_in.read ( &audio_buf, &audio_len ) )
			{
				throw 123;
			}

			if ( audio_len < 1 )
			{
				// EOF
				break;
			}

			if ( decode_buf( audio_buf, audio_len, &buf, &file_out))
			{
				throw 123;
			}
		}

		file_out.close ();

		SodaFile soda;
		if ( 0 == soda.open ( output ) )
		{
			return TapFile::TYPE_SND_1;
		}

		// Fall through, removing the temp file
	}
	catch ( ... )
	{
	}

	remove ( output );

	return TapFile::TYPE_INVALID;
}

