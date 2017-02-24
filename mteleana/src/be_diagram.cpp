/*
	Copyright (C) 2015-2016 Mark Tyler

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

#include "be.h"



#define DIAGRAM_WIDTH		201
#define DIAGRAM_HEIGHT		DIAGRAM_WIDTH

#define DIAGRAM_SECTION		25

#define GREY_HI			192
#define GREY_OTHER		128
#define GREY_LO			64



static void im_rectangle (
	unsigned char *	const	mem,
	int		const	im_w,
	int		const	im_h,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b,
	int		const	x,
	int		const	y,
	int			width,
	int			height
	)
{
	// Sanity checking

	if (	! mem		||
		im_w < 1	||
		im_h < 1	||
		width < 1	||
		height < 1	||
		x >= im_w	||
		y >= im_w
		)
	{
		return;
	}

	// Clipping

	if ( (x + width) > im_w )
	{
		width = im_w - x;
	}

	if ( (y + height) > im_h )
	{
		height = im_h - y;
	}


	int		px, pxmax, py, pymax;
	unsigned char *	dest;


	pxmax = x + width;
	pymax = y + height;

	for ( py = y; py < pymax; py ++ )
	{
		dest = mem + 3 * ( py * im_w + x );

		for ( px = x; px < pxmax; px ++ )
		{
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
		}
	}
}

static void im_pixel (
	unsigned char *	const	mem,
	int		const	im_w,
	int		const	im_h,
	unsigned char	const	r,
	unsigned char	const	g,
	unsigned char	const	b,
	int		const	x,
	int		const	y
	)
{
	// Sanity checking

	if (	! mem		||
		im_w < 1	||
		im_h < 1	||
		x >= im_w	||
		y >= im_w
		)
	{
		return;
	}


	unsigned char *	dest;


	dest = mem + 3 * ( y * im_w + x );

	*dest++ = r;
	*dest++ = g;
	*dest++ = b;
}

static void get_vote_xy (
	int	const	av,
	int	const	bv,
	int	const	ov,
	int	* const	x,
	int	* const	y
	)
{
	int		mab;
	double		p;


	x[0] = y[0] = 0;
	mab = MAX ( av, bv );

	if ( av == bv )
	{
		// Avoid divide by zero
		x[0] = DIAGRAM_WIDTH / 2;
	}
	else
	{
		p = 1 - ((double)av) / ((double)(av + bv));
		x[0] = (int)(p * DIAGRAM_WIDTH);
	}

	if ( mab == ov )
	{
		// Avoid divide by zero
		y[0] = DIAGRAM_HEIGHT / 2;
	}
	else
	{
		p = 1 - ((double)mab) / ((double)(mab + ov));
		y[0] = (int)(p * DIAGRAM_HEIGHT);
	}
}

static int get_vote_abo (
	CedSheet	* const	sheet,
	int			row,
	int		const	rowmax,
	int		* const	va,
	int		* const	vb,
	int		* const	vo,
	char	const * const	pa,
	char	const * const	pb
	)
{
	CedCell		* cell;
	int		v;


	va[0] = vb[0] = vo[0] = 0;

	// Find next seat
	for ( ; row <= rowmax; row++ )
	{
		cell = ced_sheet_get_cell ( sheet, row, FULL_COL_SEAT_NAME );
		if ( cell && cell->text )
		{
			break;
		}
	}

	// Allocate votes to parties A, B, Other
	for ( ; row <= rowmax; row++ )
	{
		v = (int)ced_sheet_get_cell_value( sheet, row, FULL_COL_VOTES );

		cell = ced_sheet_get_cell ( sheet, row, FULL_COL_PARTY );
		if ( ! cell || ! cell->text )
		{
			break;
		}

		if ( 0 == strcmp ( pa, cell->text ) )
		{
			va[0] = v;
		}
		else if ( 0 == strcmp ( pb, cell->text ) )
		{
			vb[0] = v;
		}
		else if ( 0 == vo[0] )
		{
			vo[0] = v;
		}
	}

	return row;
}

mtPixy::Image * eleanaElection::createDiagram (
	eleanaIndex	* const	eindex,
	char	const * const	party_a,
	char	const * const	party_b
	)
{
	mtPixy::Image	* im;
	int		party_a_rgb, a_r, a_g, a_b;
	int		party_b_rgb, b_r, b_g, b_b;
	int		gr_left[3], gr_right[3];
	int		x, y, px, py, row, rowmax, nr;
	int		va, vb, vo;
	unsigned char	* rgb_mem, gr_rgb[3];


	if (	! sheetResults	||
		! party_a	||
		! party_b
		)
	{
		return NULL;
	}

	im = mtPixy::image_create ( mtPixy::Image::RGB, DIAGRAM_WIDTH,
		DIAGRAM_HEIGHT );
	if ( ! im )
	{
		return NULL;
	}

	rgb_mem = im->get_canvas ();
	if ( ! rgb_mem )
	{
		// Should never happen
		return im;
	}

	party_a_rgb = eindex->getPartyRGB ( party_a );
	if ( party_a_rgb == 11842740 )
	{
		party_a_rgb = mtPixy::rgb_2_int ( GREY_HI, GREY_HI, GREY_HI );
	}

	party_b_rgb = eindex->getPartyRGB ( party_b );
	if ( party_b_rgb == 11842740 )
	{
		party_b_rgb = mtPixy::rgb_2_int ( GREY_LO, GREY_LO, GREY_LO );
	}

	a_r = mtPixy::int_2_red ( party_a_rgb );
	a_g = mtPixy::int_2_green ( party_a_rgb );
	a_b = mtPixy::int_2_blue ( party_a_rgb );

	b_r = mtPixy::int_2_red ( party_b_rgb );
	b_g = mtPixy::int_2_green ( party_b_rgb );
	b_b = mtPixy::int_2_blue ( party_b_rgb );

	// Create background

	im_rectangle ( rgb_mem, DIAGRAM_WIDTH, DIAGRAM_HEIGHT, 255, 255, 255,
		DIAGRAM_WIDTH / 2, 0, 1, DIAGRAM_HEIGHT );

	im_rectangle ( rgb_mem, DIAGRAM_WIDTH, DIAGRAM_HEIGHT, 255, 255, 255,
		0, DIAGRAM_HEIGHT / 2, DIAGRAM_WIDTH, 1 );

	for ( y = 0; y < 8; y++ )
	{
		py = DIAGRAM_SECTION * y + y / 4;

		// Calculate gradient colours for left/right edges

		gr_left[0] = ( (7-y)*a_r + y * GREY_OTHER ) / 7;
		gr_left[1] = ( (7-y)*a_g + y * GREY_OTHER ) / 7;
		gr_left[2] = ( (7-y)*a_b + y * GREY_OTHER ) / 7;

		gr_right[0] = ( (7-y)*b_r + y * GREY_OTHER ) / 7;
		gr_right[1] = ( (7-y)*b_g + y * GREY_OTHER ) / 7;
		gr_right[2] = ( (7-y)*b_b + y * GREY_OTHER ) / 7;

		for ( x = 0; x < 8; x++ )
		{
			px = DIAGRAM_SECTION * x + x / 4;

			// Calculate gradient colour for this cell

			gr_rgb[0] = (unsigned char)( ((7-x)*gr_left[0] +
					(x * gr_right[0]))/7 );

			gr_rgb[1] = (unsigned char)( ((7-x)*gr_left[1] +
					(x * gr_right[1]))/7 );

			gr_rgb[2] = (unsigned char)( ((7-x)*gr_left[2] +
					(x * gr_right[2]))/7 );

			im_rectangle ( rgb_mem, DIAGRAM_WIDTH, DIAGRAM_HEIGHT,
				gr_rgb[0], gr_rgb[1], gr_rgb[2],
				px, py, DIAGRAM_SECTION, DIAGRAM_SECTION );
		}
	}

	rowmax = 0;
	ced_sheet_get_geometry ( sheetResults, &rowmax, NULL );

	for ( row = FULL_ROW_PARTY1; row <= rowmax ; )
	{
		nr = get_vote_abo ( sheetResults, row, rowmax, &va, &vb, &vo,
			party_a, party_b );

		row = MAX ( nr, row + 1 );

		get_vote_xy ( va, vb, vo, &x, &y );

		im_pixel(rgb_mem, DIAGRAM_WIDTH, DIAGRAM_HEIGHT, 0, 0, 0, x, y);
	}

	return im;
}
