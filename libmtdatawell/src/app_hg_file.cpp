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



FileOps::FileOps ( mtDW::GlyphIndex const * const index )
	:
	m_file_in_buf		( NULL ),
	m_file_in_len		( 0 ),
	m_file_in_utf8		( 0 ),
	m_file_in_utf8_bits	( 0 ),
	m_file_in_utf8_bytes	( 0 ),
	m_file_in_utf8_encoded	( 0 ),
	m_file_in_utf8_roots	( 0 ),
	m_file_in_utf8_misc	( 0 ),
	m_file_covert_buf	( NULL ),
	m_file_covert_len	( 0 ),
	m_file_out		( NULL ),
	m_hg_index		( index )
{
}

FileOps::~FileOps ()
{
	file_in_set ( NULL, 0 );
	file_covert_set ( NULL, 0 );
	file_out_set ( NULL );
}

int FileOps::load_input_file ( char const * const filename )
{
	int len;
	char * const buf = mtkit_file_load ( filename, &len, 0, NULL );

	if ( buf )
	{
		file_in_set ( buf, len );
		return 0;
	}

	return 1;
}

int FileOps::load_covert_file ( char const * const filename )
{
	int len;
	char * const buf = mtkit_file_load ( filename, &len, 0, NULL );

	if ( buf )
	{
		int const max = mtDW::Homoglyph::INPUT_FILESIZE_MAX;

		if ( len > max )
		{
			std::cerr << "load_covert_file too large "
				<< len << " > " << max << "\n";
			free ( buf );
			return 1;
		}

		file_covert_set ( buf, len );
		return 0;
	}

	return 1;
}

int FileOps::open_output_file ( char const * const filename )
{
	mtFile * const file = mtkit_file_open_disk ( filename );

	if ( file )
	{
		file_out_set ( file );
		return 0;
	}

	return 1;
}

int FileOps::open_output_mem ()
{
	mtFile * const file = mtkit_file_open_mem ();

	if ( file )
	{
		file_out_set ( file );
		return 0;
	}

	return 1;
}

int FileOps::get_output_mem_utf8 ( std::string &result )
{
	void * buf;
	int64_t buflen;

	if ( ! m_file_out
		|| mtkit_file_get_mem ( m_file_out, &buf, &buflen )
		|| ! buf
		)
	{
		return 1;
	}


	if ( buflen < 1 )
	{
		result.clear ();
		return 0;
	}

	if ( 1 != mtkit_utf8_string_legal ( (unsigned char const *)buf,
			(size_t)buflen )
		|| mtKit::string_from_data ( result, buf, (size_t)buflen )
		)
	{
		return 1;
	}

	return 0;
}

int FileOps::load_input_utf8 ( std::string const &input )
{
	file_in_set ( strdup ( input.c_str () ), (int)(input.length ()) );

	return 0;
}

int FileOps::load_covert_utf8 ( std::string const &covert )
{
	file_covert_set ( strdup(covert.c_str()), (int)(covert.length ()) );

	return 0;
}

int FileOps::analyse_input ( std::string &info ) const
{
	int const glyph_len = get_utf8_len ();

	if ( glyph_len < 0 )
	{
		info += "Not UTF-8 input";
		return 1;
	}

	char buf[32];

	info += "Input bytes: ";
	snprintf ( buf, sizeof(buf), "%i", get_file_size () );
	info += buf;
	info += "\n";

	info += "UTF-8 glyphs: ";
	snprintf ( buf, sizeof(buf), "%i", glyph_len );
	info += buf;
	info += "\n";

	info += "UTF-8 encoded glyphs: ";
	snprintf ( buf, sizeof(buf), "%i", get_utf8_encoded_tot () );
	info += buf;
	info += "\n";

	info += "UTF-8 root glyphs: ";
	snprintf ( buf, sizeof(buf), "%i", get_utf8_roots_tot () );
	info += buf;
	info += "\n";

	info += "UTF-8 misc ASCII glyphs: ";
	snprintf ( buf, sizeof(buf), "%i", get_utf8_misc_tot () );
	info += buf;
	info += "\n";

	int const bit_size = get_utf8_bit_capacity ();

	info += "UTF-8 bit capacity: ";
	snprintf ( buf, sizeof(buf), "%i", bit_size );
	info += buf;
	info += "\n";

	info += "UTF-8 byte capacity (bits): ";
	snprintf ( buf, sizeof(buf), "%i", bit_size / 8 );
	info += buf;
	info += "\n";

	info += "UTF-8 byte capacity (ari): ";
	snprintf ( buf, sizeof(buf), "%i", get_utf8_byte_capacity () );
	info += buf;
	info += "\n";

	if ( m_file_covert_buf )
	{
		info += "Covert bytes: ";
		snprintf ( buf, sizeof(buf), "%i", get_covert_size () );
		info += buf;
		info += "\n";
	}

	return 0;
}

