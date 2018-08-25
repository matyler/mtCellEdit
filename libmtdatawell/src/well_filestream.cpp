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

#include "well.h"



#define BUFSIZE_RAW	65536



mtDW::FileStream::FileStream ( FileDB & db )
	:
	buf_file	( BUFSIZE_RAW ),
	m_zlib		(),
	m_zlib_len	( 0 ),
	m_zlib_pos	( 0 ),
	m_pos		( 0 ),
	m_fp		(),
	m_file_db	( db )
{
}

mtDW::FileStream::~FileStream ()
{
	free_zlib ();
	set_file ( NULL );
}

int mtDW::FileStream::read ( mtKit::ByteBuf & buf )
{
	buf.tot = 0;
	buf.pos = 0;

	while ( buf.tot < buf.array_len )
	{
		if ( m_zlib_pos < m_zlib_len )
		{
			size_t const dest_len = buf.array_len - buf.tot;
			size_t const src_len = m_zlib_len - m_zlib_pos;
			size_t const len = MIN ( dest_len, src_len );

			memcpy( buf.array + buf.tot, m_zlib + m_zlib_pos, len );

			buf.tot += len;
			m_zlib_pos += len;

			continue;	// Finish or load more file data
		}

		buf_file.tot = 0;
		buf_file.pos = 0;

		while ( buf_file.tot < buf_file.array_len )
		{
			if ( ! m_fp )
			{
				if ( open () )
				{
					return 1;	// Not filled
				}

				m_pos = 0;
			}

			size_t const len = buf_file.array_len - buf_file.tot;

			size_t const loaded = fread ( buf_file.array +
				buf_file.tot, 1, len, m_fp );

			if ( 0 == loaded )
			{
				if ( 0 == m_pos )
				{
					/* IMPORTANT! - if after opening a file
					it yields 0 bytes, remove it! If we
					don't do this we can end up in an
					infinite loop!!!!
					*/

					m_file_db.remove_todo_filename ();

					continue;
				}

				// EOF reached, get next file
				set_file ( NULL );

				m_file_db.increment_file_id ();

				continue;
			}

			m_pos += loaded;
			buf_file.tot += loaded;
		}

		free_zlib ();

		if ( mtkit_mem_deflate ( (unsigned char *)buf_file.array,
			buf_file.array_len, &m_zlib, &m_zlib_len,
			MTKIT_DEFLATE_LEVEL_DEFAULT,
			MTKIT_DEFLATE_MODEL_DEFAULT )
			)
		{
			return 1;	// Not filled
		}
	}

	return 0;			// Filled
}

void mtDW::FileStream::set_file ( FILE * const fp )
{
	if ( NULL != m_fp )
	{
		fclose ( m_fp );
	}

	m_fp = fp;
}

void mtDW::FileStream::free_zlib ()
{
	free ( m_zlib );
	m_zlib = NULL;
	m_zlib_len = 0;
	m_zlib_pos = 0;
}

int mtDW::FileStream::open ( uint64_t pos )
{
	try
	{
		while ( 1 )
		{
			std::string filename =
				m_file_db.get_todo_filename ();

			if ( filename.size () < 1 )
			{
				// No files left so give up

				return 1;
			}

			FILE * fp = fopen ( filename.c_str (), "rb" );
			if ( fp )
			{
				// Successfully opened this file

				if (	0 == pos ||
					0 == fseek ( fp, (long)pos, SEEK_SET )
					)
				{
					m_pos = pos;

					set_file ( fp );

					return 0;
				}

				fclose ( fp );
				fp = NULL;
			}

			// Failed to open (or seek) so remove it from the list
			m_file_db.remove_todo_filename ();

			pos = 0;
		}
	}
	catch ( ... )
	{
	}

	return 1;
}

