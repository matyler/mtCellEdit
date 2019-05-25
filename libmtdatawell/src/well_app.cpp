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

#include "well.h"



void mtDW::Well::app_card_shuffle ( std::string &output ) const
{
	output.clear ();

	WellSaveState wss ( this );

	static int const SUIT_TOTAL = 4;
	static int const CARD_TOTAL = 13;
	static int const PACK_TOTAL = SUIT_TOTAL * CARD_TOTAL;

	static char const * const suits[ SUIT_TOTAL ] =
		{ "♥", "♦", "♣", "♠" };

	static char const * const values[ CARD_TOTAL ] =
		{ "A","2","3","4","5","6","7","8","9","10", "J", "Q", "K" };

	std::vector<int> pack;

	for ( int i = 0; i < PACK_TOTAL; i++ )
	{
		pack.push_back ( i );
	}

	shuffle ( pack );

	for ( size_t i = 0; i < PACK_TOTAL; i++ )
	{
		int const num = pack[ i ];

		output += suits[ num / CARD_TOTAL];
		output += ' ';
		output += values[ num % CARD_TOTAL ];
		output += ' ';
		output += suits[ num / CARD_TOTAL];
		output += '\n';
	}
}

void mtDW::Well::app_coin_toss (
	std::string	&output,
	int		total
	) const
{
	output.clear ();

	WellSaveState wss ( this );

	total = mtkit_int_bound ( total, COIN_TOTAL_MIN, COIN_TOTAL_MAX );

	for ( int i = 0; i < total; )
	{
		int num = get_int ();

		for ( int j = 0; j < 20 && i < total; j++, i++ )
		{
			if ( num & 1 )
			{
				output += "Tail\n";
			}
			else
			{
				output += "Head\n";
			}

			num = num >> 1;
		}
	}
}



double const mtDW::Well::DECLIST_MIN_LO		= -1e100;
double const mtDW::Well::DECLIST_MIN_HI		= 1e100;
double const mtDW::Well::DECLIST_MIN_DEFAULT	= -1;
double const mtDW::Well::DECLIST_MAX_LO		= -1e100;
double const mtDW::Well::DECLIST_MAX_HI		= 1e100;
double const mtDW::Well::DECLIST_MAX_DEFAULT	= 1;



void mtDW::Well::app_declist (
	std::string	&output,
	int		total,
	double		min,
	double		max
	) const
{
	output.clear ();

	WellSaveState wss ( this );

	min = mtkit_double_bound ( min, DECLIST_MIN_LO, DECLIST_MIN_HI );
	max = mtkit_double_bound ( max, DECLIST_MAX_LO, DECLIST_MAX_HI );

	total =	mtkit_int_bound ( total, DECLIST_TOTAL_MIN, DECLIST_TOTAL_MAX );

	if ( min >= max )
	{
		min = DECLIST_MIN_LO;
		max = DECLIST_MAX_HI;
	}

	double const i_range = (double)INT_MAX - (double)INT_MIN;
	double const n_range = max - min;
	double const mult = n_range / i_range;

	for ( int i = 0; i < total; i++ )
	{
		double const num = min +
			((double)get_int () - (double)INT_MIN) * mult;

		char buf[32];
		snprintf ( buf, sizeof(buf), "%.15g", num );

		output += buf;
		output += '\n';
	}
}

void mtDW::Well::app_dice_rolls (
	std::string	&output,
	int		total,
	int		faces
	) const
{
	output.clear ();

	WellSaveState wss ( this );

	total = mtkit_int_bound ( total, DICE_TOTAL_MIN, DICE_TOTAL_MAX );
	faces = mtkit_int_bound ( faces, DICE_FACES_MIN, DICE_FACES_MAX );

	for ( int i = 0; i < total; i++ )
	{
		char buf[32];
		int const num = 1 + get_int ( faces );

		snprintf ( buf, sizeof(buf), "%i", num );

		output += buf;
		output += '\n';
	}
}

void mtDW::Well::app_intlist (
	std::string	&output,
	int		total,
	int		min,
	int		range
	) const
{
	output.clear ();

	WellSaveState wss ( this );

	total =	mtkit_int_bound ( total, INTLIST_TOTAL_MIN, INTLIST_TOTAL_MAX );
	min =	mtkit_int_bound ( min, INTLIST_MIN_LO, INTLIST_MIN_HI );
	range = mtkit_int_bound ( range, INTLIST_RANGE_MIN, INTLIST_RANGE_MAX );

	if ( min > 0 )
	{
		// Ensure (min + range - 1) never overflows INT_MAX
		range = mtkit_int_bound ( range, INTLIST_RANGE_MIN,
			(INTLIST_RANGE_MAX - min + 1) );
	}

	for ( int i = 0; i < total; i++ )
	{
		int const num = min + get_int ( range );

		char buf[32];
		snprintf ( buf, sizeof(buf), "%i", num );

		output += buf;
		output += '\n';
	}
}

void mtDW::Well::app_number_shuffle (
	std::string	&output,
	int		total
	) const
{
	output.clear ();

	WellSaveState wss ( this );
	std::vector<int> pack;

	total = mtkit_int_bound( total, NUMSHUFF_TOTAL_MIN, NUMSHUFF_TOTAL_MAX);

	pack.reserve ( (size_t)total );

	for ( int i = 0; i < total; i++ )
	{
		pack.push_back ( i );
	}

	shuffle ( pack );

	for ( int i = 0; i < total; i++ )
	{
		char buf[32];

		snprintf ( buf, sizeof(buf), "%i", pack[ (size_t)i ] );

		output += buf;
		output += '\n';
	}
}

void mtDW::Well::app_passwords (
	AppPassword	const	&app_pass,
	int		const	char_tot,
	std::string		&output,
	int			total
	) const
{
	output.clear ();

	WellSaveState wss ( this );

	total = mtkit_int_bound( total, PASSWORD_TOTAL_MIN, PASSWORD_TOTAL_MAX);

	for ( int i = 0; i < total; i++ )
	{
		std::string result;

		app_pass.get_password ( this, char_tot, result );

		output += result;
		output += '\n';
	}
}

void mtDW::Well::app_pins (
	std::string	&output,
	int		total,
	int		digits
	) const
{
	output.clear ();

	WellSaveState wss ( this );

	total = mtkit_int_bound ( total, PIN_TOTAL_MIN, PIN_TOTAL_MAX );
	digits = mtkit_int_bound ( digits, PIN_DIGITS_MIN, PIN_DIGITS_MAX );

	for ( int i = 0; i < total; i++ )
	{
		for ( int d = 0; d < digits; )
		{
			int num = get_int ( 1000000 );

			for ( int j = 0; j < 6 && d < digits; j++, d++ )
			{
				char buf[32];
				snprintf ( buf, sizeof(buf), "%i",
					num % 10 );

				output += buf;

				num /= 10;
			}
		}

		output += '\n';
	}
}

