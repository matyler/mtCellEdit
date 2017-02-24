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
{
}

mtKit::CliTab::~CliTab ()
{
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

	CliItem		* tmp_item = &m_root;
	bool		tmp_new = false;

	// Find place in tree via command, then add item
	for ( int tokn = 0; ; tokn ++ )
	{
		char * tc = mtkit_strtok ( command, " ", tokn );
		if ( ! tc )
		{
			// No more items in command list so finish here

			if ( ! tmp_new )
			{
				fprintf ( stderr, "CliTab::add_item: %s"
					" already exists\n",
					command );

				return 1;
			}

			if ( tmp_item->set_data ( NULL, func, arg_min,
				arg_max, arg_help, arg_scale ) )
			{
				fprintf ( stderr, "CliTab::add_item: Unable to"
					" set_data %s\n",
					command );

				return 1;
			}

			break;
		}

		CliItem * it = tmp_item->find_item ( tc );
		if ( it )
		{
			// Node found so drill down
			free ( tc );
			tc = NULL;
			tmp_item = it;
			continue;
		}

		// Node missing so create it
		CliItem * new_item = new CliItem;
		if ( new_item->set_data ( tc, NULL, 0, 0, NULL, 1 ) )
		{
			free ( tc );
			tc = NULL;
			delete new_item;

			return 1;
		}

		free ( tc );
		tc = NULL;

		if ( tmp_item->add_item ( new_item ) )
		{
			delete new_item;

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
		CliItem	const * match = m_root.match_args( args, &err, &ncarg );

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

				if ( res )
				{
					fprintf ( stderr, "Error running "
						"command '%s'\n\n", input );
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
	CliItem	const * match = &m_root;

	if ( ! argv )
	{
		return 1;
	}

	for ( int i = 0; argv[i]; i++ )
	{
		match = match->find_item ( argv[i] );

		if ( ! match )
		{
			fprintf ( stderr, "Unknown command: %s\n", argv[i] );

			return 1;
		}
	}

	match->print_help ();

	return 0;
}

