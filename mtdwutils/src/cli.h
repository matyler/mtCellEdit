/*
	Copyright (C) 2018-2024 Mark Tyler

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

#include <mtdatawell.h>

#include "cli_static.h"



#undef APP_NAME
#define APP_NAME	"mtDWCLI"



class Backend;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class Backend
{
public:
	Backend () : m_db_path () {}

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	void start_ui ();

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

private:
	void main_loop ();
	int get_help ( char const * const * argv ) const;

	int jtf_about			( char const * const * );
	int jtf_app_cardshuff		( char const * const * );
	int jtf_app_cointoss		( char const * const * );
	int jtf_app_declist		( char const * const * );
	int jtf_app_diceroll		( char const * const * );
	int jtf_app_homoglyph_analyse	( char const * const * );
	int jtf_app_homoglyph_clean	( char const * const * );
	int jtf_app_homoglyph_decode	( char const * const * );
	int jtf_app_homoglyph_encode	( char const * const * );
	int jtf_app_intlist		( char const * const * );
	int jtf_app_numshuff		( char const * const * );
	int jtf_app_password		( char const * const * );
	int jtf_app_pins		( char const * const * );
	int jtf_app_utf8font_clean	( char const * const * );
	int jtf_app_utf8font_encode	( char const * const * );
	int jtf_app_utf8font_list	( char const * const * );
	int jtf_butt_add_buckets	( char const * const * );
	int jtf_butt_add_otp		( char const * const * );
	int jtf_butt_add_random_otp	( char const * const * );
	int jtf_butt_delete_otp		( char const * const * );
	int jtf_butt_empty		( char const * const * );
	int jtf_butt_import_otp		( char const * const * );
	int jtf_butt_info		( char const * const * );
	int jtf_butt_list		( char const * const * );
	int jtf_butt_set_comment	( char const * const * );
	int jtf_butt_set_otp		( char const * const * );
	int jtf_butt_set_read_only	( char const * const * );
	int jtf_butt_set_read_write	( char const * const * );
	int jtf_db			( char const * const * );
	int jtf_help			( char const * const * );
	int jtf_info			( char const * const * );
	int jtf_quit			( char const * const * );
	int jtf_soda_decode		( char const * const * );
	int jtf_soda_encode		( char const * const * );
	int jtf_soda_file_info		( char const * const * );
	int jtf_soda_info		( char const * const * );
	int jtf_soda_multi_decode	( char const * const * );
	int jtf_soda_multi_encode	( char const * const * );
	int jtf_soda_set_mode		( char const * const * );
	int jtf_tap_decode		( char const * const * );
	int jtf_tap_encode		( char const * const * );
	int jtf_tap_file_info		( char const * const * );
	int jtf_tap_multi_decode	( char const * const * );
	int jtf_well_add_path		( char const * const * );
	int jtf_well_empty		( char const * const * );
	int jtf_well_info		( char const * const * );
	int jtf_well_reset_shifts	( char const * const * );
	int jtf_well_save_file		( char const * const * );
	int jtf_well_seed		( char const * const * );
	int jtf_well_seed_int		( char const * const * );

/// ----------------------------------------------------------------------------

	mtDW::PathDB		db;
	mtDW::Homoglyph		hg_index;
	mtDW::Utf8Font		font_index;

	char		const *	m_db_path;

	mtKit::CliShell		m_clishell;
	mtKit::CliTab		m_clitab;
};

