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



void mtPixy::Image::blit_idx_alpha_blend (
	unsigned char	* const	dest,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h
	)
{
	if ( ! m_alpha )
	{
		blit_idx ( dest, x, y, w, h );
		return;
	}

	if ( INDEXED != m_type || ! m_canvas || ! dest )
	{
		return;
	}

	int	const	dx1 = x < 0 ? 0 : x;
	int	const	dy1 = y < 0 ? 0 : y;
	int	const	dx2 = MIN ( w, x + m_width );
	int	const	dy2 = MIN ( h, y + m_height );
	int	const	sox = x < 0 ? -x : 0;
	int	const	soy = y < 0 ? -y : 0;

	if ( dx1 >= dx2 || dy1 >= dy2 )
	{
		return;
	}

	for ( int dy = dy1; dy < dy2; dy++ )
	{
		int		off = (dy - dy1 + soy) * m_width + sox;
		unsigned char	* d = dest + dy * w + dx1 - off;

		for ( int dx = dx1; dx < dx2 ; dx++, off++ )
		{
			if ( m_alpha[off] > 127 )
			{
				d[off] = m_canvas[off];
			}
		}
	}
}

void mtPixy::Image::blit_idx (
	unsigned char	* const	dest,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h
	)
{
	if ( INDEXED != m_type || ! m_canvas || ! dest )
	{
		return;
	}

	int	const	dx1 = x < 0 ? 0 : x;
	int	const	dy1 = y < 0 ? 0 : y;
	int	const	dx2 = MIN ( w, x + m_width );
	int	const	dy2 = MIN ( h, y + m_height );
	int	const	sox = x < 0 ? -x : 0;
	int	const	soy = y < 0 ? -y : 0;

	if ( dx1 >= dx2 || dy1 >= dy2 )
	{
		return;
	}

	for ( int dy = dy1; dy < dy2; dy++ )
	{
		memcpy ( dest + dy * w + dx1,
			m_canvas + (dy - dy1 + soy) * m_width + sox,
			(size_t)(dx2 - dx1) );
	}
}

void mtPixy::Image::blit_rgb_alpha_blend (
	Color	const * const	pal,
	unsigned char	* const	dest,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	int		const	zs
	)
{
	if ( ! m_alpha )
	{
		blit_rgb ( pal, dest, x, y, w, h, zs );
		return;
	}

	if ( ! m_canvas || ! dest )
	{
		return;
	}

	int		const	dx1 = x < 0 ? 0 : x;
	int		const	dy1 = y < 0 ? 0 : y;
	int		const	dx2 = zs < 0 ?
					MIN ( w, x + m_width / -zs ) :
					MIN ( w, x + m_width * zs );
	int		const	dy2 = zs < 0 ?
					MIN ( h, y + m_height / -zs ) :
					MIN ( h, y + m_height * zs );
	int		const	sox = x < 0 ? -x : 0;
	int		const	soy = y < 0 ? -y : 0;

	unsigned char	const	* s;
	unsigned char	const	* a;
	unsigned char		* d;


	if ( dx1 >= dx2 || dy1 >= dy2 )
	{
		return;
	}

	if ( INDEXED == m_type )
	{
		unsigned char		idx;

		if ( zs < 0 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				int const off = ((dy - dy1 + soy) * -zs) *
					m_width + sox * -zs;
				s = m_canvas + off;
				a = m_alpha + off;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					if ( a[0] > 127 )
					{
						idx = s[0];

						d[0] = pal[idx].red;
						d[1] = pal[idx].green;
						d[2] = pal[idx].blue;
					}

					s += -zs;
					a += -zs;
					d += 3;
				}
			}
		}
		else if ( zs == 1 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				int const off = (dy - dy1 + soy) * m_width +sox;
				s = m_canvas + off;
				a = m_alpha + off;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					if ( a[0] > 127 )
					{
						idx = s[0];

						d[0] = pal[idx].red;
						d[1] = pal[idx].green;
						d[2] = pal[idx].blue;
					}

					s++;
					a++;
					d += 3;
				}
			}
		}
		else if ( zs > 1 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				int const off = ((dy - dy1 + soy) / zs) *
					m_width;
				s = m_canvas + off;
				a = m_alpha + off;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					int const of = (dx - dx1 + sox) / zs;

					if ( a[of] > 127 )
					{
						idx = s[of];

						d[0] = pal[idx].red;
						d[1] = pal[idx].green;
						d[2] = pal[idx].blue;
					}

					d += 3;
				}
			}
		}
	}
	else if ( RGB == m_type )
	{
		if ( zs < 0 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				int const off = ((dy - dy1 + soy) * -zs) *
					m_width + sox * -zs;
				s = m_canvas + 3 * off;
				a = m_alpha + off;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					int const p = a[0];
					int const pp = 255 - p;

					d[0] = (unsigned char)
						((p * s[0] + pp * d[0]) / 255);
					d[1] = (unsigned char)
						((p * s[1] + pp * d[1]) / 255);
					d[2] = (unsigned char)
						((p * s[2] + pp * d[2]) / 255);

					s += 3 * -zs;
					a += -zs;
					d += 3;
				}
			}
		}
		else if ( zs == 1 )
		{
			// This is an optimization for speed at 100%.
			// Could be rolled into code below using ( zs > 0 ).
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				int const off = (dy - dy1 + soy) * m_width +sox;
				s = m_canvas + 3 * off;
				a = m_alpha + off;
				d = dest + 3* (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					int const p = a[0];
					int const pp = 255 - p;

					d[0] = (unsigned char)
						((p * s[0] + pp * d[0]) / 255);
					d[1] = (unsigned char)
						((p * s[1] + pp * d[1]) / 255);
					d[2] = (unsigned char)
						((p * s[2] + pp * d[2]) / 255);

					s += 3;
					a++;
					d += 3;
				}
			}
		}
		else if ( zs > 1 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				int const off = ((dy - dy1 + soy) / zs)*m_width;
				s = m_canvas + 3 * off;
				a = m_alpha + off;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					int const of = (dx - dx1 + sox) / zs;
					unsigned char const * so = s + 3 * of;
					int const p = a[ of ];
					int const pp = 255 - p;

					d[0] = (unsigned char)
						((p * so[0] + pp * d[0]) / 255);
					d[1] = (unsigned char)
						((p * so[1] + pp * d[1]) / 255);
					d[2] = (unsigned char)
						((p * so[2] + pp * d[2]) / 255);

					d += 3;
				}
			}
		}
	}
}

