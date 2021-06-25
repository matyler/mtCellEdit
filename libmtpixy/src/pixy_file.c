/*
	Copyright (C) 2008-2020 Mark Tyler

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



int pixy_file_detect_type ( char const * const filename )
{
	unsigned char	buf [ 16 ];
	size_t		len = 0;


	if ( filename )
	{
		FILE * fp = fopen ( filename, "rb" );
		if ( ! fp )
		{
			return PIXY_FILE_TYPE_NONE;
		}

		len = fread ( buf, 1, sizeof(buf), fp );
		buf[ sizeof(buf) - 1 ] = 0;
		fclose ( fp );
	}

	if ( len != sizeof(buf) )
	{
		return PIXY_FILE_TYPE_NONE;
	}

	if ( 0 == memcmp ( buf, "\x89PNG", 4 ) )
	{
		return PIXY_FILE_TYPE_PNG;
	}

	if ( 0 == memcmp ( buf, "\xFF\xD8", 2 ) )
	{
		return PIXY_FILE_TYPE_JPEG;
	}

	if ( 0 == memcmp ( buf, "GIF8", 4 ) )
	{
		return PIXY_FILE_TYPE_GIF;
	}

	if ( 0 == memcmp ( buf, "BM", 2 ) )
	{
		return PIXY_FILE_TYPE_BMP;
	}

	if ( 0 == memcmp ( buf, "GIMP Palette", 12 ) )
	{
		return PIXY_FILE_TYPE_GPL;
	}

	if (	mtkit_strcasestr ( (char *)buf, "xml" )
		|| mtkit_strcasestr ( (char *)buf, "svg" )
		)
	{
		return PIXY_FILE_TYPE_SVG;
	}

	return PIXY_FILE_TYPE_NONE;
}

char const * pixy_file_type_text ( int const type )
{
	char const * const filetype_text[PIXY_FILE_TYPE_MAX + 1] =
		{
			"BMP", "PNG", "JPEG", "GIF", "GPL", "SVG"
		};


	if ( type < PIXY_FILE_TYPE_MIN || type > PIXY_FILE_TYPE_MAX )
	{
		return "";
	}

	return filetype_text[ type ];
}

mtPixmap * pixy_pixmap_load (
	char	const	* const	filename,
	int		* const	file_type
	)
{
	// filename argument checked in pixy_file_detect_type

	int	const	detected = pixy_file_detect_type ( filename );
	mtPixmap	* image = NULL;

	switch ( detected )
	{
	case PIXY_FILE_TYPE_BMP:
		image = pixy_pixmap_load_bmp ( filename );
		break;

	case PIXY_FILE_TYPE_PNG:
		image = pixy_pixmap_load_png ( filename );
		break;

	case PIXY_FILE_TYPE_JPEG:
		image = pixy_pixmap_load_jpeg ( filename );
		break;

	case PIXY_FILE_TYPE_GIF:
		image = pixy_pixmap_load_gif ( filename );
		break;

	case PIXY_FILE_TYPE_GPL:
		image = pixy_pixmap_new_indexed ( 1, 1 );
		if ( image )
		{
			if ( pixy_palette_load ( &image->palette, filename ) )
			{
				pixy_pixmap_destroy ( & image );
			}
			else
			{
				image->palette_file = 1;
			}
		}
		break;

	default:
		break;
	}

	if ( file_type && image )
	{
		file_type[0] = detected;
	}

	return image;
}

int pixy_pixmap_save (
	mtPixmap const	* const	pixmap,
	char	const	* const	filename,
	int		const	file_type,
	int		const	compression
	)
{
	// image and filename args checked in subroutines

	switch ( file_type )
	{
	case PIXY_FILE_TYPE_BMP:
		return pixy_pixmap_save_bmp ( pixmap, filename );

	case PIXY_FILE_TYPE_PNG:
		return pixy_pixmap_save_png ( pixmap, filename, compression );

	case PIXY_FILE_TYPE_JPEG:
		return pixy_pixmap_save_jpeg ( pixmap, filename, compression );

	case PIXY_FILE_TYPE_GIF:
		return pixy_pixmap_save_gif ( pixmap, filename );

	case PIXY_FILE_TYPE_GPL:
		if ( ! pixmap )
		{
			return 1;
		}

		return pixy_palette_save ( &pixmap->palette, filename );

	default:
		// All other types are an error
		break;
	}

	return 1;
}

