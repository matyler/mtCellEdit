/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include "butt.h"



#define IMAGE_8BIT_WIDTH	256
#define IMAGE_8BIT_HEIGHT	201
#define IMAGE_16BIT_WIDTH	256
#define IMAGE_16BIT_HEIGHT	256



mtDW::OTPanalysis::OTPanalysis (
	Butt	&butt
	)
	:
	op		( new OTPanalysis::Op ( butt ) )
{
}

mtDW::OTPanalysis::~OTPanalysis ()
{
	delete op;
}

int mtDW::OTPanalysis::init (
	mtKit::unique_ptr<mtPixy::Image> &im_8bit,
	mtKit::unique_ptr<mtPixy::Image> &im_16bit
	)
{
	im_8bit.reset ( mtPixy::Image::create ( mtPixy::Image::TYPE_INDEXED,
		IMAGE_8BIT_WIDTH, IMAGE_8BIT_HEIGHT ) );

	im_16bit.reset ( mtPixy::Image::create ( mtPixy::Image::TYPE_INDEXED,
		IMAGE_16BIT_WIDTH, IMAGE_16BIT_HEIGHT ) );

	if ( ! im_8bit.get () || ! im_16bit.get () )
	{
		return report_error ( ERROR_HEAP_EMPTY );
	}

	im_8bit.get ()->get_palette ()->set_uniform ( 6 );

	mtPixy::Palette	* const pal = im_16bit.get ()->get_palette ();
	mtPixy::Color	* const col = pal->get_color ();

	col[0].red	= 0;
	col[0].green	= 0;
	col[0].blue	= 0;
	col[255].red	= 255;
	col[255].green	= 0;
	col[255].blue	= 0;

	pal->set_color_total ( 256 );
	pal->create_gradient ( 0, 255 );

	return 0;
}

int mtDW::OTPanalysis::analyse_bucket (
	mtPixy::Image	* const	image_8bit,
	mtPixy::Image	* const	image_16bit,
	int		const	bucket
	) const
{
	op->clear_tables ();

	int res = op->analyse_bucket ( bucket );

	if ( 0 == res )
	{
		res = op->analyse_finish ( image_8bit, image_16bit );
	}

	return res;
}

int mtDW::OTPanalysis::analyse_all_buckets (
	mtPixy::Image	* const	image_8bit,
	mtPixy::Image	* const	image_16bit
	) const
{
	op->clear_tables ();

	int res = op->analyse_all_buckets ();

	if ( 0 == res )
	{
		res = op->analyse_finish ( image_8bit, image_16bit );
	}

	return res;
}

void mtDW::OTPanalysis::get_bit_percents ( double &b1, double list[8] ) const
{
	b1 = op->m_bit_1;

	memcpy ( list, op->m_bit_list, sizeof(op->m_bit_list) );
}

double mtDW::OTPanalysis::get_byte_mean () const
{
	return op->m_byte_mean;
}

double const * mtDW::OTPanalysis::get_byte_list () const
{
	return op->m_byte_list;
}

int64_t mtDW::OTPanalysis::get_bucket_size () const
{
	return op->m_bucket_size;
}



/// ----------------------------------------------------------------------------



mtDW::OTPanalysis::Op::Op (
	Butt	&b
	)
	:
	butt		( b ),
	m_byte_mean	( 1.0 / 256.0 )
{
	clear_tables ();
}

mtDW::OTPanalysis::Op::~Op ()
{
}

void mtDW::OTPanalysis::Op::clear_tables ()
{
	m_bucket_size = 0;

	m_bit_1 = 0.0;

	for ( int i = 0; i < 8; i++ )
	{
		m_bit_list[i] = 0.0;
	}

	for ( int i = 0; i < 256; i++ )
	{
		m_byte_list[i] = 0.0;
	}

	memset ( m_bit_count, 0, sizeof ( m_bit_count ) );
	memset ( m_1byte_count, 0, sizeof ( m_1byte_count ) );
	memset ( m_2byte_count, 0, sizeof ( m_2byte_count ) );

	m_old_byte = false;
	m_old_byte_value = 0;
}

int mtDW::OTPanalysis::Op::analyse_bucket (
	int	const	bucket
	)
{
	RETURN_ON_ERROR ( butt.read_set_otp ( butt.get_otp_name (), bucket, 0 ))

	uint8_t	buf[65536];
	int64_t	todo = butt.read_get_bucket_size ();

	while ( todo > 0 )
	{
		int64_t const chunk_size = MIN( (int64_t)sizeof(buf), todo );

		RETURN_ON_ERROR( butt.read_get_data ( buf, (size_t)chunk_size ))

		for ( int i = 0; i < chunk_size; i++ )
		{
			m_bit_count[0] += (buf[i] & 1);
			m_bit_count[1] += ((buf[i] >> 1) & 1);
			m_bit_count[2] += ((buf[i] >> 2) & 1);
			m_bit_count[3] += ((buf[i] >> 3) & 1);
			m_bit_count[4] += ((buf[i] >> 4) & 1);
			m_bit_count[5] += ((buf[i] >> 5) & 1);
			m_bit_count[6] += ((buf[i] >> 6) & 1);
			m_bit_count[7] += ((buf[i] >> 7) & 1);

			m_1byte_count[ buf[i] ] ++;
		}

		if ( m_old_byte )
		{
			m_2byte_count[ m_old_byte_value ][ buf[0] ] ++;
		}

		if ( chunk_size > 1 )
		{
			for ( int i = 0; i < (chunk_size - 1); i++ )
			{
				m_2byte_count[ buf[i] ][ buf[i+1] ] ++;
			}
		}

		m_old_byte = true;
		m_old_byte_value = buf[ chunk_size - 1 ];

		m_bucket_size += chunk_size;
		todo -= chunk_size;
	}

	return 0;
}

