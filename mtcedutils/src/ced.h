/*
	Copyright (C) 2011-2020 Mark Tyler

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
#include <mtcelledit.h>



typedef std::function<int()>	FunCB;



class Global
{
public:
	Global ();
	~Global ();

	int init ();
	int command_line ( int argc, char const * const argv[] );

private:
	enum
	{
		ERROR_LOAD_FILE		= 1,
		ERROR_LIBMTCELLEDIT	= 2
	};

	int load_file ();	// Loads sheet file (name in
				// global.s_arg). Can be a sheet only
				// (tsv|csv|ledger).
		// 0 = success. Failure is not sent to stderr, but i_error is
		// set.

	void set_sheet ( CedSheet * sh );

	int ced_append ();
	int ced_cat ();
	int ced_clear ();
	int ced_cut ();
	int ced_diff ();
	int ced_eval ();
	int ced_find ();
	int ced_flip ();
	int ced_fuzzmap ();
	int ced_insert ();
	int ced_ls ();
	int ced_paste ();
	int ced_rotate ();
	int ced_set ();
	int ced_sort ();
	int ced_transpose ();

	void set_function ( char const * name );

	int print_help ();
	int print_version ();

	int parse_cellrange ( int idata[4] );

	int argcb_com ();
	int argcb_dest ();
	int argcb_range ();
	int argcb_i ();
	int argcb_o ();
	int argcb_otype ();
	int argcb_recalc ();

	int file_func ( char const * filename );

	FunCB get_function ( std::string const & txt ) const;
	std::string get_error_message ( int err ) const;
	int get_filetype ();

/// ----------------------------------------------------------------------------

	int i_csv	= 0;	// Input data format: 0 = tsv 1 = csv
	int i_clock	= 0;	// 0 = clockwise 1 = anti
	int i_dest[4]	= {0};	// Start row, last row, start col, end col
	int i_error	= 0;	// 0 = success 1 = error
	int i_ftype_in	= 0;
	int i_ftype_out	= 0;	// If CED_FILE_TYPE_NONE use i_ftype_in
	int i_num	= 0;	// 0 = Text 1 = Number
	int i_rowcol	= 0;	// 0 = row 1 = col
	int i_range[4]	= {0};	// Start row, last row, start col, end col
	int i_start	= 0;
	int i_case	= 0;	// 0 = Case insenstive, 1 = Case sensitive
	int i_total	= 0;
	int i_tmp	= 0;	// Temp scratch for several args
	int i_verbose	= 0;	// 1 = verbose 0 = normal
	int i_wildcard	= 0;	// 0 = none 1 = use * and ? as wildcards

	char	const	* s_arg = nullptr;	// Current command line argument

	CedSheet	* m_sheet = nullptr;	// Current sheet

	char	const *	m_filename_a = nullptr;		// Diff
	char	const *	m_filename_b = nullptr;

	char	const *	m_dict_filename = nullptr;	// Fuzzmap
	int		m_dict_range[4] = {0};

	FunCB		m_function = nullptr;
	std::string	m_function_name;

	std::map< std::string, FunCB > jump_table;
	std::map< std::string, int > ft_table;
	std::map< int, std::string > err_table;

	MTKIT_RULE_OF_FIVE( Global )
};

