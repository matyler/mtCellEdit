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

#include "lanter.h"



TerraMap::TerraMap ()
{
}

TerraMap::~TerraMap ()
{
	set_mem ( nullptr, 0 );
}

void TerraMap::set_mem ( int * const mem, int const rows )
{
	free ( m_mem );

	m_rows = rows;
	m_mem = mem;
}

int TerraMap::set_size ( int const size )
{
	if ( size < MAP_SIZE_MIN || size > MAP_SIZE_MAX )
	{
		std::cerr << "TerraMap::set_size: bad size=" << size << "\n";
		return 1;
	}

	int const rows = (1 << size) + 1;

	int * const mem = (int *)calloc ( (size_t)(rows * rows),
		sizeof(m_mem[0]) );
	if ( ! mem )
	{
		return 1;
	}

	set_mem ( mem, rows );

	return 0;
}

int TerraMap::create_landscape ( int const size )
{
	if ( set_size ( size ) || m_rows < 2 || ! m_mem )
	{
		return 1;
	}

	Uint32 const ticks = SDL_GetTicks ();
	std::cout << "size=" << size << " rows=" << m_rows
		<< " ticks=" << ticks << "\n";

	// Set corner values
	set_value ( 0, 0, get_random_value ( m_rows ) );
	set_value ( 0, m_rows-1, get_random_value ( m_rows ) );
	set_value ( m_rows-1, 0, get_random_value ( m_rows ) );
	set_value ( m_rows-1, m_rows-1, get_random_value ( m_rows ) );

	for ( int gap = m_rows-1; gap > 1; gap /= 2 )
	{
		int const ghalf = gap/2;

		for ( int y = 0; y < m_rows; y += gap )
		{
			for ( int x = ghalf; x < m_rows; x += gap )
			{
				int const v = (
					get_value ( x - ghalf, y ) +
					get_value ( x + ghalf, y )
					) / 2;

				set_value ( x, y, v + get_random_value (gap) );
			}
		}

		for ( int y = ghalf; y < m_rows; y += gap )
		{
			for ( int x = 0; x < m_rows; x += gap )
			{
				int const v = (
					get_value ( x, y - ghalf ) +
					get_value ( x, y + ghalf )
					) / 2;

				set_value ( x, y, v + get_random_value (gap) );
			}
		}

		for ( int y = ghalf; y < m_rows; y += gap )
		{
			for ( int x = ghalf; x < m_rows; x += gap )
			{
				int const v = (
					get_value ( x - ghalf, y - ghalf ) +
					get_value ( x - ghalf, y + ghalf ) +
					get_value ( x + ghalf, y + ghalf ) +
					get_value ( x + ghalf, y - ghalf )
					) / 4;

				set_value ( x, y, v + get_random_value (gap) );
			}
		}
	}

	std::cout << "TerraMap::create_landscape time="
		<< ((double)(SDL_GetTicks () - ticks)) << "\n";

	return 0;
}

int TerraMap::save_tsv ( char const * filename ) const
{
	if ( ! filename )
	{
		return 1;
	}

	CedSheet * sheet = ced_sheet_new ();

	for ( int r = 0; r < m_rows; r++ )
	{
		for ( int c = 0; c < m_rows; c++ )
		{
			ced_sheet_set_cell_value( sheet, r, c, get_value(c, r));
		}
	}

	if ( ced_sheet_save ( sheet, filename, CED_FILE_TYPE_TSV_CONTENT ) )
	{
		std::cerr << "Unable to save TSV file: '" << filename << "\n";
	}

	ced_sheet_destroy ( sheet );

	return 0;
}