int mtDW::OTPanalysis::Op::analyse_all_buckets ()
{
	int const tot_buckets = butt.get_bucket_total ();

	for ( int i = 0; i < tot_buckets; i++ )
	{
		RETURN_ON_ERROR ( analyse_bucket ( i ) )
	}

	return 0;
}

int mtDW::OTPanalysis::Op::analyse_finish (
	mtPixy::Image * const image_8bit,
	mtPixy::Image * const image_16bit
	)
{
	if (	! image_8bit						||
		! image_16bit						||
		image_8bit->get_width () != IMAGE_8BIT_WIDTH		||
		image_8bit->get_height () != IMAGE_8BIT_HEIGHT		||
		image_16bit->get_width () != IMAGE_16BIT_WIDTH		||
		image_16bit->get_height () != IMAGE_16BIT_HEIGHT
		)
	{
		return report_error ( ERROR_ANALYSIS_INSANITY );
	}

	int64_t bit_tot = 0;

	for ( int i = 0; i < 8; i++ )
	{
		bit_tot += m_bit_count[i];

		m_bit_list[i] =
			((double)m_bit_count[i]) / ((double)m_bucket_size);
	}

	m_bit_1 = ((double)bit_tot / 8.0) / ((double)m_bucket_size);

	for ( int i = 0; i < 256; i++ )
	{
		m_byte_list[i] = ((double)m_1byte_count[i]) /
			((double)m_bucket_size);
	}

	unsigned char * canvas;

	canvas = image_8bit->get_canvas ();
	if ( ! canvas )
	{
		return report_error ( ERROR_ANALYSIS_INSANITY );
	}

	int64_t min, max, tot;

	min = INT64_MAX;
	max = INT64_MIN;
	tot = 0;

	// Calculate max/min count for byte frequencies
	for ( int i = 0; i < 256; i++ )
	{
		min = MIN ( m_1byte_count[i], min );
		max = MAX ( m_1byte_count[i], max );
		tot += m_1byte_count[i];
	}

	double range, perc;
	int const y_max = IMAGE_8BIT_HEIGHT - 1;
	int const y_range = IMAGE_8BIT_HEIGHT;
	int64_t const mean = tot / 256;
	unsigned char const GREEN_PIXEL = 18;
	unsigned char const MID_LINE_PIXEL = 43;

	range = (double)(max - min);

	// Clear canvas
	memset ( canvas, 0, IMAGE_8BIT_HEIGHT * IMAGE_8BIT_WIDTH );

	// Average
#define BOUNDED_Y(YY) MAX( 0, MIN( IMAGE_8BIT_HEIGHT - 1, YY) )

	perc = ((double)(mean - min)) / range;
	int const y_ave = BOUNDED_Y( y_max - (int)(perc * y_range) );
	memset ( canvas + y_ave * IMAGE_8BIT_WIDTH, MID_LINE_PIXEL,
		IMAGE_8BIT_WIDTH );

	for ( int x = 0; x < 256; x++ )
	{
		perc = ((double)(m_1byte_count[x] - min)) / range;

		// Y coords flipped vertically
		int const y = BOUNDED_Y( y_max - (int)(perc * y_range) );

		canvas[ x + y*IMAGE_8BIT_WIDTH ] = GREEN_PIXEL;
	}

	canvas = image_16bit->get_canvas ();
	if ( ! canvas )
	{
		return report_error ( ERROR_ANALYSIS_INSANITY );
	}

	min = INT64_MAX;
	max = INT64_MIN;

	// Calculate max/min count for 2byte frequencies
	for ( int j = 0; j < 256; j++ )
	{
		for ( int i = 0; i < 256; i++ )
		{
			min = MIN ( m_2byte_count[i][j], min );
			max = MAX ( m_2byte_count[i][j], max );
		}
	}

	int const lo_col = 0;
	int const hi_col = 255;
	range = (double)(max - min);
	double const col_range = (double)(hi_col - lo_col);

	for ( int j = 0; j < 256; j++ )
	{
		for ( int i = 0; i < 256; i++ )
		{
			perc = ((double)(m_2byte_count[i][j] - min)) / range;

			unsigned char const col = (unsigned char)( lo_col +
				col_range * perc );

			canvas[ i + (j << 8) ] = col;
		}
	}

	return 0;
}

