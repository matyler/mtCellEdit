/*
	Copyright (C) 2007-2014 Mark Tyler

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



int mtkit_strtoddt (
	char	const	* const	input,
	double		* const result
	)
{
	int		day = 1,
			month = 1,
			year = 0,
			hour = 0,
			minute = 0,
			second = 0;
	char		* s,
			ch;


	if ( mtkit_strtoi ( input, &day, &s, 0 ) )	// DAY (or possibly HOUR)
	{
		return -1;
	}

	while ( isspace ( s[0] ) )
	{
		s++;
	}

	ch = *s++;
	switch ( ch )
	{
		case ':':		// String starts witha time
			hour = day;
			day = 1;	// Set date to 1/1/0

			goto get_time;

		case '-':
		case '/':		// String starts with a date
			break;

		default:
			return 2;	// Any other char = error
	}

	if ( mtkit_strtoi ( s, &month, &s, 0 ) )	// MONTH
	{
		return 3;
	}

	while ( isspace ( s[0] ) )
	{
		s++;
	}

	ch = *s++;

	if ( ch != '/' && ch != '-' )
	{
		// Not valid date separator

		return 4;
	}

	if ( mtkit_strtoi ( s, &year, &s, 0 ) )
	{
		// YEAR

		return 5;
	}

	if ( day > 31 )
	{
		// If day is > 31 interpret date text as y-m-d

		hour = year;
		year = day;
		day = hour;
		hour = 0;
	}

	while ( isspace ( s[0] ) )
	{
		s++;
	}

	if ( s[0] == 0 )
	{
		// String doesn't contain a time

		goto finish;
	}

	if ( mtkit_strtoi ( s, &hour, &s, 0 ) )
	{
		// HOUR

		return 6;
	}

	while ( isspace ( s[0] ) )
	{
		s++;
	}

	if ( *s++ != ':' )
	{
		// ':' should be present after hour

		return 7;
	}

get_time:

	if ( mtkit_strtoi ( s, &minute, &s, 0 ) )
	{
		// MINUTE

		return 8;
	}

	while ( isspace ( s[0] ) )
	{
		s++;
	}

	if ( s[0] == ':' )		// Second ':' so seconds given
	{
		s++;

		if ( mtkit_strtoi ( s, &second, &s, 0 ) )
		{
			// SECOND

			return 9;
		}
	}

	if ( mtkit_strnonspaces ( s ) )
	{
		// Trailing non-whitespaces are illegal

		return 10;
	}

finish:

	return mtkit_itoddt ( day, month, year, hour, minute, second, result );
}



// WARNING - February must be set by calling func!
static int mlen[12] = { 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



static int mtkit_is_leap_year (
	int	const	year
	)
	// 0 = Not leap year
	// 1 = Is leap year
{
	if ( (year % 400 ) == 0 ) return 1;
	if ( (year % 100 ) == 0 ) return 0;
	if ( (year % 4) == 0 ) return 1;	// Leap year

	return 0;				// Not leap year
}

int mtkit_itoddt (
	int		const	day,
	int		const	month,
	int		const	year,
	int		const	hour,
	int		const	minute,
	int		const	second,
	double		* const	result
	)
{
	int		i,
			j;


	// General bounds
	if ( day < 1 || day > 31 )		return 1;
	if ( month < 1 || month > 12 )		return 1;
	if ( year < 0 )				return 1;
	if ( hour < 0 || hour > 23 )		return 1;
	if ( minute < 0 || minute > 59 )	return 1;
	if ( second < 0 || second > 59 )	return 1;

	// Leap year
	if ( mtkit_is_leap_year ( year ) )
	{
		mlen[1] = 29;
	}
	else
	{
		mlen[1] = 28;
	}

	// Check day is valid for given month
	if ( day > mlen[month-1] )
	{
		return 1;
	}

	if ( result )
	{
		result[0] = ( (double)(hour * 3600 + minute * 60 + second) ) /
			86400;

		// Whole years
		result[0] += 365 * year;

		// Leap days (whole 400 year chunks before this year)
		i = year / 400;

		// 97 leap days per 400 years = 25 + 24 + 24 + 24
		i = i * 97;

		result[0] += i;

		// Leap days (whole century chunk before this year)
		i = year % 400;

		// We have passed the first year every 400 years, e.g. 2000
		if ( i > 0 )
		{
			result[0] += 1;
			// 0, 400, ..., 2000 have leap days
		}

		j = (i - 1) / 100;
		if ( i > 0 )
		{
			// *04, *08, *12, etc have leap days
			// but not 100, 200, 300
			result[0] += (i-1) / 4 - j;
		}

		// Add full months (before this month)
		for ( i = 0; i < ( month - 1 ); i++ )
		{
			result[0] += mlen[i];
		}

		result[0] += day - 1;
	}

	return 0;
}

static void tweak_final_century (
	int		* const	i,
	int		* const	tot,
	int		* const	yr
	)
{
	int		j = 0;


	// Remaining years follow 365, 365, 365, 366 pattern
	if ( i[0] >= 1461 )	// 4 year chunk: 365 + 365 + 365 + 366
	{
		j = i[0] / 1461;
		i[0] -= 1461 * j;
		tot[0] += 1461 * j;
		yr[0] += j * 4;
	}

	if ( i[0] == 1460 )
	{
		// Catch when on last day of fourth year,
		// i.e. 365 + 365 + 365 + 365

		j = 3;
	}
	else
	{
		// 0..3 years, each 365 days

		j = i[0] / 365;
	}

	i[0] -= 365 * j;
	tot[0] += 365 * j;
	yr[0] += j;
}

int mtkit_ddttoi (
	double		const	datetime,
	int		* const	day,
	int		* const	month,
	int		* const	year,
	int		* const	hour,
	int		* const	minute,
	int		* const	second
	)
{
	double		time;
	int		days,
			secs,
			i,
			j,
			tot,
			yr;


	days = (int)datetime;
	if ( datetime < 0 || days < 0 )
	{
		return 1;
	}

	time = datetime - days;
	secs = (int)( 0.5 + time * 86400 );
		// 0.5 rounds as time is always positive (no need for rint)

	if ( second )
	{
		second[0] = secs % 60;
	}

	secs /= 60;

	if ( minute )
	{
		minute[0] = secs % 60;
	}

	secs /= 60;

	if ( hour )
	{
		hour[0] = secs % 24;
	}

	tot = 0;

	i = days / 146097;		// How many whole 4 centuries are here?
	tot += i * 146097;
	yr = 400 * i;

	i = days - tot;			// i = remaining days (0..399 years)

	if ( i >= 36525 )		// 100-399 years left
	{
		i -= 36525;		// Lose the whole of the first century
		tot += 36525;
		yr += 100;

		j = i / 36524;		// Remaining whole centuries
		i -= j * 36524;
		tot += j * 36524;
		yr += j * 100;

		if ( i >= 365 )
		{
			// First year of final century has 365 days
			i -= 365;
			tot += 365;
			yr += 1;

			tweak_final_century ( &i, &tot, &yr );
		}
	}
	else				// 0-99 years left
	{
		if ( i >= 366 )
		{
			// First year of final century has 366 days

			// 1-99 years left

			i -= 366;
			tot += 366;
			yr += 1;

			tweak_final_century ( &i, &tot, &yr );
		}
	}

	// Year found
	if ( year )
	{
		year[0] = yr;
	}

	if ( mtkit_is_leap_year ( yr ) )
	{
		mlen[1] = 29;
	}
	else
	{
		mlen[1] = 28;
	}

	for ( i = 0; i < 12; i++ )
	{
		if ( ( tot + mlen[i] ) > days )
		{
			break;
		}

		tot += mlen[i];
	}

	// Month found
	if ( month )
	{
		month[0] = i + 1;
	}

	// Day is the remainder
	if ( day )
	{
		day[0] = days - tot + 1;
	}

	return 0;
}

int mtkit_ddt_weekday (
	double		const	datetime
	)
{
	int		i = (int)datetime;


	if ( i < 0 )
	{
		return -1;
	}

	return ( ( i + 6 ) % 7 );
}

