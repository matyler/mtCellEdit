/*
	Copyright (C) 2019-2022 Mark Tyler

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

#include "app_hg.h"



class DecodeFile
{
public:
	DecodeFile ();
	~DecodeFile ();

	int init ();
	int grab_bytes ();

/// ----------------------------------------------------------------------------

	mtKit::ArithDecode ari;

	mtFile * memfile;

private:
	uint8_t zmem[7];
	size_t zmem_len;

	MTKIT_RULE_OF_FIVE( DecodeFile )
};



DecodeFile::DecodeFile ()
	:
	memfile		( NULL ),
	zmem		(),
	zmem_len	( 0 )
{
}

DecodeFile::~DecodeFile ()
{
	mtkit_file_close ( memfile );
	memfile = NULL;
}

int DecodeFile::init ()
{
	memfile = mtkit_file_open_mem ();

	if ( memfile )
	{
		return 0;
	}

	return 1;
}

int DecodeFile::grab_bytes ()
{
	if ( ari.pop_mem ( zmem, zmem_len ) )
	{
		std::cerr << "grab_bytes: Unexpected ari.pop_mem error\n";
		return 1;
	}

	if ( mtkit_file_write ( memfile, zmem, (int64_t)zmem_len ) )
	{
		std::cerr << "grab_bytes: Error writing zmem.\n";
		return 1;
	}

	return 0;
}

int FileOps::decode_hg_file ()
{
	if ( ! m_file_in_buf || ! m_file_out )
	{
		return 1;
	}

	DecodeFile decode;

	unsigned char const * src = (unsigned char const *)m_file_in_buf;
	unsigned char const * const end =
		(unsigned char const *)(m_file_in_buf + m_file_in_len);

	if ( decode.init () )
	{
		return 1;
	}

	while ( src < end )
	{
		int glyph_len = mtkit_utf8_offset ( src, 1 );

		if ( glyph_len < 1 )
		{
			std::cerr << "Problem with input UTF-8\n";
			return 1;
		}

		std::string const glyph ( (char const *)src, (size_t)glyph_len);

		int node_tot = 0;
		int const index = m_hg_index->get_index(glyph, NULL, &node_tot);

		if ( index >= 0 && node_tot > 1 )
		{
			switch ( decode.ari.push_code ( index, node_tot ) )
			{
			case 0:	// OK, code packed
				break;

			case 1:	// not sent - end of capacity
				if ( decode.grab_bytes () )
				{
					return 1;
				}

				if ( decode.ari.push_code( index, node_tot ) )
				{
					std::cerr << "Unexpected ari.push_code"
						" error\n";
					return 1;
				}

				break;		// OK

			default:
				std::cerr << "Unexpected ari.push_code error\n";
				return 1;
			}
		}

		src += glyph_len;
	}

	// Grab any remaining bytes encoded
	if ( decode.ari.get_encoded_byte_count () > 0 )
	{
		if ( decode.grab_bytes () )
		{
			return 1;
		}
	}

	void * vdata;
	int64_t vlen;

	if ( mtkit_file_get_mem ( decode.memfile, &vdata, &vlen ) )
	{
		std::cerr << "Unable to extract mtkit_file_get_mem\n";
		return 1;
	}

	uint8_t const * const data = (uint8_t const *)vdata;
	size_t const tot_len = (size_t)vlen;
	size_t header_len = 1;
	size_t data_len = 0;

	if ( tot_len < header_len )
	{
		return 0;
	}

	header_len += (data[0] & 3);

	if ( tot_len < header_len )
	{
		std::cerr << "Header size exceeds total size.\n";
		return 1;
	}

	switch ( header_len )
	{
	case 2:
		data_len = data[1];
		break;
	case 3:
		data_len = data[1] + (size_t)(data[2] << 8);
		break;
	case 4:
		data_len = data[1] + (size_t)((data[2] << 8) + (data[3] << 16));
		break;
	default:
		break;
	}

	if ( (header_len + data_len) > tot_len )
	{
		std::cerr << "Header size (" << header_len << ") + "
			"data size (" << data_len << ") exceeds "
			"total size (" << tot_len << ").\n";

		return 1;
	}

	if ( data_len < 1 )
	{
		return 0;
	}

	if ( mtkit_file_write ( m_file_out, data + header_len, (int)data_len ) )
	{
		std::cerr << "Unable to write data to output\n";
		return 1;
	}

	return 0;
}

