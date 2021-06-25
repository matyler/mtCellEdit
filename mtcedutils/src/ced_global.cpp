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

#include "ced.h"



Global::Global ()
{
	jump_table[ "append" ]		= [this](){ return ced_append(); };
	jump_table[ "clear" ]		= [this](){ return ced_clear(); };
	jump_table[ "cut" ]		= [this](){ return ced_cut(); };
	jump_table[ "diff" ]		= [this](){ return ced_diff(); };
	jump_table[ "eval" ]		= [this](){ return ced_eval(); };
	jump_table[ "find" ]		= [this](){ return ced_find(); };
	jump_table[ "flip" ]		= [this](){ return ced_flip(); };
	jump_table[ "fuzzmap" ]		= [this](){ return ced_fuzzmap(); };
	jump_table[ "insert" ]		= [this](){ return ced_insert(); };
	jump_table[ "ls" ]		= [this](){ return ced_ls(); };
	jump_table[ "paste" ]		= [this](){ return ced_paste(); };
	jump_table[ "rotate" ]		= [this](){ return ced_rotate(); };
	jump_table[ "set" ]		= [this](){ return ced_set(); };
	jump_table[ "sort" ]		= [this](){ return ced_sort(); };
	jump_table[ "transpose" ]	= [this](){ return ced_transpose(); };

	ft_table[ "csv_content" ]	= CED_FILE_TYPE_CSV_CONTENT;
	ft_table[ "csv_content_noq" ]	= CED_FILE_TYPE_CSV_CONTENT_NOQ;
	ft_table[ "csv_value" ]		= CED_FILE_TYPE_CSV_VALUE;
	ft_table[ "csv_value_noq" ]	= CED_FILE_TYPE_CSV_VALUE_NOQ;
	ft_table[ "input" ]		= CED_FILE_TYPE_NONE;
	ft_table[ "ledger" ]		= CED_FILE_TYPE_LEDGER;
	ft_table[ "ledger_gz" ]		= CED_FILE_TYPE_LEDGER_GZ;
	ft_table[ "ledger_val" ]	= CED_FILE_TYPE_LEDGER_VAL;
	ft_table[ "ledger_val_gz" ]	= CED_FILE_TYPE_LEDGER_VAL_GZ;
	ft_table[ "output_html" ]	= CED_FILE_TYPE_OUTPUT_HTML;
	ft_table[ "output_tsv" ]	= CED_FILE_TYPE_OUTPUT_TSV;
	ft_table[ "output_tsv_q" ]	= CED_FILE_TYPE_OUTPUT_TSV_QUOTED;
	ft_table[ "tsv_content" ]	= CED_FILE_TYPE_TSV_CONTENT;
	ft_table[ "tsv_content_gz" ]	= CED_FILE_TYPE_TSV_CONTENT_GZ;
	ft_table[ "tsv_content_noq" ]	= CED_FILE_TYPE_TSV_CONTENT_NOQ;
	ft_table[ "tsv_value" ]		= CED_FILE_TYPE_TSV_VALUE;
	ft_table[ "tsv_value_gz" ]	= CED_FILE_TYPE_TSV_VALUE_GZ;
	ft_table[  "tsv_value_noq" ]	= CED_FILE_TYPE_TSV_VALUE_NOQ;

	err_table[ ERROR_LOAD_FILE ]	= "Unable to load file";
	err_table[ ERROR_LIBMTCELLEDIT ] = "Unexpected libmtcelledit failure";
}

Global::~Global ()
{
	set_sheet ( NULL );
}

int Global::init ()
{
	ced_init ();

	i_range[0] = 1;
	i_range[1] = CED_MAX_ROW;
	i_range[2] = 1;
	i_range[3] = CED_MAX_COLUMN;
	i_dest[0] = 1;
	i_dest[1] = 1;
	i_dest[2] = 1;
	i_dest[3] = 1;
	i_start = 1;
	i_total = 1;
	i_ftype_in = CED_FILE_TYPE_TSV_CONTENT;
	i_ftype_out = CED_FILE_TYPE_TSV_CONTENT;

	m_sheet = ced_sheet_new ();
	if ( ! m_sheet )
	{
		i_error = 1;

		return 1;
	}

	return 0;			// Success
}

void Global::set_function ( char const * name )
{
	if ( ! name )
	{
		m_function = nullptr;
	}
	else
	{
		if (	name[0] == 'c' &&
			name[1] == 'e' &&
			name[2] == 'd'
			)
		{
			name += 3;
		}

		m_function = get_function ( name );
	}

	if ( m_function )
	{
		m_function_name = name;
	}
	else
	{
		m_function_name = "";
	}
}

