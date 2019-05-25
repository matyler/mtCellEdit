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

#include "well.h"



#define BUFSIZE_RAW	65536



mtDW::FileStream::FileStream ( FileDB & db )
	:
	m_buf_file	( BUFSIZE_RAW ),
	m_buf_zlib	(),
	m_file_db	( db )
{
}

int mtDW::FileStream::read ( ByteBuf & buf )
{
	buf.set_tot ( 0 );
	buf.set_pos ( 0 );

	while ( buf.get_tot () < buf.get_size () )
	{
		size_t const z_pos = m_buf_zlib.get_pos ();
		size_t const z_len = m_buf_zlib.get_size ();

		if ( z_pos < z_len )
		{
			size_t const dest_len = buf.get_size () - buf.get_tot();
			size_t const src_len = z_len - z_pos;
			size_t const len = MIN ( dest_len, src_len );

			memcpy ( buf.get_buf () + buf.get_tot (),
				m_buf_zlib.get_buf () + z_pos, len );

			buf.set_tot ( buf.get_tot () + len );
			m_buf_zlib.set_pos ( z_pos + len );

			continue;	// Finish or load more file data
		}

		m_buf_file.set_tot ( 0 );
		m_buf_file.set_pos ( 0 );

		while ( m_buf_file.get_tot () < m_buf_file.get_size () )
		{
			if ( ! m_file.is_open () )
			{
				if ( open () )
				{
					return 1;	// Not filled
				}
			}

			size_t const len = m_buf_file.get_size () -
				m_buf_file.get_tot ();

			size_t const loaded = m_file.read ( m_buf_file.get_buf()
				+ m_buf_file.get_tot (), len );

			if ( 0 == loaded )
			{
				if ( 0 == m_file.get_pos () )
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
				m_file.close ();

				m_file_db.increment_file_id ();

				continue;
			}

			m_buf_file.set_tot ( m_buf_file.get_tot () + loaded );
		}

		unsigned char * zlib;
		size_t zlib_len;

		if ( mtkit_mem_deflate ( (unsigned char *)m_buf_file.get_buf (),
			m_buf_file.get_size (), &zlib, &zlib_len,
			MTKIT_DEFLATE_LEVEL_DEFAULT,
			MTKIT_DEFLATE_MODEL_DEFAULT )
			)
		{
			return 1;	// Not filled
		}

		m_buf_zlib.set ( zlib, zlib_len );
	}

	return 0;			// Filled
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

			if ( 0 == m_file.open ( filename.c_str (), pos ) )
			{
				return 0;
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

