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

#include "well.h"



int mtDW::WellOp::save_file (
	int		const	bytes,
	char	const * const	filename
	)
{
	if ( bytes < 1 || ! filename )
	{
		return 1;
	}

	mtKit::ByteFileWrite file;

	if ( file.open ( filename ) )
	{
		std::cerr << "Unable to open file " << filename << "\n";
		return 1;
	}

	size_t	const	tot = m_prng_buffer.array_len;
	size_t		todo = (size_t)bytes;
	size_t		done = 0;
	bool		files_ok = true;

	mtKit::SqliteTransaction trans ( m_file_db );

	for ( ; todo > 0; todo -= done )
	{
		done = 0;

		if ( m_prng_buffer.pos < m_prng_buffer.tot )
		{
			done = MIN ( todo,
				(m_prng_buffer.tot - m_prng_buffer.pos) );

			if ( file.write ( m_prng_buffer.array +
				m_prng_buffer.pos, done ) )
			{
				std::cerr << "Unable to write to file.\n";
				return 1;
			}

			m_prng_buffer.pos += done;

			continue;
		}

		if ( files_ok )
		{
			if ( m_file.read ( m_file_buffer ) )
			{
				// Don't bother trying again, no files available
				files_ok = false;

				std::cerr << "WellOp::save_file PRNG only\n";
			}
			else
			{
				uint8_t * dest = m_file_buffer.array;

				for ( size_t i = 0; i < tot; i++, dest++ )
				{
					*dest = m_bitshift.get_byte (*dest);
				}
			}
		}

		uint8_t * const pmem = m_prng_buffer.array;

		m_random.get_data ( pmem, tot );

		// PRNG buffer is full again
		m_prng_buffer.tot = tot;
		m_prng_buffer.pos = 0;

		if ( files_ok )
		{
			// We only need to XOR if the files delivered something

			uint8_t const * const fmem = m_file_buffer.array;

			for ( size_t i = 0; i < tot; i++ )
			{
				pmem[i] = fmem[i] ^ pmem[i];
			}
		}
	}

	return 0;
}