int Global::file_func ( char const * const filename )
{
	if ( m_function )
	{
		s_arg = filename;

		int const res = m_function ();

		if ( res > 0 )
		{
			fprintf ( stderr, "%s error: %s. arg = '%s'\n",
				m_function_name.c_str(),
				get_error_message ( res ).c_str(),
				filename );

			i_error = 1;	// Terminate ASAP
		}
	}

	return i_error;		// Keep parsing if no errors encountered
}

int Global::argcb_com ()
{
	set_function ( s_arg );

	if ( ! m_function )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such command '%s'\n", s_arg );
		i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

int Global::parse_cellrange ( int idata[4] )
{
	CedCellRef	r1, r2;

	if ( strchr ( s_arg, ':' ) )
	{
		// User has entered a range

		if ( ced_strtocellrange ( s_arg, &r1, &r2, NULL, 1 ) )
		{
			goto error;
		}
	}
	else
	{
		// User has entered a cell reference

		if ( ced_strtocellref ( s_arg, &r1, NULL, 1 ) )
		{
			goto error;
		}

		r2 = r1;
	}

	// Disallow relative r1/r2
	if (	r1.row_m ||
		r1.col_m ||
		r2.row_m ||
		r2.col_m
		)
	{
		goto error;
	}

	// Lowest row/col first for deterministic calculations later
	idata[0] = MIN ( r1.row_d, r2.row_d );
	idata[1] = MAX ( r1.row_d, r2.row_d );
	idata[2] = MIN ( r1.col_d, r2.col_d );
	idata[3] = MAX ( r1.col_d, r2.col_d );

	return 0;

error:

	fprintf ( stderr, "Error: Bad cell range or reference '%s'\n", s_arg );
	i_error = 1;

	return 1;
}

int Global::argcb_dest ()
{
	return parse_cellrange ( i_dest );
}

int Global::argcb_range ()
{
	return parse_cellrange ( i_range );
}

int Global::argcb_i ()
{
	if ( load_file () )
	{
		fprintf ( stderr, "Error: Unable to load file '%s'\n", s_arg );

		i_error = 1;

		return 1;		// Failure, so stop parsing
	}

	return 0;			// Continue parsing args
}

static int check_stdinout (
	char	const * const	filename
	)
{
	if (	filename		&&
		filename[0] == '-'	&&
		filename[1] == 0
		)
	{
		return 0;
	}

	return -1;
}

static int check_load_stdin (
	char	const * const	filename,
	char	**	const	buf,
	size_t		* const	buflen
	)
{
	int		res = -1;


	if ( 0 == check_stdinout ( filename ) )
	{
		res = mtkit_file_load_stdin ( buf, buflen );
		if ( res )
		{
			fprintf ( stderr, "Error during mtkit_file_load_stdin."
				"\n" );
		}
	}

	return res;
}

int Global::argcb_o ()
{
	int		ftype;


	if ( i_ftype_out == CED_FILE_TYPE_NONE )
	{
		ftype = i_ftype_in;
	}
	else
	{
		ftype = i_ftype_out;
	}

	if ( 0 == check_stdinout ( s_arg ) )
	{
		int		res = 0;
		mtFile		* file;
		void		* buf;


		file = ced_sheet_save_mem ( m_sheet, ftype );
		if (	mtkit_file_write ( file, "", 1 )	||
			mtkit_file_get_mem ( file, &buf, NULL )
			)
		{
			fprintf ( stderr, "Error: Unable to save file to stdout"
				"\n" );

			i_error = 1;
			res = 1;
		}
		else
		{
			fputs ( (char const *)buf, stdout );
		}

		mtkit_file_close ( file );

		return res;
	}

	if ( ced_sheet_save ( m_sheet, s_arg, ftype ) )
	{
		fprintf ( stderr, "Error: Unable to save file '%s'\n", s_arg );

		i_error = 1;

		return 1;		// Failure, so stop parsing
	}

	return 0;			// Continue parsing args
}

int Global::argcb_otype ()
{
	if ( strcmp ( s_arg, "list" ) == 0 )
	{
		printf ( "Valid file types:\n\n" );

		for ( auto && it : ft_table )
		{
			std::cout << it.second << "\n";
		}
	}
	else if ( get_filetype () )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such file format '%s'\n", s_arg );

		i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

int Global::argcb_recalc ()
{
	for ( int i = 0; i < i_tmp; i++ )
	{
		ced_sheet_recalculate ( m_sheet, NULL, 0 );
		ced_sheet_recalculate ( m_sheet, NULL, 1 );
	}

	return 0;			// Continue parsing args
}

int Global::print_version ()
{
	if ( m_function )
	{
		printf ( "ced%s - Part of %s\n\n", m_function_name.c_str(),
			VERSION );
	}
	else
	{
		printf ( "%s\n\n", VERSION );
	}

	return 1;	// Stop parsing
}

int Global::print_help ()
{
	print_version ();

	if ( m_function )
	{
		printf ("For further information consult the man page "
			"ced%s(1) or the mtCellEdit Handbook.\n"
			"\n",
			m_function_name.c_str() );
	}
	else
	{
		printf ("For further information consult the man page "
			"%s(1) or the mtCellEdit Handbook.\n"
			"\n",
			BIN_NAME );
	}

	return 1;	// Stop parsing
}

int Global::command_line ( int argc, char const * const argv[] )
{
	// Establish what command has been used to call this program
	s_arg = strrchr ( argv[0], MTKIT_DIR_SEP );

	if ( ! s_arg )
	{
		s_arg = argv[0];
	}
	else
	{
		s_arg ++;
	}

	set_function ( s_arg );

	mtKit::Arg args ( [this]( char const * const filename )
		{
			return file_func ( filename );
		} );

	args.add ( "-help",	[this,argv]() { return print_help (); } );
	args.add ( "-version",	[this]() { return print_version (); } );
	args.add ( "anti",	i_clock, 1 );
	args.add ( "case",	i_case, 1 );
	args.add ( "clock",	i_clock, 0 );
	args.add ( "col",	i_rowcol, 1 );
	args.add ( "com",	s_arg, [this]() { return argcb_com(); } );
	args.add ( "csv",	i_csv, 1 );
	args.add ( "dest",	s_arg, [this]() { return argcb_dest(); } );
	args.add ( "i",		s_arg, [this]() { return argcb_i(); } );
	args.add ( "num",	i_num, 1 );
	args.add ( "o",		s_arg, [this]() { return argcb_o(); } );
	args.add ( "otype",	s_arg, [this]() { return argcb_otype(); } );
	args.add ( "range",	s_arg, [this]() { return argcb_range(); } );
	args.add ( "recalc",	i_tmp, [this]() { return argcb_recalc(); } );
	args.add ( "row",	i_rowcol, 0 );
	args.add ( "start",	i_start );
	args.add ( "total",	i_total );
	args.add ( "tsv",	i_csv, 0 );
	args.add ( "v",		i_verbose, 1 );
	args.add ( "wildcard",	i_wildcard, 1 );

	args.parse ( argc, argv );

	return i_error;
}

int Global::load_file ()
{
	CedSheet	* new_sheet = NULL;
	int		new_type;
	int		p;
	char		* buf = NULL;
	size_t		buflen;


	if ( ! s_arg )
	{
		i_error = 1;

		return 1;		// Fail
	}

	if ( s_arg[0] == 0 )
	{
		// User sent us "" which means action the current sheet

		return 0;
	}

	p = check_load_stdin ( s_arg, &buf, &buflen );

	if ( i_csv )
	{
		switch ( p )
		{
		case -1:	// No stdin requested
			new_sheet = ced_sheet_load_csv ( s_arg, NULL );
			break;

		case 0:		// Loaded stdin
			new_sheet = ced_sheet_load_csv_mem ( buf, buflen, NULL);
			break;
		}

		new_type = CED_FILE_TYPE_CSV_CONTENT;
	}
	else
	{
		switch ( p )
		{
		case -1:	// No stdin requested
			new_sheet = ced_sheet_load ( s_arg, NULL,
				&new_type );
			break;

		case 0:		// Loaded stdin
			new_sheet = ced_sheet_load_mem ( buf, buflen, NULL,
				&new_type );
			break;
		}
	}

	free ( buf );

	if ( ! new_sheet )
	{
		i_error = 1;

		return 1;		// Fail
	}

	set_sheet ( new_sheet );

	i_ftype_in = new_type;

	return 0;			// Success
}

void Global::set_sheet ( CedSheet * const sh )
{
	ced_sheet_destroy ( m_sheet );
	m_sheet = sh;
}

FunCB Global::get_function ( std::string const & txt ) const
{
	auto const it = jump_table.find ( txt );

	if ( it == jump_table.end() )
	{
		return nullptr;
	}

	return it->second;
}

std::string Global::get_error_message ( int const err ) const
{
	auto const it = err_table.find ( err );

	if ( it == err_table.end() )
	{
		return "Unknown";
	}

	return it->second;
}

int Global::get_filetype ()
{
	auto const it = ft_table.find ( s_arg );

	if ( it != ft_table.end() )
	{
		i_ftype_out = it->second;
		return 0;
	}

	return 1;			// Not found
}

