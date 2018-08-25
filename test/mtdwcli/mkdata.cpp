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

#include "mkdata.h"



class Backend
{
public:
	Backend ();
	~Backend ();

	static int command_line ( int argc, char const * const * argv );
		// 0 = Continue running
		// 1 = Terminate program, returning exit.value()

	inline void set_path ( char const * const path ) { m_path = path; }
	inline char const * get_path () const { return m_path; }
	inline mtKit::Random & get_random () { return m_random; }

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

private:

/// ----------------------------------------------------------------------------

	char		const *	m_path;

	mtKit::Random		m_random;
};



Backend backend;



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	if ( 0 == backend.command_line ( argc, argv ) )
	{
		char const * const path = backend.get_path ();

		if ( path )
		{
			try
			{
				{ CreatePNG png( path, backend.get_random() ); }
				{ CreateFLAC flac( path ); }
			}
			catch ( ... )
			{
				backend.exit.set_value ( 1 );
			}
		}
	}

	return backend.exit.value ();
}

Backend::Backend ()
	:
	m_path ()
{
	m_random.set_seed_by_time ();
}

Backend::~Backend ()
{
}

static int arg_file (
	char	const * const	filename,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	backend.set_path ( filename );

	return 0;			// Continue
}

int Backend::command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	mtArg	const	arg_list[] = {
			{ NULL, 0, NULL, 0, NULL }
			};

	mtkit_arg_parse ( argc, argv, arg_list, arg_file, NULL, NULL );

	return 0;			// Continue
}

