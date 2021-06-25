/*
	Copyright (C) 2020 Mark Tyler

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



namespace mtKit
{

class ArgCallback;
class ArgDouble;
class ArgInt;
class ArgString;
class ArgStringCpp;
class ArgSwitch;



class ArgCallback : public ArgBase
{
public:
	explicit ArgCallback ( ArgCB const callback ) : m_callback (callback)
	{}

	int action (
		int & ARG_UNUSED ( argi ),
		int ARG_UNUSED ( argc ),
		char const * const * ARG_UNUSED ( argv )
		) const override
	{
		return emit_callback ();
	}

	int emit_callback () const
	{
		if ( m_callback )
		{
			return ( m_callback() ? Arg::ARG_CALLBACK_EXIT : 0 );
		}

		return 0;
	}

protected:
	ArgCB	const	m_callback;
};

class ArgDouble : public ArgCallback
{
public:
	explicit ArgDouble (
		double & variable,
		ArgCB const callback = nullptr
		)
		:
		ArgCallback	( callback ),
		m_variable	( variable )
	{}

	int action (
		int & argi,
		int const argc,
		char const * const * const argv
		) const override
	{
		if (	++argi >= argc
			|| mtkit_strtod ( argv[ argi ], &m_variable, NULL, 1 )
			)
		{
			return Arg::ARG_ERROR;
		}

		return emit_callback ();
	}

private:
	double	& m_variable;
};

class ArgInt : public ArgCallback
{
public:
	explicit ArgInt (
		int & variable,
		ArgCB const callback = nullptr
		)
		:
		ArgCallback	( callback ),
		m_variable	( variable )
	{}

	int action (
		int & argi,
		int const argc,
		char const * const * const argv
		) const override
	{
		if (	++argi >= argc
			|| mtkit_strtoi ( argv[ argi ], &m_variable, NULL, 1 )
			)
		{
			return Arg::ARG_ERROR;
		}

		return emit_callback ();
	}

private:
	int	& m_variable;
};

class ArgString : public ArgCallback
{
public:
	explicit ArgString (
		char const *& variable,
		ArgCB const callback = nullptr
		)
		:
		ArgCallback	( callback ),
		m_variable	( variable )
	{}

	int action (
		int & argi,
		int const argc,
		char const * const * const argv
		) const override
	{
		if ( ++argi >= argc )
		{
			return Arg::ARG_ERROR;
		}

		m_variable = argv[ argi ];

		return emit_callback ();
	}

private:
	char const *& m_variable;
};

class ArgStringCpp : public ArgCallback
{
public:
	explicit ArgStringCpp (
		std::string & variable,
		ArgCB const callback = nullptr
		)
		:
		ArgCallback	( callback ),
		m_variable	( variable )
	{}

	int action (
		int & argi,
		int const argc,
		char const * const * const argv
		) const override
	{
		if ( ++argi >= argc )
		{
			return Arg::ARG_ERROR;
		}

		m_variable = argv[ argi ];

		return emit_callback ();
	}

private:
	std::string & m_variable;
};

class ArgSwitch : public ArgCallback
{
public:
	ArgSwitch (
		int & variable,
		int value,
		ArgCB const callback = nullptr
		)
		:
		ArgCallback	( callback ),
		m_variable	( variable ),
		m_value		( value )
	{}

	int action (
		int & ARG_UNUSED ( argi ),
		int ARG_UNUSED ( argc ),
		char const * const * ARG_UNUSED ( argv )
		) const override
	{
		m_variable = m_value;

		return emit_callback ();
	}

private:
	int		& m_variable;
	int	const	m_value;
};



void Arg::add (
	char	const * const	arg,
	ArgCB		const	cb
	)
{
	add ( arg, new mtKit::ArgCallback ( cb ) );
}

void Arg::add (
	char	const * const	arg,
	double			& var,
	ArgCB		const	cb
	)
{
	add ( arg, new ArgDouble ( var, cb ) );
}

void Arg::add (
	char	const * const	arg,
	int			& var,
	ArgCB		const	cb
	)
{
	add ( arg, new ArgInt ( var, cb ) );
}

void Arg::add (
	char	const * const	arg,
	char		const *& var,
	ArgCB		const	cb
	)
{
	add ( arg, new ArgString ( var, cb ) );
}

void Arg::add (
	char	const * const	arg,
	std::string		& var,
	ArgCB		const	cb
	)
{
	add ( arg, new ArgStringCpp ( var, cb ) );
}

void Arg::add (
	char	const * const	arg,
	int			& var,
	int		const	val,
	ArgCB		const	cb
	)
{
	add ( arg, new ArgSwitch ( var, val, cb ) );
}

}	// namespace mtKit



/// ----------------------------------------------------------------------------



static void err_fn (
	int			const	arg,
	int			const	argc,
	char	const * const * const	argv
	)
{
	fprintf ( stderr, "err_fn: Argument ERROR! - arg=%i/%i", arg, argc );

	if ( arg < argc )
	{
		fprintf ( stderr, " '%s'", argv[arg] );
	}

	fprintf ( stderr, "\n" );
}

mtKit::Arg::Arg (
	ArgFileCB	const	file_func,
	ArgErrorCB	const	error_func
	)
	:
	m_func_file	( file_func ),
	m_func_error	( error_func ? error_func : err_fn )
{
}

int mtKit::Arg::parse (
	int			const	argc,
	char	const * const * const	argv
	) const
{
	for ( int i = 1; i < argc; i++ )
	{
		if ( argv[i][0] != '-' )
		{
			// This argument is not a switch so it must be an input
			// file

			if ( m_func_file )
			{
				if ( m_func_file ( argv[i] ) )
				{
					return ARG_CALLBACK_EXIT;
				}
			}

			continue;
		}

		// Switch found starting with '-'
		auto const it = m_arg.find ( argv[i] + 1 );

		if ( it == m_arg.end () )
		{
			// No such node found so argument hasn't been defined
			// in arg_list

			emit_error ( i, argc, argv );

			return ARG_ERROR;
		}

		ArgBase const &node = *it->second.get ();

		if ( int const res = node.action ( i, argc, argv ) )
		{
			if ( res == ARG_ERROR )
			{
				emit_error ( i, argc, argv );
			}

			return res;
		}
	}

	return ARG_OK;	// Continue
}

void mtKit::Arg::add (
	char	const * const	argument,
	ArgBase	const * const	node
	)
{
	// Temp store which destroys node in the event of an exception
	std::unique_ptr<ArgBase const> ptr ( node );

	// NOTE: this code isn't used as arg duplications aren't detected.
//	m_arg[ argument ] = std::unique_ptr<ArgBase const>( node );
//	ptr.release ();

	// NOTE: this code is used to help detect arg duplications at runtime.

	auto const ret = m_arg.insert ( std::make_pair ( argument,
		std::move (ptr) ) );

	if ( ret.second == false )
	{
		std::cerr << "Argument '" << argument << "' already defined\n";
		throw 123;
	}
}

void mtKit::Arg::emit_error (
	int			const	argi,
	int			const	argc,
	char	const * const * const	argv
	) const
{
	if ( m_func_error )
	{
		m_func_error ( argi, argc, argv );
	}
}

