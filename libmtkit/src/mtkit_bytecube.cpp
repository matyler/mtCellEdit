/*
	Copyright (C) 2017 Mark Tyler

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



unsigned char * mtKit::ByteCube::create_bytecube (
	size_t	const	n
	)
{
	if ( n < 2 || n > 256 )
	{
		return NULL;
	}

	unsigned char * mem = (unsigned char *)calloc ( n*n*n, 1 );

	if ( ! mem )
	{
		fprintf ( stderr, "Unable to create cube %i\n", (int)n );
	}

	return mem;
}

int mtKit::ByteCube::count_bits (
	int	const	b
	)
{
	return (	((b>>0)&1) + ((b>>1)&1) + ((b>>2)&1) + ((b>>3)&1) +
			((b>>4)&1) + ((b>>5)&1) + ((b>>6)&1) + ((b>>7)&1) );
}

int mtKit::ByteCube::encode (
	unsigned char	const *	const	mem,
	unsigned char	**	const	buf,
	size_t		*	const	buflen
	)
{
	if ( ! mem || ! buf || ! buflen )
	{
		return 1;
	}

	int		res = 1;
	unsigned char * cf[7] = {
			create_bytecube ( 2 ),
			create_bytecube ( 4 ),
			create_bytecube ( 8 ),
			create_bytecube ( 16 ),
			create_bytecube ( 32 ),
			create_bytecube ( 64 ),
			create_bytecube ( 128 )
			};

	for ( int i = 0; i < 7; i++ )
	{
		if ( ! cf[i] )
		{
			goto error;
		}
	}

	// Populate the new cubes
	{
		unsigned char	const	* src = mem;
		int			ss = 256;	// Source Size

		for ( int i = 6; i >= 0; i-- )
		{
			unsigned char * dest = cf[i];
			int const ds = ss / 2;		// Destination Size
			int const ds2 = ds * ds;

			for ( int b = 0; b < ds; b++ )
			{
				int const bo = ds2 * b;

				int const bs0 = ss*ss*(2*b + 0);
				int const bs1 = ss*ss*(2*b + 1);

				for ( int g = 0; g < ds; g++ )
				{
					int const go = ds * g + bo;

					int const gs0 = ss*(2*g + 0);
					int const gs1 = ss*(2*g + 1);

					for ( int r = 0; r < ds; r++ )
					{
						int const r0 = 2*r + 0;
						int const r1 = 2*r + 1;
						int const tot =
							src[ r0 + gs0 + bs0 ] |
							src[ r1 + gs0 + bs0 ] |
							src[ r0 + gs1 + bs0 ] |
							src[ r1 + gs1 + bs0 ] |
							src[ r0 + gs0 + bs1 ] |
							src[ r1 + gs0 + bs1 ] |
							src[ r0 + gs1 + bs1 ] |
							src[ r1 + gs1 + bs1 ];

						dest[r + go] = tot ? 1 : 0;
					}
				}
			}

			src = dest;
			ss /= 2;
		}
	}

	// Serialize the cubes in turn, and write to memory
	{
		unsigned char const * cro[8] = {
			cf[0], cf[1], cf[2], cf[3], cf[4], cf[5], cf[6],
			mem };
		int btot = 1;

		// Calculate memory size required for each cube
		for ( int i = 0; i < 7; i++ )
		{
			unsigned char	const	* src = cro[i];
			int		const	cs = (1 << (i + 1));
			int		const	mtot = cs * cs * cs;
			int			tot = 0;

			for ( int j = 0; j < mtot; j++, src++ )
			{
				if ( src[0] )
				{
					tot++;
				}
			}

			btot += tot;
		}

		buflen[0] = (size_t)btot;
		buf[0] = (unsigned char *)calloc ( buflen[0], 1 );

		if ( ! buf[0] )
		{
			goto error;
		}

		// Serialize the cubes in turn, putting them into the memory

		unsigned char * dest = buf[0];
		unsigned char const * src = cro[0];

		// Special case the first cube

		*dest++ = (unsigned char)(
			src[0 + 2*0 + 4*0] << 0	|
			src[1 + 2*0 + 4*0] << 1	|
			src[0 + 2*1 + 4*0] << 2	|
			src[1 + 2*1 + 4*0] << 3	|
			src[0 + 2*0 + 4*1] << 4	|
			src[1 + 2*0 + 4*1] << 5	|
			src[0 + 2*1 + 4*1] << 6	|
			src[1 + 2*1 + 4*1] << 7
			);

		for ( int i = 0; i < 7; i++ )
		{
			unsigned char	const	* cube = cro[i];
			int		const	cs = (1 << (i + 1));

			src = cro[i + 1];
			int		const	ss = (1 << (i + 2));

			for ( int b = 0; b < cs; b++ )
			{
				int const bs0 = ss*ss*(2*b + 0);
				int const bs1 = ss*ss*(2*b + 1);

				for ( int g = 0; g < cs; g++ )
				{
					int const gs0 = ss*(2*g + 0);
					int const gs1 = ss*(2*g + 1);

					int const coff = cs*g + cs*cs*b;

					for ( int r = 0; r < cs; r++ )
					{
						if ( 0 == cube[ r + coff ] )
						{
							continue;
						}

						int const r0 = 2*r + 0;
						int const r1 = 2*r + 1;

						int const tot =
						src[ r0 + gs0 + bs0 ] << 0 |
						src[ r1 + gs0 + bs0 ] << 1 |
						src[ r0 + gs1 + bs0 ] << 2 |
						src[ r1 + gs1 + bs0 ] << 3 |
						src[ r0 + gs0 + bs1 ] << 4 |
						src[ r1 + gs0 + bs1 ] << 5 |
						src[ r0 + gs1 + bs1 ] << 6 |
						src[ r1 + gs1 + bs1 ] << 7;

						*dest++ = (unsigned char)tot;
					}
				}
			}
		}
	}

	res = 0;		// Success

error:
	for ( int i = 0; i < 7; i++ )
	{
		free ( cf[i] );
		cf[i] = 0;
	}

	return res;
}

int mtKit::ByteCube::decode (
	unsigned char	const *	const	mem,
	size_t			const	memlen,
	unsigned char	**	const	buf
	)
{
	if ( ! mem || memlen < 1 || ! buf )
	{
		return 1;
	}

	int		res = 1;
	unsigned char * cf[8] = {
			create_bytecube ( 2 ),
			create_bytecube ( 4 ),
			create_bytecube ( 8 ),
			create_bytecube ( 16 ),
			create_bytecube ( 32 ),
			create_bytecube ( 64 ),
			create_bytecube ( 128 ),
			create_bytecube ( 256 )
			};
	unsigned char	const *		src;
	unsigned char	const * const	srclim = mem + memlen;

	for ( int i = 0; i < 8; i++ )
	{
		if ( ! cf[i] )
		{
			goto error;
		}
	}

	// Validate input bytes
	{
		int bytetot = 1;
		src = mem;

		for ( int i = 0; i < 8; i++ )
		{
			if ( src + bytetot > srclim )
			{
				fprintf ( stderr, "Invalid ByteCube stream: "
					"too few bytes.\n" );
				return 1;
			}

			int nbt = 0;

			for ( int j = 0; j < bytetot; j++ )
			{
				nbt += count_bits ( *src++ );
			}

			bytetot = nbt;
		}

		if ( src != srclim )
		{
			fprintf ( stderr, "Invalid ByteCube stream: "
				"too many bytes.\n" );
			return 1;
		}
	}

	// Populate the cubes
	{
		// Special case the first cube
		src = mem;
		int byte = *src++;
		unsigned char * dest = cf[0];

		dest[0 + 2*0 + 4*0] = (byte>>0) & 1;
		dest[1 + 2*0 + 4*0] = (byte>>1) & 1;
		dest[0 + 2*1 + 4*0] = (byte>>2) & 1;
		dest[1 + 2*1 + 4*0] = (byte>>3) & 1;
		dest[0 + 2*0 + 4*1] = (byte>>4) & 1;
		dest[1 + 2*0 + 4*1] = (byte>>5) & 1;
		dest[0 + 2*1 + 4*1] = (byte>>6) & 1;
		dest[1 + 2*1 + 4*1] = (byte>>7) & 1;

		int ss = 2;	// Source Size

		// Expand input stream into the remaining 7 cubes
		for ( int i = 0; i < 7; i++ )
		{
			unsigned char const * cube = cf[i];
			dest = cf[i + 1];

			int const ds = ss * 2;		// Destination Size
			int const ds2 = ds * ds;
			int const ss2 = ss * ss;

			for ( int b = 0; b < ss; b++ )
			{
				int const b0 = ds2*(2*b + 0);
				int const b1 = ds2*(2*b + 1);

				for ( int g = 0; g < ss; g++ )
				{
					int const g0 = ds*(2*g + 0);
					int const g1 = ds*(2*g + 1);

					for ( int r = 0; r < ss; r++ )
					{
						if (0==cube[r + ss*g + ss2*b ] )
						{
							continue;
						}

						byte = *src++;

						int const r0 = 2*r + 0;
						int const r1 = 2*r + 1;

						dest[r0+g0+b0] = (byte>>0) & 1;
						dest[r1+g0+b0] = (byte>>1) & 1;
						dest[r0+g1+b0] = (byte>>2) & 1;
						dest[r1+g1+b0] = (byte>>3) & 1;
						dest[r0+g0+b1] = (byte>>4) & 1;
						dest[r1+g0+b1] = (byte>>5) & 1;
						dest[r0+g1+b1] = (byte>>6) & 1;
						dest[r1+g1+b1] = (byte>>7) & 1;
					}
				}
			}

			ss *= 2;
		}
	}

	res = 0;		// Success

	// Success, so pass back the final cube
	buf[0] = cf[7];
	cf[7] = NULL;

error:
	for ( int i = 0; i < 8; i++ )
	{
		free ( cf[i] );
		cf[i] = NULL;
	}

	return res;
}

