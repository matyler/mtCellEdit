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

#include "soda.h"



class UtreeNew
{
public:
	UtreeNew () : root (), soda (), file (), err (0), m_buf ()
	{
		root = mtkit_utree_new_root ();
		if ( root )
		{
			soda = mtkit_utree_new_element ( root,
				SODA_UTREE_ROOT_NAME );
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



int mtDW::Soda::Op::encode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	) const
{
	if ( ! input || ! output )
	{
		return report_error ( ERROR_SODA_ENCODE_INSANITY );
	}

	mtKit::ChunkFile::Save file_out;

	if ( file_out.open ( output, SODA_FILE_ID ) )
	{
		return report_error ( ERROR_SODA_OPEN_OUTPUT );
	}

	mtKit::ByteFileRead file_in;

	if ( file_in.open ( input, 0 ) )
	{
		return report_error ( ERROR_SODA_OPEN_INPUT );
	}

	UtreeNew utree;
	if ( ! utree.soda )
	{
		return report_error ( ERROR_SODA_UTREE_ALLOC );
	}

	// HEADER - encoding mode
	int const mode_raw = (NULL == butt || encode_raw ()) ? 1 : 0;
	utree.set ( SODA_HEADER_ITEM_MODE, mode_raw );

	// HEADER - filesize
	struct stat st;
	if ( stat ( input, &st ) )
	{
		return report_error ( ERROR_SODA_OPEN_INFO );
	}
	uint64_t const filesize = (uint64_t)st.st_size;
	utree.set ( SODA_HEADER_ITEM_SIZE, filesize );

	int bucket_pos = 0;
	int bucket = 0;
	std::string butt_name;

	// HEADER - optional items when XOR encoding
	if ( ! mode_raw )
	{
		bucket_pos = butt->get_bucket_position ();
		utree.set ( SODA_HEADER_ITEM_POS, bucket_pos );

		bucket = butt->get_bucket_used ();
		utree.set ( SODA_HEADER_ITEM_BUCKET, bucket );

		butt_name = butt->get_otp_name ();
		utree.set ( SODA_HEADER_ITEM_BUTT, butt_name.c_str () );
	}

	if ( utree.err )
	{
		return report_error ( ERROR_SODA_UTREE_ALLOC );
	}

	// HEADER - dump to the chunkfile
	utree.create_output ();
	void * mem;
	int64_t mem_len;

	if ( mtkit_file_get_mem ( utree.file, &mem, &mem_len ) )
	{
		return report_error ( ERROR_SODA_UTREE_ALLOC );
	}

	if ( file_out.put_chunk ( (uint8_t *)mem, (uint32_t)mem_len,
		SODA_FILE_CHUNK_ID )
		)
	{
		return report_error ( ERROR_SODA_OPEN_OUTPUT );
	}

	ButtSaveState bss ( butt );

	try
	{
		ByteBuf	buf ( SODA_CHUNK_SIZE );
		ByteBuf	otp ( SODA_CHUNK_SIZE );
		uint64_t grand = 0;

		uint8_t * const dest = buf.get_buf ();
		uint8_t * const src = otp.get_buf ();
		size_t	const	destlen = buf.get_size ();

		while ( 1 )
		{
			size_t const tot = file_in.read ( dest,	destlen );

			grand += (uint64_t)tot;

			if ( tot < 1 )
			{
				// EOF

				if ( grand != filesize )
				{
					return report_error (
						ERROR_SODA_ENCODE_SIZE );
				}

				break;		// Success, all done
			}

			if ( ! mode_raw )
			{
				RETURN_ON_ERROR( butt->get_otp_data( src, tot ))

				for ( size_t i = 0; i < tot; i++ )
				{
					dest[i] ^= src[i];
				}
			}

			if ( file_out.put_chunk ( dest, (uint32_t)tot,
				SODA_FILE_CHUNK_ID ) )
			{
				return report_error ( ERROR_SODA_ENCODE_WRITE );
			}
		}
	}
	catch ( ... )
	{
		return report_error ( ERROR_SODA_ENCODE_EXCEPTION );
	}

	db_add_encode ( input, filesize, mode_raw, butt_name, bucket,
		bucket_pos );

	return 0;
}

int mtDW::Soda::Op::multi_encode (
	Butt			* const	butt,
	char		const * const	input,
	char		const * const	output,
	char	const * const * const	otp_names
	) const
{
	if ( ! input || ! output || ! otp_names )
	{
		return report_error ( ERROR_SODA_ENCODE_INSANITY );
	}

	mtKit::SqliteTransaction trans ( m_db );
	FilenameSwap	name ( output );

	if ( butt )
	{
		name.m_res = butt->set_otp ( otp_names[0] );
	}

	if ( 0 == name.m_res )
	{
		name.m_res = encode ( butt, input, output );
	}

	if ( 0 == name.m_res )
	{
		for ( int i = 1; otp_names[i]; i++ )
		{
			if ( butt )
			{
				name.m_res = butt->set_otp ( otp_names[i] );
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

