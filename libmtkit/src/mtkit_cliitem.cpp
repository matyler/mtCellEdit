/*
	Copyright (C) 2016-2020 Mark Tyler

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



int mtKit::CliItem::add_item ( CliItem * const item )
{
	// Needed in case of exception
	std::unique_ptr< CliItem > tmp ( item );

	auto const ret = m_items.insert ( std::make_pair ( item->m_key,
		std::move ( tmp ) ) );

	if ( ret.second == false )
	{
		return 1;
	}

	return 0;
}

int mtKit::CliItem::set_data (
	CliFunc		const	func,
	int		const	arg_min,
	int		const	arg_max,
	char	const * const	arg_help,
	int		const	arg_scale
	)
{
	if ( arg_min < 0 || arg_max < arg_min || arg_scale < 1 )
	{
		return 1;
	}

	if ( func )
	{
		m_func = func;
	}

	m_arg_min = arg_min;
	m_arg_max = arg_max;
	m_arg_scale = arg_scale;

	if ( arg_help )
	{
		m_arg_help = arg_help;
	}

	return 0;
}

mtKit::CliItem * mtKit::CliItem::find ( std::string const & key ) const
{
	auto const it = m_items.find ( key );

	if ( it == m_items.end() )
	{
		return nullptr;
	}

	return it->second.get();
}

mtKit::CliItem const * mtKit::CliItem::match_args (
	char	const * const * const	argv,
	int			* const	cli_error,
	int			* const	ncargs
	) const
{
	if ( ! argv )
	{
		return NULL;
	}

	int		i, errnum = -3;
	CliItem	const	* match = this;

	for ( i = 0; argv[i]; i++ )
	{
		match = match->find ( argv[i] );
		if ( ! match )
		{
			// Unmatched command
			errnum = i;

			goto finish;
		}

		if ( match->m_items.size() == 0 )
		{
			// No more commands in this line so rest must be args
			// (not commands)

			i++;
			break;
		}
	}

	if ( match == this || ! match->m_func )
	{
		// Not enough commands were matched to reach a valid
		// item

		errnum = -1;
		goto finish;
	}

	if ( ncargs )
	{
		ncargs[0] = i;
	}

	int		st;

	// Count args remaining
	for ( st = i; argv[st]; st ++ )
	{
	}

	st = st - i;

	if (	st < ( match->m_arg_min * match->m_arg_scale )	||
		0 != (st % match->m_arg_scale )
		)
	{
		// Too few args
		errnum = -1;
	}
	else if ( st > ( match->m_arg_max * match->m_arg_scale ) )
	{
		// Too many args
		errnum = -2;
	}
	else
	{
		errnum = 0;
	}

finish:
	if ( cli_error )
	{
		cli_error[0] = errnum;
	}

	return match;
}

int mtKit::CliItem::callback (
	char	const * const * const	argv
	) const
{
	if ( ! argv || ! m_func )
	{
		return 1;
	}

	return m_func ( argv );
}

int mtKit::CliItem::print_help_item () const
{
	std::string info = " ... ";

	if ( m_func )
	{
		if ( m_arg_help.size() > 0 )
		{
			info = m_arg_help;
		}
		else
		{
			// Do sub-functions also exist?

			if ( m_items.size() > 0 )
			{
				info = "[...]";
			}
			else
			{
				info = "";
			}
		}
	}

	printf ( "%-14s %s\n", m_key.c_str(), info.c_str() );

	return 0;
}

int mtKit::CliItem::print_help () const
{
	printf ( "\n" );

	for ( auto && item : m_items )
	{
		item.second.get()->print_help_item ();
	}

	printf ( "\n" );

	return 0;
}

