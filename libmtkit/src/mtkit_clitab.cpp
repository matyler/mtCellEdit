/*
	Copyright (C) 2016-2017 Mark Tyler

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



mtKit::CliTab::CliTab ()
	:
	m_root ( new CliItem ( "", nullptr, 0, 0, "", 1 ) )
{
}

mtKit::CliTab::~CliTab ()
{
//	delete m_root;
}

int mtKit::CliTab::add_item (
	char	const * const	command,
	CliFunc		const	func,
	int		const	arg_min,
	int		const	arg_max,
	char	const * const	arg_help,
	int		const	arg_scale
	)
{
	if ( ! command || ! func )
	{
		return 1;
	}

	CliItem		* tmp_item = m_root.get();
	bool		tmp_new = false;

	auto const get_token = [command]( std::string & str, int n )
		{
			char * tc = mtkit_strtok ( command, " ", n );
			if ( ! tc )
			{
				return 1;
			}

			str = tc;
			free ( tc );

			return 0;
		};

	// Find place in tree via command, then add item
	for ( int tokn = 0; ; tokn ++ )
	{
		std::string tk;

		if ( get_token ( tk, tokn ) )
		{
			// No more items in command list so finish here

			if ( ! tmp_new )
			{
				fprintf ( stderr, "CliTab::add_item: %s"
					" already exists\n",
					command );

				return 1;
			}

			if ( tmp_item->set_data ( func, arg_min, arg_max,
				arg_help, arg_scale ) )
			{
				fprintf ( stderr, "CliTab::add_item: Unable to"
					" set_data %s\n",
					command );

				return 1;
			}

			break;
		}

		CliItem * const it = tmp_item->find ( tk );
		if ( it )
		{
			// Node found so drill down
			tmp_item = it;
			continue;
		}

		// Node missing so create it
		auto const new_item = new CliItem (tk, NULL, 0, 0, "", 1);

		if ( tmp_item->add_item ( new_item ) )
		{
			std::cerr << "CliItem '" << tk << "' already defined\n";

			return 1;
		}

		tmp_item = new_item;
		tmp_new = true;
	}

	return 0;
}

int mtKit::CliTab::parse (
	char	const * const	input
	) const
{
	if ( ! input || ! input[0] )
	{
		return 1;
	}

	if ( input[0] == '#' )
	{
		return 0;
	}

	char ** args = mtkit_string_argv ( input );
	if ( ! args )
	{
		fprintf ( stderr,
			"CliTab::parse failure: unable to create argv\n\n" );

		return 1;
	}

	int		res = 0;

	if ( args[0] )
	{
		int		err = 0, ncarg = 0;
		CliItem	const * match = m_root->match_args( args, &err, &ncarg);

		if ( ! match )
		{
			if ( err >= 0 )
			{
				fprintf ( stderr,
					"Unknown command: %s\n\n",
					args[err] );
			}
			else
			{
				fprintf ( stderr,
					"Unknown error after no command match"
					" : %i\n\n", err );
			}
		}
		else
		{
			// Match was made
			if ( err == 0 )
			{
				res = match->callback ( args + ncarg );

				if ( 1 == res )
				{
					// 1 => callback requests us to report
					fprintf ( stderr, "Error running "
						"command '%s'\n\n", input );
				}
				else
				{
					// 2 => callback has reported this error
				}
			}
			else if ( err == -1 )
			{
				fprintf ( stderr, "Too few arguments.\n\n" );
			}
			else if ( err == -2 )
			{
				fprintf ( stderr, "Too many arguments.\n\n" );
			}
			else
			{
				fprintf ( stderr, "Unknown error after"
					" matching command : %i.\n\n", err );
			}
		}
	}

	mtkit_string_argv_free ( args );

	return res;
}

int mtKit::CliTab::print_help (
	char	const * const * const	argv
	) const
{
	if ( ! argv )
	{
		return 1;
	}

	CliItem	const * match = m_root.get();

	for ( int i = 0; argv[i]; i++ )
	{
		match = match->find ( argv[i] );

		if ( ! match )
		{
			fprintf ( stderr, "Unknown command: %s\n", argv[i] );

			return 1;
		}
	}

	match->print_help ();

	return 0;
}