int FileOps::cleanse_file ()
{
	if ( ! m_file_in_buf || ! m_file_out )
	{
		return 1;
	}

	unsigned char const * src = (unsigned char const *)m_file_in_buf,
		* end = src + m_file_in_len;

	for ( ; src < end; )
	{
		int const glyph_len = mtkit_utf8_offset ( src, 1 );

		if ( glyph_len < 1 )
		{
			std::cerr << "cleanse_file: unexpected UTF-8 error.\n";
			return 1;
		}

		std::string st;

		if ( mtKit::string_from_data ( st, src, (size_t)glyph_len ) )
		{
			std::cerr << "cleanse_file: unexpected UTF-8 error.\n";
			return 1;
		}

		char root = 0;

		if ( 0 == m_hg_index->get_root_bits ( st, &root, NULL, NULL ) )
		{
			if ( mtkit_file_write ( m_file_out, &root, 1 ) )
			{
				std::cerr << "cleanse_file: unexpected "
					"mtkit_file_write error.\n";
				return 1;
			}
		}
		else
		{
			if ( mtkit_file_write ( m_file_out, src, glyph_len ) )
			{
				std::cerr << "cleanse_file: unexpected "
					"mtkit_file_write error.\n";
				return 1;
			}
		}

		src += glyph_len;
	}

	return 0;
}

static void count_utf8 (
	mtDW::GlyphIndex const * const hg_index,
	char	* const	buf,
	int	const	len,
	int		&root_tot,
	int		&bit_tot,
	int		&byte_tot,
	int		&enc_tot,
	int		&misc_tot
	)
{
	unsigned char const * src = (unsigned char const *)buf,
		* end = src + len;

	root_tot = 0;
	bit_tot = 0;
	byte_tot = 0;
	enc_tot = 0;
	misc_tot = 0;

	try
	{
		uint8_t const zmem[7] = {0};
		mtKit::ArithEncode ari;

		for ( ; src < end; )
		{
			int const glyph_len = mtkit_utf8_offset ( src, 1 );

			if ( glyph_len < 1 )
			{
				throw 123;
			}

			std::string st;

			if ( mtKit::string_from_data ( st, src,
				(size_t)glyph_len ) )
			{
				throw 123;
			}

			int bits = 0;
			int nodes = 0;
			int const idx = hg_index->get_index( st, &bits, &nodes);

			if ( idx >= 0 && bits > 0 )
			{
				int junk;
				if ( ari.pop_code ( nodes, junk ) )
				{
					byte_tot += (int)sizeof(zmem);

					ari.push_mem ( zmem, sizeof(zmem) );

					if ( ari.pop_code ( nodes, junk ) )
					{
						std::cerr << "count_utf8: "
							"Unexpected double "
							"error: ar.pop_code\n";
						throw 123;
					}
				}

				bit_tot += bits;

				if ( glyph_len == 1 )
				{
					root_tot ++;
				}
				else
				{
					enc_tot ++;
				}
			}
			else
			{
				misc_tot ++;
			}

			src += glyph_len;
		}

		byte_tot += ari.get_encoded_byte_count ();
	}
	catch ( ... )
	{
		std::cerr << "count_utf8: unexpected UTF-8 error.\n";
	}
}

