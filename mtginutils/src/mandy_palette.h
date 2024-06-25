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

#ifndef MTMANDY_PALETTE_H_
#define MTMANDY_PALETTE_H_



#include <mtpixy.h>



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API

class MandelPalette;



class MandelPalette
{
public:
	MandelPalette ();	// Default palette created here
	~MandelPalette ();

	enum
	{
		GRADIENT_SIZE_MIN	= 0,
		GRADIENT_SIZE_MAX	= 15,

		PRIMARY_SIZE_MIN	= 2,
		PRIMARY_SIZE_MAX	= 256
	};

	void set_gradients ( size_t grads );
	void set_primary_colors ( std::vector<mtColor> const & cols );

	void mix_gradient_palette ();

	inline std::vector<mtColor> const & get_palette () const
	{
		return m_palette;
	}

private:
	size_t			m_grads = 31;	// Colours between primaries
	std::vector<mtColor>	m_primary {
		{ 160,	32,	32 },	// Red
		{ 224,	224,	64 },	// Yellow
		{ 0,	128,	0 },	// Green
		{ 64,	192,	192 },	// Cyan
		{ 64,	64,	224 },	// Blue
		{ 224,	96,	224 }	// Magenta
		};
	std::vector<mtColor>	m_palette;

	MTKIT_RULE_OF_FIVE( MandelPalette )
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTMANDY_PALETTE_H_

