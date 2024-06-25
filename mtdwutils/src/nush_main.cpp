/*
	Copyright (C) 2022-2024 Mark Tyler

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

#include "nush.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Core	core;

	if ( core.command_line ( argc, argv ) )
	{
		return core.exit.value();
	}

// Future expansion

	return core.exit.value();
}



namespace {

static int print_version ()
{
	printf ( BIN_NAME " - Part of %s\n\n", VERSION );

	return 1;		// Stop parsing
}

static int print_help ()
{
	print_version ();

	printf ("For further information consult the man page "
		"%s(1) or the mtCellEdit Handbook.\n"
		"\n"
		, BIN_NAME );

	return 1;		// Stop parsing
}

}		// namespace {



int Core::evaluate_line ( char const * const text )
{
	int res = 1;

	switch ( m_num_mode )
	{
	case MODE_DOUBLE:
		res = numeval.evaluate_double ( text );
		break;

	case MODE_FLOAT:
		res = numeval.evaluate_float ( text );
		break;

	case MODE_INTEGER:
		res = numeval.evaluate_integer ( text );
		break;

	case MODE_RATIONAL:
		res = numeval.evaluate_rational ( text );
		break;

	default:
		std::cerr << "Bad number mode - " << m_num_mode << "\n";
		return 1;
	}

	if ( res )
	{
		std::cerr << numeval.error() << "\n";
		return 0;
	}

	if ( m_verbose )
	{
		std::cout << text << "\n";
	}

	switch ( m_num_mode )
	{
	case MODE_DOUBLE:
		std::cout << numeval.result_double().to_string() << "\n";
		break;

	case MODE_FLOAT:
		std::cout << numeval.result_float().to_string() << "\n";
		break;

	case MODE_INTEGER:
		std::cout << numeval.result_integer().to_string() << "\n";
		break;

	case MODE_RATIONAL:
		std::cout << numeval.result_rational().to_string() << "\n";
		break;
	}

	return 0;		// Continue parsing next item
}

int Core::print_funcs () const
{
	switch ( m_num_mode )
	{
	case MODE_DOUBLE:
		return numeval.print_double_funcs ();

	case MODE_FLOAT:
		return numeval.print_float_funcs ();

	case MODE_INTEGER:
		return numeval.print_integer_funcs ();

	case MODE_RATIONAL:
		return numeval.print_rational_funcs ();
	}

	std::cerr << "Bad number mode - " << m_num_mode << "\n";

	return 1;
}

int Core::print_vars () const
{
	switch ( m_num_mode )
	{
	case MODE_DOUBLE:
		return numeval.print_double_vars ();

	case MODE_FLOAT:
		return numeval.print_float_vars ();

	case MODE_INTEGER:
		return numeval.print_integer_vars ();

	case MODE_RATIONAL:
		return numeval.print_rational_vars ();
	}

	std::cerr << "Bad number mode - " << m_num_mode << "\n";

	return 1;
}

int Core::command_line (
	int			const	argc,
	char	const * const *	const	argv
	)
{
	mtKit::Arg args ( [this]( char const * const text )
		{
			return evaluate_line ( text );
		} );

	int stop = 0, inum;

	args.add ( "-help",	stop, 1, print_help );
	args.add ( "-version",	stop, 1, print_version );
	args.add ( "b",		inum, [&inum, this]()
		{
			numeval.set_number_size ( inum );

			if ( m_verbose )
			{
				std::cout << "New number bits = "
					<< numeval.num_bits()
					<< " significant digits = "
					<< numeval.result_float().
						get_str_ndigits()
					<< "\n";
			}

			return 0;
		} );
	args.add ( "cli",	[this]() { return cli_mode (); } );
	args.add ( "funcs",	[this]() { return print_funcs (); } );
	args.add ( "i",		m_arg_i, [this]() { return argcb_i(); } );
	args.add ( "integer",	m_num_mode, MODE_INTEGER );
	args.add ( "double",	m_num_mode, MODE_DOUBLE );
	args.add ( "float",	m_num_mode, MODE_FLOAT );
	args.add ( "rational",	m_num_mode, MODE_RATIONAL );
	args.add ( "v",		m_verbose, 1 );
	args.add ( "vars",	[this]() { return print_vars (); } );

	if ( args.parse ( argc, argv ) || stop )
	{
		return 1;		// Quit program
	}

	return 0;			// Continue program
}

int Core::cli_mode ()
{
	while ( false == exit.aborted () )
	{
		std::string const & line = m_clishell.read_line( BIN_NAME" > ");

		if (	m_clishell.finished () ||
			(line[0] == '.' && line[1] == 0)
			)
		{
			break;
		}
		else if ( line[0] == '#' )
		{
			// Comment
		}
		else if ( line[0] )
		{
			// String isn't empty so do something

			m_clishell.add_history ();

			if ( evaluate_line ( line.c_str() ) )
			{
				break;
			}
		}
		else
		{
			// Quietly ignore empty lines
		}
	}

	return 0;		// Continue parsing
}



/// ----------------------------------------------------------------------------

class LoadLine
{
public:
	LoadLine () {}
	~LoadLine () { set_line ( nullptr ); }

	void set_line ( char * txt )	{ free ( m_line ); m_line = txt; }

	void set_fp_stdin ()		{ m_fp = stdin; }
	int set_fp_filename ( char const * filename );

	char * load_line ();
		// nullptr => EOF or error

private:
	char		* m_line = nullptr;	// Owned
	FILE		* m_fp = nullptr;	// Not owned

	mtKit::ByteFileRead	m_file;

	MTKIT_RULE_OF_FIVE( LoadLine )
};



int LoadLine::set_fp_filename ( char const * const filename )
{
	if ( m_file.open ( filename, 0 ) )
	{
		return 1;
	}

	m_fp = m_file.get_fp();

	return 0;
}

char * LoadLine::load_line ()
{
	char * txt = mtkit_file_readline ( m_fp, nullptr, nullptr );

	set_line ( txt );

	return txt;
}

/// ----------------------------------------------------------------------------



int Core::argcb_i ()
{
	if ( ! m_arg_i )
	{
		return 1;
	}

	LoadLine ll;

	if ( m_arg_i[0] == '-' && m_arg_i[1] == 0 )
	{
		ll.set_fp_stdin ();
	}
	else
	{
		if ( ll.set_fp_filename ( m_arg_i ) )
		{
			std::cerr << "Unable to open file '" << m_arg_i
				<< "'\n";
			return 1;
		}
	}

	while ( char * txt = ll.load_line() )
	{
		evaluate_line ( txt );
	}

	return 0;
}

