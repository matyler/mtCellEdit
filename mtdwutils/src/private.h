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

#include <mtkit.h>
#include <mtdatawell.h>



typedef std::function<int()>	FunCB;



class Global
{
public:
	Global ();
	~Global ();

	int command_line ( int argc, char const * const argv[] );

private:
	enum
	{
		ERROR_UNKNOWN		= -1,

		ERROR_NONE		= 0,

		ERROR_BOTTLE_MISSING	= 1,
		ERROR_SODA_ENCODE,
		ERROR_SODA_DECODE,
		ERROR_BOTTLE_ENCODE,
		ERROR_BOTTLE_DECODE,
		ERROR_DB_OPEN,
		ERROR_FONT_ENCODE,
		ERROR_FONT_DECODE,
		ERROR_HG_ENCODE,
		ERROR_HG_DECODE,

		ERROR_MAX
	};


	std::string get_error_text ( int code ) const;
	FunCB get_function ( std::string const & txt ) const;

	void set_function ( char const * name );

	int dw_decbot ();
	int dw_decfont ();
	int dw_dechg ();
	int dw_decsoda ();
	int dw_encbot ();
	int dw_encfont ();
	int dw_enchg ();
	int dw_encsoda ();

	void print_help ();
	void print_version ();
	int parse_com ();
	int file_func ( char const * filename );

	MTKIT_RULE_OF_FIVE( Global )

/// ----------------------------------------------------------------------------

	mtDW::Database	db;
	mtDW::Homoglyph	hg_index;
	mtDW::Utf8Font	font_index;
	mtKit::Exit	exit;

	int		i_font = 5;
	int		i_verbose = 0;

	char	const	* s_arg = nullptr;	// Current command line argument
	char	const	* s_bottle = nullptr;
	char	const	* s_o = nullptr;
	char	const	* s_db_path = nullptr;

	FunCB		m_function = nullptr;
	std::string	m_function_name;

	std::map< std::string, FunCB > jump_table;
	std::map< int, std::string > err_table;
};

