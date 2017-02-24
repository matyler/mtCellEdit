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

#ifdef U_GIF
	#include <gif_lib.h>
#endif



#include "private.h"



#ifdef U_GIF



#ifndef GIFLIB_MAJOR
	#define GIFLIB_MAJOR 4
#endif

#if GIFLIB_MAJOR >= 5

	#if GIFLIB_MINOR >= 1

		// GIFLIB 5.1+

		#define DGIFOPENFILENAME( X )	DGifOpenFileName ( X, NULL )
		#define DGIFCLOSEFILE( X )	DGifCloseFile ( X, NULL )

		#define EGIFOPENFILENAME( X )	EGifOpenFileName ( X, 0, NULL )
		#define EGIFCLOSEFILE( X )	EGifCloseFile ( X, NULL )

		#define GIFMAKEMAPOBJECT( X )	GifMakeMapObject ( X, NULL )
		#define GIFFREEMAPOBJECT( X )	GifFreeMapObject ( X )

	#else
		// GIFLIB 5.0

		#define DGIFOPENFILENAME( X )	DGifOpenFileName ( X, NULL )
		#define DGIFCLOSEFILE( X )	DGifCloseFile ( X )

		#define EGIFOPENFILENAME( X )	EGifOpenFileName ( X, 0, NULL )
		#define EGIFCLOSEFILE( X )	EGifCloseFile ( X )

		#define GIFMAKEMAPOBJECT( X )	GifMakeMapObject ( X, NULL )
		#define GIFFREEMAPOBJECT( X )	GifFreeMapObject ( X )

	#endif

#else
	// GIFLIB 4.x

	#define DGIFOPENFILENAME( X )	DGifOpenFileName ( X )
	#define DGIFCLOSEFILE( X )	DGifCloseFile ( X )

	#define EGIFOPENFILENAME( X )	EGifOpenFileName ( X, FALSE )
	#define EGIFCLOSEFILE( X )	EGifCloseFile ( X )

	#define GIFMAKEMAPOBJECT( X )	MakeMapObject ( X, NULL )
	#define GIFFREEMAPOBJECT( X )	FreeMapObject ( X )

#endif



mtPixy::Image * mtPixy::image_load_gif (
	char	const	* const	filename
	)
{
	if ( ! filename )
	{
		return NULL;
	}


	// GIF interlace pattern: Y0, DY, ...
	unsigned char	interlace[10] = { 0, 1, 0, 8, 4, 8, 2, 4, 1, 2 };
	unsigned char	* src;
	GifFileType	* giffy;
	GifRecordType	gif_rec;
	GifByteType	* byte_ext;
	ColorMapObject	* cmap = NULL;
	int		i, j, k, kx, n, w, h, dy, val;
	int		frame = 0;
	Image		* image = NULL;


	giffy = DGIFOPENFILENAME ( filename );

	if ( ! giffy )
	{
		return NULL;
	}

	while ( 1 )
	{
		if ( DGifGetRecordType ( giffy, &gif_rec ) == GIF_ERROR )
		{
			goto fail;
		}

		if ( gif_rec == TERMINATE_RECORD_TYPE )
		{
			break;
		}
		else if ( gif_rec == EXTENSION_RECORD_TYPE )
		{
			if ( DGifGetExtension ( giffy, &val, &byte_ext ) ==
				GIF_ERROR )
			{
				goto fail;
			}

			while ( byte_ext )
			{
				if ( DGifGetExtensionNext ( giffy, &byte_ext )
					== GIF_ERROR )
				{
					goto fail;
				}
			}
		}
		else if ( gif_rec == IMAGE_DESC_RECORD_TYPE )
		{
			if ( frame ++ ) // Multipage GIF - notify user
			{
				goto fail;
			}

			if ( DGifGetImageDesc ( giffy ) == GIF_ERROR )
			{
				goto fail;
			}

			// Get palette
			cmap = giffy->SColorMap ? giffy->SColorMap :
				giffy->Image.ColorMap;
			if ( ! cmap )
			{
				goto fail;
			}

			j = cmap->ColorCount;
			if ( j > Palette::COLOR_TOTAL_MAX || j < 1 )
			{
				goto fail;
			}

			w = giffy->Image.Width;
			h = giffy->Image.Height;

			image = image_create ( Image::INDEXED, w, h );
			if ( ! image )
			{
				goto fail;
			}


			Palette		* const pal = image->get_palette ();
			Color		* const pal_col = pal->get_color ();


			pal->set_color_total ( j );

			for ( i = 0; i < j; i++ )
			{
				pal_col[i].red = cmap->Colors[i].Red;
				pal_col[i].green = cmap->Colors[i].Green;
				pal_col[i].blue = cmap->Colors[i].Blue;
			}

			pal->set_correct ();
			image->set_file_flag ( Image::FLAG_PALETTE_LOADED );

			if ( giffy->Image.Interlace )
			{
				k = 2;
				kx = 10;
			}
			else
			{
				k = 0;
				kx = 2;
			}

			src = image->get_canvas ();

			for ( n = 0; k < kx; k += 2 )
			{
				dy = interlace[k + 1];

				for ( i = interlace[k]; i < h; n++ , i += dy )
				{
					if ( DGifGetLine ( giffy, src + i * w, w
						) == GIF_ERROR )
					{
						goto fail;
					}
				}
			}
		}
	}

	DGIFCLOSEFILE ( giffy );

	return image;

fail:
	DGIFCLOSEFILE ( giffy );

	delete image;

	return NULL;
}

int mtPixy::Image::save_gif (
	char	const	* const	filename
	) const
{
	if ( ! filename || m_type != INDEXED )
	{
		return 1;
	}


	ColorMapObject	* gif_map;
	GifFileType	* giffy;
	int		i, coltot;
	Color	const * const palcol = m_palette.get_color ();


	gif_map = GIFMAKEMAPOBJECT ( 256 );

	if ( ! gif_map )
	{
		return 1;
	}

	giffy = EGIFOPENFILENAME ( filename );

	if ( ! giffy )
	{
		goto fail0;
	}

	coltot = m_palette.get_color_total ();

	for ( i = 0; i < coltot; i++ )
	{
		gif_map->Colors[i].Red = palcol[i].red;
		gif_map->Colors[i].Green = palcol[i].green;
		gif_map->Colors[i].Blue = palcol[i].blue;
	}

	for ( ; i < 256; i++ )
	{
		gif_map->Colors[i].Red =
			gif_map->Colors[i].Green =
			gif_map->Colors[i].Blue	= 0;
	}

	if ( EGifPutScreenDesc ( giffy, m_width, m_height, 256, 0, gif_map ) ==
		GIF_ERROR )
	{
		goto fail;
	}

	if ( EGifPutImageDesc ( giffy, 0, 0, m_width, m_height, 0, NULL ) ==
		GIF_ERROR )
	{
		goto fail;
	}

	for ( i = 0; i < m_height; i++ )
	{
		EGifPutLine ( giffy, m_canvas + i * m_width, m_width );
	}

	EGIFCLOSEFILE ( giffy );
	GIFFREEMAPOBJECT ( gif_map );

	return 0;

fail:
	EGIFCLOSEFILE ( giffy );
fail0:
	GIFFREEMAPOBJECT ( gif_map );

	return 1;
}



#else	// U_GIF



mtPixy::Image * mtPixy::image_load_gif (
	char	const	* const	ARG_UNUSED ( filename )
	)
{
	return NULL;
}

int mtPixy::Image::save_gif (
	char	const	* const	ARG_UNUSED ( filename )
	)
{
	return 1;
}



#endif	// U_GIF

