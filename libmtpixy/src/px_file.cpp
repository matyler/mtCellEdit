/*
	Copyright (C) 2016-2017 Mark Tyler

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
	char const * const filetype_text[] =
		{
			"BMP", "PNG", "JPEG", "GIF", "GPL", "PIXY", "BP24"
		};


	if ( t < TYPE_MIN || t > TYPE_MAX )
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
			return TYPE_NONE;
		}

		len = fread ( buf, 1, sizeof(buf), fp );
		fclose ( fp );
	}

	if ( len != sizeof(buf) )
	{
		return TYPE_NONE;
	}

	if ( 0 == memcmp ( buf, "\x89PNG", 4 ) )
	{
		return TYPE_PNG;
	}

	if ( 0 == memcmp ( buf, "\xFF\xD8", 2 ) )
	{
		return TYPE_JPEG;
	}

	if ( 0 == memcmp ( buf, "GIF8", 4 ) )
	{
		return TYPE_GIF;
	}

	if ( 0 == memcmp ( buf, "BM", 2 ) )
	{
		return TYPE_BMP;
	}

	if ( 0 == memcmp ( buf, "GIMP Palette", 12 ) )
	{
		return TYPE_GPL;
	}

	if ( 0 == memcmp ( buf, "\0mtCPixyInfo", 12 ) )
	{
		return TYPE_PIXY;
	}

	if ( 0 == memcmp ( buf, "\0mtCBp24IceC", 12 ) )
	{
		return TYPE_BP24;
	}

	return TYPE_NONE;
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
	case File::TYPE_BMP:
		image = image_load_bmp ( filename );
		break;

	case File::TYPE_PNG:
		image = image_load_png ( filename );
		break;

	case File::TYPE_JPEG:
		image = image_load_jpeg ( filename );
		break;

	case File::TYPE_GIF:
		image = image_load_gif ( filename );
		break;

	case File::TYPE_GPL:
		image = image_create ( Image::TYPE_INDEXED, 1, 1 );
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
		break;

	case File::TYPE_PIXY:
		image = image_load_pixy ( filename );
		break;

	case File::TYPE_BP24:
		image = image_load_bp24 ( filename );
		break;

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
	case File::TYPE_BMP:
		return save_bmp ( filename );

	case File::TYPE_PNG:
		return save_png ( filename, compression );

	case File::TYPE_JPEG:
		return save_jpeg ( filename, compression );

	case File::TYPE_GIF:
		return save_gif ( filename );

	case File::TYPE_GPL:
		return m_palette.save ( filename );

	case File::TYPE_PIXY:
		return save_pixy ( filename, compression );

	case File::TYPE_BP24:
		return save_bp24 ( filename, compression );

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

