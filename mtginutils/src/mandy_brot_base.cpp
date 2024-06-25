/*
	Copyright (C) 2021-2024 Mark Tyler

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

#include "mandy_brot.h"



int MandelbrotDouble::build ()
{
	m_cx		= m_settings.cx.get_number_double ();
	m_cy		= m_settings.cy.get_number_double ();
	m_range_w	= m_settings.range_w.get_number_double ();
	m_range_h	= m_settings.range_h.get_number_double ();

	m_xo		= m_cx - m_range_w / 2.0;
	m_yo		= m_cy - m_range_h / 2.0;

	return build_init ();
}



/// ----------------------------------------------------------------------------



int MandelbrotFloat::build ()
{
	m_cx.set_number ( m_settings.cx );
	m_cy.set_number ( m_settings.cy );
	m_range_w.set_number ( m_settings.range_w );
	m_range_h.set_number ( m_settings.range_h );

	m_xo		= m_cx - m_range_w / 2.0;
	m_yo		= m_cy - m_range_h / 2.0;

	return build_init ();
}

void MandelbrotFloat::build_single_worker ()
{
	mpfr_set_default_prec ( NUMBER_PRECISION_BITS );
	MandelbrotBase::build_single_worker ();
}

void MandelbrotFloat::build_multi_workers ()
{
	mpfr_set_default_prec ( NUMBER_PRECISION_BITS );
	MandelbrotBase::build_multi_workers ();
}



/// ----------------------------------------------------------------------------



int MandelbrotFloatFast::build ()
{
	m_cx.set_number ( m_settings.cx );
	m_cy.set_number ( m_settings.cy );
	m_range_w.set_number ( m_settings.range_w );
	m_range_h.set_number ( m_settings.range_h );

	m_xo		= m_cx - m_range_w / 2.0;
	m_yo		= m_cy - m_range_h / 2.0;

	return build_init ();
}

void MandelbrotFloatFast::build_single_worker ()
{
	mpfr_set_default_prec ( NUMBER_PRECISION_BITS );
	MandelbrotBase::build_single_worker ();
}

void MandelbrotFloatFast::build_multi_workers ()
{
	mpfr_set_default_prec ( NUMBER_PRECISION_BITS );
	MandelbrotBase::build_multi_workers ();
}

size_t MandelbrotFloatFast::calculate ( CalcTmp & tmp ) const
{
	tmp.x2 = m_knum_0;
	tmp.y2 = m_knum_0;
	tmp.x2y2 = m_knum_0;
	tmp.w = m_knum_0;

	size_t i = 0;

	do
	{
		// x1 = x2 - y2 + x
		mpfr_sub ( tmp.x1.get_num(), tmp.x2.get_num(),
			tmp.y2.get_num(), tmp.x2.get_rnd() );
		tmp.x1 += tmp.mbx;

		// y1 = w - x2 - y2 + y
		mpfr_sub( tmp.y1.get_num(), tmp.w.get_num(),
			tmp.x2y2.get_num(), tmp.w.get_rnd() );
		tmp.y1 += tmp.mby;

		// x2 = x1 * x1
		mpfr_mul ( tmp.x2.get_num(), tmp.x1.get_num(),
			tmp.x1.get_num(), tmp.x1.get_rnd() );

		// y2 = y1 * y1
		mpfr_mul ( tmp.y2.get_num(), tmp.y1.get_num(),
			tmp.y1.get_num(), tmp.y1.get_rnd() );

		// w = (x1 + y1) * (x1 + y1)
		mpfr_add ( tmp.w.get_num(), tmp.x1.get_num(),
			tmp.y1.get_num(), tmp.x1.get_rnd() );
		tmp.w *= tmp.w;

		i++;
		if ( i >= m_settings.depth_max )
		{
			return i;
		}

		// x2y2 = (x2 + y2)
		mpfr_add ( tmp.x2y2.get_num(), tmp.x2.get_num(),
			tmp.y2.get_num(), tmp.x2.get_rnd());

	} while ( tmp.x2y2 <= m_knum_4  );
	// while (i < depth_max && (x2 + y2) <= 4.0)

	return i;
}

void MandelbrotFloatFast::render_line ( int const y ) const
{
	size_t	const	pal_size = m_pal.size();
	CalcTmp		tmp;
	unsigned char	* d = m_dest + m_stride * y;


	tmp.y_r.set_number ( y );

	// mby = yo + (y / h_dec) * range_h
	mpfr_mul ( tmp.mby.get_num(), tmp.y_r.get_num(), m_range_h.get_num(),
		tmp.y_r.get_rnd() );
	tmp.mby /= m_h_dec;
	tmp.mby += m_yo;

	tmp.x_r = m_knum_0;

	for ( int x = 0; x < m_w; x++ )
	{
		// mbx = xo + (x / w_dec) * range_w
		mpfr_mul ( tmp.mbx.get_num(),
			tmp.x_r.get_num(),
			m_range_w.get_num(),
			tmp.x_r.get_rnd() );
		tmp.mbx /= m_w_dec;
		tmp.mbx += m_xo;

		size_t	const	i = this->calculate ( tmp );

		unsigned char	r, g, b;

		if ( i < m_settings.depth_max )
		{
			size_t const col = i % pal_size;

			r = m_pal[ col ].red;
			g = m_pal[ col ].green;
			b = m_pal[ col ].blue;
		}
		else
		{
			r = g = b = 0;
		}

		*d++ = r;
		*d++ = g;
		*d++ = b;

		tmp.x_r += m_knum_1;

		switch ( m_settings.thread[0].get_status() )
		{
		case mtGin::Thread::THREAD_TERMINATED:
			return;
		}
	}
}



/// ----------------------------------------------------------------------------



int MandelbrotRational::build ()
{
	m_cx.set_number ( m_settings.cx );
	m_cy.set_number ( m_settings.cy );
	m_range_w.set_number ( m_settings.range_w );
	m_range_h.set_number ( m_settings.range_h );

	m_xo		= m_cx - m_range_w / 2;
	m_yo		= m_cy - m_range_h / 2;

	return build_init ();
}

