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

#include "well.h"



static int m_stderr = 0;



void mtDW::set_stderr_less ()
{
	m_stderr--;
}

void mtDW::set_stderr_more ()
{
	m_stderr++;
}

int report_error ( int const error )
{
	if ( m_stderr >= 0 )
	{
		std::cerr << mtDW::get_error_text ( error ) << "\n";
	}

	return error;
}

static std::map<int, char const *> const err_table = {

	{ ERROR_ANALYSIS_INSANITY,	"Analysis sanity failure" },

	{ ERROR_AUDIO_BAD_CHANNELS,	"Audio file has bad channels" },
	{ ERROR_AUDIO_DECODE_EXCEPTION,	"Audio decode exception failure" },
	{ ERROR_AUDIO_DECODE_INSANITY,	"Audio decode sanity failure" },
	{ ERROR_AUDIO_ENCODE,		"Unable to encode input to audio" },
	{ ERROR_AUDIO_ENCODE_INSANITY,	"Audio encode sanity failure" },
	{ ERROR_AUDIO_OPEN_INPUT,	"Unable to open audio input file" },
	{ ERROR_AUDIO_OPEN_OUTPUT,	"Unable to open audio output file" },
	{ ERROR_AUDIO_READ,		"Unable to read audio file" },
	{ ERROR_AUDIO_TOO_SMALL,	"Audio bottle too small" },
	{ ERROR_AUDIO_WRITE,		"Unable to write audio output file" },
	{ ERROR_AUDIO_WRITE_INSANITY,	"Audio write sanity failure" },
	{ ERROR_AUDIO_ZERO_INPUT,	"Zero length input file for audio" },

	{ ERROR_BUTT_OTP_DELETE_ACTIVE,	"Unable to delete active OTP" },
	{ ERROR_BUTT_OTP_EXISTS,	"Butt OTP already exists" },
	{ ERROR_BUTT_OTP_MISSING,	"Butt OTP missing" },
	{ ERROR_BUTT_OTP_NO_WELL,	"Butt OTP Well missing" },
	{ ERROR_BUTT_OTP_OPEN_BUCKET,	"Butt OTP unable to open bucket" },
	{ ERROR_BUTT_OTP_READ_BUFFER,	"Butt OTP missing buffer (read)" },
	{ ERROR_BUTT_OTP_READ_ONLY,	"Butt OTP is read only" },

	{ ERROR_DISK_OTP_READ_ONLY,	"Disk OTP is read only" },

	{ ERROR_HEAP_EMPTY,		"Heap memory exhausted" },

	{ ERROR_IMAGE_DECODE_EXCEPTION,	"Image decode exception failure" },
	{ ERROR_IMAGE_ENCODE_INSANITY,	"Image encode sanity failure" },
	{ ERROR_IMAGE_INVALID_BOTTLE,	"Image bottle invalid type" },
	{ ERROR_IMAGE_OPEN_OUTPUT,	"Unable to open image output file" },
	{ ERROR_IMAGE_TOO_SMALL,	"Image bottle too small" },
	{ ERROR_IMAGE_WRITE,		"Unable to write image output file" },

	{ ERROR_IMPORT_OTP_BAD_DIR,	"Only directories can be imported" },
	{ ERROR_IMPORT_OTP_EXISTS,	"OTP name already exists" },
	{ ERROR_IMPORT_OTP_OPEN_DIR,	"Unable to open directory" },

	{ ERROR_LOAD_INPUT,		"Unable to load input file" },

	{ ERROR_OTP_NAME_ILLEGAL,	"OTP name has an illegal character" },
	{ ERROR_OTP_NAME_TOO_LARGE,	"OTP name has too many characters" },
	{ ERROR_OTP_NAME_TOO_SMALL,	"OTP name has too few characters" },

	{ ERROR_SODA_BAD_CHUNK_ID,	"Invalid Soda file data chunk ID" },
	{ ERROR_SODA_BAD_HEADER,	"Unable to find Soda header" },
	{ ERROR_SODA_BAD_HEADER_ID,	"Invalid Soda file header chunk ID" },
	{ ERROR_SODA_BIG_CHUNK,		"Soda file chunk too large" },
	{ ERROR_SODA_DECODE_INSANITY,	"Soda decode sanity failure" },
	{ ERROR_SODA_DECODE_NO_BUTT,	"Soda decode no Butt avalailable" },
	{ ERROR_SODA_DECODE_NO_XOR,	"Unable to allocate XOR buffer" },
	{ ERROR_SODA_ENCODE_EXCEPTION,	"Soda encode unexpected exception" },
	{ ERROR_SODA_ENCODE_INSANITY,	"Soda encode sanity failure" },
	{ ERROR_SODA_ENCODE_SIZE,	"Input filesize not matched" },
	{ ERROR_SODA_ENCODE_WRITE,	"Unable to write Soda output file" },
	{ ERROR_SODA_FILE_CHUNK_1,	"Unable to read first Soda chunk" },
	{ ERROR_SODA_FILE_ID,		"Invalid Soda file ID" },
	{ ERROR_SODA_FILE_WRITE,	"Problem writing to output file" },
	{ ERROR_SODA_MISSING_DATA,	"Missing bytes in input file" },
	{ ERROR_SODA_OPEN_INFO,		"Unable to stat file info" },
	{ ERROR_SODA_OPEN_INPUT,	"Unable to open Soda input file" },
	{ ERROR_SODA_OPEN_INSANITY,	"Soda file open sanity failure" },
	{ ERROR_SODA_OPEN_OUTPUT,	"Unable to open Soda output file" },
	{ ERROR_SODA_UTREE_ALLOC,	"Unable to allocate Utree" },

	{ ERROR_TAP_BOTTLE_INVALID,	"Bottle does not contain any Soda" },
	{ ERROR_TAP_DECODE_INSANITY,	"Tap decode sanity failure" },
	{ ERROR_TAP_ENCODE_BAD_BOTTLE,	"Unkown input bottle type" },
	{ ERROR_TAP_ENCODE_INSANITY,	"Tap encode sanity failure" },
	{ ERROR_TAP_ENCODE_SAVE_PNG,	"Unable to save output PNG bottle" },
	{ ERROR_TAP_OPEN_SODA_INSANITY,	"Tap open Soda sanity failure" },
	{ ERROR_TAP_UNKNOWN_BOTTLE,	"Unknown bottle type" },

	{ ERROR_WELL_SAVE_FILE_INSANITY,"Save file sanity failure" },
	{ ERROR_WELL_SAVE_FILE_OPEN,	"Unable to open file" },
	{ ERROR_WELL_SAVE_FILE_WRITE,	"Unable to write to file" },

	{ ERROR_NONE,			"Success" }
	};



char const * mtDW::get_error_text ( int const error )
{
#ifdef DEBUG

	// DEBUG mode checks to ensure the table is correctly formed

	static int check;

	if ( 0 == check )
	{
		std::cerr << "\nChecking error table ...\n";

		check = 1;

		size_t const size = err_table.size ();
		size_t const expected = (size_t)(ERROR_MAX - ERROR_MIN);

		if ( size != expected )
		{
			std::cerr << "err_table poorly formed."
				<< "size=" << size << " "
				<< "expected=" << expected
				<< "\n";

			exit ( 1 );
		}

		std::cerr << "Error table is valid: " << size <<" items\n\n";
	}

#endif



	auto const it = err_table.find ( error );

	if ( it != err_table.end () )
	{
		return it->second;
	}

	return "Unknown";
}