void FileOps::file_in_set (
	char	* const	buf,
	int	const	len
	)
{
	if ( m_file_in_buf )
	{
		free ( m_file_in_buf );
	}

	m_file_in_buf = buf;
	m_file_in_len = len;

	if (	buf &&
		1 == mtkit_utf8_string_legal (
			(unsigned char const *)m_file_in_buf,
			(size_t)m_file_in_len )
		)
	{
		m_file_in_utf8 = mtkit_utf8_len (
			(unsigned char const *)m_file_in_buf,
			(size_t)m_file_in_len );

		count_utf8 ( m_hg_index, buf, len,
			m_file_in_utf8_roots,
			m_file_in_utf8_bits,
			m_file_in_utf8_bytes,
			m_file_in_utf8_encoded,
			m_file_in_utf8_misc
			);
	}
	else
	{
		m_file_in_utf8 = -1;
		m_file_in_utf8_roots = 0;
		m_file_in_utf8_bits = 0;
		m_file_in_utf8_bytes = 0;
		m_file_in_utf8_encoded = 0;
		m_file_in_utf8_misc = 0;
	}
}

void FileOps::file_covert_set (
	char	* const	buf,
	int	const	len
	)
{
	if ( m_file_covert_buf )
	{
		free ( m_file_covert_buf );
	}

	m_file_covert_buf = buf;
	m_file_covert_len = len;
}

void FileOps::file_out_set ( mtFile * const file_out )
{
	if ( m_file_out )
	{
		mtkit_file_close ( m_file_out );
	}

	m_file_out = file_out;
}



/// ----------------------------------------------------------------------------



int mtDW::Homoglyph::file_analyse (
	char	const * const	filename,
	std::string		&info
	) const
{
	info.clear ();

	if ( ! filename )
	{
		info += "Bad argument";
		return 1;
	}

	FileOps fops ( this );

	if ( fops.load_input_file ( filename ) )
	{
		info += "Unable to load file";
		return 1;
	}

	return fops.analyse_input ( info );
}

int mtDW::Homoglyph::file_encode (
	char	const * const	input_utf8,
	char	const * const	input_bin,
	char	const * const	output_utf8,
	Well		* const	well,
	std::string		&info
	) const
{
	info.clear ();

	if ( ! input_utf8 || ! input_bin || ! output_utf8 )
	{
		info = "Bad argument";
		return 1;
	}

	FileOps fops ( this );

	if ( fops.load_input_file ( input_utf8 ) )
	{
		info = "Unable to load input UTF-8 file";
		return 1;
	}

	int const glyph_len = fops.get_utf8_len ();

	if ( glyph_len < 1 )
	{
		info += "Not UTF-8 input file";
		return 1;
	}

	if ( fops.load_covert_file ( input_bin ) )
	{
		info = "Unable to load covert file";
		return 1;
	}

	if ( fops.open_output_file ( output_utf8 ) )
	{
		info = "Unable to open output file";
		return 1;
	}

	if ( fops.encode_hg_file ( well ) )
	{
		info = "Unable to encode file";
		return 1;
	}

	return 0;
}

int mtDW::Homoglyph::file_decode (
	char	const * const	input_utf8,
	char	const * const	output_bin,
	std::string		&info
	) const
{
	info.clear ();

	if ( ! input_utf8 || ! output_bin )
	{
		info = "Bad argument";
		return 1;
	}

	FileOps fops ( this );

	if ( fops.load_input_file ( input_utf8 ) )
	{
		info = "Unable to load input UTF-8 file";
		return 1;
	}

	int const glyph_len = fops.get_utf8_len ();

	if ( glyph_len < 1 )
	{
		info = "Not UTF-8 input file";
		return 1;
	}

	if ( fops.open_output_file ( output_bin ) )
	{
		info = "Unable to open output file";
		return 1;
	}

	if ( fops.decode_hg_file () )
	{
		info = "Unable to decode file";
		return 1;
	}

	return 0;
}

