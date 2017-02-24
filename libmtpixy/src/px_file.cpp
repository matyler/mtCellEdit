/*
	Copyright (C) 2016 Mark Tyler

	Code ideas and portions from mtPaint:
	Copyright (C) 2004-2006 Mark Tyler
	Copyright (C) 2006-2016 Dmitry Groshev

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



char const * mtPixy::File::type_text (
	Type	const	t
	)
{
	static char const * const filetype_text[] =
		{
			"BMP", "PNG", "JPEG", "GIF", "GPL"
		};


	if ( t < BMP || t > GPL )
	{
		return "";
	}

	return filetype_text[ t ];
}

mtPixy::File::Type mtPixy::File::detect_type (
	char	const	* const	filename
	)
{
	unsigned char	buf [ 16 ];
	size_t		len = 0;


	if ( filename )
	{
		FILE		* fp;


		fp = fopen ( filename, "rb" );
		if ( ! fp )
		{
			return NONE;
		}

		len = fread ( buf, 1, sizeof(buf), fp );
		fclose ( fp );
	}

	if ( len != sizeof(buf) )
	{
		return NONE;
	}

	if ( 0 == memcmp ( buf, "\x89PNG", 4 ) )
	{
		return PNG;
	}

	if ( 0 == memcmp ( buf, "\xFF\xD8", 2 ) )
	{
		return JPEG;
	}

	if ( 0 == memcmp ( buf, "GIF8", 4 ) )
	{
		return GIF;
	}

	if ( 0 == memcmp ( buf, "BM", 2 ) )
	{
		return BMP;
	}

	if ( 0 == memcmp ( buf, "GIMP Palette", 12 ) )
	{
		return GPL;
	}

	return NONE;
}

mtPixy::Image * mtPixy::image_load (
	char	const	* const	filename,
	File::Type	* const	newtyp
	)
{
	// filename argument checked in subroutines

	File::Type	const	detected = File::detect_type ( filename );
	Image			* image = NULL;

	switch ( detected )
	{
	case File::BMP:
		image = image_load_bmp ( filename );
		break;

	case File::PNG:
		image = image_load_png ( filename );
		break;

	case File::JPEG:
		image = image_load_jpeg ( filename );
		break;

	case File::GIF:
		image = image_load_gif ( filename );
		break;

	case File::GPL:
		image = image_create ( Image::INDEXED, 1, 1 );
		if ( image )
		{
			Palette * pal = image->get_palette ();

			if ( pal->load ( filename ) )
			{
				delete image;
				image = NULL;
			}
			else
			{
				image->set_file_flag (
					Image::FLAG_PALETTE_LOADED );
			}
		}

	default:
		break;
	}

	if ( newtyp && image )
	{
		newtyp[0] = detected;
	}

	return image;
}

int mtPixy::Image::save (
	char	const	* const	filename,
	File::Type	const	filetype,
	int		const	compression
	) const
{
	// image and filename args checked in subroutines

	switch ( filetype )
	{
	case File::BMP:
		return save_bmp ( filename );

	case File::PNG:
		return save_png ( filename, compression );

	case File::JPEG:
		return save_jpeg ( filename, compression );

	case File::GIF:
		return save_gif ( filename );

	case File::GPL:
		return m_palette.save ( filename );

	default:
		// All other types are an error
		break;
	}

	return 1;
}

void mtPixy::Image::set_file_flag (
	int	const	n
	)
{
	m_file_flag = n;
}

int mtPixy::Image::get_file_flag () const
{
	return m_file_flag;
}

