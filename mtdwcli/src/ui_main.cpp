/*
	Copyright (C) 2018-2020 Mark Tyler

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

#include "ui.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Backend backend;

#ifdef DEBUG
	// Check the error tables are correct
	mtDW::get_error_text ( 0 );
	mtDW::get_error_text ( 0 );
#endif

	if ( 0 == backend.command_line ( argc, argv ) )
	{
		backend.start_ui ();
	}

	return backend.exit.value ();
}



#include <readline/readline.h>
#include <readline/history.h>



#define JTFUNC( X ) [this](char const * const * f ) { return jtf_ ## X (f); }



void Backend::main_loop ()
{
	if (	0
		|| m_clitab.add_item ( "about", JTFUNC( about ) )

		|| m_clitab.add_item ( "app cardshuff", JTFUNC( app_cardshuff ))
		|| m_clitab.add_item ( "app cointoss", JTFUNC( app_cointoss ),
			1, 1, "<INTEGER>" )
		|| m_clitab.add_item ( "app declist", JTFUNC( app_declist ),
			3, 3, "<INTEGER> <DECIMAL> <DECIMAL>" )
		|| m_clitab.add_item ( "app diceroll", JTFUNC( app_diceroll ),
			2, 2, "<INTEGER> <INTEGER>" )

		|| m_clitab.add_item ( "app homoglyph analyse",
			JTFUNC( app_homoglyph_analyse ), 1, 1,
			"<INPUT FILENAME UTF-8>" )
		|| m_clitab.add_item ( "app homoglyph clean",
			JTFUNC(app_homoglyph_clean), 2, 2,
			"<INPUT FILENAME UTF-8> <OUTPUT FILENAME UTF-8>" )
		|| m_clitab.add_item ( "app homoglyph encode",
			JTFUNC( app_homoglyph_encode ), 3, 3,
			"<INPUT FILENAME UTF-8> <INPUT FILENAME BIN> "
			"<OUTPUT FILENAME UTF-8>" )
		|| m_clitab.add_item ( "app homoglyph decode",
			JTFUNC( app_homoglyph_decode ), 2, 2,
			"<INPUT FILENAME UTF-8> <OUTPUT FILENAME BIN>" )

		|| m_clitab.add_item ( "app intlist", JTFUNC( app_intlist ),
			3, 3, "<INTEGER> <INTEGER> <INTEGER>" )
		|| m_clitab.add_item ( "app numshuff", JTFUNC( app_numshuff ),
			1, 1, "<INTEGER>" )
		|| m_clitab.add_item ( "app password", JTFUNC( app_password ),
			2, 7,
			"<INTEGER> <INTEGER> [lower] [upper] [num] [other] "
			"[STRING]" )
		|| m_clitab.add_item ( "app pins", JTFUNC( app_pins ), 2, 2,
			"<INTEGER> <INTEGER>" )

		|| m_clitab.add_item ( "app utf8font clean",
			JTFUNC( app_utf8font_clean ), 2, 2,
			"<INPUT FILENAME UTF-8> <OUTPUT FILENAME UTF-8>" )
		|| m_clitab.add_item ( "app utf8font encode",
			JTFUNC( app_utf8font_encode ), 3, 3,
			"<INPUT FILENAME UTF-8> <INTEGER> "
			"<OUTPUT FILENAME UTF-8>" )
		|| m_clitab.add_item ( "app utf8font list",
			JTFUNC( app_utf8font_list ), 0, 0, NULL )

		|| m_clitab.add_item ( "butt add buckets",
			JTFUNC( butt_add_buckets ), 1, 1,"<INTEGER>")
		|| m_clitab.add_item ( "butt add otp", JTFUNC( butt_add_otp ),
			1, 1, "<STRING>" )
		|| m_clitab.add_item ( "butt add random otp",
			JTFUNC( butt_add_random_otp ) )
		|| m_clitab.add_item ( "butt delete otp",
			JTFUNC( butt_delete_otp ), 1, 1, "<STRING>" )
		|| m_clitab.add_item ( "butt empty", JTFUNC( butt_empty ) )
		|| m_clitab.add_item ( "butt import otp",
			JTFUNC( butt_import_otp ), 1, 1, "<PATH>" )
		|| m_clitab.add_item ( "butt info", JTFUNC( butt_info ) )
		|| m_clitab.add_item ( "butt list", JTFUNC( butt_list ) )
		|| m_clitab.add_item ( "butt set comment",
			JTFUNC( butt_set_comment ), 1, 1, "<STRING>" )
		|| m_clitab.add_item ( "butt set otp", JTFUNC( butt_set_otp ),
			1, 1, "<STRING>" )
		|| m_clitab.add_item ( "butt set read_only",
			JTFUNC( butt_set_read_only ) )
		|| m_clitab.add_item ( "butt set read_write",
			JTFUNC( butt_set_read_write ) )

		|| m_clitab.add_item ( "db", JTFUNC( db ), 1, 1, "<PATH>" )

		|| m_clitab.add_item ( "help", JTFUNC(help), 0, 100, "[ARG]...")
		|| m_clitab.add_item ( "info", JTFUNC( info ) )
		|| m_clitab.add_item ( "q", JTFUNC( quit ) )
		|| m_clitab.add_item ( "quit", JTFUNC( quit ) )

		|| m_clitab.add_item ( "soda decode", JTFUNC( soda_decode ),
			2, 2, "<INPUT FILENAME> <OUTPUT FILENAME>")
		|| m_clitab.add_item ( "soda encode", JTFUNC( soda_encode ),
			2, 2, "<INPUT FILENAME> <OUTPUT FILENAME>" )
		|| m_clitab.add_item ( "soda file info",
			JTFUNC( soda_file_info ), 1, 1, "<FILENAME>" )
		|| m_clitab.add_item ( "soda info", JTFUNC( soda_info ) )
		|| m_clitab.add_item ( "soda multi decode",
			JTFUNC( soda_multi_decode ),
			2, 2, "<INPUT FILENAME> <OUTPUT FILENAME>" )
		|| m_clitab.add_item ( "soda multi encode",
			JTFUNC( soda_multi_encode ), 4, 18,
			"<INPUT FILENAME> <OUTPUT FILENAME> <BUTT NAME>..." )
		|| m_clitab.add_item ( "soda set mode", JTFUNC( soda_set_mode ),
			1, 1, "<INTEGER>" )

		|| m_clitab.add_item ( "tap decode", JTFUNC( tap_decode ), 2, 2,
			"<INPUT FILENAME> <OUTPUT FILENAME>" )
		|| m_clitab.add_item ( "tap encode", JTFUNC( tap_encode ), 3, 3,
			"<INPUT BOTTLE> <INPUT FILE> <OUTPUT BOTTLE>" )
		|| m_clitab.add_item ( "tap file info",
			JTFUNC( tap_file_info ), 1, 1, "<FILENAME>" )
		|| m_clitab.add_item ( "tap multi decode",
			JTFUNC( tap_multi_decode ),
			2, 2, "<INPUT FILENAME> <OUTPUT FILENAME>" )

		|| m_clitab.add_item ( "well add path", JTFUNC( well_add_path ),
			1, 1, "<PATH>" )
		|| m_clitab.add_item ( "well empty", JTFUNC( well_empty ) )
		|| m_clitab.add_item ( "well info", JTFUNC( well_info ) )
		|| m_clitab.add_item ( "well reset shifts",
			JTFUNC( well_reset_shifts ) )
		|| m_clitab.add_item ( "well save file",
			JTFUNC( well_save_file ), 2, 2, "<BYTES> <FILENAME>" )
		|| m_clitab.add_item ( "well seed", JTFUNC( well_seed ) )
		|| m_clitab.add_item ( "well seed int", JTFUNC( well_seed_int ),
			1, 1, "<INTEGER>" )
		)
	{
		exit.abort ();
		exit.set_value ( 1 );

		std::cerr << "Error: main_loop().init_table()\n";

		return;
	}

	while ( false == exit.aborted () )
	{
		char * line = readline ( APP_NAME" > " );

		if ( ! line )
		{
			break;
		}

		if ( line[0] )
		{
			add_history ( line );
		}

		if ( line[0] == '#' )
		{
			// Comment
		}
		else
		{
			m_clitab.parse ( line );
		}

		free ( line );
		line = NULL;
	}

	clear_history ();
}

void Backend::start_ui ()
{
	if ( db.open ( m_db_path ) )
	{
		exit.set_value ( 1 );
		exit.abort ();

		return;
	}

	main_loop ();
}

int Backend::command_line (
	int			const	argc,
	char	const * const * const	argv
	)
{
	m_db_path = NULL;

	int	show_version	= 0;
	int	show_about	= 1;
	int	tab_text	= 0;

	mtKit::Arg args;

	args.add ( "-help",	show_version, 2 );
	args.add ( "-version",	show_version, 1 );
	args.add ( "db",	m_db_path );
	args.add ( "q",		show_about, 0 );
	args.add ( "t",		tab_text, 1 );

	args.parse ( argc, argv );

	if ( show_version )
	{
		printf ( "%s (part of %s)\n\n", argv[0], VERSION );

		if ( 2 == show_version )
		{
			printf (
				"For further information consult the man page "
				"%s(1) or the mtCellEdit Handbook.\n"
				"\n", argv[0] );
		}

		return 1;		// Quit program
	}

	if ( show_about )
	{
		jtf_about ( NULL );
	}

	rl_variable_bind ( "expand-tilde", "on" );

	if ( tab_text )
	{
		rl_bind_key ( '\t', rl_insert );
	}

	return 0;			// Continue program
}

int Backend::get_help (
	char	const * const * const	argv
	) const
{
	return m_clitab.print_help ( argv );
}

