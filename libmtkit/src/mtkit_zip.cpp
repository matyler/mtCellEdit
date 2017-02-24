/*
	Copyright (C) 2009-2016 Mark Tyler

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

#include "private.h"



typedef struct mtZipFileInfo	mtZipFileInfo;



struct mtZipFileInfo
{
	unsigned char	* buf;
	int		buf_size;

	mtZipFileInfo	* next;
};

struct mtZip
{
	char		* mem,		// Memory (loaded from) or (to be saved
					// to) ZIP file (NULL = empty)
			* afilename	// Allocated filename string
			;
	char	const	* cfilename;	// Constant Directory/Filename of this
					// memory data (NULL = empty)

	char	const	* archive_path;	// Full Path/Filename of ZIP archive
	char	const	* archive_file;	// Filename of ZIP archive

	int32_t		size,		// Number of bytes in 'mem'
					// File modified date/time:
			mod_year;	// 1980-2107

	int8_t		mod_month,	// 1..12
			mod_day,	// 1..31
			mod_hour,	// 0..23
			mod_minute,	// 0..59
			mod_second,	// 0..59

			deflate		// 1 => Try to deflate the data when
					// saving 0 = don't
			;

	// Used by save only

	mtZipFileInfo	* flist1,	// First item in file list
			* flistn;	// Last item in file list

	FILE		* fp;
	int		error;		// 1 = File error so don't try to save
					// any more data
	uint32_t	cd_start,
			cd_entries;
};



// Zip header byte offsets.  Little endian so 1 = lsb 2 = msb
enum
{
	ZHBO_ID1			= 0,
	ZHBO_ID2			= 1,
	ZHBO_ID3			= 2,
	ZHBO_ID4			= 3,
	ZHBO_VERSION1			= 4,
	ZHBO_VERSION2			= 5,
	ZHBO_GENERAL_FLAG1		= 6,
	ZHBO_GENERAL_FLAG2		= 7,
	ZHBO_COMPRESSION_METHOD1	= 8,
	ZHBO_COMPRESSION_METHOD2	= 9,
	ZHBO_MOD_FILE_TIME1		= 10,
	ZHBO_MOD_FILE_TIME2		= 11,
	ZHBO_MOD_FILE_DATE1		= 12,
	ZHBO_MOD_FILE_DATE2		= 13,
	ZHBO_CRC32_1			= 14,
	ZHBO_CRC32_2			= 15,
	ZHBO_CRC32_3			= 16,
	ZHBO_CRC32_4			= 17,
	ZHBO_COMPRESSED_SIZE1		= 18,
	ZHBO_COMPRESSED_SIZE2		= 19,
	ZHBO_COMPRESSED_SIZE3		= 20,
	ZHBO_COMPRESSED_SIZE4		= 21,
	ZHBO_UNCOMPRESSED_SIZE1		= 22,
	ZHBO_UNCOMPRESSED_SIZE2		= 23,
	ZHBO_UNCOMPRESSED_SIZE3		= 24,
	ZHBO_UNCOMPRESSED_SIZE4		= 25,
	ZHBO_FILENAME_LENGTH1		= 26,
	ZHBO_FILENAME_LENGTH2		= 27,
	ZHBO_EXTRA_FIELD_LENGTH1	= 28,
	ZHBO_EXTRA_FIELD_LENGTH2	= 29,

	ZIP_HEADER_SIZE			= 30,

	ZIP_EOF_HEADER_SIZE		= 22,
	ZIP_CDS_HEADER_SIZE		= 46,
	ZIP_DATA_DESCRIPTOR_SIZE	= 14
};



#define VALIDATE_NUM( NUM, MIN, MAX ) if ( (NUM) < (MIN) || (NUM) > (MAX) ) (NUM) = (MIN);
#define VALIDATE_NUM_MAX( NUM, MIN, MAX ) if ( (NUM) > (MAX) ) (NUM) = (MIN);



static void validate_date (
	mtZip		* const	zip
	)
{
	VALIDATE_NUM ( zip->mod_year,	1980, 2107 );
	VALIDATE_NUM ( zip->mod_month,	1, 12 );
	VALIDATE_NUM ( zip->mod_day,	1, 31 );

	VALIDATE_NUM_MAX ( zip->mod_hour,	0, 23 );
	VALIDATE_NUM_MAX ( zip->mod_minute,	0, 59 );
	VALIDATE_NUM_MAX ( zip->mod_second,	0, 59 );
}



// Convert bytes from "Local file header" to "Central directory structure"
// >0 = reference from LFH, <= 0 = use the negative of this as byte
static int head2head[ZIP_CDS_HEADER_SIZE] = {
	-0x50,				-0x4b,
	-0x01,				-0x02,
	-23,				0,
	ZHBO_VERSION1,			ZHBO_VERSION2,
	0,				0,
	ZHBO_COMPRESSION_METHOD1,	ZHBO_COMPRESSION_METHOD2,
	ZHBO_MOD_FILE_TIME1,		ZHBO_MOD_FILE_TIME2,
	ZHBO_MOD_FILE_DATE1,		ZHBO_MOD_FILE_DATE2,
	ZHBO_CRC32_1,			ZHBO_CRC32_2,
	ZHBO_CRC32_3,			ZHBO_CRC32_4,
	ZHBO_COMPRESSED_SIZE1,		ZHBO_COMPRESSED_SIZE2,
	ZHBO_COMPRESSED_SIZE3,		ZHBO_COMPRESSED_SIZE4,
	ZHBO_UNCOMPRESSED_SIZE1,	ZHBO_UNCOMPRESSED_SIZE2,
	ZHBO_UNCOMPRESSED_SIZE3,	ZHBO_UNCOMPRESSED_SIZE4,
	ZHBO_FILENAME_LENGTH1,		ZHBO_FILENAME_LENGTH2,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0
	};



static int mt_deflate (
	unsigned char	**	const	outbuf,
	uLong		*	const	out_size,
	unsigned char	*	const	inbuf,
	uLong			const	in_size
	)
{
	int		err;
	z_stream	zs;


	memset ( &zs, 0, sizeof ( zs ) );
	zs.next_in = inbuf;
	zs.avail_in = (uInt)in_size;

	err = deflateInit2 ( &zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
		-15, 8, Z_DEFAULT_STRATEGY );

	if ( err )
	{
		return -1;
	}

	out_size[0] = deflateBound ( &zs, in_size );
	if ( out_size[0] < 1 )
	{
		err = -2;
		goto exit;
	}

	outbuf[0] = (unsigned char *)calloc ( 1, out_size[0] );
	if ( ! outbuf[0] )
	{
		err = -3;
		goto exit;
	}

	zs.next_out = outbuf[0];
	zs.avail_out = (uInt)out_size[0];

	err = deflate ( &zs, Z_FINISH );
	if ( err != Z_STREAM_END )		// Only accept full deflate
	{
		err = -4;
		goto exit;
	}

	err = 0;
	out_size[0] = zs.total_out;

exit:
	deflateEnd ( &zs );

	return err;
}


static int mt_inflate (
	unsigned char	**	const	outbuf,
	uInt			const	out_size,
	unsigned char	*	const	inbuf,
	uInt			const	in_size
	)
{
	int		err = 0;
	z_stream	zs;


	memset ( &zs, 0, sizeof ( zs ) );
	zs.next_in = inbuf;
	zs.avail_in = in_size;

	err = inflateInit2 ( &zs, -15 );
	if ( err )
	{
		return MTKIT_ZIP_ERROR_ZLIB;
	}

	// We add an extra byte at end for 0 NUL byte convenience
	outbuf[0] = (unsigned char *)calloc ( 1, ((size_t)out_size) + 1 );
	if ( ! outbuf[0] )
	{
		err = MTKIT_ZIP_ERROR_MEM;

		goto exit;
	}

	zs.next_out = outbuf[0];
	zs.avail_out = out_size;

	err = inflate ( &zs, Z_FINISH );
	if ( err != Z_STREAM_END )	// Only accept full deflate
	{
		err = MTKIT_ZIP_ERROR_ZLIB;
		free ( outbuf[0] );
		outbuf[0] = NULL;

		goto exit;
	}
	err = 0;

exit:
	inflateEnd ( &zs );

	return err;
}

mtZip * mtkit_zip_save_open (
	char	const * const	filename
	)
{
	mtZip		* zip;


	if ( ! filename )
	{
		return NULL;
	}

	zip = (mtZip *)calloc ( sizeof ( mtZip ), 1 );
	if ( ! zip )
	{
		return NULL;
	}

	zip->archive_path = filename;
	zip->archive_file = strrchr ( filename, MTKIT_DIR_SEP );

	if ( ! zip->archive_file )
	{
		zip->archive_file = filename;
	}
	else
	{
		zip->archive_file ++;
	}

	zip->fp = fopen ( filename, "wb" );
	if ( ! zip->fp )
	{
		free ( zip );

		return NULL;
	}

	return zip;
}

int mtkit_zip_save_file (
	mtZip		* const	zip,
	char	const	* const	name,
	void		* const	mem,
	int32_t		const	mem_len,
	int8_t		const	deflate,
	int32_t		const	year,
	int8_t		const	month,
	int8_t		const	day,
	int8_t		const	hour,
	int8_t		const	minute,
	int8_t		const	second
	)
{
	int		i;
	uint32_t	ui,
			buf_size;
	uint8_t		header[ZIP_HEADER_SIZE] = {
				0x50, 0x4b, 0x03, 0x04,
				20, 0		// DOS v2.0
				},
			* buf,
			* buf_free = NULL;
	mtZipFileInfo	* finfo;
	uLong		zbuf_size;


	if (	! zip		||
		zip->error	||
		( ! mem && mem_len > 0 )
		)
	{
		return MTKIT_ZIP_ERROR;
	}

	zip->cfilename	= name;
	zip->mem	= (char *)mem;
	zip->size	= mem_len;
	zip->deflate	= deflate;
	zip->mod_year	= year;
	zip->mod_month	= month;
	zip->mod_day	= day;
	zip->mod_hour	= hour;
	zip->mod_minute	= minute;
	zip->mod_second	= second;

	// Default is to save data without deflating it
	buf		= (unsigned char *)zip->mem;
	buf_size	= (uint32_t)zip->size;
	buf_free	= NULL;
	header[ZHBO_COMPRESSION_METHOD1] = 0;

	if ( zip->deflate && buf_size > 64 )
	{
		// Attempt to deflate the data
		if ( mt_deflate ( &buf_free, &zbuf_size, buf, (uLong)zip->size))
		{
			zip->error = MTKIT_ZIP_ERROR_ZLIB;

			goto error;
		}
		else if ( zbuf_size < (uLong)zip->size )
		{
			// Only use deflated result if it is deflated
			buf = buf_free;
			buf_size = (uint32_t)zbuf_size;
			header[ZHBO_COMPRESSION_METHOD1] = 8;
		}
	}

	// Populate header & write to file
	ui = 0;
	if ( zip->size > 0 )
	{
		ui = (uint32_t)crc32 ( ui, (unsigned char *)zip->mem,
			(uInt)zip->size );
	}

	header[ZHBO_CRC32_1] = (uint8_t)( ui );
	header[ZHBO_CRC32_2] = (uint8_t)( ui >> 8 );
	header[ZHBO_CRC32_3] = (uint8_t)( ui >> 16 );
	header[ZHBO_CRC32_4] = (uint8_t)( ui >> 24 );

	validate_date ( zip );

	ui =	(((uint32_t)zip->mod_second) >> 1) |
		(((uint32_t)zip->mod_minute) << 5) |
		(((uint32_t)zip->mod_hour) << 11);

	header[ZHBO_MOD_FILE_TIME1] = (uint8_t)( ui );
	header[ZHBO_MOD_FILE_TIME2] = (uint8_t)( ui >> 8 );

	ui =	(uint32_t)zip->mod_day |
		( ((uint32_t)zip->mod_month) << 5) |
		( ((uint32_t)zip->mod_year - 1980) << 9 );

	header[ZHBO_MOD_FILE_DATE1] = (uint8_t)( ui );
	header[ZHBO_MOD_FILE_DATE2] = (uint8_t)( ui >> 8 );

	header[ZHBO_COMPRESSED_SIZE1] = (uint8_t)( buf_size );
	header[ZHBO_COMPRESSED_SIZE2] = (uint8_t)( buf_size >> 8 );
	header[ZHBO_COMPRESSED_SIZE3] = (uint8_t)( buf_size >> 16 );
	header[ZHBO_COMPRESSED_SIZE4] = (uint8_t)( buf_size >> 24 );
	header[ZHBO_UNCOMPRESSED_SIZE1] = (uint8_t)( zip->size );
	header[ZHBO_UNCOMPRESSED_SIZE2] = (uint8_t)( zip->size >> 8 );
	header[ZHBO_UNCOMPRESSED_SIZE3] = (uint8_t)( zip->size >> 16 );
	header[ZHBO_UNCOMPRESSED_SIZE4] = (uint8_t)( zip->size >> 24 );

	if ( ! zip->cfilename )
	{
		ui = 0;
	}
	else
	{
		size_t		len = strlen ( zip->cfilename );


		if ( len > 1000000 )
		{
			goto error;
		}

		ui = (uint32_t)len;
	}

	header[ZHBO_FILENAME_LENGTH1] = (uint8_t)( ui );
	header[ZHBO_FILENAME_LENGTH2] = (uint8_t)( ui >> 8 );

	if ( fwrite ( &header, 1, ZIP_HEADER_SIZE, zip->fp ) != ZIP_HEADER_SIZE)
	{
		zip->error = MTKIT_ZIP_ERROR_FILE;

		goto error;
	}

	// Write the dir/filename if one exists
	if ( ui )
	{
		if ( fwrite ( zip->cfilename, 1, ui, zip->fp ) != ui )
		{
			zip->error = MTKIT_ZIP_ERROR_FILE;

			goto error;
		}
	}

	// Create a new mtZipFileInfo, and place it in the list
	finfo = (mtZipFileInfo *)calloc ( 1, sizeof ( mtZipFileInfo ) );
	if ( ! finfo )
	{
		zip->error = MTKIT_ZIP_ERROR_MEM;

		goto error;
	}

	if ( zip->flistn )
	{
		zip->flistn->next = finfo;
		zip->flistn = finfo;
	}
	else
	{
		zip->flist1 = finfo;
	}

	zip->flistn = finfo;

	// Allocate buffer and populate it
	finfo->buf_size = (int)ui + ZIP_CDS_HEADER_SIZE;
	finfo->buf = (unsigned char *)calloc ( 1, (size_t)finfo->buf_size );

	if ( ! finfo->buf )
	{
		zip->error = MTKIT_ZIP_ERROR_MEM;

		goto error;
	}

	for ( i = 0; i < ZIP_CDS_HEADER_SIZE; i++ )
	{
		if ( head2head[i] < 1 )
		{
			finfo->buf[i] = (unsigned char)( -head2head[i] );
		}
		else
		{
			finfo->buf[i] = header[ head2head[i] ];
		}
	}

	finfo->buf[42] = (unsigned char)( zip->cd_start );
	finfo->buf[43] = (unsigned char)( zip->cd_start >> 8 );
	finfo->buf[44] = (unsigned char)( zip->cd_start >> 16 );
	finfo->buf[45] = (unsigned char)( zip->cd_start >> 24 );

	if ( ui )
	{
		memcpy ( finfo->buf + ZIP_CDS_HEADER_SIZE, zip->cfilename, ui );
	}

	if ( buf_size > 0 )
	{
		// Write mem chunk to file - bail out on error
		if ( fwrite ( buf, 1, buf_size, zip->fp ) != buf_size )
		{
			zip->error = MTKIT_ZIP_ERROR_FILE;

			goto error;
		}
	}

	zip->cd_start += ZIP_HEADER_SIZE + ui + buf_size;
	zip->cd_entries ++;

error:
	free ( buf_free );

	return zip->error;		// Success/Fail
}

int mtkit_zip_save_close (
	mtZip		* const	zip
	)
{
	int		res;
	uint32_t	cd_size = 0;
	uint8_t		header[ZIP_HEADER_SIZE] = { 0 };
	mtZipFileInfo	* finfo;


	if ( ! zip )
	{
		return 1;
	}

	res = zip->error;

	cd_size = 0;

	// Create the 'Central directory structure' by reading data from the
	// file info list

	if ( ! res )
	{
		for ( finfo = zip->flist1; finfo; finfo = finfo->next )
		{
			if ( fwrite ( finfo->buf, 1, (size_t)finfo->buf_size,
				zip->fp ) != (size_t)finfo->buf_size )
			{
				res = MTKIT_ZIP_ERROR_FILE;

				break;
			}

			cd_size += (uint32_t)finfo->buf_size;
		}
	}

	if ( ! res )
	{
		// Write 'End of central dir record'
		header[ZHBO_ID1] = 0x50;
		header[ZHBO_ID2] = 0x4b;
		header[ZHBO_ID3] = 0x05;
		header[ZHBO_ID4] = 0x06;
		header[8] = (uint8_t)( zip->cd_entries );
		header[9] = (uint8_t)( zip->cd_entries >> 8 );
		header[10] = (uint8_t)( zip->cd_entries );
		header[11] = (uint8_t)( zip->cd_entries >> 8 );
		header[12] = (uint8_t)( cd_size );
		header[13] = (uint8_t)( cd_size >> 8 );
		header[14] = (uint8_t)( cd_size >> 16 );
		header[15] = (uint8_t)( cd_size >> 24 );
		header[16] = (uint8_t)( zip->cd_start );
		header[17] = (uint8_t)( zip->cd_start >> 8 );
		header[18] = (uint8_t)( zip->cd_start >> 16 );
		header[19] = (uint8_t)( zip->cd_start >> 24 );

		if ( fwrite ( &header, 1, ZIP_EOF_HEADER_SIZE, zip->fp ) !=
			ZIP_EOF_HEADER_SIZE )
		{
			res = MTKIT_ZIP_ERROR_FILE;
		}
	}

	// Destroy the list
	for ( finfo = zip->flist1; finfo; finfo = zip->flist1 )
	{
		free ( finfo->buf );
		zip->flist1 = finfo->next;
		free ( finfo );
	}

	fclose ( zip->fp );
	free ( zip );

	return res;
}

static void flush_zip_mem (		// Release all memory
	mtZip		* const	zip
	)
{
	free ( zip->afilename );
	free ( zip->mem );
	zip->afilename = NULL;
	zip->mem = NULL;
}

int mtkit_zip_load (
	char		const	* const	filename,
	mtZipLoadFunc		const	callback,
	void			* const	user_data
	)
{
	FILE		* fp;
	int		res = 0;
	unsigned char	header[ZIP_HEADER_SIZE], * buf;
	uint32_t	crc1, crc2, size_comp, ui;
	mtZip		zip;


	memset ( &zip, 0, sizeof(zip) );

	if ( ! filename || ! callback )
	{
		return MTKIT_ZIP_ERROR;
	}

	zip.archive_path = filename;
	zip.archive_file = strrchr ( filename, MTKIT_DIR_SEP );

	if ( ! zip.archive_file )
	{
		zip.archive_file = filename;
	}
	else
	{
		zip.archive_file ++;
	}

	fp = fopen ( filename, "rb" );
	if ( ! fp )
	{
		return MTKIT_ZIP_ERROR_FILE;
	}

	do
	{
		if ( ZIP_HEADER_SIZE != fread ( header, 1, ZIP_HEADER_SIZE, fp))
		{
			res = MTKIT_ZIP_ERROR_FILE;
			break;
		}

		// End of central directory structure reached, so stop here
		if (	header[ZHBO_ID1] == 0x50 &&
			header[ZHBO_ID2] == 0x4b &&
			header[ZHBO_ID3] == 0x05 &&
			header[ZHBO_ID4] == 0x06
			)
		{
			break;
		}

		// Central directory structure reached, so stop here
		if (	header[ZHBO_ID1] == 0x50 &&
			header[ZHBO_ID2] == 0x4b &&
			header[ZHBO_ID3] == 0x01 &&
			header[ZHBO_ID4] == 0x02
			)
		{
			break;
		}

		// Invalid header ID so stop here reporting an error
		if (	header[ZHBO_ID1] != 0x50 ||
			header[ZHBO_ID2] != 0x4b ||
			header[ZHBO_ID3] != 0x03 ||
			header[ZHBO_ID4] != 0x04
			)
		{
			res = MTKIT_ZIP_ERROR_FILE;
			break;
		}

		zip.deflate = 0;

		if ( header[ZHBO_COMPRESSION_METHOD1] == 8 )
		{
			zip.deflate = 1;
		}
		else if ( header[ZHBO_COMPRESSION_METHOD1] != 0 )
		{
			res = MTKIT_ZIP_ERROR_FILE;
			break;
		}

		size_comp = (uint32_t)header[ZHBO_COMPRESSED_SIZE1] +
			(((uint32_t)header[ZHBO_COMPRESSED_SIZE2]) << 8) +
			(((uint32_t)header[ZHBO_COMPRESSED_SIZE3]) << 16) +
			(((uint32_t)header[ZHBO_COMPRESSED_SIZE4]) << 24);

		zip.size = (int32_t)header[ZHBO_UNCOMPRESSED_SIZE1] +
			(((int32_t)header[ZHBO_UNCOMPRESSED_SIZE2]) << 8) +
			(((int32_t)header[ZHBO_UNCOMPRESSED_SIZE3]) << 16) +
			(((int32_t)header[ZHBO_UNCOMPRESSED_SIZE4]) << 24);

		ui = (uint32_t)header[ZHBO_MOD_FILE_DATE1] +
			(((uint32_t)header[ZHBO_MOD_FILE_DATE2]) << 8);

		zip.mod_year	= 1980 + (int32_t)((ui >> 9) & 255);
		zip.mod_month	= (ui >> 5) & 15;
		zip.mod_day	= ui & 31;

		ui = (uint32_t)header[ZHBO_MOD_FILE_TIME1] +
			(((uint32_t)header[ZHBO_MOD_FILE_TIME2]) << 8);

		zip.mod_hour	= (ui >> 11) & 127;
		zip.mod_minute	= (ui >> 5) & 63;
		zip.mod_second	= (ui << 1) & 63;

		ui = (uint32_t)header[ZHBO_FILENAME_LENGTH1] +
			(((uint32_t)header[ZHBO_FILENAME_LENGTH2]) << 8);

		if ( ui )
		{
			zip.afilename = (char *)calloc ( 1, ui + 1 );
			if ( ! zip.afilename )
			{
				res = MTKIT_ZIP_ERROR_MEM;
				break;
			}

			if ( ui != fread ( zip.afilename, 1, ui, fp ) )
			{
				res = MTKIT_ZIP_ERROR_FILE;
				break;
			}
		}

		ui = (uint32_t)header[ZHBO_EXTRA_FIELD_LENGTH1] +
			(((uint32_t)header[ZHBO_EXTRA_FIELD_LENGTH2]) << 8);

		if ( ui )
		{
			// skip forward by the extra field length + descriptor
			if ( fseek ( fp, (long)ui, SEEK_CUR ) )
			{
				res = MTKIT_ZIP_ERROR_FILE;
				break;
			}
		}

		// We add an extra byte at end for 0 NUL byte convenience
		zip.mem = (char *)calloc ( 1, ((size_t)size_comp) + 1 );
		if ( ! zip.mem )
		{
			res = MTKIT_ZIP_ERROR_MEM;
			break;
		}

		if ( zip.size > 0 )
		{
			ui = (uint32_t)fread ( zip.mem, 1, size_comp, fp );
			if ( ui != size_comp )
			{
				res = MTKIT_ZIP_ERROR_FILE;
				break;
			}

			if ( zip.deflate )
			{
				res = mt_inflate ( &buf, (uInt)zip.size,
					(unsigned char *)zip.mem, size_comp );

				if ( res )
				{
					break;
				}

				free ( zip.mem );
				zip.mem = (char *)buf;
			}

			// check crc is correct
			crc1 =	(uint32_t)header[ZHBO_CRC32_1] +
				(uint32_t)(header[ZHBO_CRC32_2] << 8) +
				(uint32_t)(header[ZHBO_CRC32_3] << 16) +
				(uint32_t)(header[ZHBO_CRC32_4] << 24);

			crc2 = (uint32_t)crc32 ( 0, (unsigned char *)zip.mem,
				(uInt)zip.size );

			if ( crc1 != crc2 )
			{
				res = MTKIT_ZIP_ERROR_FILE;
				break;
			}
		}

/*
NOTE - these few lines of checks are for when the user created this zip file
WITHOUT using the -D switch and so directories are saved in the file.  We need
to skip these to avoid reading in directories as files.
Zero sized files ending with '/' seem safe to ignore as a directory.
*/

		int i;


		if (	! zip.afilename		||
			zip.afilename[0] == 0	||
			(zip.size == 0 &&
				zip.afilename[ strlen ( zip.afilename ) - 1 ] ==
				'/' )
			)
		{
			goto next;
		}

		i = callback ( zip.afilename, zip.mem, zip.size,
			zip.mod_year, zip.mod_month, zip.mod_day, zip.mod_hour,
			zip.mod_minute, zip.mod_second, user_data );

		if ( i )
		{
			if ( i == MTKIT_ZIP_OK_DONT_FREE )
			{
				// Callback claims the memory
				zip.mem = NULL;
			}
			else
			{
				if ( i == MTKIT_ZIP_STOP )
				{
					res = 0;
				}
				else
				{
					res = i;
				}

				break;	// Stop reading the ZIP file
			}
		}

next:
		// cleanup mem allocs
		flush_zip_mem ( &zip );

		if ( header[ZHBO_GENERAL_FLAG1] & 8 )
		{
			// skip data descriptor if it exists
			if ( fseek ( fp, 12, SEEK_CUR ) )
			{
				res = MTKIT_ZIP_ERROR_FILE;
				break;
			}
		}
	}
	while ( 1 );

	// cleanup mem allocs if we bailed out
	flush_zip_mem ( &zip );
	fclose ( fp );

	return res;
}

