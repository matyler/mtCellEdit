/*
	Copyright (C) 2017-2018 Mark Tyler

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



int mtPixy::Image::save_bp24 (
	char	const * const	filename,
	int		const	compression
	) const
{
	int const bpp = get_canvas_bpp ();
	unsigned char const * rgb = get_canvas ();

	if ( bpp != 3 || ! rgb )
	{
		fprintf ( stderr, "No 24bpp image canvas.\n" );
		return 1;
	}

	unsigned char	* cube;

	cube = mtKit::ByteCube::create_bytecube ( 256 );
	if ( ! cube )
	{
		fprintf ( stderr, "Unable to allocate cube.\n" );
		return 1;
	}

	int const pixtot = get_width () * get_height ();
	unsigned char const * src = rgb;

	for ( int i = 0; i < pixtot; i++ )
	{
		int const r = *src++;
		int const g = *src++;
		int const b = *src++;

		cube[ r + 256*g + 256*256*b ] = 1;
	}

	unsigned char	* buf;
	size_t		buflen;

	if ( mtKit::ByteCube::encode ( cube, &buf, &buflen ) )
	{
		fprintf ( stderr, "Unable to encode cube.\n" );
		free ( cube );
		cube = NULL;
		return 1;
	}

	free ( cube );
	cube = NULL;

	mtKit::ChunkFile::Save file;

	if ( file.open ( filename, "Bp24" ) )
	{
		fprintf ( stderr, "Unable to open file %s\n", filename );
		free ( buf );
		buf = NULL;
		return 1;
	}

	file.set_encoding_deflate ( compression, MTKIT_DEFLATE_MODEL_DEFAULT );

	if ( file.put_chunk ( buf, (uint32_t)buflen, "IceC" ) )
	{
		fprintf ( stderr, "Unable to save chunk to file\n" );
		free ( buf );
		buf = NULL;
		return 1;
	}

	free ( buf );
	buf = NULL;

	return 0;
}

static mtPixy::Image * create_colormap (
	unsigned char		* buf,
	size_t		const	buflen
	)
{
	unsigned char	* cube;
	int	const	cres = mtKit::ByteCube::decode ( buf, buflen, &cube );

	free ( buf );
	buf = NULL;

	if ( cres )
	{
		return NULL;
	}

	int const iw = 4096;
	int const ih = 4096;

	mtPixy::Image * im = mtPixy::Image::create ( mtPixy::Image::TYPE_RGB,
		iw, ih );

	if ( ! im )
	{
		free ( cube );
		cube = NULL;

		return NULL;
	}

	if ( im->create_alpha () )
	{
		free ( cube );
		cube = NULL;

		delete im;
		im = NULL;

		return NULL;
	}

	unsigned char * canvas = im->get_canvas ();
	unsigned char * alpha = im->get_alpha ();

	if ( ! canvas || ! alpha )
	{
		free ( cube );
		cube = NULL;

		delete im;
		im = NULL;

		return NULL;
	}

	for ( int b = 0; b < 256; b++ )
	{
		int const bo = b * 256 * 256;
		int const bx = 256 * (b & 15);
		int const by = 256 * ((b/16) & 15);

		for ( int g = 0; g < 256; g++ )
		{
			int const go = g * 256 + bo;

			for ( int r = 0; r < 256; r++ )
			{
				if ( 0 == cube [ r + go ] )
				{
					continue;
				}

				int const x = r + bx;
				int const y = g + by;

				alpha [ x + iw * y ] = 255;

				int const rgbo = 3*(x + iw * y);

				canvas [ rgbo + 0 ] = (unsigned char)r;
				canvas [ rgbo + 1 ] = (unsigned char)g;
				canvas [ rgbo + 2 ] = (unsigned char)b;
			}
		}
	}

	free ( cube );
	cube = NULL;

	return im;
}

mtPixy::Image * mtPixy::Image::load_bp24 (
	char	const * const	filename
	)
{
	mtKit::ChunkFile::Load	file;
	char			id[mtKit::ChunkFile::CHUNK_HEADER_SIZE];

	if ( file.open ( filename, id ) )
	{
		fprintf ( stderr, "Unable to open file %s\n", filename );
		return NULL;
	}

	if ( 0 != memcmp ( id, "Bp24", 4 ) )
	{
		fprintf ( stderr, "Not a Bp24 chunk file.\n" );
		return NULL;
	}

	while (1)
	{
		uint8_t		* buf;
		uint32_t	buflen;
		int	const	res = file.get_chunk( &buf, &buflen, id, NULL );

		switch ( res )
		{
		case mtKit::ChunkFile::INT_SUCCESS:
			if ( 0 != memcmp ( id, "IceC", 4 ) )
			{
				free ( buf );
				buf = NULL;
				continue;
			}

			// Subroutine free's memory
			return create_colormap ( buf, buflen );

		default:
			return NULL;	// EOF or error means no load happened
		}
	}

	return NULL;		// Nothing loaded
}

