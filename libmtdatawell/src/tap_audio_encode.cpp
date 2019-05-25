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



static size_t encode_with_input (
	short		*	mem,
	size_t		const	memlen,
	mtDW::ByteBuf	* const	buf
	)
{
	size_t const buf_todo = buf->get_size () - buf->get_pos ();
	size_t const bytes = MIN ( memlen / 8, buf_todo );
	uint8_t const * const src = buf->get_buf () + buf->get_pos ();

	for ( size_t i = 0; i < bytes; i++ )
	{
		mem[0] = (short)( (mem[0] & 0xFFFE) | ((src[i] >> 0) & 1) );
		mem[1] = (short)( (mem[1] & 0xFFFE) | ((src[i] >> 1) & 1) );
		mem[2] = (short)( (mem[2] & 0xFFFE) | ((src[i] >> 2) & 1) );
		mem[3] = (short)( (mem[3] & 0xFFFE) | ((src[i] >> 3) & 1) );
		mem[4] = (short)( (mem[4] & 0xFFFE) | ((src[i] >> 4) & 1) );
		mem[5] = (short)( (mem[5] & 0xFFFE) | ((src[i] >> 5) & 1) );
		mem[6] = (short)( (mem[6] & 0xFFFE) | ((src[i] >> 6) & 1) );
		mem[7] = (short)( (mem[7] & 0xFFFE) | ((src[i] >> 7) & 1) );

		mem += 8;
	}

	buf->set_pos ( buf->get_pos () + bytes );

	return bytes;
}

static void encode_with_well (
	mtDW::Well	* const	well,
	short		*	dst,
	size_t		const	memlen,
	mtDW::ByteBuf	* const	buf
	)
{
	if ( ! well || ! buf->get_buf () )
	{
		return;
	}

	// Reduce usage of Well data as much as possible
	uint64_t const max_bytes = MIN ( memlen, buf->get_size () );
	uint64_t const major = max_bytes / 8;
	uint64_t const minor = max_bytes % 8;
	uint64_t const bytes = major + MIN ( minor, 1 );

	well->get_data ( buf->get_buf (), (size_t)bytes );

	uint8_t const * const src = buf->get_buf ();

	if ( major > 0 )
	{
		// Major bytes at the beginning - lumps of 8

		for ( size_t i = 0; i < major; i++ )
		{
			dst[0] = (short)((dst[0] & 0xFFFE) | ((src[i]>>0) & 1));
			dst[1] = (short)((dst[1] & 0xFFFE) | ((src[i]>>1) & 1));
			dst[2] = (short)((dst[2] & 0xFFFE) | ((src[i]>>2) & 1));
			dst[3] = (short)((dst[3] & 0xFFFE) | ((src[i]>>3) & 1));
			dst[4] = (short)((dst[4] & 0xFFFE) | ((src[i]>>4) & 1));
			dst[5] = (short)((dst[5] & 0xFFFE) | ((src[i]>>5) & 1));
			dst[6] = (short)((dst[6] & 0xFFFE) | ((src[i]>>6) & 1));
			dst[7] = (short)((dst[7] & 0xFFFE) | ((src[i]>>7) & 1));

			dst += 8;
		}
	}

	if ( minor > 0 )
	{
		// Minor bytes at the end

		int b = src[ major ];

		for ( size_t i = 0; i < minor; i++ )
		{
			dst[i] = (short)( (dst[i] & 0xFFFE) | (b & 1) );

			b = b >> 1;
		}
	}
}

int mtDW::Tap::Op::encode_audio (
	TapAudioRead	* const	audio_in,
	Well		* const	well,
	char	const * const	input,
	char	const * const	output
	)
{
	if ( ! audio_in || ! input || ! output )
	{
		return report_error ( ERROR_AUDIO_ENCODE_INSANITY );
	}

	SF_INFO const * audio_info = audio_in->get_info ();
	if ( ! audio_info )
	{
		return report_error ( ERROR_AUDIO_OPEN_INPUT );
	}

	if ( audio_info->channels < 1 )
	{
		return report_error ( ERROR_AUDIO_BAD_CHANNELS );
	}

	size_t const channels = (size_t)audio_info->channels;

	int tot;
	char * mem = mtkit_file_load ( input, &tot, 0, 0 );

	if ( ! mem )
	{
		return report_error ( ERROR_LOAD_INPUT );
	}

	ByteBuf buf;

	buf.set ( (uint8_t *)mem, (size_t)tot );

	uint64_t todo = (uint64_t)tot;

	TapAudioWrite audio_out;

	RETURN_ON_ERROR ( audio_out.open ( audio_in->get_info (), output ) )

	short * audio_buf = NULL;
	size_t audio_len = 0;
	size_t audio_done = 0;

	while ( todo > 0 )
	{
		RETURN_ON_ERROR ( audio_in->read ( &audio_buf, &audio_len ) )

		if ( audio_len < 8 )
		{
			return report_error ( ERROR_AUDIO_TOO_SMALL );
		}

		if ( (8 * todo) < audio_len )
		{
			// Keep on the (channels * frames) boundary
			audio_done = (size_t)(8*todo + ((8 * todo) % channels));
		}
		else
		{
			audio_done = audio_len;
		}

		size_t const done = encode_with_input ( audio_buf, audio_done,
				&buf );

		if ( done < 1 )
		{
			return report_error ( ERROR_AUDIO_ENCODE );
		}

		todo -= done;

		RETURN_ON_ERROR ( audio_out.write ( audio_buf, audio_done ) )
	}

	if ( audio_len < 1 )
	{
		return report_error ( ERROR_AUDIO_ZERO_INPUT );
	}

	buf.allocate ( audio_len );
	// If this allocation fails we don't care (Well encoding is skipped)

	if ( audio_done < audio_len )
	{
		// Fill the remainder of the buffer using the well
		encode_with_well ( well, audio_buf + audio_done,
			audio_len - audio_done, &buf );

		RETURN_ON_ERROR ( audio_out.write ( audio_buf + audio_done,
			audio_len - audio_done ) )
	}

	while ( 1 )
	{
		RETURN_ON_ERROR ( audio_in->read ( &audio_buf, &audio_len ) )

		if ( audio_len < 1 )
		{
			// End of input audio file
			break;
		}

		encode_with_well ( well, audio_buf, audio_len, &buf );

		RETURN_ON_ERROR ( audio_out.write ( audio_buf, audio_len ) )
	}

	return 0;
}

