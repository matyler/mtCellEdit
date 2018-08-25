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

#include "soda.h"



class UtreeNew
{
public:
	UtreeNew () : root (), soda (), file (), err (0), m_buf ()
	{
		root = mtkit_utree_new_root ();
		if ( root )
		{
			soda = mtkit_utree_new_element ( root, UTREE_ROOT_NAME);
		}
	}

	~UtreeNew ()
	{
		if ( root )
		{
			mtkit_utree_destroy_node ( root );
			root = NULL;
			soda = NULL;
			mtkit_file_close ( file );
			file = NULL;
		}
	}

	void set ( char const * const name, char const * const val )
	{
		err |= mtkit_utree_set_attribute_str ( soda, name, val );
	}

	void set ( char const * const name, int num )
	{
		snprintf ( m_buf, sizeof(m_buf), "%i", num );
		err |= mtkit_utree_set_attribute_str ( soda, name, m_buf );
	}

	void set ( char const * const name, uint64_t num )
	{
		snprintf ( m_buf, sizeof(m_buf), "%" PRIu64, num );
		err |= mtkit_utree_set_attribute_str ( soda, name, m_buf );
	}

	void create_output ()
	{
		mtkit_file_close ( file );
		file = mtkit_utree_save_file_mem ( root,
			MTKIT_UTREE_OUTPUT_DEFAULT );
	}

	mtUtreeNode  	* root;
	mtUtreeNode  	* soda;
	mtFile		* file;
	int		err;

private:
	char		m_buf[64];
};



int mtDW::SodaOp::encode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	) const
{
	mtKit::ChunkFile::Save file_out;

	if ( file_out.open ( output, FILE_ID ) )
	{
		return 1;
	}

	mtKit::ByteFileRead file_in;

	if ( file_in.open ( input, 0 ) )
	{
		return 1;
	}

	UtreeNew utree;
	if ( ! utree.soda )
	{
		return 1;
	}

	// HEADER - encoding mode
	int const mode_raw = (NULL == butt || encode_raw ()) ? 1 : 0;
	utree.set ( HEADER_ITEM_MODE, mode_raw );

	// HEADER - filesize
	struct stat st;
	if ( stat ( input, &st ) )
	{
		return 1;
	}
	uint64_t const filesize = (uint64_t)st.st_size;
	utree.set ( HEADER_ITEM_SIZE, filesize );

	int bucket_pos = 0;
	int bucket = 0;
	std::string butt_name;

	// HEADER - optional items when XOR encoding
	if ( ! mode_raw )
	{
		bucket_pos = butt->get_bucket_position ();
		utree.set ( HEADER_ITEM_POS, bucket_pos );

		bucket = butt->get_bucket_used ();
		utree.set ( HEADER_ITEM_BUCKET, bucket );

		butt_name = butt->get_name ();
		utree.set ( HEADER_ITEM_BUTT, butt_name.c_str () );
	}

	if ( utree.err )
	{
		return 1;
	}

	// HEADER - dump to the chunkfile
	utree.create_output ();
	void * mem;
	int64_t mem_len;

	if ( mtkit_file_get_mem ( utree.file, &mem, &mem_len ) )
	{
		return 1;
	}

	if ( file_out.put_chunk ( (uint8_t *)mem, (uint32_t)mem_len,
		FILE_CHUNK_ID )
		)
	{
		return 1;
	}

	try
	{
		mtKit::ByteBuf	buf ( CHUNK_SIZE );
		mtKit::ByteBuf	otp ( CHUNK_SIZE );
		uint64_t grand = 0;

		while ( 1 )
		{
			size_t const tot = file_in.read ( buf.array,
				buf.array_len );

			grand += (uint64_t)tot;

			if ( tot < 1 )
			{
				// EOF

				if ( grand != filesize )
				{
					std::cerr << "Filesize not matched.\n";
				}

				break;
			}

			if ( ! mode_raw )
			{
				if ( butt->otp_get_data ( otp.array, tot ) )
				{
					throw 123;
				}

				for ( size_t i = 0; i < tot; i++ )
				{
					buf.array[i] ^= otp.array[i];
				}
			}

			if ( file_out.put_chunk ( buf.array, (uint32_t)tot,
				FILE_CHUNK_ID ) )
			{
				throw 123;
			}
		}
	}
	catch ( ... )
	{
		return 1;
	}

	db_add_encode ( input, filesize, mode_raw, butt_name, bucket,
		bucket_pos );

	return 0;
}

int mtDW::SodaOp::multi_encode (
	Butt			* const	butt,
	char		const * const	input,
	char		const * const	output,
	char	const * const * const	butt_names
	) const
{
	mtKit::SqliteTransaction trans ( m_db );
	FilenameSwap	name ( output );

	if ( butt )
	{
		name.m_res = butt->set_name ( butt_names[0] );
	}

	if ( 0 == name.m_res )
	{
		name.m_res = encode ( butt, input, output );
	}

	if ( 0 == name.m_res )
	{
		for ( int i = 1; butt_names[i]; i++ )
		{
			if ( butt )
			{
				name.m_res = butt->set_name ( butt_names[i] );
			}

			if ( 0 == name.m_res )
			{
				name.m_res = encode ( butt, name.f1, name.f2 );
			}

			if ( name.m_res )
			{
				break;
			}

			name.swap ();
		}
	}

	return name.m_res;
}

