/*
	Copyright (C) 2016 Mark Tyler

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



mtPixy::Image::Image (
	Type		const	imtype,
	int		const	w,
	int		const	h,
	int		* const	err
	)
	:
	m_type		( imtype ),
	m_canvas_bpp	( 0 ),
	m_file_flag	( 0 ),
	m_palette	( 2 ),
	m_canvas	( NULL ),
	m_alpha		( NULL ),
	m_width		( w ),
	m_height	( h )
{
	int		res = 0;


	if (	w < WIDTH_MIN	||
		w > WIDTH_MAX	||
		h < HEIGHT_MIN	||
		h > HEIGHT_MAX
		)
	{
		res = 1;
		goto finish;
	}

	switch ( imtype )
	{
	case ALPHA:
		res = create_alpha ();
		break;

	case INDEXED:
		m_canvas_bpp = 1;
		res = create_canvas ();
		break;

	case RGB:
		m_canvas_bpp = 3;
		res = create_canvas ();
		break;

	default:
		res = 1;
	}

finish:
	if ( res )
	{
		// Error so tidy up
		m_type = ALPHA;
		m_canvas_bpp = 0;
		m_width = 1;
		m_height = 1;
	}

	if ( err )
	{
		err[0] = res;
	}
}

mtPixy::Image::~Image ()
{
	destroy_canvas ();
	destroy_alpha ();
}

mtPixy::Image * mtPixy::image_create (
	Image::Type	const	imtype,
	int		const	w,
	int		const	h
	)
{
	int		err = 0;
	Image		* image = new Image ( imtype, w, h, &err );


	if ( err )
	{
		delete image;
		return NULL;
	}

	return image;
}

int mtPixy::Image::copy_canvas (
	Image	const * const	im
	)
{
	if (	! im				||
		m_width != im->m_width		||
		m_height != im->m_height	||
		m_type != im->m_type
		)
	{
		return 1;
	}

	if ( ! im->m_canvas )
	{
		free ( m_canvas );
		m_canvas = NULL;
		return 0;
	}

	size_t tot = (size_t)(m_width * m_height * m_canvas_bpp);

	if ( ! m_canvas )
	{
		m_canvas = (unsigned char *)malloc ( tot );
		if ( ! m_canvas )
		{
			return 1;
		}
	}

	memcpy ( m_canvas, im->m_canvas, tot );

	return 0;
}

int mtPixy::Image::copy_alpha (
	Image	const * const	im
	)
{
	if (	! im			||
		m_width != im->m_width	||
		m_height != im->m_height
		)
	{
		return 1;
	}

	if ( ! im->m_alpha )
	{
		free ( m_alpha );
		m_alpha = NULL;
		return 0;
	}

	size_t tot = (size_t)(m_width * m_height);

	if ( ! m_alpha )
	{
		m_alpha = (unsigned char *)malloc ( tot );
		if ( ! m_alpha )
		{
			return 1;
		}
	}

	memcpy ( m_alpha, im->m_alpha, tot );

	return 0;
}

int mtPixy::Image::create_indexed_canvas ()
{
	unsigned char * nc = (unsigned char *)calloc (
		(size_t)(m_width * m_height), 1 );
	if ( ! nc )
	{
		return 1;
	}

	m_type = INDEXED;
	m_canvas_bpp = 1;

	free ( m_canvas );
	m_canvas = nc;

	return 0;
}

int mtPixy::Image::create_rgb_canvas ()
{
	unsigned char * nc = (unsigned char *)calloc (
		(size_t)(m_width * m_height), 3 );
	if ( ! nc )
	{
		return 1;
	}

	m_type = RGB;
	m_canvas_bpp = 3;

	free ( m_canvas );
	m_canvas = nc;

	return 0;
}

int mtPixy::Image::move_alpha_destroy (
	Image	* const	im
	)
{
	if (	! im				||
		m_width != im->m_width		||
		m_height != im->m_height
		)
	{
		return 1;
	}

	free ( m_alpha );
	m_alpha = im->m_alpha;
	im->m_alpha = NULL;
	delete im;

	return 0;
}

mtPixy::Image * mtPixy::Image::duplicate ()
{
	Image		* image = image_create ( m_type, m_width, m_height );


	if ( ! image )
	{
		return NULL;
	}

	image->m_palette.copy ( &m_palette );

	if (	image->copy_canvas ( this )	||
		image->copy_alpha ( this )
		)
	{
		delete image;
		return NULL;
	}

	return image;
}

void mtPixy::Image::set_data (
	int		const	w,
	int		const	h,
	unsigned char	* const	canv,
	unsigned char	* const	alp
	)
{
	m_width = w;
	m_height = h;

	free ( m_canvas );
	m_canvas = canv;

	free ( m_alpha );
	m_alpha = alp;
}

mtPixy::Image * mtPixy::image_from_data (
	Image::Type	const	imtype,
	int		const	w,
	int		const	h,
	unsigned char	* const	canv,
	unsigned char	* const	alp
	)
{
	int		err = 0;
	Image		* image = new Image ( imtype, 1, 1, &err );


	if ( err )
	{
		delete image;
		return NULL;
	}

	image->set_data ( w, h, canv, alp );

	return image;
}

mtPixy::Image * mtPixy::image_from_cairo (
	cairo_surface_t	* const	surface
	)
{
	Image		* image = NULL;
	cairo_format_t	format;
	int		w, h, y, s_stride, d_stride;
	unsigned char	* src, * dest, * s, * d, * s_end;


	src = cairo_image_surface_get_data ( surface );
	if ( ! src )
	{
		return NULL;
	}

	format = cairo_image_surface_get_format ( surface );
	if ( format != CAIRO_FORMAT_ARGB32 && format != CAIRO_FORMAT_RGB24 )
	{
		return NULL;
	}

	w = cairo_image_surface_get_width ( surface );
	h = cairo_image_surface_get_height ( surface );

	image = image_create ( Image::RGB, w, h );
	if ( ! image )
	{
		return NULL;
	}

	dest = image->get_canvas ();
	if ( ! dest )
	{
		delete image;
		return NULL;
	}


	// Copy data from Cairo to mtImage
	s_stride = cairo_image_surface_get_stride ( surface );

	d_stride = w * 3;


	uint32_t	one = 1,
			le = ( (uint8_t *) &one )[0];
			// 1 = Little endian 0 = Big endian


	if ( le ) for ( y = 0; y < h; y++ )	// Little endian
	{
		s = src + y * s_stride;
		d = dest + y * d_stride;
		s_end = s + w * 4;

		for ( ; s < s_end; s += 4 )
		{
			*d++ = s[2];		// Red
			*d++ = s[1];		// Green
			*d++ = s[0];		// Blue
		}
	}
	else for ( y = 0; y < h; y++ )		// Big endian
	{
		s = src + y * s_stride;
		d = dest + y * d_stride;
		s_end = s + w * 4;

		for ( ; s < s_end; s += 4 )
		{
			*d++ = s[1];		// Red
			*d++ = s[2];		// Green
			*d++ = s[3];		// Blue
		}
	}

	return image;
}

mtPixy::Image * mtPixy::Image::resize (
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h
	)
{
	// Arguments are all checked by subroutines

	Image		* i;


	i = image_create ( m_type, w, h );
	if ( ! i )
	{
		return NULL;
	}

	if ( m_alpha && i->create_alpha () )
	{
		delete i;
		return NULL;
	}

	if ( i->paste ( this, -x, -y ) )
	{
		delete i;
		return NULL;
	}

	i->get_palette ()->copy ( &m_palette );

	return i;
}

mtPixy::Image * mtPixy::Image::resize_trim_by_alpha (
	int		&minx,
	int		&miny
	)
{
	if ( ! m_alpha )
	{
		return NULL;
	}

	minx = m_width - 1;
	miny = m_height - 1;
	int maxx = 0;
	int maxy = 0;

	unsigned char const * src = m_alpha;

	for ( int y = 0; y < m_height; y++ )
	{
		for ( int x = 0; x < m_width; x++ )
		{
			if ( *src++ )
			{
				minx = MIN ( x, minx );
				miny = MIN ( y, miny );
				maxx = MAX ( x, maxx );
				maxy = MAX ( y, maxy );
			}
		}
	}

	if (	minx > maxx		||
		miny > maxy		||
		(	minx == 0		&&
			miny == 0		&&
			maxx == (m_width - 1)	&&
			maxy == (m_height - 1)
			)
		)
	{
		return NULL;
	}

	return resize ( minx, miny, maxx - minx + 1, maxy - miny + 1 );
}

int mtPixy::Image::create_canvas ()
{
	unsigned char	* newcan;


	switch ( m_type )
	{
	case INDEXED:
	case RGB:
		break;

	default:
		// Only valid new canvas is Indexed/RGB
		return 1;
	}

	newcan = (unsigned char *)calloc ( (size_t)(m_canvas_bpp * m_width),
		(size_t)m_height );
	if ( ! newcan )
	{
		return 1;
	}

	free ( m_canvas );
	m_canvas = newcan;

	return 0;
}

int mtPixy::Image::create_alpha ()
{
	unsigned char	* newalp;


	newalp = (unsigned char *)calloc ( (size_t)m_width, (size_t)m_height );
	if ( ! newalp )
	{
		return 1;
	}

	free ( m_alpha );
	m_alpha = newalp;

	return 0;
}

void mtPixy::Image::destroy_canvas ()
{
	free ( m_canvas );
	m_canvas = NULL;

	m_canvas_bpp = 0;
	m_type = ALPHA;
}

int mtPixy::Image::destroy_alpha ()
{
	if ( ! m_alpha )
	{
		return 1;
	}

	free ( m_alpha );
	m_alpha = NULL;

	return 0;
}

unsigned char * mtPixy::Image::get_canvas ()
{
	return m_canvas;
}

unsigned char * mtPixy::Image::get_alpha ()
{
	return m_alpha;
}

mtPixy::Image::Type mtPixy::Image::get_type () const
{
	return m_type;
}

int mtPixy::Image::get_canvas_bpp () const
{
	return m_canvas_bpp;
}

int mtPixy::Image::get_width () const
{
	return m_width;
}

int mtPixy::Image::get_height () const
{
	return m_height;
}

mtPixy::Palette * mtPixy::Image::get_palette ()
{
	return &m_palette;
}

void mtPixy::image_print_geometry (
	Image	* const	im,
	char	* const	buf,
	size_t	const	buflen
	)
{
	if ( ! im )
	{
		snprintf ( buf, buflen, "NONE" );
		return;
	}

	switch ( im->get_type () )
	{
	case Image::RGB:
		snprintf ( buf, buflen, "%i x %i x RGB",
			im->get_width (), im->get_height () );
		break;

	case Image::INDEXED:
		snprintf ( buf, buflen, "%i x %i x %i",
			im->get_width (), im->get_height (),
			im->get_palette ()->get_color_total () );
		break;

	default:
		snprintf ( buf, buflen, "%i x %i x ???",
			im->get_width (), im->get_height () );
		break;
	}

	if ( im->get_alpha () )
	{
		mtkit_strnncat ( buf, " + A", buflen );
	}
}

