/*
	Copyright (C) 2018 Mark Tyler

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



std::string mtKit::realpath ( std::string const & path )
{
	std::string res;

	char * real = ::realpath ( path.c_str (), NULL );
	if ( real )
	{
		try
		{
			res = real;
		}
		catch ( ... )
		{
		}

		free ( real );
		real = NULL;
	}

	return res;
}

int mtKit::string_from_data (
	std::string		& str,
	void	const * const	data,
	size_t		const	size
	)
{
	if ( ! data )
	{
		return 1;
	}

	int res = 1;

	char * buf = (char *)malloc ( size + 1 );
	if ( buf )
	{
		if ( size > 0 )
		{
			memcpy ( buf, data, size );
		}

		buf[ size ] = 0;

		try
		{
			str = buf;
			res = 0;	// Success
		}
		catch ( ... )
		{
		}

		free ( buf );
		buf = NULL;
	}

	return res;
}

void mtKit::ByteBuf::load ( std::string const &filename )
{
	FILE * fp = fopen ( filename.c_str (), "rb" );
	if ( ! fp )
	{
		tot = 0;
		return;
	}

	tot = fread ( array, 1, array_len, fp );

	fclose ( fp );
	fp = NULL;
}

int mtKit::ByteBuf::save ( std::string const &filename )
{
	FILE * fp = fopen ( filename.c_str (), "wb" );
	if ( ! fp )
	{
		return 1;
	}

	int const res = ( array_len == fwrite ( array, 1, array_len, fp )
			? 0 : 1 );

	fclose ( fp );
	fp = NULL;

	return res;
}

void mtKit::Random::set_seed_by_time ()
{
	m_seed = (uint64_t)time ( NULL );
}

int mtKit::Random::get_int ()
{
	m_seed = m_seed * 6364136223846793005ULL + 1442695040888963407;

	return (int)(m_seed >> 32);
}

int mtKit::Random::get_int ( int const modulo )
{
	// Return an integer from 0 to (modulo - 1) ensuring that it is evenly
	// distributed.

	if ( modulo < 2 )
	{
		return 0;
	}

	int const floor = ((INT_MAX - modulo) + 1) % modulo;

	int res = 0;

	do
	{
		// Lose negative int's
		res = get_int () & INT_MAX;

	} while ( res < floor );

	return (res % modulo);
}

void mtKit::Random::get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	)
{
	uint8_t * const end = buf + buflen;

	for ( uint8_t * dest = buf; dest < end; dest++ )
	{
		dest[0] = (uint8_t)get_int ();
	}
}



#include <vector>



mtKit::BitShifter::BitShifter ()
	:
	m_pos		( 0 ),
	m_shifts	(),
	m_salt		( 0 )
{
	m_shifts[0] = 0;
	m_shifts[1] = 1;
	m_shifts[2] = 2;
	m_shifts[3] = 3;
	m_shifts[4] = 4;
	m_shifts[5] = 5;
	m_shifts[6] = 6;
	m_shifts[7] = 7;
}

mtKit::BitShifter::~BitShifter ()
{
}

int mtKit::BitShifter::set_shifts ( mtKit::Random &random )
{
	static const int BARREL_COMBINATIONS = 8 * 7 * 6 * 5 * 4 * 3 * 2;

	int rand_int = random.get_int ( BARREL_COMBINATIONS );

	try
	{
		std::vector<int> items;

		for ( int i = 0; i < 8; i++ )
		{
			items.push_back ( i );
		}

		for ( int i = 8; i > 0; i-- )
		{
			int const x = rand_int % i;

			m_shifts[ 8 - i ] = items.at ( (size_t)x );
			items.erase ( items.begin () + x );

			rand_int /= i;
		}
	}
	catch ( ... )
	{
		return 1;
	}

	m_salt = 0;

/*
	for ( int i = 0; i < 8; i++ )
	{
		printf("m_shifts[%i] = %i\n", i, m_shifts[i] );
	}
*/

	return 0;
}

int mtKit::BitShifter::set_shifts ( int const shifts[8] )
{
	// Validate that the inputs are 0..7 in some order (no duplicates)
	int val[8] = {0};
	int err = 0;

	for ( int i = 0; i < 8; i++ )
	{
		int const k = shifts[i] & 7;

		err += val[ k ];
		val[ k ]++;
	}

	if ( err )
	{
		// shifts[] contains a duplicate number which is not allowed
		return 1;
	}

	for ( int i = 0; i < 8; i++ )
	{
		m_shifts[i] = shifts[i] & 7;
	}

	return 0;
}

void mtKit::BitShifter::get_shifts ( int shifts[8] ) const
{
	for ( int i = 0; i < 8; i++ )
	{
		shifts[i] = m_shifts[i];
	}
}

uint8_t mtKit::BitShifter::get_byte ( uint8_t const input )
{
	int const s = m_shifts[ m_pos ];
	int const t = m_salt;

	m_pos = (m_pos + 1) & 7;
	m_salt ^= input;

	return (uint8_t)( ( (input << s) | (input >> (8 - s)) ) ^ t );
}

