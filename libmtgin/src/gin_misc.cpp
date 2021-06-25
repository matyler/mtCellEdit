/*
	Copyright (C) 2021 Mark Tyler

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



std::string mtGin::clipboard()
{
	std::string text;

	if ( SDL_HasClipboardText() )
	{
		char * tmp = SDL_GetClipboardText();
		if ( tmp )
		{
			text = tmp;
			SDL_free ( tmp );
		}
	}

	return text;
}

void mtGin::clipboard_set (
	char	const *	const	text
	)
{
	SDL_SetClipboardText ( text );
}

int mtGin::cpu_cores ()
{
	return SDL_GetCPUCount();
}

int mtGin::cpu_endian ()
{
	return (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? 0 : 1;
}

/// CALLBACK -------------------------------------------------------------------

mtGin::Callback::Callback ()
{
}

mtGin::Callback::~Callback ()
{
}

void mtGin::Callback::start ()
{
	m_active = 1;
}

void mtGin::Callback::stop ()
{
	m_active = 0;
}

void mtGin::Callback::set ( CallbackFunc func )
{
	m_func = func;
}

void mtGin::Callback::run () const
{
	if ( m_active && m_func )
	{
		m_func ();
	}
}

/// SURFACE --------------------------------------------------------------------

SDL_Surface * mtGin::surface_from_pixmap ( mtPixmap const * const pixmap )
{
	if ( ! pixmap )
	{
		return nullptr;
	}

	SDL_Surface		* surface = nullptr;
	mtPixy::Pixmap		tmp;
	unsigned char		* canvas;
	unsigned char		* alpha;
	int		const	bpp = pixy_pixmap_get_bytes_per_pixel ( pixmap);
	int		const	w = pixy_pixmap_get_width ( pixmap );
	int		const	h = pixy_pixmap_get_height ( pixmap );

	switch ( bpp )
	{
	case 0:
		tmp.reset ( pixy_pixmap_duplicate ( pixmap ) );
		pixy_pixmap_create_rgb_canvas ( tmp.get() );

		alpha = pixy_pixmap_get_alpha ( tmp.get() );
		canvas = pixy_pixmap_get_canvas ( tmp.get() );
		break;
	case 1:
		tmp.reset ( pixy_pixmap_convert_to_rgb ( pixmap ) );

		alpha = pixy_pixmap_get_alpha ( tmp.get() );
		canvas = pixy_pixmap_get_canvas ( tmp.get() );
		break;
	case 3:
		alpha = pixy_pixmap_get_alpha ( pixmap );
		canvas = pixy_pixmap_get_canvas ( pixmap );
		break;
	default:
		return nullptr;
	}

	if ( ! canvas )
	{
		return nullptr;
	}

	if ( alpha )
	{
		size_t	const	sz = (size_t)( w*h*4 );
		unsigned char * const argb = (unsigned char *)malloc ( sz );

		unsigned char * r = canvas;
		unsigned char * a = alpha;
		unsigned char * dest = argb;
		unsigned char * dest_end = dest + sz;

		for ( ; dest < dest_end; )
		{
			*dest++ = *a++;		// Alpha
			*dest++ = *r++;		// Red
			*dest++ = *r++;		// Green
			*dest++ = *r++;		// Blue
		}

		SDL_Surface * tsurf = SDL_CreateRGBSurfaceFrom ( argb, w, h, 32,
			4*w,

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			0x00ff0000,
			0x0000ff00,
			0x000000ff,
			0xff000000
#else
			0x0000ff00,
			0x00ff0000,
			0xff000000,
			0x000000ff
#endif
			);

		SDL_PixelFormat * fmt = SDL_AllocFormat (
			SDL_PIXELFORMAT_RGBA8888 );

		surface = SDL_ConvertSurface ( tsurf, fmt, 0 );

		SDL_FreeFormat ( fmt );
		SDL_FreeSurface ( tsurf );
		free ( argb );
	}
	else	// No alpha channel
	{
		SDL_Surface * tsurf = SDL_CreateRGBSurfaceFrom ( canvas, w, h,
			24, 3*w,

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			0x00ff0000,
			0x0000ff00,
			0x000000ff,
			0x00000000
#else
			0x000000ff,
			0x0000ff00,
			0x00ff0000,
			0x00000000
#endif
			);

		SDL_PixelFormat * fmt = SDL_AllocFormat (
			SDL_PIXELFORMAT_RGBA8888 );

		surface = SDL_ConvertSurface ( tsurf, fmt, 0 );

		SDL_FreeFormat ( fmt );
		SDL_FreeSurface ( tsurf );
	}

	return surface;
}

