/*
	Copyright (C) 2019-2020 Mark Tyler

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



class EncodeFile
{
public:
	explicit EncodeFile ( FileOps * fops );
	~EncodeFile ();

	int init ( char const * buf, int buflen, int capacity );

/// ----------------------------------------------------------------------------

	unsigned char	* m_buf;
	size_t		m_buflen;

private:
	FileOps	const * const	m_fops;

	EncodeFile ( const EncodeFile & );	// Disable copy constructor
	EncodeFile & operator = (const EncodeFile &); // Disable = operator
};



EncodeFile::EncodeFile ( FileOps * fops )
	:
	m_buf		( NULL ),
	m_buflen	( 0 ),
	m_fops		( fops )
{
}

EncodeFile::~EncodeFile ()
{
	free ( m_buf );
	m_buf = NULL;
	m_buflen = 0;
}

int EncodeFile::init (
	char	const * const	buf,
	int		const	buflen,
	int		const	capacity
	)
{
	uint8_t header[4] = {0};
	int tot = 1;	// First byte of header

	if ( buflen > 65535 )
	{
		header[0] = 3;
	}
	else if ( buflen > 255 )
	{
		header[0] = 2;
	}
	else if ( buflen > 0 )
	{
		header[0] = 1;
	}
	// Zero length input files need no extra bytes in the header

	tot += header[0];
	header[1] = (unsigned char)(buflen);
	header[2] = (unsigned char)(buflen >> 8);
	header[3] = (unsigned char)(buflen >> 16);

	size_t const head_size = (size_t)(tot);

	tot += buflen;

	if ( tot > capacity )
	{
		std::cerr << "Not enough capacity to store covert data.\n";
		return 1;
	}

	tot += 1;	// Extra byte needed for padding at the end.

	m_buf = (unsigned char *)calloc ( (size_t)tot, 1 );
	if ( ! m_buf )
	{
		std::cerr << "EncodeFile::init - Unable to allocate memory.\n";
		return 1;
	}

	m_buflen = (size_t)tot;

	memcpy ( m_buf, header, head_size );

	if ( buflen > 0 )
	{
		memcpy ( m_buf + head_size, buf, (size_t)buflen );
	}

	return 0;
}



int FileOps::encode_hg_file ( mtDW::Well * const well )
{
	if ( ! m_file_in_buf || ! m_file_covert_buf || ! m_file_out )
	{
		return 1;
	}

	EncodeFile enc ( this );

	if ( enc.init ( m_file_covert_buf, m_file_covert_len,
		m_file_in_utf8_bytes
		) )
	{
		return 1;
	}

	uint8_t const * buf = enc.m_buf;
	size_t bufpos = 0;
	size_t buflen = enc.m_buflen;
	mtKit::ArithEncode ari;
	unsigned char rnd[8192] = {0};
	uint8_t zmem[7] = {0};
	unsigned char const * src = (unsigned char const *)m_file_in_buf;
	unsigned char const * const end =
		(unsigned char const *)(m_file_in_buf + m_file_in_len);

	while ( src < end )
	{
		int glyph_len = mtkit_utf8_offset ( src, 1 );

		if ( glyph_len < 1 )
		{
			std::cerr << "Problem with input UTF-8\n";
			return 1;
		}

		std::string glyph ( (char const *)src, (size_t)glyph_len );

		int bit_tot = 0;
		char root = 0;
		int node_tot = 0;

		m_hg_index->get_root_bits ( glyph, &root, &bit_tot, &node_tot );

		if ( node_tot > 1 )
		{
			int byte;

			if ( ari.pop_code ( node_tot, byte ) )
			{
				// Get another 7 covert bytes to be encoded

				for ( size_t i = 0; i < sizeof(zmem); i++ )
				{
					if ( bufpos >= buflen )
					{
						if ( well )
						{
							well->get_data ( rnd,
								sizeof(rnd) );
						}

						buf = rnd;
						buflen = sizeof(rnd);
						bufpos = 0;
					}

					zmem[i] = buf[ bufpos++ ];
				}

				ari.push_mem ( zmem, sizeof(zmem) );

				if ( ari.pop_code ( node_tot, byte ) )
				{
					std::cerr <<
						"Unexpected double "
						"error: ar.pop_code\n";
					return 1;
				}
			}

			if ( m_hg_index->get_node ( root, byte, glyph ) )
			{
				std::cerr << "Error finding glyph index. '"
					<< glyph << "'[" << byte << "]"
					<< "\n";
				return 1;
			}
		}

		if ( mtkit_file_write ( m_file_out, glyph.c_str (),
			(int)glyph.size () )
			)
		{
			std::cerr << "Unable to write data to output\n";
			return 1;
		}

		src += glyph_len;
	}

	return 0;
}


int FileOps::encode_utf8font_file ( int const type )
{
	if ( ! m_file_in_buf
		|| ! m_file_out
		|| type < mtDW::Utf8Font::TYPE_MIN
		|| type > mtDW::Utf8Font::TYPE_MAX
		)
	{
		return 1;
	}

	EncodeFile enc ( this );

	unsigned char const * src = (unsigned char const *)m_file_in_buf;
	unsigned char const * const end =
		(unsigned char const *)(m_file_in_buf + m_file_in_len);

	while ( src < end )
	{
		int glyph_len = mtkit_utf8_offset ( src, 1 );

		if ( glyph_len < 1 )
		{
			std::cerr << "Problem with input UTF-8\n";
			return 1;
		}

		std::string glyph ( (char const *)src, (size_t)glyph_len );

		char root = 0;

		if ( 1 == glyph_len )
		{
			root = (char)src[0];
		}
		else
		{
			m_hg_index->get_root_bits ( glyph, &root, NULL, NULL );
			// Any failure here is not a problem
		}

		if ( root )
		{
			int const nums[ mtDW::Utf8Font::TYPE_MAX + 1 ] = {
				0, 1, 0, 0, 0, 0, 0, 0, 2, 3, 0, 0, 4, 5
				};
			int const t = (root<'0' || root>'9')? type : nums[type];
			m_hg_index->get_node ( root, t, glyph );
			// Any failure here is not a problem
		}

		if ( mtkit_file_write ( m_file_out, glyph.c_str (),
			(int)glyph.size () )
			)
		{
			std::cerr << "Unable to write data to output\n";
			return 1;
		}

		src += glyph_len;
	}

	return 0;
}