void mtPixy::Image::blit_rgb (
	Color	const * const	pal,
	unsigned char	* const	dest,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	int		const	zs
	)
{
	if ( ! m_canvas || ! dest )
	{
		return;
	}

	int		const	dx1 = x < 0 ? 0 : x;
	int		const	dy1 = y < 0 ? 0 : y;
	int		const	dx2 = zs < 0 ?
				MIN ( w, x + m_width / -zs ) :
				MIN ( w, x + m_width * zs );
	int		const	dy2 = zs < 0 ?
				MIN ( h, y + m_height / -zs ) :
				MIN ( h, y + m_height * zs );
	int		const	sox = x < 0 ? -x : 0;
	int		const	soy = y < 0 ? -y : 0;

	unsigned char	const	* s;
	unsigned char		* d;


	if ( dx1 >= dx2 || dy1 >= dy2 )
	{
		return;
	}

	if ( INDEXED == m_type )
	{
		unsigned char		idx;

		if ( zs < 0 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				s = m_canvas + ((dy - dy1 + soy) * -zs) *
					m_width + sox * -zs;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					idx = s[0];

					*d++ = pal[idx].red;
					*d++ = pal[idx].green;
					*d++ = pal[idx].blue;

					s += -zs;
				}
			}
		}
		else if ( zs == 1 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				s = m_canvas + (dy - dy1 + soy) * m_width + sox;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					idx = *s++;

					*d++ = pal[idx].red;
					*d++ = pal[idx].green;
					*d++ = pal[idx].blue;
				}
			}
		}
		else if ( zs > 1 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				s = m_canvas + ((dy - dy1 + soy) / zs) *m_width;
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					unsigned char const * so =
						s + (dx - dx1 + sox) / zs;

					idx = so[0];

					*d++ = pal[idx].red;
					*d++ = pal[idx].green;
					*d++ = pal[idx].blue;
				}
			}
		}
	}
	else if ( RGB == m_type )
	{
		if ( zs < 0 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				s = m_canvas + 3*( ((dy - dy1 + soy) * -zs) *
					m_width + sox * -zs );
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					*d++ = s[0];
					*d++ = s[1];
					*d++ = s[2];

					s += 3 * -zs;
				}
			}
		}
		else if ( zs == 1 )
		{
			// This is an optimization for speed at 100%.
			// Could be rolled into code below using ( zs > 0 ).
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				s = m_canvas + 3* ((dy - dy1 + soy) * m_width
					+ sox);
				d = dest + 3* (dy * w + dx1);

				memcpy ( d, s, (size_t)(3 * (dx2 - dx1)) );
			}
		}
		else if ( zs > 1 )
		{
			for ( int dy = dy1; dy < dy2; dy++ )
			{
				s = m_canvas + 3*( ((dy - dy1 + soy) / zs) *
					m_width );
				d = dest + 3 * (dy * w + dx1);

				for ( int dx = dx1; dx < dx2; dx++ )
				{
					unsigned char const * const so =
						s + 3 * ((dx - dx1 + sox) / zs);

					*d++ = so[0];
					*d++ = so[1];
					*d++ = so[2];
				}
			}
		}
	}
}

