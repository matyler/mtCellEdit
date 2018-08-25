/*
	Copyright (C) 2018 Mark Tyler

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

#include "static.h"



class Backend;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



extern Backend backend;		// Singleton global backend instance



class Backend
{
public:
	Backend ();
	~Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	void start_ui ();

	int get_help ( char const * const * argv ) const;

	inline mtDW::Well * get_well () { return m_well.get (); }
	inline mtDW::Butt * get_butt () { return m_butt.get (); }
	inline mtDW::Soda * get_soda () { return m_soda.get (); }
	inline mtDW::Tap * get_tap () { return m_tap.get (); }

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

private:
	void main_loop ();

/// ----------------------------------------------------------------------------

	mtKit::unique_ptr<mtDW::Well>	m_well;
	mtKit::unique_ptr<mtDW::Butt>	m_butt;
	mtKit::unique_ptr<mtDW::Soda>	m_soda;
	mtKit::unique_ptr<mtDW::Tap>	m_tap;

	char		const *	m_conf_path;

	mtKit::CliTab		m_clitab;
	mtKit::Random		m_random;
};



int jtf_about			( char const * const * );
int jtf_butt_add_buckets	( char const * const * );
int jtf_butt_add_name		( char const * const * );
int jtf_butt_info		( char const * const * );
int jtf_butt_list		( char const * const * );
int jtf_butt_set_name		( char const * const * );
int jtf_help			( char const * const * );
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
int jtf_well_save_file		( char const * const * );
int jtf_well_seed		( char const * const * );
int jtf_well_seed_int		( char const * const * );

