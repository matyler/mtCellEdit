/*
	Copyright (C) 2012-2022 Mark Tyler

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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <langinfo.h>
#include <locale.h>
#include <iconv.h>

// Enforce strictness
#define ZLIB_CONST

#include <zlib.h>



#include "mtkit.h"



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif



#ifdef __cplusplus

namespace mtKit
{



class CliItem
{
public:
	CliItem (
		std::string const & key,
		CliFunc func,
		int arg_min,
		int arg_max,
		std::string const & arg_help,
		int arg_scale
		)
		:
		m_key		( key ),
		m_func		( func ),
		m_arg_min	( arg_min ),
		m_arg_max	( arg_max ),
		m_arg_help	( arg_help ),
		m_arg_scale	( arg_scale )
		{}
	~CliItem () {}

	int add_item ( CliItem * item ); // "item" deleted on fail.

	int set_data (
		CliFunc func,		// NULL = Don't set
		int arg_min,
		int arg_max,
		char const * arg_help,	// NULL = Don't set
		int arg_scale
		);

	CliItem * find ( std::string const & key ) const;
	CliItem const * match_args (
		char const * const * argv,
		int * cli_error,
		int * ncargs
		) const;
	int callback ( char const * const * argv ) const;
	int print_help () const;
	int print_help_item () const;

private:
	std::string	const	m_key;
	CliFunc			m_func;
	int			m_arg_min;
	int			m_arg_max;
	std::string		m_arg_help;
	int			m_arg_scale;

	std::map< std::string, std::unique_ptr<CliItem> > m_items;

	MTKIT_RULE_OF_FIVE( CliItem )
};



}	// namespace mtKit



#endif	// __cplusplus



#ifndef DEBUG
#pragma GCC visibility pop
#endif

