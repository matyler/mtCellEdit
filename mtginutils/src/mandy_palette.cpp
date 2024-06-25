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

#include "mandy_palette.h"



MandelPalette::MandelPalette ()
{
	mix_gradient_palette ();
}

MandelPalette::~MandelPalette ()
{
}

void MandelPalette::set_gradients ( size_t const grads )
{
	if ( grads >= GRADIENT_SIZE_MIN && grads <= GRADIENT_SIZE_MAX )
	{
		m_grads = grads;
	}
}

void MandelPalette::set_primary_colors ( std::vector<mtColor> const & cols )
{
	if ( cols.size() >= PRIMARY_SIZE_MIN && cols.size() <= PRIMARY_SIZE_MAX)
	{
		m_primary = cols;
	}
}

void MandelPalette::mix_gradient_palette ()
{
	size_t const cols = m_primary.size();
	size_t const tot = (m_grads + 1) * cols;

	m_palette.resize ( tot );

	size_t c2 = 1;
	size_t pc = 0;

	for ( size_t c1 = 0; c1 < cols; c1++ )
	{
		m_palette[ pc ] = m_primary[ c1 ];

		pc++;

		for ( size_t grad = 1; grad <= m_grads; grad++ )
		{
			double const p1 = (double)grad / (double)(m_grads + 1);
			double const p2 = 1.0 - p1;

			double const r =	p2 * m_primary[c1].red +
						p1 * m_primary[c2].red;

			double const g =	p2 * m_primary[c1].green +
						p1 * m_primary[c2].green;

			double const b =	p2 * m_primary[c1].blue +
						p1 * m_primary[c2].blue;

			m_palette[ pc ].red	= (unsigned char)(r + 0.5);
			m_palette[ pc ].green	= (unsigned char)(g + 0.5);
			m_palette[ pc ].blue	= (unsigned char)(b + 0.5);

			pc++;
		}

		c2 = (c2 + 1) % cols;
	}
}

